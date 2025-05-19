/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adap-tab-thumbnail-private.h"

#include "adap-fading-label-private.h"
#include "adap-gizmo-private.h"
#include "adap-tab-view-private.h"
#include "adap-timed-animation.h"

#define FADE_TRANSITION_DURATION 250
#define PINNED_MARGIN 10

struct _AdapTabThumbnail
{
  GtkWidget parent_instance;

  GtkWidget *contents;
  GtkWidget *icon_title_box;
  GtkWidget *overlay;
  GtkPicture *picture;
  GtkWidget *icon_stack;
  GtkImage *icon;
  GtkSpinner *spinner;
  GtkImage *indicator_icon;
  GtkWidget *indicator_btn;
  GtkWidget *close_btn;
  GtkWidget *unpin_icon;
  GtkWidget *needs_attention_revealer;
  GtkWidget *pinned_box;
  GtkDropTarget *drop_target;
  GdkDragAction preferred_action;

  AdapTabView *view;
  AdapTabPage *page;
  gboolean pinned;

  gboolean inverted;

  AdapAnimation *fade_animation;
};

G_DEFINE_FINAL_TYPE (AdapTabThumbnail, adap_tab_thumbnail, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_VIEW,
  PROP_PINNED,
  PROP_PAGE,
  PROP_INVERTED,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_EXTRA_DRAG_VALUE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static inline void
set_style_class (GtkWidget  *widget,
                 const char *style_class,
                 gboolean    enabled)
{
  if (enabled)
    gtk_widget_add_css_class (widget, style_class);
  else
    gtk_widget_remove_css_class (widget, style_class);
}

static void
update_tooltip (AdapTabThumbnail *self)
{
  AdapTabPage *page = adap_tab_thumbnail_get_page (ADAP_TAB_THUMBNAIL (self));
  const char *tooltip = adap_tab_page_get_tooltip (page);

  if (tooltip && g_strcmp0 (tooltip, "") != 0)
    gtk_widget_set_tooltip_markup (GTK_WIDGET (self), tooltip);
  else
    gtk_widget_set_tooltip_text (GTK_WIDGET (self),
                                 adap_tab_page_get_title (page));
}

static void
update_spinner (AdapTabThumbnail *self)
{
  gboolean loading = self->page && adap_tab_page_get_loading (self->page);
  gboolean mapped = gtk_widget_get_mapped (GTK_WIDGET (self));

  /* Don't use CPU when not needed */
  gtk_spinner_set_spinning (self->spinner, loading && mapped);
}

static void
update_icon (AdapTabThumbnail *self)
{
  GIcon *gicon = adap_tab_page_get_icon (self->page);
  gboolean loading = adap_tab_page_get_loading (self->page);
  const char *name = loading ? "spinner" : "icon";

  gtk_image_set_from_gicon (self->icon, gicon);
  gtk_widget_set_visible (self->icon_stack,
                          (gicon != NULL || loading));
  gtk_stack_set_visible_child_name (GTK_STACK (self->icon_stack), name);
}

static void
update_loading (AdapTabThumbnail *self)
{
  update_icon (self);
  update_spinner (self);
  set_style_class (GTK_WIDGET (self), "loading",
                   adap_tab_page_get_loading (self->page));
}

static void
update_indicator (AdapTabThumbnail *self)
{
  GIcon *indicator = adap_tab_page_get_indicator_icon (self->page);
  gboolean activatable = self->page && adap_tab_page_get_indicator_activatable (self->page);
  gboolean was_visible = gtk_widget_get_visible (self->indicator_btn);

  gtk_image_set_from_gicon (self->indicator_icon, indicator);
  gtk_widget_set_visible (self->indicator_btn, indicator != NULL);
  gtk_widget_set_can_target (self->indicator_btn, activatable);

  if (gtk_widget_get_visible (self->indicator_btn) != was_visible) {
    if (self->pinned)
      gtk_widget_queue_resize (self->pinned_box);
    else
      gtk_widget_queue_allocate (GTK_WIDGET (self->overlay));
  }

  set_style_class (GTK_WIDGET (self), "indicator", indicator != NULL);
}

static void
close_idle_cb (AdapTabThumbnail *self)
{
  adap_tab_view_close_page (self->view, self->page);
  g_object_unref (self);
}

static void
close_clicked_cb (AdapTabThumbnail *self)
{
  if (!self->page)
    return;

  /* When animations are disabled, we don't want to immediately remove the
   * whole tab mid-click. Instead, defer it until the click has happened.
   */
  g_idle_add_once ((GSourceOnceFunc) close_idle_cb, g_object_ref (self));
}

static void
unpin_idle_cb (AdapTabThumbnail *self)
{
  adap_tab_view_set_page_pinned (self->view, self->page, FALSE);
  g_object_unref (self);
}

static void
unpin_clicked_cb (AdapTabThumbnail *self)
{
  if (!self->page)
    return;

  /* When animations are disabled, we don't want to immediately unpin the
   * whole tab mid-click. Instead, defer it until the click has happened.
   */
  g_idle_add_once ((GSourceOnceFunc) unpin_idle_cb, g_object_ref (self));
}

static void
indicator_clicked_cb (AdapTabThumbnail *self)
{
  if (!self->page)
    return;

  g_signal_emit_by_name (self->view, "indicator-activated", self->page);
}

static GdkDragAction
make_action_unique (GdkDragAction actions)
{
  if (actions & GDK_ACTION_COPY)
    return GDK_ACTION_COPY;

  if (actions & GDK_ACTION_MOVE)
    return GDK_ACTION_MOVE;

  if (actions & GDK_ACTION_LINK)
    return GDK_ACTION_LINK;

  return 0;
}

static gboolean
drop_cb (AdapTabThumbnail *self,
         GValue          *value)
{
  gboolean ret = GDK_EVENT_PROPAGATE;
  GdkDrop *drop = gtk_drop_target_get_current_drop (self->drop_target);
  GdkDragAction preferred_action = gdk_drop_get_actions (drop);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, value, preferred_action, &ret);

  return ret;
}

static GdkDragAction
extra_drag_enter_cb (AdapTabThumbnail *self)
{
  const GValue *value = gtk_drop_target_get_value (self->drop_target);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, value, &self->preferred_action);
  self->preferred_action = make_action_unique (self->preferred_action);

  return self->preferred_action;
}

static GdkDragAction
extra_drag_motion_cb (AdapTabThumbnail *self)
{
  return self->preferred_action;
}

static void
extra_drag_notify_value_cb (AdapTabThumbnail *self)
{
  const GValue *value = gtk_drop_target_get_value (self->drop_target);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, value, &self->preferred_action);
  self->preferred_action = make_action_unique (self->preferred_action);
}

static void
fade_animation_value_cb (double           value,
                         AdapTabThumbnail *self)
{
  if (self->pinned) {
    gtk_widget_set_opacity (self->unpin_icon, value);
    gtk_widget_set_opacity (self->icon_title_box, value);
  } else {
    gtk_widget_set_opacity (self->close_btn, value);
  }

  gtk_widget_set_opacity (self->indicator_btn, value);
  gtk_widget_set_opacity (self->needs_attention_revealer, value);
}

static void
adap_tab_thumbnail_map (GtkWidget *widget)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (widget);

  GTK_WIDGET_CLASS (adap_tab_thumbnail_parent_class)->map (widget);

  update_spinner (self);
}

static void
adap_tab_thumbnail_unmap (GtkWidget *widget)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (widget);

  GTK_WIDGET_CLASS (adap_tab_thumbnail_parent_class)->unmap (widget);

  update_spinner (self);
}

static void
measure_pinned_tab (AdapGizmo       *gizmo,
                    GtkOrientation  orientation,
                    int             for_size,
                    int            *minimum,
                    int            *natural,
                    int            *minimum_baseline,
                    int            *natural_baseline)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (gtk_widget_get_ancestor (GTK_WIDGET (gizmo),
                                             ADAP_TYPE_TAB_THUMBNAIL));

  if (orientation == GTK_ORIENTATION_VERTICAL) {
    gtk_widget_measure (self->icon_title_box, orientation, for_size,
                        minimum, natural, minimum_baseline, natural_baseline);
  } else {
    int box_min, box_nat, button_min, button_nat, indicator_min, indicator_nat;

    gtk_widget_measure (self->icon_title_box, orientation, for_size,
                        &box_min, &box_nat, NULL, NULL);

    if (gtk_widget_should_layout (self->indicator_btn))
      gtk_widget_measure (self->unpin_icon, orientation, for_size,
                          &button_min, &button_nat, NULL, NULL);
    else
      button_min = button_nat = 0;

    if (gtk_widget_should_layout (self->indicator_btn))
      gtk_widget_measure (self->indicator_btn, orientation, for_size,
                          &indicator_min, &indicator_nat, NULL, NULL);
    else
      indicator_min = indicator_nat = 0;

    if (minimum)
      *minimum = box_min + button_min + indicator_min;
    if (natural)
      *natural = box_nat + button_nat + indicator_nat;
    if (minimum_baseline)
      *minimum_baseline = -1;
    if (natural_baseline)
      *natural_baseline = -1;
  }
}

static void
allocate_pinned_tab (AdapGizmo *gizmo,
                     int       width,
                     int       height,
                     int       baseline)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (gtk_widget_get_ancestor (GTK_WIDGET (gizmo),
                                             ADAP_TYPE_TAB_THUMBNAIL));
  int left_margin = 0, right_margin = 0;
  int box_pos, box_width;
  gboolean is_rtl;

  if (gtk_widget_should_layout (self->unpin_icon))
    gtk_widget_measure (self->unpin_icon, GTK_ORIENTATION_HORIZONTAL, -1,
                          &right_margin, NULL, NULL, NULL);
  if (gtk_widget_should_layout (self->indicator_btn))
    gtk_widget_measure (self->indicator_btn, GTK_ORIENTATION_HORIZONTAL, -1,
                        &left_margin, NULL, NULL, NULL);

  gtk_widget_measure (self->icon_title_box, GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &box_width, NULL, NULL);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (gizmo)) == GTK_TEXT_DIR_RTL;

  if (is_rtl != self->inverted) {
    int tmp = left_margin;
    left_margin = right_margin;
    right_margin = tmp;
  }

  left_margin = MAX (left_margin, PINNED_MARGIN);
  right_margin = MAX (right_margin, PINNED_MARGIN);

  box_width = MIN (width - right_margin - left_margin, box_width);
  box_pos = (width - box_width) / 2;

  if (box_pos + box_width > width - right_margin)
    box_pos = width - right_margin - box_width;
  if (box_pos < left_margin)
    box_pos = left_margin;

  gtk_widget_allocate (self->icon_title_box, box_width, height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (box_pos, 0)));
}

static void
adap_tab_thumbnail_constructed (GObject *object)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (object);

  G_OBJECT_CLASS (adap_tab_thumbnail_parent_class)->constructed (object);

  gtk_widget_set_visible (self->unpin_icon, self->pinned);
  gtk_widget_set_visible (self->close_btn, !self->pinned);

  if (self->pinned) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "pinned");

    self->pinned_box = adap_gizmo_new ("widget",
                                      measure_pinned_tab,
                                      allocate_pinned_tab,
                                      NULL, NULL, NULL, NULL);
    gtk_widget_add_css_class (self->pinned_box, "pinned-box");
    gtk_widget_set_can_target (self->pinned_box, FALSE);
    gtk_overlay_add_overlay (GTK_OVERLAY (self->overlay), self->pinned_box);
    gtk_overlay_set_measure_overlay (GTK_OVERLAY (self->overlay),
                                     self->pinned_box , TRUE);

    g_object_ref (self->icon_title_box);

    gtk_box_remove (GTK_BOX (self->contents), self->icon_title_box);
    gtk_widget_set_parent (self->icon_title_box, self->pinned_box );

    g_object_unref (self->icon_title_box);

    gtk_widget_set_halign (self->icon_title_box, GTK_ALIGN_FILL);

    gtk_widget_set_visible (GTK_WIDGET (self->picture), FALSE);
  }
}

static void
adap_tab_thumbnail_dispose (GObject *object)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (object);

  adap_tab_thumbnail_set_page (self, NULL);

  g_clear_object (&self->fade_animation);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADAP_TYPE_TAB_THUMBNAIL);

  G_OBJECT_CLASS (adap_tab_thumbnail_parent_class)->dispose (object);
}

static void
adap_tab_thumbnail_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, self->view);
    break;

  case PROP_PAGE:
    g_value_set_object (value, adap_tab_thumbnail_get_page (self));
    break;

  case PROP_PINNED:
    g_value_set_boolean (value, self->pinned);
    break;

  case PROP_INVERTED:
    g_value_set_boolean (value, adap_tab_thumbnail_get_inverted (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_thumbnail_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdapTabThumbnail *self = ADAP_TAB_THUMBNAIL (object);

  switch (prop_id) {
  case PROP_VIEW:
    self->view = g_value_get_object (value);
    break;

  case PROP_PAGE:
    adap_tab_thumbnail_set_page (self, g_value_get_object (value));
    break;

  case PROP_PINNED:
    self->pinned = g_value_get_boolean (value);
    break;

  case PROP_INVERTED:
    adap_tab_thumbnail_set_inverted (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_thumbnail_class_init (AdapTabThumbnailClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_tab_thumbnail_dispose;
  object_class->constructed = adap_tab_thumbnail_constructed;
  object_class->get_property = adap_tab_thumbnail_get_property;
  object_class->set_property = adap_tab_thumbnail_set_property;

  widget_class->map = adap_tab_thumbnail_map;
  widget_class->unmap = adap_tab_thumbnail_unmap;

  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         ADAP_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_PINNED] =
    g_param_spec_boolean ("pinned", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_PAGE] =
    g_param_spec_object ("page", NULL, NULL,
                         ADAP_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  G_TYPE_BOOLEAN,
                  2,
                  G_TYPE_VALUE,
                  GDK_TYPE_DRAG_ACTION);

  signals[SIGNAL_EXTRA_DRAG_VALUE] =
    g_signal_new ("extra-drag-value",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  GDK_TYPE_DRAG_ACTION,
                  1,
                  G_TYPE_VALUE);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-tab-thumbnail.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, contents);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, overlay);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, icon_title_box);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, picture);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, icon_stack);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, icon);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, spinner);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, indicator_icon);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, indicator_btn);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, close_btn);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, unpin_icon);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, needs_attention_revealer);
  gtk_widget_class_bind_template_child (widget_class, AdapTabThumbnail, drop_target);
  gtk_widget_class_bind_template_callback (widget_class, close_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, unpin_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, indicator_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, drop_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_motion_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_notify_value_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "tabthumbnail");

  g_type_ensure (ADAP_TYPE_FADING_LABEL);
}

static void
adap_tab_thumbnail_init (AdapTabThumbnail *self)
{
  AdapAnimationTarget *target;

  gtk_widget_init_template (GTK_WIDGET (self));

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc) fade_animation_value_cb,
                                              self, NULL);

  self->fade_animation = adap_timed_animation_new (GTK_WIDGET (self),
                                                  0, 1,
                                                  FADE_TRANSITION_DURATION,
                                                  target);
}

AdapTabThumbnail *
adap_tab_thumbnail_new (AdapTabView *view,
                       gboolean    pinned)
{
  g_return_val_if_fail (ADAP_IS_TAB_VIEW (view), NULL);

  return g_object_new (ADAP_TYPE_TAB_THUMBNAIL,
                       "view", view,
                       "pinned", pinned,
                       NULL);
}

AdapTabPage *
adap_tab_thumbnail_get_page (AdapTabThumbnail *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_THUMBNAIL (self), NULL);

  return self->page;
}

void
adap_tab_thumbnail_set_page (AdapTabThumbnail *self,
                            AdapTabPage      *page)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));
  g_return_if_fail (page == NULL || ADAP_IS_TAB_PAGE (page));

  if (self->page == page)
    return;

  if (self->page) {
    g_signal_handlers_disconnect_by_func (self->page, update_tooltip, self);
    g_signal_handlers_disconnect_by_func (self->page, update_icon, self);
    g_signal_handlers_disconnect_by_func (self->page, update_indicator, self);
    g_signal_handlers_disconnect_by_func (self->page, update_loading, self);
  }

  g_set_object (&self->page, page);

  if (self->page) {
    GdkPaintable *paintable = adap_tab_page_get_paintable (self->page);

    gtk_picture_set_paintable (GTK_PICTURE (self->picture), paintable);

    update_tooltip (self);
    update_spinner (self);
    update_icon (self);
    update_indicator (self);
    update_loading (self);

    g_signal_connect_object (self->page, "notify::title",
                             G_CALLBACK (update_tooltip), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::tooltip",
                             G_CALLBACK (update_tooltip), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::icon",
                             G_CALLBACK (update_icon), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-icon",
                             G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-activatable",
                             G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::loading",
                             G_CALLBACK (update_loading), self,
                             G_CONNECT_SWAPPED);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAGE]);
}

gboolean
adap_tab_thumbnail_get_inverted (AdapTabThumbnail *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_THUMBNAIL (self), FALSE);

  return self->inverted;
}

void
adap_tab_thumbnail_set_inverted (AdapTabThumbnail *self,
                                gboolean         inverted)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));

  inverted = !!inverted;

  if (self->inverted == inverted)
    return;

  self->inverted = inverted;

  if (inverted) {
    gtk_widget_set_halign (self->close_btn, GTK_ALIGN_START);
    gtk_widget_set_halign (self->unpin_icon, GTK_ALIGN_START);
    gtk_widget_set_halign (self->indicator_btn, GTK_ALIGN_END);
  } else {
    gtk_widget_set_halign (self->close_btn, GTK_ALIGN_END);
    gtk_widget_set_halign (self->unpin_icon, GTK_ALIGN_END);
    gtk_widget_set_halign (self->indicator_btn, GTK_ALIGN_START);
  }

  if (self->pinned)
    gtk_widget_queue_resize (self->pinned_box);
  else
    gtk_widget_queue_allocate (GTK_WIDGET (self->overlay));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

void
adap_tab_thumbnail_setup_extra_drop_target (AdapTabThumbnail *self,
                                           GdkDragAction    actions,
                                           GType           *types,
                                           gsize            n_types)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  gtk_drop_target_set_actions (self->drop_target, actions);
  gtk_drop_target_set_gtypes (self->drop_target, types, n_types);

  self->preferred_action = make_action_unique (actions);
}

void
adap_tab_thumbnail_set_extra_drag_preload (AdapTabThumbnail *self,
                                          gboolean         preload)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));

  gtk_drop_target_set_preload (self->drop_target, preload);
}

GtkWidget *
adap_tab_thumbnail_get_thumbnail (AdapTabThumbnail *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_THUMBNAIL (self), NULL);

  return self->overlay;
}

void
adap_tab_thumbnail_fade_out (AdapTabThumbnail *self)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));

  adap_animation_reset (self->fade_animation);

  gtk_widget_set_opacity (self->overlay, 0);
}

void
adap_tab_thumbnail_fade_in (AdapTabThumbnail *self)
{
  g_return_if_fail (ADAP_IS_TAB_THUMBNAIL (self));

  gtk_widget_set_opacity (self->overlay, 1);

  adap_animation_play (self->fade_animation);
}
