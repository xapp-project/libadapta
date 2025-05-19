/*
 * Copyright (C) 2020-2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-tab-private.h"

#include "adw-bidi-private.h"
#include "adw-fading-label-private.h"
#include "adw-gizmo-private.h"
#include "adw-timed-animation.h"

#define FADE_WIDTH 18.0f
#define CLOSE_BTN_ANIMATION_DURATION 150

#define BASE_WIDTH 118
#define BASE_WIDTH_PINNED 26

#define ATTENTION_INDICATOR_PINNED_WIDTH 14
#define ATTENTION_INDICATOR_WIDTH_MULTIPLIER 0.6
#define ATTENTION_INDICATOR_MIN_WIDTH 20
#define ATTENTION_INDICATOR_MAX_WIDTH 180
#define ATTENTION_INDICATOR_ANIMATION_DURATION 250

struct _AdwTab
{
  GtkWidget parent_instance;

  GtkWidget *title;
  GtkWidget *icon_stack;
  GtkImage *icon;
  GtkSpinner *spinner;
  GtkImage *indicator_icon;
  GtkWidget *indicator_btn;
  GtkWidget *close_btn;
  GtkWidget *needs_attention_indicator;
  GtkDropTarget *drop_target;
  GdkDragAction preferred_action;

  AdwTabView *view;
  AdwTabPage *page;
  gboolean pinned;
  gboolean dragging;

  gboolean hovering;
  gboolean selected;
  gboolean inverted;
  gboolean title_inverted;
  gboolean close_overlap;
  gboolean show_close;
  gboolean fully_visible;

  AdwAnimation *close_btn_animation;
  AdwAnimation *needs_attention_animation;
};

G_DEFINE_FINAL_TYPE (AdwTab, adw_tab, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_VIEW,
  PROP_PINNED,
  PROP_DRAGGING,
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
close_btn_animation_value_cb (double  value,
                              AdwTab *self)
{
  gtk_widget_set_opacity (self->close_btn, value);
  gtk_widget_set_can_target (self->close_btn, value > 0);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
attention_indicator_animation_value_cb (double  value,
                                        AdwTab *self)
{
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
update_state (AdwTab *self)
{
  GtkStateFlags new_state;
  gboolean show_close;

  new_state = gtk_widget_get_state_flags (GTK_WIDGET (self)) &
    ~GTK_STATE_FLAG_SELECTED;

  if (self->selected || self->dragging)
    new_state |= GTK_STATE_FLAG_SELECTED;

  gtk_widget_set_state_flags (GTK_WIDGET (self), new_state, TRUE);

  show_close = (self->hovering && self->fully_visible) || self->selected || self->dragging;

  if (self->show_close != show_close) {
    self->show_close = show_close;

    adw_timed_animation_set_value_from (ADW_TIMED_ANIMATION (self->close_btn_animation),
                                        gtk_widget_get_opacity (self->close_btn));
    adw_timed_animation_set_value_to (ADW_TIMED_ANIMATION (self->close_btn_animation),
                                      self->show_close ? 1 : 0);
    adw_animation_play (self->close_btn_animation);
  }
}

static void
update_tooltip (AdwTab *self)
{
  const char *tooltip = adw_tab_page_get_tooltip (self->page);

  if (tooltip && g_strcmp0 (tooltip, "") != 0)
    gtk_widget_set_tooltip_markup (GTK_WIDGET (self), tooltip);
  else
    gtk_widget_set_tooltip_text (GTK_WIDGET (self),
                                 adw_tab_page_get_title (self->page));
}

static void
update_title (AdwTab *self)
{
  const char *title = adw_tab_page_get_title (self->page);
  PangoDirection title_direction = PANGO_DIRECTION_NEUTRAL;
  GtkTextDirection direction = gtk_widget_get_direction (GTK_WIDGET (self));
  gboolean title_inverted;

  if (title)
    title_direction = adw_find_base_dir (title, -1);

  title_inverted =
    (title_direction == PANGO_DIRECTION_LTR && direction == GTK_TEXT_DIR_RTL) ||
    (title_direction == PANGO_DIRECTION_RTL && direction == GTK_TEXT_DIR_LTR);

  if (self->title_inverted != title_inverted) {
    self->title_inverted = title_inverted;
    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  update_tooltip (self);
}

static void
update_spinner (AdwTab *self)
{
  gboolean loading = self->page && adw_tab_page_get_loading (self->page);
  gboolean mapped = gtk_widget_get_mapped (GTK_WIDGET (self));

  /* Don't use CPU when not needed */
  gtk_spinner_set_spinning (self->spinner, loading && mapped);
}

static void
update_icons (AdwTab *self)
{
  GIcon *gicon = adw_tab_page_get_icon (self->page);
  gboolean loading = adw_tab_page_get_loading (self->page);
  GIcon *indicator = adw_tab_page_get_indicator_icon (self->page);
  const char *name = loading ? "spinner" : "icon";

  if (self->pinned && !gicon)
    gicon = adw_tab_view_get_default_icon (self->view);

  gtk_image_set_from_gicon (self->icon, gicon);
  gtk_widget_set_visible (self->icon_stack,
                          (gicon != NULL || loading) &&
                          (!self->pinned || indicator == NULL));
  gtk_stack_set_visible_child_name (GTK_STACK (self->icon_stack), name);

  gtk_widget_set_visible (self->indicator_btn, indicator != NULL);
}

static void
update_indicator (AdwTab *self)
{
  gboolean activatable = self->page && adw_tab_page_get_indicator_activatable (self->page);
  gboolean clickable = activatable && (self->selected || (!self->pinned && self->fully_visible));

  gtk_widget_set_can_target (self->indicator_btn, clickable);
}

static void
update_needs_attention (AdwTab *self)
{
  gboolean needs_attention = adw_tab_page_get_needs_attention (self->page);

  adw_timed_animation_set_value_from (ADW_TIMED_ANIMATION (self->needs_attention_animation),
                                      adw_animation_get_value (self->needs_attention_animation));
  adw_timed_animation_set_value_to (ADW_TIMED_ANIMATION (self->needs_attention_animation),
                                    needs_attention ? 1 : 0);
  adw_animation_play (self->needs_attention_animation);

  set_style_class (GTK_WIDGET (self), "needs-attention", needs_attention);
}

static void
update_loading (AdwTab *self)
{
  update_icons (self);
  update_spinner (self);
  set_style_class (GTK_WIDGET (self), "loading",
                   adw_tab_page_get_loading (self->page));
}

static void
update_selected (AdwTab *self)
{
  self->selected = self->dragging;

  if (self->page)
    self->selected |= adw_tab_page_get_selected (self->page);

  update_state (self);
  update_indicator (self);
}

static void
close_idle_cb (AdwTab *self)
{
  adw_tab_view_close_page (self->view, self->page);
  g_object_unref (self);
}

static void
close_clicked_cb (AdwTab *self)
{
  if (!self->page)
    return;

  /* When animations are disabled, we don't want to immediately remove the
   * whole tab mid-click. Instead, defer it until the click has happened.
   */
  g_idle_add_once ((GSourceOnceFunc) close_idle_cb, g_object_ref (self));
}

static void
indicator_clicked_cb (AdwTab *self)
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

static void
enter_cb (AdwTab             *self,
          double              x,
          double              y,
          GtkEventController *controller)
{
  self->hovering = TRUE;

  update_state (self);
}

static void
motion_cb (AdwTab             *self,
           double              x,
           double              y,
           GtkEventController *controller)
{
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  if (input_source == GDK_SOURCE_TOUCHSCREEN)
    return;

  if (self->hovering)
    return;

  self->hovering = TRUE;

  update_state (self);
}

static void
leave_cb (AdwTab             *self,
          GtkEventController *controller)
{
  self->hovering = FALSE;

  update_state (self);
}

static gboolean
drop_cb (AdwTab *self,
         GValue *value)
{
  gboolean ret = GDK_EVENT_PROPAGATE;
  GdkDrop *drop = gtk_drop_target_get_current_drop (self->drop_target);
  GdkDragAction preferred_action = gdk_drop_get_actions (drop);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, value, preferred_action, &ret);

  return ret;
}

static GdkDragAction
extra_drag_enter_cb (AdwTab *self)
{
  const GValue *value = gtk_drop_target_get_value (self->drop_target);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, value, &self->preferred_action);
  self->preferred_action = make_action_unique (self->preferred_action);

  return self->preferred_action;
}

static GdkDragAction
extra_drag_motion_cb (AdwTab *self)
{
  return self->preferred_action;
}

static void
extra_drag_notify_value_cb (AdwTab *self)
{
  const GValue *value = gtk_drop_target_get_value (self->drop_target);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, value, &self->preferred_action);
  self->preferred_action = make_action_unique (self->preferred_action);
}

static gboolean
activate_cb (AdwTab   *self,
             GVariant *args)
{
  GtkWidget *child;

  if (!self->page || !self->view)
    return GDK_EVENT_PROPAGATE;

  child = adw_tab_page_get_child (self->page);

  gtk_widget_grab_focus (child);

  return GDK_EVENT_STOP;
}

static void
adw_tab_measure (GtkWidget      *widget,
                 GtkOrientation  orientation,
                 int             for_size,
                 int            *minimum,
                 int            *natural,
                 int            *minimum_baseline,
                 int            *natural_baseline)
{
  AdwTab *self = ADW_TAB (widget);
  int min = 0, nat = 0;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    nat = self->pinned ? BASE_WIDTH_PINNED : BASE_WIDTH;
  } else {
    int child_min, child_nat;

    gtk_widget_measure (self->icon_stack, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->title, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->close_btn, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->indicator_btn, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->needs_attention_indicator, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static inline void
measure_child (GtkWidget *child,
               int        height,
               int       *width)
{
  if (gtk_widget_get_visible (child))
    gtk_widget_measure (child, GTK_ORIENTATION_HORIZONTAL, height, NULL, width, NULL, NULL);
  else
    *width = 0;
}

static inline void
allocate_child (GtkWidget *child,
                int        parent_width,
                int        parent_height,
                int        x,
                int        width,
                int        baseline)
{
  GtkAllocation child_alloc;

  if (gtk_widget_get_direction (child) == GTK_TEXT_DIR_RTL)
    child_alloc.x = parent_width - width - x;
  else
    child_alloc.x = x;

  child_alloc.y = 0;
  child_alloc.width = width;
  child_alloc.height = parent_height;

  gtk_widget_size_allocate (child, &child_alloc, baseline);
}

static int
get_attention_indicator_width (AdwTab *self,
                               int     center_width)
{
  double base_width;

  if (self->pinned) {
    base_width = ATTENTION_INDICATOR_PINNED_WIDTH;
  } else {
    base_width = center_width * ATTENTION_INDICATOR_WIDTH_MULTIPLIER;
    base_width = CLAMP (base_width, ATTENTION_INDICATOR_MIN_WIDTH, ATTENTION_INDICATOR_MAX_WIDTH);
  }

  return base_width * adw_animation_get_value (self->needs_attention_animation);
}

static void
adw_tab_size_allocate (GtkWidget *widget,
                       int        width,
                       int        height,
                       int        baseline)
{
  AdwTab *self = ADW_TAB (widget);
  int indicator_width, close_width, icon_width, title_width, needs_attention_width;
  int center_x, center_width = 0;
  int start_width = 0, end_width = 0;
  int needs_attention_x;

  measure_child (self->icon_stack, height, &icon_width);
  measure_child (self->title, height, &title_width);
  measure_child (self->indicator_btn, height, &indicator_width);
  measure_child (self->close_btn, height, &close_width);
  measure_child (self->needs_attention_indicator, height, &needs_attention_width);

  if (gtk_widget_get_visible (self->indicator_btn)) {
    if (self->pinned) {
      /* Center it in a pinned tab */
      allocate_child (self->indicator_btn, width, height,
                      (width - indicator_width) / 2, indicator_width,
                      baseline);
    } else if (self->inverted) {
      allocate_child (self->indicator_btn, width, height,
                      width - indicator_width, indicator_width,
                      baseline);

      end_width = indicator_width;
    } else {
      allocate_child (self->indicator_btn, width, height,
                      0, indicator_width, baseline);

      start_width = indicator_width;
    }
  }

  if (gtk_widget_get_visible (self->close_btn)) {
    if (self->inverted) {
      allocate_child (self->close_btn, width, height,
                      0, close_width, baseline);

      start_width = close_width;
    } else {
      allocate_child (self->close_btn, width, height,
                      width - close_width, close_width, baseline);

      if (self->title_inverted)
        end_width = close_width;
    }
  }

  center_width = MIN (width - start_width - end_width,
                      icon_width + title_width);
  center_x = CLAMP ((width - center_width) / 2,
                    start_width,
                    width - center_width - end_width);

  self->close_overlap = !self->inverted &&
                        !self->title_inverted &&
                        gtk_widget_get_visible (self->title) &&
                        gtk_widget_get_visible (self->close_btn) &&
                        center_x + center_width > width - close_width;

  needs_attention_width = MAX (needs_attention_width,
                               get_attention_indicator_width (self, center_width));
  needs_attention_x = (width - needs_attention_width) / 2;

  allocate_child (self->needs_attention_indicator, width, height,
                  needs_attention_x, needs_attention_width, baseline);

  if (gtk_widget_get_visible (self->icon_stack)) {
    allocate_child (self->icon_stack, width, height,
                    center_x, icon_width, baseline);

    center_x += icon_width;
    center_width -= icon_width;
  }

  if (gtk_widget_get_visible (self->title))
    allocate_child (self->title, width, height,
                    center_x, center_width, baseline);
}

static void
adw_tab_map (GtkWidget *widget)
{
  AdwTab *self = ADW_TAB (widget);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->map (widget);

  update_spinner (self);
}

static void
adw_tab_unmap (GtkWidget *widget)
{
  AdwTab *self = ADW_TAB (widget);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->unmap (widget);

  update_spinner (self);
}

static void
adw_tab_snapshot (GtkWidget   *widget,
                  GtkSnapshot *snapshot)
{
  AdwTab *self = ADW_TAB (widget);
  float opacity = gtk_widget_get_opacity (self->close_btn);
  gboolean draw_fade = self->close_overlap && opacity > 0;

  gtk_widget_snapshot_child (widget, self->needs_attention_indicator, snapshot);
  gtk_widget_snapshot_child (widget, self->indicator_btn, snapshot);
  gtk_widget_snapshot_child (widget, self->icon_stack, snapshot);

  if (draw_fade) {
    gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
    int width = gtk_widget_get_width (widget);
    int height = gtk_widget_get_height (widget);
    float offset = gtk_widget_get_width (self->close_btn) +
                   gtk_widget_get_margin_end (self->title);

    gtk_snapshot_push_mask (snapshot, GSK_MASK_MODE_INVERTED_ALPHA);

    if (!is_rtl) {
      gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (width, 0));
      gtk_snapshot_scale (snapshot, -1, 1);
    }

    gtk_snapshot_append_linear_gradient (snapshot,
                                         &GRAPHENE_RECT_INIT (0, 0, FADE_WIDTH + offset, height),
                                         &GRAPHENE_POINT_INIT (offset, 0),
                                         &GRAPHENE_POINT_INIT (FADE_WIDTH + offset, 0),
                                         (GskColorStop[2]) {
                                             { 0, { 0, 0, 0, opacity } },
                                             { 1, { 0, 0, 0, 0 } },
                                         },
                                         2);
    gtk_snapshot_pop (snapshot);
  }

  gtk_widget_snapshot_child (widget, self->title, snapshot);

  if (draw_fade)
    gtk_snapshot_pop (snapshot);

  gtk_widget_snapshot_child (widget, self->close_btn, snapshot);
}

static void
adw_tab_direction_changed (GtkWidget        *widget,
                           GtkTextDirection  previous_direction)
{
  AdwTab *self = ADW_TAB (widget);

  update_title (self);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->direction_changed (widget,
                                                              previous_direction);
}

static void
adw_tab_constructed (GObject *object)
{
  AdwTab *self = ADW_TAB (object);

  G_OBJECT_CLASS (adw_tab_parent_class)->constructed (object);

  if (self->pinned) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "pinned");
    gtk_widget_set_visible (self->title, FALSE);
    gtk_widget_set_visible (self->close_btn, FALSE);
    gtk_widget_set_margin_start (self->icon_stack, 0);
    gtk_widget_set_margin_end (self->icon_stack, 0);
  }

  g_signal_connect_object (self->view, "notify::default-icon",
                           G_CALLBACK (update_icons), self,
                           G_CONNECT_SWAPPED);
}

static void
adw_tab_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  AdwTab *self = ADW_TAB (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, self->view);
    break;

  case PROP_PAGE:
    g_value_set_object (value, adw_tab_get_page (self));
    break;

  case PROP_PINNED:
    g_value_set_boolean (value, self->pinned);
    break;

  case PROP_DRAGGING:
    g_value_set_boolean (value, adw_tab_get_dragging (self));
    break;

  case PROP_INVERTED:
    g_value_set_boolean (value, adw_tab_get_inverted (self));
    break;

    default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  AdwTab *self = ADW_TAB (object);

  switch (prop_id) {
  case PROP_VIEW:
    self->view = g_value_get_object (value);
    break;

  case PROP_PAGE:
    adw_tab_set_page (self, g_value_get_object (value));
    break;

  case PROP_PINNED:
    self->pinned = g_value_get_boolean (value);
    break;

  case PROP_DRAGGING:
    adw_tab_set_dragging (self, g_value_get_boolean (value));
    break;

  case PROP_INVERTED:
    adw_tab_set_inverted (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_dispose (GObject *object)
{
  AdwTab *self = ADW_TAB (object);

  adw_tab_set_page (self, NULL);

  g_clear_object (&self->close_btn_animation);
  g_clear_object (&self->needs_attention_animation);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_TAB);

  G_OBJECT_CLASS (adw_tab_parent_class)->dispose (object);
}

static void
adw_tab_class_init (AdwTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_dispose;
  object_class->constructed = adw_tab_constructed;
  object_class->get_property = adw_tab_get_property;
  object_class->set_property = adw_tab_set_property;

  widget_class->measure = adw_tab_measure;
  widget_class->size_allocate = adw_tab_size_allocate;
  widget_class->map = adw_tab_map;
  widget_class->unmap = adw_tab_unmap;
  widget_class->snapshot = adw_tab_snapshot;
  widget_class->direction_changed = adw_tab_direction_changed;

  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         ADW_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_PINNED] =
    g_param_spec_boolean ("pinned", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_DRAGGING] =
    g_param_spec_boolean ("dragging", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_PAGE] =
    g_param_spec_object ("page", NULL, NULL,
                         ADW_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

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

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-tab.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwTab, title);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, icon_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, icon);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, spinner);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, indicator_icon);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, indicator_btn);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, close_btn);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, needs_attention_indicator);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, drop_target);
  gtk_widget_class_bind_template_callback (widget_class, close_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, indicator_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, motion_cb);
  gtk_widget_class_bind_template_callback (widget_class, leave_cb);
  gtk_widget_class_bind_template_callback (widget_class, drop_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_motion_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_notify_value_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_space,     0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Space,  0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Return,    0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_ISO_Enter, 0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Enter,  0, (GtkShortcutFunc) activate_cb, NULL);

  gtk_widget_class_set_css_name (widget_class, "tab");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TAB);

  g_type_ensure (ADW_TYPE_FADING_LABEL);
  g_type_ensure (ADW_TYPE_GIZMO);
}

static void
adw_tab_init (AdwTab *self)
{
  AdwAnimationTarget *target;

  gtk_widget_init_template (GTK_WIDGET (self));

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              close_btn_animation_value_cb,
                                              self, NULL);
  self->close_btn_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 0,
                             CLOSE_BTN_ANIMATION_DURATION, target);

  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->close_btn_animation),
                                  ADW_EASE_IN_OUT_CUBIC);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              attention_indicator_animation_value_cb,
                                              self, NULL);
  self->needs_attention_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 0,
                             ATTENTION_INDICATOR_ANIMATION_DURATION, target);

  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->needs_attention_animation),
                                  ADW_EASE_IN_OUT_CUBIC);
}

AdwTab *
adw_tab_new (AdwTabView *view,
             gboolean    pinned)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (view), NULL);

  return g_object_new (ADW_TYPE_TAB,
                       "view", view,
                       "pinned", pinned,
                       NULL);
}

AdwTabPage *
adw_tab_get_page (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), NULL);

  return self->page;
}

void
adw_tab_set_page (AdwTab     *self,
                  AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB (self));
  g_return_if_fail (page == NULL || ADW_IS_TAB_PAGE (page));

  if (self->page == page)
    return;

  if (self->page) {
    g_signal_handlers_disconnect_by_func (self->page, update_selected, self);
    g_signal_handlers_disconnect_by_func (self->page, update_title, self);
    g_signal_handlers_disconnect_by_func (self->page, update_tooltip, self);
    g_signal_handlers_disconnect_by_func (self->page, update_icons, self);
    g_signal_handlers_disconnect_by_func (self->page, update_indicator, self);
    g_signal_handlers_disconnect_by_func (self->page, update_needs_attention, self);
    g_signal_handlers_disconnect_by_func (self->page, update_loading, self);
  }

  g_set_object (&self->page, page);

  if (self->page) {
    update_selected (self);
    update_state (self);
    update_title (self);
    update_tooltip (self);
    update_spinner (self);
    update_icons (self);
    update_indicator (self);
    update_needs_attention (self);
    update_loading (self);

    g_signal_connect_object (self->page, "notify::selected",
                             G_CALLBACK (update_selected), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::title",
                             G_CALLBACK (update_title), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::tooltip",
                             G_CALLBACK (update_tooltip), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::icon",
                             G_CALLBACK (update_icons), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-icon",
                             G_CALLBACK (update_icons), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-activatable",
                             G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::needs-attention",
                             G_CALLBACK (update_needs_attention), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::loading",
                             G_CALLBACK (update_loading), self,
                             G_CONNECT_SWAPPED);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAGE]);
}

gboolean
adw_tab_get_dragging (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), FALSE);

  return self->dragging;
}

void
adw_tab_set_dragging (AdwTab   *self,
                      gboolean  dragging)
{
  g_return_if_fail (ADW_IS_TAB (self));

  dragging = !!dragging;

  if (self->dragging == dragging)
    return;

  self->dragging = dragging;

  update_state (self);
  update_selected (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DRAGGING]);
}

gboolean
adw_tab_get_inverted (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), FALSE);

  return self->inverted;
}

void
adw_tab_set_inverted (AdwTab   *self,
                      gboolean  inverted)
{
  g_return_if_fail (ADW_IS_TAB (self));

  inverted = !!inverted;

  if (self->inverted == inverted)
    return;

  self->inverted = inverted;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

void
adw_tab_set_fully_visible (AdwTab   *self,
                           gboolean  fully_visible)
{
  g_return_if_fail (ADW_IS_TAB (self));

  fully_visible = !!fully_visible;

  if (self->fully_visible == fully_visible)
    return;

  self->fully_visible = fully_visible;

  update_state (self);
  update_indicator (self);
}

void
adw_tab_setup_extra_drop_target (AdwTab        *self,
                                 GdkDragAction  actions,
                                 GType         *types,
                                 gsize          n_types)
{
  g_return_if_fail (ADW_IS_TAB (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  gtk_drop_target_set_actions (self->drop_target, actions);
  gtk_drop_target_set_gtypes (self->drop_target, types, n_types);

  self->preferred_action = make_action_unique (actions);
}

void
adw_tab_set_extra_drag_preload (AdwTab   *self,
                                gboolean  preload)
{
  g_return_if_fail (ADW_IS_TAB (self));

  gtk_drop_target_set_preload (self->drop_target, preload);
}

gboolean
adw_tab_can_click_at (AdwTab *self,
                      float   x,
                      float   y)
{
  GtkWidget *picked;

  g_return_val_if_fail (ADW_IS_TAB (self), FALSE);

  picked = gtk_widget_pick (GTK_WIDGET (self), x, y, GTK_PICK_DEFAULT);

  if (picked == NULL)
    return TRUE;

  if (picked == self->close_btn || gtk_widget_is_ancestor (picked, self->close_btn))
    return FALSE;

  if (picked == self->indicator_btn || gtk_widget_is_ancestor (picked, self->indicator_btn))
    return FALSE;

  return TRUE;
}
