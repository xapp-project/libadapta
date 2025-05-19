/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 * Author: Adrien Plazas <adrien.plazas@puri.sm>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Forked from the GTK+ 3.24.2 GtkStack widget initially written by Alexander
 * Larsson, and heavily modified for libadwaita by Adrien Plazas on behalf of
 * Purism SPC 2019.
 */

#include "config.h"

#include "adw-squeezer.h"

#include "adw-animation-util.h"
#include "adw-easing.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"

/**
 * AdwSqueezer:
 *
 * A best fit container.
 *
 * <picture>
 *   <source srcset="squeezer-wide-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="squeezer-wide.png" alt="squeezer-wide">
 * </picture>
 * <picture>
 *   <source srcset="squeezer-narrow-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="squeezer-narrow.png" alt="squeezer-narrow">
 * </picture>
 *
 * The `AdwSqueezer` widget is a container which only shows the first of its
 * children that fits in the available size. It is convenient to offer different
 * widgets to represent the same data with different levels of detail, making
 * the widget seem to squeeze itself to fit in the available space.
 *
 * Transitions between children can be animated as fades. This can be controlled
 * with [property@Squeezer:transition-type].
 *
 * ## CSS nodes
 *
 * `AdwSqueezer` has a single CSS node with name `squeezer`.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */

/**
 * AdwSqueezerPage:
 *
 * An auxiliary class used by [class@Squeezer].
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */

/**
 * AdwSqueezerTransitionType:
 * @ADW_SQUEEZER_TRANSITION_TYPE_NONE: No transition
 * @ADW_SQUEEZER_TRANSITION_TYPE_CROSSFADE: A cross-fade
 *
 * Describes the possible transitions in a [class@Squeezer] widget.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */

struct _AdwSqueezerPage {
  GObject parent_instance;

  GtkWidget *widget;
  GtkWidget *last_focus;
  gboolean enabled;
};

G_DEFINE_FINAL_TYPE (AdwSqueezerPage, adw_squeezer_page, G_TYPE_OBJECT)

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_ENABLED,
  LAST_PAGE_PROP
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _AdwSqueezer
{
  GtkWidget parent_instance;

  GList *children;

  AdwSqueezerPage *visible_child;
  AdwFoldThresholdPolicy switch_threshold_policy;

  gboolean homogeneous;

  gboolean allow_none;

  AdwSqueezerTransitionType transition_type;
  guint transition_duration;

  AdwSqueezerPage *last_visible_child;
  gboolean transition_running;
  AdwAnimation *animation;

  int last_visible_widget_width;
  int last_visible_widget_height;

  gboolean interpolate_size;

  float xalign;
  float yalign;

  GtkOrientation orientation;

  GtkSelectionModel *pages;
};

enum  {
  PROP_0,
  PROP_VISIBLE_CHILD,
  PROP_HOMOGENEOUS,
  PROP_SWITCH_THRESHOLD_POLICY,
  PROP_ALLOW_NONE,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_TYPE,
  PROP_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  PROP_XALIGN,
  PROP_YALIGN,
  PROP_PAGES,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_PAGES + 1,
};

static GParamSpec *props[LAST_PROP];

static void adw_squeezer_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSqueezer, adw_squeezer, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_squeezer_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
adw_squeezer_page_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwSqueezerPage *self = ADW_SQUEEZER_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, adw_squeezer_page_get_child (self));
    break;
  case PAGE_PROP_ENABLED:
    g_value_set_boolean (value, adw_squeezer_page_get_enabled (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_squeezer_page_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwSqueezerPage *self = ADW_SQUEEZER_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->widget, g_value_get_object (value));
    break;
  case PAGE_PROP_ENABLED:
    adw_squeezer_page_set_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_squeezer_page_finalize (GObject *object)
{
  AdwSqueezerPage *self = ADW_SQUEEZER_PAGE (object);

  g_clear_object (&self->widget);

  if (self->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);

  G_OBJECT_CLASS (adw_squeezer_page_parent_class)->finalize (object);
}

static void
adw_squeezer_page_class_init (AdwSqueezerPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_squeezer_page_get_property;
  object_class->set_property = adw_squeezer_page_set_property;
  object_class->finalize = adw_squeezer_page_finalize;

  /**
   * AdwSqueezerPage:child: (attributes org.gtk.Property.get=adw_squeezer_page_get_child)
   *
   * The the squeezer child to which the page belongs.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezerPage:enabled: (attributes org.gtk.Property.get=adw_squeezer_page_get_enabled org.gtk.Property.set=adw_squeezer_page_set_enabled)
   *
   * Whether the child is enabled.
   *
   * If a child is disabled, it will be ignored when looking for the child
   * fitting the available size best.
   *
   * This allows to programmatically and prematurely hide a child even if it
   * fits in the available space.
   *
   * This can be used e.g. to ensure a certain child is hidden below a certain
   * window width, or any other constraint you find suitable.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  page_props[PAGE_PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);
}

static void
adw_squeezer_page_init (AdwSqueezerPage *self)
{
  self->enabled = TRUE;
}

#define ADW_TYPE_SQUEEZER_PAGES (adw_squeezer_pages_get_type ())

G_DECLARE_FINAL_TYPE (AdwSqueezerPages, adw_squeezer_pages, ADW, SQUEEZER_PAGES, GObject)

struct _AdwSqueezerPages
{
  GObject parent_instance;
  AdwSqueezer *squeezer;
};

static GType
adw_squeezer_pages_get_item_type (GListModel *model)
{
  return ADW_TYPE_SQUEEZER_PAGE;
}

static guint
adw_squeezer_pages_get_n_items (GListModel *model)
{
  AdwSqueezerPages *self = ADW_SQUEEZER_PAGES (model);

  return g_list_length (self->squeezer->children);
}

static gpointer
adw_squeezer_pages_get_item (GListModel *model,
                             guint       position)
{
  AdwSqueezerPages *self = ADW_SQUEEZER_PAGES (model);
  AdwSqueezerPage *page;

  page = g_list_nth_data (self->squeezer->children, position);

  if (!page)
    return NULL;

  return g_object_ref (page);
}

static void
adw_squeezer_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_squeezer_pages_get_item_type;
  iface->get_n_items = adw_squeezer_pages_get_n_items;
  iface->get_item = adw_squeezer_pages_get_item;
}

static gboolean
adw_squeezer_pages_is_selected (GtkSelectionModel *model,
                                guint              position)
{
  AdwSqueezerPages *self = ADW_SQUEEZER_PAGES (model);
  AdwSqueezerPage *page;

  page = g_list_nth_data (self->squeezer->children, position);

  return page && page == self->squeezer->visible_child;
}

static void
adw_squeezer_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = adw_squeezer_pages_is_selected;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSqueezerPages, adw_squeezer_pages, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_squeezer_pages_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_squeezer_pages_selection_model_init))

static void
adw_squeezer_pages_init (AdwSqueezerPages *pages)
{
}

static void
adw_squeezer_pages_class_init (AdwSqueezerPagesClass *klass)
{
}

static AdwSqueezerPages *
adw_squeezer_pages_new (AdwSqueezer *squeezer)
{
  AdwSqueezerPages *pages;

  pages = g_object_new (ADW_TYPE_SQUEEZER_PAGES, NULL);
  pages->squeezer = squeezer;

  return pages;
}

static GtkOrientation
get_orientation (AdwSqueezer *self)
{
  return self->orientation;
}

static void
set_orientation (AdwSqueezer    *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify (G_OBJECT (self), "orientation");
}

static AdwSqueezerPage *
find_page_for_widget (AdwSqueezer *self,
                      GtkWidget   *child)
{
  AdwSqueezerPage *page;
  GList *l;

  for (l = self->children; l != NULL; l = l->next) {
    page = l->data;
    if (page->widget == child)
      return page;
  }

  return NULL;
}

static void
transition_cb (double       value,
               AdwSqueezer *self)
{
  if (!self->homogeneous)
    gtk_widget_queue_resize (GTK_WIDGET (self));
  else
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
set_transition_running (AdwSqueezer *self,
                        gboolean     running)
{
  if (self->transition_running == running)
    return;

  self->transition_running = running;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
}

static void
transition_done_cb (AdwSqueezer *self)
{
  if (self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }

  adw_animation_reset (self->animation);

  set_transition_running (self, FALSE);
}

static void
set_visible_child (AdwSqueezer               *self,
                   AdwSqueezerPage           *page,
                   AdwSqueezerTransitionType  transition_type,
                   guint                      transition_duration)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkRoot *root;
  GtkWidget *focus;
  gboolean contains_focus = FALSE;
  guint old_pos = GTK_INVALID_LIST_POSITION;
  guint new_pos = GTK_INVALID_LIST_POSITION;

  /* If we are being destroyed, do not bother with transitions and
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick the first visible. */
  if (!page && !self->allow_none) {
    GList *l;

    for (l = self->children; l; l = l->next) {
      AdwSqueezerPage *p = l->data;
      if (gtk_widget_get_visible (p->widget)) {
        page = p;
        break;
      }
    }
  }

  if (page == self->visible_child)
    return;

  if (page != NULL && self->pages) {
    guint position;
    GList *l;

    for (l = self->children, position = 0; l; l = l->next, position++) {
      AdwSqueezerPage *p = l->data;
      if (p == self->visible_child)
        old_pos = position;
      else if (p == page)
        new_pos = position;
    }
  }

  root = gtk_widget_get_root (widget);
  if (root)
    focus = gtk_root_get_focus (root);
  else
    focus = NULL;

  if (focus &&
      self->visible_child &&
      self->visible_child->widget &&
      gtk_widget_is_ancestor (focus, self->visible_child->widget)) {
    contains_focus = TRUE;

    if (self->visible_child->last_focus)
      g_object_remove_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                                    (gpointer *)&self->visible_child->last_focus);
    self->visible_child->last_focus = focus;
    g_object_add_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                               (gpointer *)&self->visible_child->last_focus);
  }

  if (self->transition_running)
    adw_animation_skip (self->animation);

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      self->last_visible_child = self->visible_child;
      self->last_visible_widget_width = gtk_widget_get_width (self->last_visible_child->widget);
      self->last_visible_widget_height = gtk_widget_get_height (self->last_visible_child->widget);
    } else {
      gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
    }
  }

  self->visible_child = page;

  if (page) {
    gtk_widget_set_child_visible (page->widget, TRUE);

    if (contains_focus) {
      if (page->last_focus)
        gtk_widget_grab_focus (page->last_focus);
      else
        gtk_widget_child_focus (page->widget, GTK_DIR_TAB_FORWARD);
    }
  }

  if (self->homogeneous)
    gtk_widget_queue_allocate (widget);
  else
    gtk_widget_queue_resize (widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);

  if (self->pages) {
    if (old_pos == GTK_INVALID_LIST_POSITION && new_pos == GTK_INVALID_LIST_POSITION)
      ; /* nothing to do */
    else if (old_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, new_pos, 1);
    else if (new_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, old_pos, 1);
    else
      gtk_selection_model_selection_changed (self->pages,
                                             MIN (old_pos, new_pos),
                                             MAX (old_pos, new_pos) - MIN (old_pos, new_pos) + 1);
  }

  if (self->transition_type == ADW_SQUEEZER_TRANSITION_TYPE_NONE ||
      (self->last_visible_child == NULL && !self->allow_none))
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation), 0);
  else
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation),
                                      self->transition_duration);

  set_transition_running (self, TRUE);
  adw_animation_play (self->animation);
}

static void
update_child_visible (AdwSqueezer     *self,
                      AdwSqueezerPage *page)
{
  gboolean enabled;

  enabled = page->enabled && gtk_widget_get_visible (page->widget);

  if (self->visible_child == NULL && enabled)
    set_visible_child (self, page, self->transition_type, self->transition_duration);
  else if (self->visible_child == page && !enabled)
    set_visible_child (self, NULL, self->transition_type, self->transition_duration);

  if (page == self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static void
squeezer_child_visibility_notify_cb (GObject    *obj,
                                     GParamSpec *pspec,
                                     gpointer    user_data)
{
  AdwSqueezer *self = ADW_SQUEEZER (user_data);
  GtkWidget *child = GTK_WIDGET (obj);
  AdwSqueezerPage *page;

  page = find_page_for_widget (self, child);
  g_return_if_fail (page != NULL);

  update_child_visible (self, page);
}

static void
add_page (AdwSqueezer     *self,
          AdwSqueezerPage *page)
{
  g_return_if_fail (page->widget != NULL);

  self->children = g_list_append (self->children, g_object_ref (page));

  gtk_widget_set_child_visible (page->widget, FALSE);
  gtk_widget_set_parent (page->widget, GTK_WIDGET (self));

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), g_list_length (self->children) - 1, 0, 1);

  g_signal_connect (page->widget, "notify::visible",
                    G_CALLBACK (squeezer_child_visibility_notify_cb), self);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (page->widget))
    set_visible_child (self, page, self->transition_type, self->transition_duration);

  if (self->homogeneous || self->visible_child == page)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
squeezer_remove (AdwSqueezer *self,
                 GtkWidget   *child,
                 gboolean     in_dispose)
{
  AdwSqueezerPage *page;
  gboolean was_visible;

  page = find_page_for_widget (self, child);
  if (!page)
    return;

  self->children = g_list_remove (self->children, page);

  g_signal_handlers_disconnect_by_func (child,
                                        squeezer_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  g_clear_object (&page->widget);

  if (self->visible_child == page)
    {
      if (in_dispose)
        self->visible_child = NULL;
      else
        set_visible_child (self, NULL, self->transition_type, self->transition_duration);
    }

  if (self->last_visible_child == page)
    self->last_visible_child = NULL;

  gtk_widget_unparent (child);

  g_object_unref (page);

  if (self->homogeneous && was_visible)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
adw_squeezer_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwSqueezer *self = ADW_SQUEEZER (object);

  switch (property_id) {
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, adw_squeezer_get_visible_child (self));
    break;
  case PROP_HOMOGENEOUS:
    g_value_set_boolean (value, adw_squeezer_get_homogeneous (self));
    break;
  case PROP_SWITCH_THRESHOLD_POLICY:
    g_value_set_enum (value, adw_squeezer_get_switch_threshold_policy (self));
    break;
  case PROP_ALLOW_NONE:
    g_value_set_boolean (value, adw_squeezer_get_allow_none (self));
    break;
  case PROP_TRANSITION_DURATION:
    g_value_set_uint (value, adw_squeezer_get_transition_duration (self));
    break;
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, adw_squeezer_get_transition_type (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, adw_squeezer_get_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, adw_squeezer_get_interpolate_size (self));
    break;
  case PROP_XALIGN:
    g_value_set_float (value, adw_squeezer_get_xalign (self));
    break;
  case PROP_YALIGN:
    g_value_set_float (value, adw_squeezer_get_yalign (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  case PROP_PAGES:
    g_value_take_object (value, adw_squeezer_get_pages (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_squeezer_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwSqueezer *self = ADW_SQUEEZER (object);

  switch (property_id) {
  case PROP_HOMOGENEOUS:
    adw_squeezer_set_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_SWITCH_THRESHOLD_POLICY:
    adw_squeezer_set_switch_threshold_policy (self, g_value_get_enum (value));
    break;
  case PROP_ALLOW_NONE:
    adw_squeezer_set_allow_none (self, g_value_get_boolean (value));
    break;
  case PROP_TRANSITION_DURATION:
    adw_squeezer_set_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_TRANSITION_TYPE:
    adw_squeezer_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    adw_squeezer_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_XALIGN:
    adw_squeezer_set_xalign (self, g_value_get_float (value));
    break;
  case PROP_YALIGN:
    adw_squeezer_set_yalign (self, g_value_get_float (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_squeezer_snapshot_crossfade (GtkWidget   *widget,
                                 GtkSnapshot *snapshot)
{
  AdwSqueezer *self = ADW_SQUEEZER (widget);
  double progress = adw_animation_get_value (self->animation);

  gtk_snapshot_push_cross_fade (snapshot, progress);

  if (self->last_visible_child)
    gtk_widget_snapshot_child (widget,
                               self->last_visible_child->widget,
                               snapshot);

  gtk_snapshot_pop (snapshot);

  if (self->visible_child)
    gtk_widget_snapshot_child (widget,
                               self->visible_child->widget,
                               snapshot);
  gtk_snapshot_pop (snapshot);
}


static void
adw_squeezer_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  AdwSqueezer *self = ADW_SQUEEZER (widget);

  if (self->visible_child || self->allow_none) {
    if (self->transition_running &&
        self->transition_type != ADW_SQUEEZER_TRANSITION_TYPE_NONE) {
      gtk_snapshot_push_clip (snapshot,
                              &GRAPHENE_RECT_INIT(
                                  0, 0,
                                  gtk_widget_get_width (widget),
                                  gtk_widget_get_height (widget)
                              ));

      switch (self->transition_type)
        {
        case ADW_SQUEEZER_TRANSITION_TYPE_CROSSFADE:
          adw_squeezer_snapshot_crossfade (widget, snapshot);
          break;
        case ADW_SQUEEZER_TRANSITION_TYPE_NONE:
        default:
          g_assert_not_reached ();
        }

      gtk_snapshot_pop (snapshot);
    } else if (self->visible_child) {
      gtk_widget_snapshot_child (widget,
                                 self->visible_child->widget,
                                 snapshot);
    }
  }
}

static void
adw_squeezer_size_allocate (GtkWidget *widget,
                            int        width,
                            int        height,
                            int        baseline)
{
  AdwSqueezer *self = ADW_SQUEEZER (widget);
  AdwSqueezerPage *page = NULL;
  GList *l;
  GtkAllocation child_allocation;

  for (l = self->children; l; l = l->next) {
    GtkWidget *child = NULL;
    int child_min, child_nat;
    int compare_size;

    page = l->data;
    child = page->widget;

    if (!gtk_widget_get_visible (child))
      continue;

    if (!page->enabled)
      continue;

    if (self->orientation == GTK_ORIENTATION_VERTICAL)
      compare_size = height;
    else
      compare_size = width;

    gtk_widget_measure (child, self->orientation, -1,
                        &child_min, &child_nat, NULL, NULL);

    if (child_min <= compare_size && self->switch_threshold_policy == ADW_FOLD_THRESHOLD_POLICY_MINIMUM)
      break;

    if (child_nat <= compare_size && self->switch_threshold_policy == ADW_FOLD_THRESHOLD_POLICY_NATURAL)
      break;
  }

  if (l == NULL && self->allow_none)
    page = NULL;

  set_visible_child (self, page,
                     self->transition_type,
                     self->transition_duration);

  child_allocation.x = 0;
  child_allocation.y = 0;

  if (self->last_visible_child) {
    int min;

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                          -1, &min, NULL, NULL, NULL);
      child_allocation.width = MAX (min, width);
      gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_VERTICAL,
                          child_allocation.width, &min, NULL, NULL, NULL);
      child_allocation.height = MAX (min, height);
    } else {
      gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_VERTICAL,
                          -1, &min, NULL, NULL, NULL);
      child_allocation.height = MAX (min, height);
      gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                          child_allocation.height, &min, NULL, NULL, NULL);
      child_allocation.width = MAX (min, width);
    }

    if (child_allocation.width > width) {
      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x = (width - child_allocation.width) * (1 - self->xalign);
      else
        child_allocation.x = (width - child_allocation.width) * self->xalign;
    }

    if (child_allocation.height > height)
      child_allocation.y = (height - child_allocation.height) * self->yalign;

    gtk_widget_size_allocate (self->last_visible_child->widget, &child_allocation, -1);
  }

  child_allocation.width = width;
  child_allocation.height = height;
  child_allocation.x = 0;
  child_allocation.y = 0;

  if (self->visible_child) {
    int min;

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                          -1, &min, NULL, NULL, NULL);
      child_allocation.width = MAX (min, width);
      gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_VERTICAL,
                          child_allocation.width, &min, NULL, NULL, NULL);
      child_allocation.height = MAX (min, height);
    } else {
      gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_VERTICAL,
                          -1, &min, NULL, NULL, NULL);
      child_allocation.height = MAX (min, height);
      gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                          child_allocation.height, &min, NULL, NULL, NULL);
      child_allocation.width = MAX (min, width);
    }

    if (child_allocation.width > width) {
      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        child_allocation.x = (width - child_allocation.width) * (1 - self->xalign);
      else
        child_allocation.x = (width - child_allocation.width) * self->xalign;
    }

    if (child_allocation.height > height)
      child_allocation.y = (height - child_allocation.height) * self->yalign;

    gtk_widget_size_allocate (self->visible_child->widget, &child_allocation, -1);
  }
}

static void
adw_squeezer_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  AdwSqueezer *self = ADW_SQUEEZER (widget);
  int child_min, child_nat;
  GList *l;
  int min = 0, nat = 0;

  for (l = self->children; l != NULL; l = l->next) {
    AdwSqueezerPage *page = l->data;
    GtkWidget *child = page->widget;

    if (self->orientation != orientation && !self->homogeneous &&
        self->visible_child != page)
      continue;

    if (!gtk_widget_get_visible (child))
      continue;

    /* Disabled children are taken into account when measuring the widget, to
     * keep its size request and allocation consistent. This avoids the
     * appearant size and position of a child to changes suddenly when a larger
     * child gets enabled/disabled.
     */
    if (self->orientation == orientation)
      gtk_widget_measure (child, orientation, -1,
                          &child_min, &child_nat, NULL, NULL);
    else
      gtk_widget_measure (child, orientation, for_size,
                          &child_min, &child_nat, NULL, NULL);

    if (self->orientation == orientation) {
      if (self->allow_none)
        min = 0;
      else
        min = min == 0 ? child_min : MIN (min, child_min);
    } else {
      min = MAX (min, child_min);
    }

    nat = MAX (nat, child_nat);
  }

  if (self->orientation != orientation && !self->homogeneous &&
      self->interpolate_size &&
      (self->last_visible_child != NULL || self->allow_none)) {
    double t = adw_animation_get_value (self->animation);
    t = adw_easing_ease (ADW_EASE_OUT_CUBIC, t);

    if (orientation == GTK_ORIENTATION_VERTICAL) {
      min = adw_lerp (self->last_visible_widget_height, min, t);
      nat = adw_lerp (self->last_visible_widget_height, nat, t);
    } else {
      min = adw_lerp (self->last_visible_widget_width, min, t);
      nat = adw_lerp (self->last_visible_widget_width, nat, t);
    }
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

static void
adw_squeezer_dispose (GObject *object)
{
  AdwSqueezer *self = ADW_SQUEEZER (object);
  GtkWidget *child;

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), 0,
                                g_list_length (self->children), 0);

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self))))
    squeezer_remove (self, child, TRUE);

  g_clear_object (&self->animation);

  G_OBJECT_CLASS (adw_squeezer_parent_class)->dispose (object);
}

static void
adw_squeezer_finalize (GObject *object)
{
  AdwSqueezer *self = ADW_SQUEEZER (object);

  if (self->pages)
    g_object_remove_weak_pointer (G_OBJECT (self->pages),
                                  (gpointer *) &self->pages);

  G_OBJECT_CLASS (adw_squeezer_parent_class)->finalize (object);
}

static void
adw_squeezer_class_init (AdwSqueezerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_squeezer_get_property;
  object_class->set_property = adw_squeezer_set_property;
  object_class->dispose = adw_squeezer_dispose;
  object_class->finalize = adw_squeezer_finalize;

  widget_class->size_allocate = adw_squeezer_size_allocate;
  widget_class->snapshot = adw_squeezer_snapshot;
  widget_class->measure = adw_squeezer_measure;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdwSqueezer:visible-child: (attributes org.gtk.Property.get=adw_squeezer_get_visible_child)
   *
   * The currently visible child.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:homogeneous: (attributes org.gtk.Property.get=adw_squeezer_get_homogeneous org.gtk.Property.set=adw_squeezer_set_homogeneous)
   *
   * Whether all children have the same size for the opposite orientation.
   *
   * For example, if a squeezer is horizontal and is homogeneous, it will
   * request the same height for all its children. If it isn't, the squeezer may
   * change size when a different child becomes visible.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_HOMOGENEOUS] =
    g_param_spec_boolean ("homogeneous", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:switch-threshold-policy: (attributes org.gtk.Property.get=adw_squeezer_get_switch_threshold_policy org.gtk.Property.set=adw_squeezer_set_switch_threshold_policy)
   *
   * The switch threshold policy.
   *
   * Determines when the squeezer will switch children.
   *
   * If set to `ADW_FOLD_THRESHOLD_POLICY_MINIMUM`, it will only switch when the
   * visible child cannot fit anymore. With `ADW_FOLD_THRESHOLD_POLICY_NATURAL`,
   * it will switch as soon as the visible child doesn't get their natural size.
   *
   * This can be useful if you have a long ellipsizing label and want to let it
   * ellipsize instead of immediately switching.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_SWITCH_THRESHOLD_POLICY] =
    g_param_spec_enum ("switch-threshold-policy", NULL, NULL,
                       ADW_TYPE_FOLD_THRESHOLD_POLICY,
                       ADW_FOLD_THRESHOLD_POLICY_NATURAL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:allow-none: (attributes org.gtk.Property.get=adw_squeezer_get_allow_none org.gtk.Property.set=adw_squeezer_set_allow_none)
   *
   * Whether to allow squeezing beyond the last child's minimum size.
   *
   * If set to `TRUE`, the squeezer can shrink to the point where no child can
   * be shown. This is functionally equivalent to appending a widget with 0×0
   * minimum size.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_ALLOW_NONE] =
    g_param_spec_boolean ("allow-none", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:transition-duration: (attributes org.gtk.Property.get=adw_squeezer_get_transition_duration org.gtk.Property.set=adw_squeezer_set_transition_duration)
   *
   * The transition animation duration, in milliseconds.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration", NULL, NULL,
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:transition-type: (attributes org.gtk.Property.get=adw_squeezer_get_transition_type org.gtk.Property.set=adw_squeezer_set_transition_type)
   *
   * The type of animation used for transitions between children.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type", NULL, NULL,
                       ADW_TYPE_SQUEEZER_TRANSITION_TYPE,
                       ADW_SQUEEZER_TRANSITION_TYPE_NONE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:transition-running: (attributes org.gtk.Property.get=adw_squeezer_get_transition_running)
   *
   * Whether a transition is currently running.
   *
   * If a transition is impossible, the property value will be set to `TRUE` and
   * then immediately to `FALSE`, so it's possible to rely on its notifications
   * to know that a transition has happened.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_TRANSITION_RUNNING] =
    g_param_spec_boolean ("transition-running", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:interpolate-size: (attributes org.gtk.Property.get=adw_squeezer_get_interpolate_size org.gtk.Property.set=adw_squeezer_set_interpolate_size)
   *
   * Whether the squeezer interpolates its size when changing the visible child.
   *
   * If `TRUE`, the squeezer will interpolate its size between the one of the
   * previous visible child and the one of the new visible child, according to
   * the set transition duration and the orientation, e.g. if the squeezer is
   * horizontal, it will interpolate the its height.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_INTERPOLATE_SIZE] =
    g_param_spec_boolean ("interpolate-size", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:xalign: (attributes org.gtk.Property.get=adw_squeezer_get_xalign org.gtk.Property.set=adw_squeezer_set_xalign)
   *
   * The horizontal alignment, from 0 (start) to 1 (end).
   *
   * This affects the children allocation during transitions, when they exceed
   * the size of the squeezer.
   *
   * For example, 0.5 means the child will be centered, 0 means it will keep the
   * start side aligned and overflow the end side, and 1 means the opposite.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_XALIGN] =
    g_param_spec_float ("xalign", NULL, NULL,
                        0.0, 1.0,
                        0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:yalign: (attributes org.gtk.Property.get=adw_squeezer_get_yalign org.gtk.Property.set=adw_squeezer_set_yalign)
   *
   * The vertical alignment, from 0 (top) to 1 (bottom).
   *
   * This affects the children allocation during transitions, when they exceed
   * the size of the squeezer.
   *
   * For example, 0.5 means the child will be centered, 0 means it will keep the
   * top side aligned and overflow the bottom side, and 1 means the opposite.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_YALIGN] =
    g_param_spec_float ("yalign", NULL, NULL,
                        0.0, 1.0,
                        0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwSqueezer:pages: (attributes org.gtk.Property.get=adw_squeezer_get_pages)
   *
   * A selection model with the squeezer's pages.
   *
   * This can be used to keep an up-to-date view. The model also implements
   * [iface@Gtk.SelectionModel] and can be used to track the visible page.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
   */
  props[PROP_PAGES] =
    g_param_spec_object ("pages", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "squeezer");
}

static void
adw_squeezer_init (AdwSqueezer *self)
{
  AdwAnimationTarget *target;

  self->homogeneous = TRUE;
  self->transition_duration = 200;
  self->transition_type = ADW_SQUEEZER_TRANSITION_TYPE_NONE;
  self->xalign = 0.5;
  self->yalign = 0.5;

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) transition_cb,
                                              self, NULL);
  self->animation = adw_timed_animation_new (GTK_WIDGET (self), 0, 1,
                                             self->transition_duration,
                                             target);
  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->animation),
                                  ADW_LINEAR);
  g_signal_connect_swapped (self->animation, "done",
                            G_CALLBACK (transition_done_cb), self);
}

static void
adw_squeezer_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  if (ADW_IS_SQUEEZER_PAGE (child))
    add_page (ADW_SQUEEZER (buildable), ADW_SQUEEZER_PAGE (child));
  else if (GTK_IS_WIDGET (child))
    adw_squeezer_add (ADW_SQUEEZER (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_squeezer_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_squeezer_buildable_add_child;
}

/**
 * adw_squeezer_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a squeezer page
 *
 * Returns the squeezer child to which @self belongs.
 *
 * Returns: (transfer none): the child to which @self belongs
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
GtkWidget *
adw_squeezer_page_get_child (AdwSqueezerPage *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER_PAGE (self), NULL);

  return self->widget;
}

/**
 * adw_squeezer_page_get_enabled: (attributes org.gtk.Method.get_property=enabled)
 * @self: a squeezer page
 *
 * Gets whether @self is enabled.
 *
 * Returns: whether @self is enabled
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
gboolean
adw_squeezer_page_get_enabled (AdwSqueezerPage *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER_PAGE (self), FALSE);

  return self->enabled;
}

/**
 * adw_squeezer_page_set_enabled: (attributes org.gtk.Method.set_property=enabled)
 * @self: a squeezer page
 * @enabled: whether @self is enabled
 *
 * Sets whether @self is enabled.
 *
 * If a child is disabled, it will be ignored when looking for the child
 * fitting the available size best.
 *
 * This allows to programmatically and prematurely hide a child even if it fits
 * in the available space.
 *
 * This can be used e.g. to ensure a certain child is hidden below a certain
 * window width, or any other constraint you find suitable.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_page_set_enabled (AdwSqueezerPage *self,
                               gboolean         enabled)
{
  g_return_if_fail (ADW_IS_SQUEEZER_PAGE (self));

  enabled = !!enabled;

  if (enabled == self->enabled)
    return;

  self->enabled = enabled;

  if (self->widget && gtk_widget_get_parent (self->widget)) {
    AdwSqueezer *squeezer = ADW_SQUEEZER (gtk_widget_get_parent (self->widget));

    gtk_widget_queue_resize (GTK_WIDGET (squeezer));
    update_child_visible (squeezer, self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ENABLED]);
}

/**
 * adw_squeezer_new:
 *
 * Creates a new `AdwSqueezer`.
 *
 * Returns: the newly created `AdwSqueezer`
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
GtkWidget *
adw_squeezer_new (void)
{
  return g_object_new (ADW_TYPE_SQUEEZER, NULL);
}

/**
 * adw_squeezer_add:
 * @self: a squeezer
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Returns: (transfer none): the [class@SqueezerPage] for @child
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
AdwSqueezerPage *
adw_squeezer_add (AdwSqueezer *self,
                  GtkWidget   *child)
{
  AdwSqueezerPage *page;

  g_return_val_if_fail (ADW_IS_SQUEEZER (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  page = g_object_new (ADW_TYPE_SQUEEZER_PAGE, NULL);
  page->widget = g_object_ref (child);

  add_page (self, page);

  g_object_unref (page);

  return page;
}

/**
 * adw_squeezer_remove:
 * @self: a squeezer
 * @child: the child to remove
 *
 * Removes a child widget from @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_remove (AdwSqueezer *self,
                     GtkWidget   *child)
{
  GList *l;
  guint position;

  g_return_if_fail (ADW_IS_SQUEEZER (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  for (l = self->children, position = 0; l; l = l->next, position++) {
    AdwSqueezerPage *page = l->data;

    if (page->widget == child)
      break;
  }

  squeezer_remove (self, child, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 1, 0);
}

/**
 * adw_squeezer_get_page:
 * @self: a squeezer
 * @child: a child of @self
 *
 * Returns the [class@SqueezerPage] object for @child.
 *
 * Returns: (transfer none): the page object for @child
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
AdwSqueezerPage *
adw_squeezer_get_page (AdwSqueezer *self,
                       GtkWidget   *child)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return find_page_for_widget (self, child);
}

/**
 * adw_squeezer_get_visible_child: (attributes org.gtk.Method.get_property=visible-child)
 * @self: a squeezer
 *
 * Gets the currently visible child of @self.
 *
 * Returns: (transfer none) (nullable): the visible child
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
GtkWidget *
adw_squeezer_get_visible_child (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), NULL);

  return self->visible_child ? self->visible_child->widget : NULL;
}

/**
 * adw_squeezer_get_homogeneous: (attributes org.gtk.Method.get_property=homogeneous)
 * @self: a squeezer
 *
 * Gets whether all children have the same size for the opposite orientation.
 *
 * Returns: whether @self is homogeneous
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
gboolean
adw_squeezer_get_homogeneous (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), FALSE);

  return self->homogeneous;
}

/**
 * adw_squeezer_set_homogeneous: (attributes org.gtk.Method.set_property=homogeneous)
 * @self: a squeezer
 * @homogeneous: whether @self is homogeneous
 *
 * Sets whether all children have the same size for the opposite orientation.
 *
 * For example, if a squeezer is horizontal and is homogeneous, it will request
 * the same height for all its children. If it isn't, the squeezer may change
 * size when a different child becomes visible.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_homogeneous (AdwSqueezer *self,
                              gboolean     homogeneous)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  homogeneous = !!homogeneous;

  if (self->homogeneous == homogeneous)
    return;

  self->homogeneous = homogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET(self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HOMOGENEOUS]);
}

/**
 * adw_squeezer_get_switch_threshold_policy: (attributes org.gtk.Method.get_property=switch-threshold-policy)
 * @self: a squeezer
 *
 * Gets the switch threshold policy for @self.
 *
 * Returns: the fold threshold policy
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
AdwFoldThresholdPolicy
adw_squeezer_get_switch_threshold_policy (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), ADW_FOLD_THRESHOLD_POLICY_NATURAL);

  return self->switch_threshold_policy;
}


/**
 * adw_squeezer_set_switch_threshold_policy: (attributes org.gtk.Method.set_property=switch-threshold-policy)
 * @self: a squeezer
 * @policy: the policy to use
 *
 * Sets the switch threshold policy for @self.
 *
 * Determines when the squeezer will switch children.
 *
 * If set to `ADW_FOLD_THRESHOLD_POLICY_MINIMUM`, it will only switch when the
 * visible child cannot fit anymore. With `ADW_FOLD_THRESHOLD_POLICY_NATURAL`,
 * it will switch as soon as the visible child doesn't get their natural size.
 *
 * This can be useful if you have a long ellipsizing label and want to let it
 * ellipsize instead of immediately switching.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_switch_threshold_policy (AdwSqueezer            *self,
                                          AdwFoldThresholdPolicy  policy)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));
  g_return_if_fail (policy <= ADW_FOLD_THRESHOLD_POLICY_NATURAL);

  if (self->switch_threshold_policy == policy)
    return;

  self->switch_threshold_policy = policy;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SWITCH_THRESHOLD_POLICY]);
}

/**
 * adw_squeezer_get_allow_none: (attributes org.gtk.Method.get_property=allow-none)
 * @self: a squeezer
 *
 * Gets whether to allow squeezing beyond the last child's minimum size.
 *
 * Returns: whether @self allows squeezing beyond the last child
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
gboolean
adw_squeezer_get_allow_none (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), FALSE);

  return self->allow_none;
}

/**
 * adw_squeezer_set_allow_none: (attributes org.gtk.Method.set_property=allow-none)
 * @self: a squeezer
 * @allow_none: whether @self allows squeezing beyond the last child
 *
 * Sets whether to allow squeezing beyond the last child's minimum size.
 *
 * If set to `TRUE`, the squeezer can shrink to the point where no child can be
 * shown. This is functionally equivalent to appending a widget with 0×0 minimum
 * size.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_allow_none (AdwSqueezer *self,
                             gboolean     allow_none)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  allow_none = !!allow_none;

  if (self->allow_none == allow_none)
    return;

  self->allow_none = allow_none;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_NONE]);
}

/**
 * adw_squeezer_get_transition_duration: (attributes org.gtk.Method.get_property=transition-duration)
 * @self: a squeezer
 *
 * Gets the transition animation duration for @self.
 *
 * Returns: the transition duration, in milliseconds
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
guint
adw_squeezer_get_transition_duration (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), 0);

  return self->transition_duration;
}

/**
 * adw_squeezer_set_transition_duration: (attributes org.gtk.Method.set_property=transition-duration)
 * @self: a squeezer
 * @duration: the new duration, in milliseconds
 *
 * Sets the transition animation duration for @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_transition_duration (AdwSqueezer *self,
                                      guint        duration)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  if (self->transition_duration == duration)
    return;

  self->transition_duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_DURATION]);
}

/**
 * adw_squeezer_get_transition_type: (attributes org.gtk.Method.get_property=transition-type)
 * @self: a squeezer
 *
 * Gets the type of animation used for transitions between children in @self.
 *
 * Returns: the current transition type of @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
AdwSqueezerTransitionType
adw_squeezer_get_transition_type (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), ADW_SQUEEZER_TRANSITION_TYPE_NONE);

  return self->transition_type;
}

/**
 * adw_squeezer_set_transition_type: (attributes org.gtk.Method.set_property=transition-type)
 * @self: a squeezer
 * @transition: the new transition type
 *
 * Sets the type of animation used for transitions between children in @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_transition_type (AdwSqueezer               *self,
                                  AdwSqueezerTransitionType  transition)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  if (self->transition_type == transition)
    return;

  self->transition_type = transition;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_TYPE]);
}

/**
 * adw_squeezer_get_transition_running: (attributes org.gtk.Method.get_property=transition-running)
 * @self: a squeezer
 *
 * Gets whether a transition is currently running for @self.
 *
 * If a transition is impossible, the property value will be set to `TRUE` and
 * then immediately to `FALSE`, so it's possible to rely on its notifications
 * to know that a transition has happened.
 *
 * Returns: whether a transition is currently running
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
gboolean
adw_squeezer_get_transition_running (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), FALSE);

  return self->transition_running;
}

/**
 * adw_squeezer_get_interpolate_size: (attributes org.gtk.Method.get_property=interpolate-size)
 * @self: A squeezer
 *
 * Gets whether @self interpolates its size when changing the visible child.
 *
 * Returns: whether the size is interpolated
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
gboolean
adw_squeezer_get_interpolate_size (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), FALSE);

  return self->interpolate_size;
}

/**
 * adw_squeezer_set_interpolate_size: (attributes org.gtk.Method.set_property=interpolate-size)
 * @self: A squeezer
 * @interpolate_size: whether to interpolate the size
 *
 * Sets whether @self interpolates its size when changing the visible child.
 *
 * If `TRUE`, the squeezer will interpolate its size between the one of the
 * previous visible child and the one of the new visible child, according to the
 * set transition duration and the orientation, e.g. if the squeezer is
 * horizontal, it will interpolate the its height.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_interpolate_size (AdwSqueezer *self,
                                   gboolean     interpolate_size)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  interpolate_size = !!interpolate_size;

  if (self->interpolate_size == interpolate_size)
    return;

  self->interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}

/**
 * adw_squeezer_get_xalign: (attributes org.gtk.Method.get_property=xalign)
 * @self: a squeezer
 *
 * Gets the horizontal alignment, from 0 (start) to 1 (end).
 *
 * Returns: the alignment value
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
float
adw_squeezer_get_xalign (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), 0.5);

  return self->xalign;
}

/**
 * adw_squeezer_set_xalign: (attributes org.gtk.Method.set_property=xalign)
 * @self: a squeezer
 * @xalign: the new alignment value
 *
 * Sets the horizontal alignment, from 0 (start) to 1 (end).
 *
 * This affects the children allocation during transitions, when they exceed the
 * size of the squeezer.
 *
 * For example, 0.5 means the child will be centered, 0 means it will keep the
 * start side aligned and overflow the end side, and 1 means the opposite.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_xalign (AdwSqueezer *self,
                         float        xalign)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  xalign = CLAMP (xalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->xalign, xalign, FLT_EPSILON))
    return;

  self->xalign = xalign;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_XALIGN]);
}

/**
 * adw_squeezer_get_yalign: (attributes org.gtk.Method.get_property=yalign)
 * @self: a squeezer
 *
 * Gets the vertical alignment, from 0 (top) to 1 (bottom).
 *
 * Returns: the alignment value
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
float
adw_squeezer_get_yalign (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), 0.5);

  return self->yalign;
}

/**
 * adw_squeezer_set_yalign: (attributes org.gtk.Method.set_property=yalign)
 * @self: a squeezer
 * @yalign: the new alignment value
 *
 * Sets the vertical alignment, from 0 (top) to 1 (bottom).
 *
 * This affects the children allocation during transitions, when they exceed the
 * size of the squeezer.
 *
 * For example, 0.5 means the child will be centered, 0 means it will keep the
 * top side aligned and overflow the bottom side, and 1 means the opposite.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
void
adw_squeezer_set_yalign (AdwSqueezer *self,
                         float        yalign)
{
  g_return_if_fail (ADW_IS_SQUEEZER (self));

  yalign = CLAMP (yalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->yalign, yalign, FLT_EPSILON))
    return;

  self->yalign = yalign;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_YALIGN]);
}

/**
 * adw_squeezer_get_pages: (attributes org.gtk.Method.get_property=pages)
 * @self: a squeezer
 *
 * Returns a [iface@Gio.ListModel] that contains the pages of @self.
 *
 * This can be used to keep an up-to-date view. The model also implements
 * [iface@Gtk.SelectionModel] and can be used to track the visible page.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the squeezer's children
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwsqueezer)
 */
GtkSelectionModel *
adw_squeezer_get_pages (AdwSqueezer *self)
{
  g_return_val_if_fail (ADW_IS_SQUEEZER (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  self->pages = GTK_SELECTION_MODEL (adw_squeezer_pages_new (self));
  g_object_add_weak_pointer (G_OBJECT (self->pages), (gpointer *) &self->pages);

  return self->pages;
}
