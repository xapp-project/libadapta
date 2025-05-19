/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
 * Copyright (C) 2020, 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-overlay-split-view.h"

#include <math.h>

#include "adw-animation-util.h"
#include "adw-bin.h"
#include "adw-gizmo-private.h"
#include "adw-length-unit.h"
#include "adw-shadow-helper-private.h"
#include "adw-spring-animation.h"
#include "adw-swipeable.h"
#include "adw-swipe-tracker-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwOverlaySplitView:
 *
 * A widget presenting sidebar and content side by side or as an overlay.
 *
 * <picture>
 *   <source srcset="overlay-split-view-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="overlay-split-view.png" alt="overlay-split-view">
 * </picture>
 * <picture>
 *   <source srcset="overlay-split-view-collapsed-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="overlay-split-view-collapsed.png" alt="overlay-split-view-collapsed">
 * </picture>
 *
 * `AdwOverlaySplitView` has two children: sidebar and content, and displays
 * them side by side.
 *
 * When [property@OverlaySplitView:collapsed] is set to `TRUE`, the sidebar is
 * instead shown as an overlay above the content widget.
 *
 * The sidebar can be hidden or shown using the
 * [property@OverlaySplitView:show-sidebar] property.
 *
 * Sidebar can be displayed before or after the content, this can be controlled
 * with the [property@OverlaySplitView:sidebar-position] property.
 *
 * Collapsing the split view automatically hides the sidebar widget, and
 * uncollapsing it shows the sidebar. If this behavior is not desired, the
 * [property@OverlaySplitView:pin-sidebar] property can be used to override it.
 *
 * `AdwOverlaySplitView` supports an edge swipe gesture for showing the sidebar,
 * and a swipe from the sidebar for hiding it. Gestures are only supported on
 * touchscreen, but not touchpad. Gestures can be controlled with the
 * [property@OverlaySplitView:enable-show-gesture] and
 * [property@OverlaySplitView:enable-hide-gesture] properties.
 *
 * See also [class@NavigationSplitView].
 *
 * `AdwOverlaySplitView` is typically used together with an [class@Breakpoint]
 * setting the `collapsed` property to `TRUE` on small widths, as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="width-request">360</property>
 *   <property name="height-request">200</property>
 *   <property name="default-width">800</property>
 *   <property name="default-height">800</property>
 *   <child>
 *     <object class="AdwBreakpoint">
 *       <condition>max-width: 400sp</condition>
 *       <setter object="split_view" property="collapsed">True</setter>
 *     </object>
 *   </child>
 *   <property name="content">
 *     <object class="AdwOverlaySplitView" id="split_view">
 *       <property name="sidebar">
 *         <!-- ... -->
 *       </property>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * `AdwOverlaySplitView` is often used for implementing the
 * [utility pane](https://developer.gnome.org/hig/patterns/containers/utility-panes.html)
 * pattern.
 *
 * ## Sizing
 *
 * When not collapsed, `AdwOverlaySplitView` changes the sidebar width
 * depending on its own width.
 *
 * If possible, it tries to allocate a fraction of the total width, controlled
 * with the [property@OverlaySplitView:sidebar-width-fraction] property.
 *
 * The sidebar also has minimum and maximum sizes, controlled with the
 * [property@OverlaySplitView:min-sidebar-width] and
 * [property@OverlaySplitView:max-sidebar-width] properties.
 *
 * The minimum and maximum sizes are using the length unit specified with the
 * [property@OverlaySplitView:sidebar-width-unit].
 *
 * By default, sidebar is using 25% of the total width, with 180sp as the
 * minimum size and 280sp as the maximum size.
 *
 * When collapsed, the preferred width fraction is ignored and the sidebar uses
 * [property@OverlaySplitView:max-sidebar-width] when possible.
 *
 * ## Header Bar Integration
 *
 * When used inside `AdwOverlaySplitView`, [class@HeaderBar] will automatically
 * hide the window buttons in the middle.
 *
 * ## `AdwOverlaySplitView` as `GtkBuildable`
 *
 * The `AdwOverlaySplitView` implementation of the [iface@Gtk.Buildable]
 * interface supports setting the sidebar widget by specifying “sidebar” as the
 * “type” attribute of a `<child>` element, Specifying “content” child type or
 * omitting it results in setting the content widget.
 *
 * ## CSS nodes
 *
 * `AdwOverlaySplitView` has a single CSS node with the name
 * `overlay-split-view`.
 *
 * It contains two nodes with the name `widget`, containing the sidebar and
 * content children.
 *
 * When not collapsed, they have the `.sidebar-view` and `.content-view` style
 * classes respectively.
 *
 * ```
 * overlay-split-view
 * ├── widget.sidebar-pane
 * │   ╰── [sidebar child]
 * ╰── widget.content-pane
 *     ╰── [content child]
 * ```
 *
 * When collapsed, the one containing the sidebar child has the `.background`
 * style class and the other one has no style classes.
 *
 * ```
 * overlay-split-view
 * ├── widget.background
 * │   ╰── [sidebar child]
 * ╰── widget
 *     ╰── [content child]
 * ```
 *
 * ## Accessibility
 *
 * `AdwOverlaySplitView` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

struct _AdwOverlaySplitView
{
  GtkWidget parent_instance;

  GtkWidget *content_bin;
  GtkWidget *sidebar_bin;
  GtkWidget *shield;

  GtkPackType sidebar_position;
  gboolean show_sidebar;
  gboolean pin_sidebar;
  gboolean collapsed;

  double show_progress;
  AdwAnimation *animation;

  AdwShadowHelper *shadow_helper;

  gboolean enable_show_gesture;
  gboolean enable_hide_gesture;
  AdwSwipeTracker *swipe_tracker;
  gboolean swipe_detected;
  gboolean swipe_active;

  GtkEventController *shortcut_controller;

  double min_sidebar_width;
  double max_sidebar_width;
  double sidebar_width_fraction;
  AdwLengthUnit sidebar_width_unit;

  int sidebar_width;
};

static void adw_overlay_split_view_buildable_init (GtkBuildableIface *iface);
static void adw_overlay_split_view_swipeable_init (AdwSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwOverlaySplitView, adw_overlay_split_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_overlay_split_view_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADW_TYPE_SWIPEABLE, adw_overlay_split_view_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_SIDEBAR,
  PROP_SIDEBAR_POSITION,
  PROP_SHOW_SIDEBAR,
  PROP_COLLAPSED,
  PROP_PIN_SIDEBAR,
  PROP_ENABLE_SHOW_GESTURE,
  PROP_ENABLE_HIDE_GESTURE,
  PROP_MIN_SIDEBAR_WIDTH,
  PROP_MAX_SIDEBAR_WIDTH,
  PROP_SIDEBAR_WIDTH_FRACTION,
  PROP_SIDEBAR_WIDTH_UNIT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static inline GtkPackType
get_start_or_end (AdwOverlaySplitView *self)
{
  GtkTextDirection direction = gtk_widget_get_direction (GTK_WIDGET (self));
  gboolean is_rtl = direction == GTK_TEXT_DIR_RTL;

  return is_rtl ? GTK_PACK_END : GTK_PACK_START;
}

static void
update_swipe_tracker (AdwOverlaySplitView *self)
{
  gboolean reverse = self->sidebar_position == get_start_or_end (self);

  if (!self->swipe_tracker)
    return;

  adw_swipe_tracker_set_reversed (self->swipe_tracker, reverse);
  adw_swipe_tracker_set_enabled (self->swipe_tracker, self->enable_show_gesture ||
                                                      self->enable_hide_gesture);
}

static void
update_shield (AdwOverlaySplitView *self)
{
  gtk_widget_set_child_visible (self->shield,
                          self->collapsed && self->show_progress > 0);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
set_show_progress (double               progress,
                   AdwOverlaySplitView *self)
{
  self->show_progress = progress;

  update_shield (self);

  if (self->collapsed)
    gtk_widget_queue_allocate (GTK_WIDGET (self));
  else
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
animate_sidebar (AdwOverlaySplitView *self,
                 double               to,
                 double               velocity)
{
  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->animation),
                                       self->show_progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->animation), to);

  if (!G_APPROX_VALUE (self->show_progress, to, DBL_EPSILON))
    adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->animation),
                                               velocity / adw_swipeable_get_distance (ADW_SWIPEABLE (self)));
  else
    adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->animation),
                                               velocity);

  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->animation),
                                  to < 0.5);

  adw_animation_play (self->animation);
}

static void
set_show_sidebar (AdwOverlaySplitView *self,
                  gboolean             show_sidebar,
                  gboolean             animate,
                  double               velocity)
{
  show_sidebar = !!show_sidebar;

  if (self->show_sidebar == show_sidebar)
    return;

  self->show_sidebar = show_sidebar;

  if (show_sidebar)
    gtk_widget_set_child_visible (self->sidebar_bin, TRUE);

  if (animate) {
    if (!self->swipe_active)
      animate_sidebar (self, show_sidebar ? 1 : 0, velocity);
  } else {
    set_show_progress (show_sidebar ? 1 : 0, self);

    if (!show_sidebar)
      gtk_widget_set_child_visible (self->sidebar_bin, FALSE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_SIDEBAR]);
}

static void
prepare_cb (AdwSwipeTracker        *tracker,
            AdwNavigationDirection  direction,
            AdwOverlaySplitView    *self)
{
  gboolean fully_opened, fully_closed;

  self->swipe_detected = FALSE;

  fully_opened = G_APPROX_VALUE (self->show_progress, 1, DBL_EPSILON) ||
                 self->show_progress > 1;
  fully_closed = G_APPROX_VALUE (self->show_progress, 0, DBL_EPSILON) ||
                 self->show_progress < 0;

  if (fully_opened && !self->collapsed &&
      direction == ADW_NAVIGATION_DIRECTION_FORWARD)
    return;

  if (fully_closed && !self->enable_show_gesture)
    return;

  if (fully_opened && !self->enable_hide_gesture)
    return;

  self->swipe_detected = TRUE;
}

static void
begin_swipe_cb (AdwSwipeTracker     *tracker,
                AdwOverlaySplitView *self)
{
  if (!self->swipe_detected)
    return;

  adw_animation_pause (self->animation);
  gtk_widget_set_child_visible (self->sidebar_bin, TRUE);

  self->swipe_detected = FALSE;
  self->swipe_active = TRUE;
}

static void
update_swipe_cb (AdwSwipeTracker     *tracker,
                 double               progress,
                 AdwOverlaySplitView *self)
{
  if (!self->swipe_active)
    return;

  set_show_progress (progress, self);
}

static void
end_swipe_cb (AdwSwipeTracker     *tracker,
              double               velocity,
              double               to,
              AdwOverlaySplitView *self)
{
  if (!self->swipe_active)
    return;

  self->swipe_active = FALSE;

  if ((to > 0) == self->show_sidebar)
    animate_sidebar (self, to, velocity);
  else
    set_show_sidebar (self, to > 0, TRUE, velocity);
}

static void
released_cb (GtkGestureClick     *gesture,
             int                  n_press,
             double               x,
             double               y,
             AdwOverlaySplitView *self)
{
  adw_overlay_split_view_set_show_sidebar (self, FALSE);
}

static inline void
get_preferred_size (GtkWidget      *widget,
                    GtkOrientation  orientation,
                    int            *min,
                    int            *nat)
{
  gtk_widget_measure (widget, orientation, -1, min, nat, NULL, NULL);
}

static int
get_sidebar_width (AdwOverlaySplitView *self,
                   int                  width,
                   gboolean             collapsed)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (self));
  int sidebar_min, sidebar_max;

  gtk_widget_measure (self->sidebar_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sidebar_min, NULL, NULL, NULL);

  sidebar_min = MAX (sidebar_min,
                     ceil (adw_length_unit_to_px (self->sidebar_width_unit,
                                                  self->min_sidebar_width,
                                                  settings)));

  sidebar_max = MAX (sidebar_min,
                     ceil (adw_length_unit_to_px (self->sidebar_width_unit,
                                                  self->max_sidebar_width,
                                                  settings)));

  if (collapsed)
    return CLAMP (width, sidebar_min, sidebar_max);

  return CLAMP ((int) (width * self->sidebar_width_fraction), sidebar_min, sidebar_max);
}

static void
measure_sidebar (GtkWidget      *widget,
                 GtkOrientation  orientation,
                 int             for_size,
                 int            *minimum,
                 int            *natural,
                 int            *minimum_baseline,
                 int            *natural_baseline)
{
  GtkWidget *child = adw_bin_get_child (ADW_BIN (widget));

  if (!child) {
    if (minimum)
      *minimum = 0;
    if (natural)
      *natural = 0;
    if (minimum_baseline)
      *minimum_baseline = -1;
    if (natural_baseline)
      *natural_baseline = -1;

    return;
  }

  gtk_widget_measure (child, orientation, for_size,
                      minimum, natural, minimum_baseline, natural_baseline);
}

static void
allocate_sidebar (GtkWidget *widget,
                  int        width,
                  int        height,
                  int        baseline)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (gtk_widget_get_parent (widget));
  GtkWidget *child = adw_bin_get_child (ADW_BIN (widget));

  if (!child)
    return;

  if (width > self->sidebar_width) {
    GskTransform *transform = NULL;

    if (self->sidebar_position == get_start_or_end (self))
      transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width - self->sidebar_width, 0));

    gtk_widget_allocate (child, self->sidebar_width, height, baseline, transform);

    return;
  }

  gtk_widget_allocate (child, width, height, baseline, NULL);
}

static void
measure_uncollapsed (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);
  int sidebar_min = 0, sidebar_nat = 0;
  int content_min = 0, content_nat = 0;

  gtk_widget_measure (self->sidebar_bin, orientation, -1,
                      &sidebar_min, &sidebar_nat, NULL, NULL);
  gtk_widget_measure (self->content_bin, orientation, -1,
                      &content_min, &content_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    GtkSettings *settings = gtk_widget_get_settings (widget);
    int sidebar_max;
    double progress;

    sidebar_min = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->min_sidebar_width,
                                                           settings));
    sidebar_max = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->max_sidebar_width,
                                                           settings));

    /* Ignore sidebar's own natural width and instead estimate it from content
     * and fraction */
    sidebar_nat = ceil (content_nat * self->sidebar_width_fraction /
                        (1.0 - self->sidebar_width_fraction));
    sidebar_nat = CLAMP (sidebar_nat, sidebar_min, sidebar_max);

    progress = CLAMP (self->show_progress, 0, 1);

    if (minimum)
      *minimum = (int) ((double) sidebar_min * progress) + content_min;
    if (natural)
      *natural = (int) ((double) sidebar_nat * progress) + content_nat;
  } else {
    if (minimum)
      *minimum = MAX (sidebar_min, content_min);
    if (natural)
      *natural = MAX (sidebar_nat, content_nat);
  }
}

static void
allocate_uncollapsed (GtkWidget *widget,
                      int        width,
                      int        height,
                      int        baseline)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);
  int content_min, sidebar_width, sidebar_offset;
  GskTransform *transform;

  gtk_widget_measure (self->content_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &content_min, NULL, NULL, NULL);

  sidebar_width = MIN (get_sidebar_width (self, width, FALSE), width - content_min);
  sidebar_offset = (int) (sidebar_width * self->show_progress);

  self->sidebar_width = sidebar_width;

  if (sidebar_offset > sidebar_width) {
    sidebar_width = sidebar_offset;
    sidebar_offset = self->sidebar_width;
  }

  if (self->sidebar_position == get_start_or_end (self)) {
    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sidebar_offset - self->sidebar_width, 0));
    gtk_widget_allocate (self->sidebar_bin, sidebar_width, height, baseline, transform);

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sidebar_offset, 0));
    gtk_widget_allocate (self->content_bin, width - sidebar_offset, height, baseline, transform);
  } else {
    if (sidebar_width > self->sidebar_width)
      transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width - sidebar_width, 0));
    else
      transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width - sidebar_offset, 0));

    gtk_widget_allocate (self->sidebar_bin, sidebar_width, height, baseline, transform);
    gtk_widget_allocate (self->content_bin, width - sidebar_offset, height, baseline, NULL);
  }
}

static void
measure_collapsed (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);

  int content_min = 0, content_nat = 0;
  int sidebar_min = 0, sidebar_nat = 0;

  gtk_widget_measure (self->content_bin, orientation, -1, &content_min, &content_nat, NULL, NULL);
  gtk_widget_measure (self->sidebar_bin, orientation, -1, &sidebar_min, &sidebar_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    GtkSettings *settings = gtk_widget_get_settings (widget);
    int sidebar_max;

    sidebar_min = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->min_sidebar_width,
                                                           settings));
    sidebar_max = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->max_sidebar_width,
                                                           settings));

    sidebar_nat = CLAMP (sidebar_nat, sidebar_min, sidebar_max);
  }

  if (minimum)
    *minimum = MAX (content_min, sidebar_min);
  if (natural)
    *natural = MAX (content_nat, sidebar_nat);
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_collapsed (GtkWidget *widget,
                    int        width,
                    int        height,
                    int        baseline)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);
  int sidebar_width, sidebar_pos, sidebar_offset;
  double shadow_progress;

  sidebar_width = get_sidebar_width (self, width, TRUE);
  self->sidebar_width = sidebar_width;

  sidebar_offset = (int) ((double) sidebar_width * self->show_progress);

  if (self->sidebar_position != get_start_or_end (self)) {
    sidebar_pos = width - sidebar_offset;

    if (sidebar_offset > sidebar_width)
      sidebar_width = sidebar_offset;
  } else {
    sidebar_pos = sidebar_offset - sidebar_width;

    if (sidebar_offset > sidebar_width) {
      sidebar_pos = 0;
      sidebar_width = sidebar_offset;
    }
  }

  if (gtk_widget_should_layout (self->content_bin))
    gtk_widget_allocate (self->content_bin, width, height, baseline, NULL);

  if (gtk_widget_should_layout (self->sidebar_bin))
    gtk_widget_allocate (self->sidebar_bin, sidebar_width, height, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sidebar_pos, 0)));

  if (gtk_widget_should_layout (self->shield))
    gtk_widget_allocate (self->shield, width, height, baseline, NULL);

  shadow_progress = 1 - MIN (self->show_progress, self->collapsed ? 1 : 0);

  if (self->sidebar_position == get_start_or_end (self)) {
    adw_shadow_helper_size_allocate (self->shadow_helper, width, height,
                                     baseline, sidebar_offset, 0,
                                     shadow_progress, GTK_PAN_DIRECTION_LEFT);
  } else {
    adw_shadow_helper_size_allocate (self->shadow_helper, width, height,
                                     baseline, -sidebar_offset, 0,
                                     shadow_progress, GTK_PAN_DIRECTION_RIGHT);
  }
}

static gboolean
escape_shortcut_cb (AdwOverlaySplitView *self)
{
  if (G_APPROX_VALUE (self->show_progress, 0, DBL_EPSILON) ||
      self->show_progress < 0 ||
      !self->collapsed)
    return GDK_EVENT_PROPAGATE;

  adw_overlay_split_view_set_show_sidebar (ADW_OVERLAY_SPLIT_VIEW (self), FALSE);

  return GDK_EVENT_STOP;
}

static void
update_collapsed (AdwOverlaySplitView *self)
{
  if (self->collapsed) {
    gtk_widget_set_layout_manager (GTK_WIDGET (self),
                                   gtk_custom_layout_new (adw_widget_get_request_mode,
                                                          measure_collapsed,
                                                          allocate_collapsed));

    gtk_widget_remove_css_class (self->content_bin, "content-pane");
    gtk_widget_remove_css_class (self->sidebar_bin, "sidebar-pane");
    gtk_widget_add_css_class (self->sidebar_bin, "background");
  } else {
    gtk_widget_set_layout_manager (GTK_WIDGET (self),
                                   gtk_custom_layout_new (adw_widget_get_request_mode,
                                                          measure_uncollapsed,
                                                          allocate_uncollapsed));

    gtk_widget_add_css_class (self->content_bin, "content-pane");
    gtk_widget_add_css_class (self->sidebar_bin, "sidebar-pane");
    gtk_widget_remove_css_class (self->sidebar_bin, "background");
  }
}

static void
animation_done_cb (AdwOverlaySplitView *self)
{
  if (self->show_progress < 0.5)
    gtk_widget_set_child_visible (self->sidebar_bin, FALSE);
}

static void
adw_overlay_split_view_snapshot (GtkWidget   *widget,
                                 GtkSnapshot *snapshot)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);

  gtk_widget_snapshot_child (widget, self->content_bin, snapshot);

  if (self->show_progress > 0)
    gtk_widget_snapshot_child (widget, self->sidebar_bin, snapshot);

  adw_shadow_helper_snapshot (self->shadow_helper, snapshot);
}

static void
adw_overlay_split_view_direction_changed (GtkWidget        *widget,
                                          GtkTextDirection  previous_direction)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (widget);

  update_swipe_tracker (self);

  GTK_WIDGET_CLASS (adw_overlay_split_view_parent_class)->direction_changed (widget,
                                                                             previous_direction);
}

static void
adw_overlay_split_view_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_overlay_split_view_get_content (self));
    break;
  case PROP_SIDEBAR:
    g_value_set_object (value, adw_overlay_split_view_get_sidebar (self));
    break;
  case PROP_COLLAPSED:
    g_value_set_boolean (value, adw_overlay_split_view_get_collapsed (self));
    break;
  case PROP_SIDEBAR_POSITION:
    g_value_set_enum (value, adw_overlay_split_view_get_sidebar_position (self));
    break;
  case PROP_SHOW_SIDEBAR:
    g_value_set_boolean (value, adw_overlay_split_view_get_show_sidebar (self));
    break;
  case PROP_PIN_SIDEBAR:
    g_value_set_boolean (value, adw_overlay_split_view_get_pin_sidebar (self));
    break;
  case PROP_ENABLE_SHOW_GESTURE:
    g_value_set_boolean (value, adw_overlay_split_view_get_enable_show_gesture (self));
    break;
  case PROP_ENABLE_HIDE_GESTURE:
    g_value_set_boolean (value, adw_overlay_split_view_get_enable_hide_gesture (self));
    break;
  case PROP_MIN_SIDEBAR_WIDTH:
    g_value_set_double (value, adw_overlay_split_view_get_min_sidebar_width (self));
    break;
  case PROP_MAX_SIDEBAR_WIDTH:
    g_value_set_double (value, adw_overlay_split_view_get_max_sidebar_width (self));
    break;
  case PROP_SIDEBAR_WIDTH_FRACTION:
    g_value_set_double (value, adw_overlay_split_view_get_sidebar_width_fraction (self));
    break;
  case PROP_SIDEBAR_WIDTH_UNIT:
    g_value_set_enum (value, adw_overlay_split_view_get_sidebar_width_unit (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_overlay_split_view_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_overlay_split_view_set_content (self, g_value_get_object (value));
    break;
  case PROP_SIDEBAR:
    adw_overlay_split_view_set_sidebar (self, g_value_get_object (value));
    break;
  case PROP_COLLAPSED:
    adw_overlay_split_view_set_collapsed (self, g_value_get_boolean (value));
    break;
  case PROP_SIDEBAR_POSITION:
    adw_overlay_split_view_set_sidebar_position (self, g_value_get_enum (value));
    break;
  case PROP_SHOW_SIDEBAR:
    adw_overlay_split_view_set_show_sidebar (self, g_value_get_boolean (value));
    break;
  case PROP_PIN_SIDEBAR:
    adw_overlay_split_view_set_pin_sidebar (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_SHOW_GESTURE:
    adw_overlay_split_view_set_enable_show_gesture (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_HIDE_GESTURE:
    adw_overlay_split_view_set_enable_hide_gesture (self, g_value_get_boolean (value));
    break;
  case PROP_MIN_SIDEBAR_WIDTH:
    adw_overlay_split_view_set_min_sidebar_width (self, g_value_get_double (value));
    break;
  case PROP_MAX_SIDEBAR_WIDTH:
    adw_overlay_split_view_set_max_sidebar_width (self, g_value_get_double (value));
    break;
  case PROP_SIDEBAR_WIDTH_FRACTION:
    adw_overlay_split_view_set_sidebar_width_fraction (self, g_value_get_double (value));
    break;
  case PROP_SIDEBAR_WIDTH_UNIT:
    adw_overlay_split_view_set_sidebar_width_unit (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_overlay_split_view_dispose (GObject *object)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (object);

  g_clear_pointer (&self->sidebar_bin, gtk_widget_unparent);
  g_clear_pointer (&self->content_bin, gtk_widget_unparent);

  g_clear_pointer (&self->shield, gtk_widget_unparent);

  g_clear_object (&self->shadow_helper);
  g_clear_object (&self->swipe_tracker);
  g_clear_object (&self->animation);

  self->shortcut_controller = NULL;

  G_OBJECT_CLASS (adw_overlay_split_view_parent_class)->dispose (object);
}

static void
adw_overlay_split_view_class_init (AdwOverlaySplitViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_overlay_split_view_get_property;
  object_class->set_property = adw_overlay_split_view_set_property;
  object_class->dispose = adw_overlay_split_view_dispose;

  widget_class->snapshot = adw_overlay_split_view_snapshot;
  widget_class->direction_changed = adw_overlay_split_view_direction_changed;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwOverlaySplitView:content: (attributes org.gtk.Property.get=adw_overlay_split_view_get_content org.gtk.Property.set=adw_overlay_split_view_set_content)
   *
   * The content widget.
   *
   * Since: 1.4
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:sidebar: (attributes org.gtk.Property.get=adw_overlay_split_view_get_sidebar org.gtk.Property.set=adw_overlay_split_view_set_sidebar)
   *
   * The sidebar widget.
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR] =
    g_param_spec_object ("sidebar", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:collapsed: (attributes org.gtk.Property.get=adw_overlay_split_view_get_collapsed org.gtk.Property.set=adw_overlay_split_view_set_collapsed)
   *
   * Whether the split view is collapsed.
   *
   * When collapsed, the sidebar widget is presented as an overlay above the
   * content widget, otherwise they are displayed side by side.
   *
   * Since: 1.4
   */
  props[PROP_COLLAPSED] =
    g_param_spec_boolean ("collapsed", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:sidebar-position: (attributes org.gtk.Property.get=adw_overlay_split_view_get_sidebar_position org.gtk.Property.set=adw_overlay_split_view_set_sidebar_position)
   *
   * The sidebar position.
   *
   * If it's set to `GTK_PACK_START`, the sidebar is displayed before the content,
   * if `GTK_PACK_END`, it's displayed after the content.
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR_POSITION] =
    g_param_spec_enum ("sidebar-position", NULL, NULL,
                       GTK_TYPE_PACK_TYPE,
                       GTK_PACK_START,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:show-sidebar: (attributes org.gtk.Property.get=adw_overlay_split_view_get_show_sidebar org.gtk.Property.set=adw_overlay_split_view_set_show_sidebar)
   *
   * Whether the sidebar widget is shown.
   *
   * Since: 1.4
   */
  props[PROP_SHOW_SIDEBAR] =
    g_param_spec_boolean ("show-sidebar", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:pin-sidebar: (attributes org.gtk.Property.get=adw_overlay_split_view_get_pin_sidebar org.gtk.Property.set=adw_overlay_split_view_set_pin_sidebar)
   *
   * Whether the sidebar widget is pinned.
   *
   * By default, collapsing @self automatically hides the sidebar widget, and
   * uncollapsing it shows the sidebar. If set to `TRUE`, sidebar visibility
   * never changes on its own.
   *
   * Since: 1.4
   */
  props[PROP_PIN_SIDEBAR] =
    g_param_spec_boolean ("pin-sidebar", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:enable-show-gesture: (attributes org.gtk.Property.get=adw_overlay_split_view_get_enable_show_gesture org.gtk.Property.set=adw_overlay_split_view_set_enable_show_gesture)
   *
   * Whether the sidebar can be opened with an edge swipe gesture.
   *
   * Only touchscreen swipes are supported.
   *
   * Since: 1.4
   */
  props[PROP_ENABLE_SHOW_GESTURE] =
    g_param_spec_boolean ("enable-show-gesture", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:enable-hide-gesture: (attributes org.gtk.Property.get=adw_overlay_split_view_get_enable_hide_gesture org.gtk.Property.set=adw_overlay_split_view_set_enable_hide_gesture)
   *
   * Whether the sidebar can be closed with a swipe gesture.
   *
   * Only touchscreen swipes are supported.
   *
   * Since: 1.4
   */
  props[PROP_ENABLE_HIDE_GESTURE] =
    g_param_spec_boolean ("enable-hide-gesture", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:min-sidebar-width: (attributes org.gtk.Property.get=adw_overlay_split_view_get_min_sidebar_width org.gtk.Property.set=adw_overlay_split_view_set_min_sidebar_width)
   *
   * The minimum sidebar width.
   *
   * Minimum width is affected by
   * [property@OverlaySplitView:sidebar-width-unit].
   *
   * The sidebar widget can still be allocated with larger width if its own
   * minimum width exceeds it.
   *
   * Since: 1.4
   */
  props[PROP_MIN_SIDEBAR_WIDTH] =
    g_param_spec_double ("min-sidebar-width", NULL, NULL,
                         0, G_MAXDOUBLE, 180,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:max-sidebar-width: (attributes org.gtk.Property.get=adw_overlay_split_view_get_max_sidebar_width org.gtk.Property.set=adw_overlay_split_view_set_max_sidebar_width)
   *
   * The maximum sidebar width.
   *
   * Maximum width is affected by
   * [property@OverlaySplitView:sidebar-width-unit].
   *
   * The sidebar widget can still be allocated with larger width if its own
   * minimum width exceeds it.
   *
   * Since: 1.4
   */
  props[PROP_MAX_SIDEBAR_WIDTH] =
    g_param_spec_double ("max-sidebar-width", NULL, NULL,
                         0, G_MAXDOUBLE, 280,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:sidebar-width-fraction: (attributes org.gtk.Property.get=adw_overlay_split_view_get_sidebar_width_fraction org.gtk.Property.set=adw_overlay_split_view_set_sidebar_width_fraction)
   *
   * The preferred sidebar width as a fraction of the total width.
   *
   * The preferred width is additionally limited by
   * [property@OverlaySplitView:min-sidebar-width] and
   * [property@OverlaySplitView:max-sidebar-width].
   *
   * The sidebar widget can be allocated with larger width if its own minimum
   * width exceeds the preferred width.
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR_WIDTH_FRACTION] =
    g_param_spec_double ("sidebar-width-fraction", NULL, NULL,
                         0, 1, 0.25,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwOverlaySplitView:sidebar-width-unit: (attributes org.gtk.Property.get=adw_overlay_split_view_get_sidebar_width_unit org.gtk.Property.set=adw_overlay_split_view_set_sidebar_width_unit)
   *
   * The length unit for minimum and maximum sidebar widths.
   *
   * See [property@OverlaySplitView:min-sidebar-width] and
   * [property@OverlaySplitView:max-sidebar-width].
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR_WIDTH_UNIT] =
    g_param_spec_enum ("sidebar-width-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_SP,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "overlay-split-view");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_overlay_split_view_init (AdwOverlaySplitView *self)
{
  GtkEventController *gesture;
  GtkShortcut *shortcut;
  AdwAnimationTarget *target;

  self->sidebar_position = GTK_PACK_START;
  self->show_sidebar = TRUE;
  self->pin_sidebar = FALSE;
  self->show_progress = 1;
  self->collapsed = FALSE;
  self->enable_show_gesture = TRUE;
  self->enable_hide_gesture = TRUE;
  self->min_sidebar_width = 180;
  self->max_sidebar_width = 280;
  self->sidebar_width_fraction = 0.25;
  self->sidebar_width_unit = ADW_LENGTH_UNIT_SP;

  self->shadow_helper = adw_shadow_helper_new (GTK_WIDGET (self));
  self->swipe_tracker = adw_swipe_tracker_new (ADW_SWIPEABLE (self));
  adw_swipe_tracker_set_enabled (self->swipe_tracker, FALSE);
  adw_swipe_tracker_set_upper_overshoot (self->swipe_tracker, TRUE);

  g_signal_connect (self->swipe_tracker, "prepare", G_CALLBACK (prepare_cb), self);
  g_signal_connect (self->swipe_tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self);

  self->content_bin = adw_bin_new ();
  gtk_widget_set_parent (self->content_bin, GTK_WIDGET (self));

  self->shield = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_parent (self->shield, GTK_WIDGET (self));

  self->sidebar_bin = adw_bin_new ();
  gtk_widget_set_layout_manager (self->sidebar_bin,
                                 gtk_custom_layout_new (adw_widget_get_request_mode,
                                                        measure_sidebar,
                                                        allocate_sidebar));
  gtk_widget_set_parent (self->sidebar_bin, GTK_WIDGET (self));

  gesture = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture),
                                              GTK_PHASE_CAPTURE);
  g_signal_connect_object (gesture, "released", G_CALLBACK (released_cb), self, 0);
  gtk_widget_add_controller (self->shield, gesture);

  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape, 0),
                               gtk_callback_action_new ((GtkShortcutFunc) escape_shortcut_cb, NULL, NULL));

  self->shortcut_controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (self->shortcut_controller),
                                     GTK_SHORTCUT_SCOPE_MANAGED);
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (self->shortcut_controller),
                                        shortcut);
  gtk_widget_add_controller (GTK_WIDGET (self), self->shortcut_controller);

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              set_show_progress,
                                              self, NULL);
  self->animation =
    adw_spring_animation_new (GTK_WIDGET (self), 0, 0,
                             adw_spring_params_new (1, 0.5, 500), target);

  g_signal_connect_swapped (self->animation, "done",
                            G_CALLBACK (animation_done_cb), self);

  update_shield (self);
  update_collapsed (self);
  update_swipe_tracker (self);
}

static void
adw_overlay_split_view_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  if (!g_strcmp0 (type, "content"))
    adw_overlay_split_view_set_content (ADW_OVERLAY_SPLIT_VIEW (buildable),
                                        GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "sidebar"))
    adw_overlay_split_view_set_sidebar (ADW_OVERLAY_SPLIT_VIEW (buildable),
                                        GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_overlay_split_view_set_content (ADW_OVERLAY_SPLIT_VIEW (buildable),
                                        GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_overlay_split_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_overlay_split_view_add_child;
}

static double
adw_overlay_split_view_get_distance (AdwSwipeable *swipeable)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (swipeable);

  return gtk_widget_get_width (self->sidebar_bin);
}

static double *
adw_overlay_split_view_get_snap_points (AdwSwipeable *swipeable,
                                        int          *n_snap_points)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (swipeable);
  gboolean can_open = self->show_progress > 0 || self->enable_show_gesture || self->swipe_active;
  gboolean can_close = self->show_progress < 1 || self->enable_hide_gesture || self->swipe_active;
  double *points;

  if (!can_open && !can_close)
    return NULL;

  if (can_open && can_close) {
    points = g_new0 (double, 2);

    if (n_snap_points)
      *n_snap_points = 2;

    points[0] = 0;
    points[1] = 1;

    return points;
  }

  points = g_new0 (double, 1);

  if (n_snap_points)
    *n_snap_points = 1;

  points[0] = can_open ? 1 : 0;

  return points;
}

static double
adw_overlay_split_view_get_progress (AdwSwipeable *swipeable)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (swipeable);

  return self->show_progress;
}

static double
adw_overlay_split_view_get_cancel_progress (AdwSwipeable *swipeable)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (swipeable);

  return round (self->show_progress);
}

static void
adw_overlay_split_view_get_swipe_area (AdwSwipeable           *swipeable,
                                       AdwNavigationDirection  navigation_direction,
                                       gboolean                is_drag,
                                       GdkRectangle           *rect)
{
  AdwOverlaySplitView *self = ADW_OVERLAY_SPLIT_VIEW (swipeable);
  int sidebar_width;

  if (!is_drag) {
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;

    return;
  }

  sidebar_width = gtk_widget_get_width (self->sidebar_bin);
  sidebar_width = (int) (sidebar_width * self->show_progress);

  rect->width = MAX (sidebar_width, ADW_SWIPE_BORDER);
  rect->y = 0;
  rect->height = gtk_widget_get_height (GTK_WIDGET (self));

  if (self->sidebar_position == get_start_or_end (self))
    rect->x = 0;
  else
    rect->x = gtk_widget_get_width (GTK_WIDGET (self)) - rect->width;
}

static void
adw_overlay_split_view_swipeable_init (AdwSwipeableInterface *iface)
{
  iface->get_distance = adw_overlay_split_view_get_distance;
  iface->get_snap_points = adw_overlay_split_view_get_snap_points;
  iface->get_progress = adw_overlay_split_view_get_progress;
  iface->get_cancel_progress = adw_overlay_split_view_get_cancel_progress;
  iface->get_swipe_area = adw_overlay_split_view_get_swipe_area;
}

/**
 * adw_overlay_split_view_new:
 *
 * Creates a new `AdwOverlaySplitView`.
 *
 * Returns: the newly created `AdwOverlaySplitView`
 *
 * Since: 1.4
 */
GtkWidget *
adw_overlay_split_view_new (void)
{
  return g_object_new (ADW_TYPE_OVERLAY_SPLIT_VIEW, NULL);
}

/**
 * adw_overlay_split_view_get_sidebar: (attributes org.gtk.Method.get_property=sidebar)
 * @self: an overlay split view
 *
 * Gets the sidebar widget for @self.
 *
 * Returns: (transfer none) (nullable): the sidebar widget for @self
 *
 * Since: 1.4
 */
GtkWidget *
adw_overlay_split_view_get_sidebar (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), NULL);

  return adw_bin_get_child (ADW_BIN (self->sidebar_bin));
}

/**
 * adw_overlay_split_view_set_sidebar: (attributes org.gtk.Method.set_property=sidebar)
 * @self: an overlay split view
 * @sidebar: (nullable): the sidebar widget
 *
 * Sets the sidebar widget for @self.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_sidebar (AdwOverlaySplitView *self,
                                    GtkWidget           *sidebar)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));
  g_return_if_fail (sidebar == NULL || GTK_IS_WIDGET (sidebar));

  if (sidebar)
    g_return_if_fail (gtk_widget_get_parent (sidebar) == NULL);

  if (sidebar == adw_overlay_split_view_get_sidebar (self))
    return;

  adw_bin_set_child (ADW_BIN (self->sidebar_bin), sidebar);

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR]);
}

/**
 * adw_overlay_split_view_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: an overlay split view
 *
 * Gets the content widget for @self.
 *
 * Returns: (transfer none) (nullable): the content widget for @self
 *
 * Since: 1.4
 */
GtkWidget *
adw_overlay_split_view_get_content (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), NULL);

  return adw_bin_get_child (ADW_BIN (self->content_bin));
}

/**
 * adw_overlay_split_view_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: an overlay split view
 * @content: (nullable): the content widget
 *
 * Sets the content widget for @self.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_content (AdwOverlaySplitView *self,
                                    GtkWidget           *content)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  if (content == adw_overlay_split_view_get_content (self))
    return;

  adw_bin_set_child (ADW_BIN (self->content_bin), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_overlay_split_view_get_collapsed: (attributes org.gtk.Method.get_property=collapsed)
 * @self: an overlay split view
 *
 * Gets whether @self is collapsed.
 *
 * Returns: whether @self is collapsed
 *
 * Since: 1.4
 */
gboolean
adw_overlay_split_view_get_collapsed (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), FALSE);

  return self->collapsed;
}

/**
 * adw_overlay_split_view_set_collapsed: (attributes org.gtk.Method.set_property=collapsed)
 * @self: an overlay split view
 * @collapsed: whether @self is collapsed
 *
 * Sets whether @self view is collapsed.
 *
 * When collapsed, the sidebar widget is presented as an overlay above the
 * content widget, otherwise they are displayed side by side.
 *
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_collapsed (AdwOverlaySplitView *self,
                                      gboolean             collapsed)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  collapsed = !!collapsed;

  if (self->collapsed == collapsed)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  self->collapsed = collapsed;

  update_shield (self);

  if (!self->pin_sidebar)
    set_show_sidebar (self, !self->collapsed, FALSE, 0);

  update_collapsed (self);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  if (!collapsed) {
    GtkPanDirection shadow_direction;

    if (self->sidebar_position == get_start_or_end (self))
      shadow_direction = GTK_PAN_DIRECTION_LEFT;
    else
      shadow_direction = GTK_PAN_DIRECTION_RIGHT;

    adw_shadow_helper_size_allocate (self->shadow_helper,
                                     gtk_widget_get_width (GTK_WIDGET (self)),
                                     gtk_widget_get_height (GTK_WIDGET (self)),
                                     -1, 0, 0, 1, shadow_direction);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLLAPSED]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_overlay_split_view_get_sidebar_position: (attributes org.gtk.Method.get_property=sidebar-position)
 * @self: an overlay split view
 *
 * Gets the sidebar position for @self.
 *
 * Returns: the sidebar position for @self
 *
 * Since: 1.4
 */
GtkPackType
adw_overlay_split_view_get_sidebar_position (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), GTK_PACK_START);

  return self->sidebar_position;
}

/**
 * adw_overlay_split_view_set_sidebar_position: (attributes org.gtk.Method.set_property=sidebar-position)
 * @self: an overlay split view
 * @position: the new position
 *
 * Sets the sidebar position for @self.
 *
 * If it's set to `GTK_PACK_START`, the sidebar is displayed before the content,
 * if `GTK_PACK_END`, it's displayed after the content.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_sidebar_position (AdwOverlaySplitView *self,
                                             GtkPackType          position)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));
  g_return_if_fail (position <= GTK_PACK_END);

  if (self->sidebar_position == position)
    return;

  self->sidebar_position = position;

  if (position == GTK_PACK_END)
    gtk_widget_add_css_class (self->sidebar_bin, "end");
  else
    gtk_widget_remove_css_class (self->sidebar_bin, "end");

  if (self->show_progress > 0)
    gtk_widget_queue_allocate (GTK_WIDGET (self));

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR_POSITION]);
}

/**
 * adw_overlay_split_view_get_show_sidebar: (attributes org.gtk.Method.get_property=show-sidebar)
 * @self: an overlay split view
 *
 * Gets whether the sidebar widget is shown for @self.
 *
 * Returns: `TRUE` if the sidebar widget is shown
 *
 * Since: 1.4
 */
gboolean
adw_overlay_split_view_get_show_sidebar (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), FALSE);

  return self->show_sidebar;
}

/**
 * adw_overlay_split_view_set_show_sidebar: (attributes org.gtk.Method.set_property=show-sidebar)
 * @self: an overlay split view
 * @show_sidebar: whether to show the sidebar widget
 *
 * Sets whether the sidebar widget is shown for @self.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_show_sidebar (AdwOverlaySplitView *self,
                                         gboolean             show_sidebar)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  set_show_sidebar (self, show_sidebar, TRUE, 0);
}

/**
 * adw_overlay_split_view_get_pin_sidebar: (attributes org.gtk.Method.get_property=pin-sidebar)
 * @self: an overlay split view
 *
 * Gets whether the sidebar widget is pinned for @self.
 *
 * Returns: whether if the sidebar widget is pinned
 *
 * Since: 1.4
 */
gboolean
adw_overlay_split_view_get_pin_sidebar (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), FALSE);

  return self->pin_sidebar;
}

/**
 * adw_overlay_split_view_set_pin_sidebar: (attributes org.gtk.Method.set_property=pin-sidebar)
 * @self: an overlay split view
 * @pin_sidebar: whether to pin the sidebar widget
 *
 * Sets whether the sidebar widget is pinned for @self.
 *
 * By default, collapsing @self automatically hides the sidebar widget, and
 * uncollapsing it shows the sidebar. If set to `TRUE`, sidebar visibility never
 * changes on its own.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_pin_sidebar (AdwOverlaySplitView *self,
                                        gboolean             pin_sidebar)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  pin_sidebar = !!pin_sidebar;

  if (self->pin_sidebar == pin_sidebar)
    return;

  self->pin_sidebar = pin_sidebar;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PIN_SIDEBAR]);
}

/**
 * adw_overlay_split_view_get_enable_show_gesture: (attributes org.gtk.Method.get_property=enable-show-gesture)
 * @self: an overlay split view
 *
 * Gets whether @self can be opened with an edge swipe gesture.
 *
 * Returns: `TRUE` if @self can be opened with a swipe gesture
 *
 * Since: 1.4
 */
gboolean
adw_overlay_split_view_get_enable_show_gesture (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), FALSE);

  return self->enable_show_gesture;
}

/**
 * adw_overlay_split_view_set_enable_show_gesture: (attributes org.gtk.Method.set_property=enable-show-gesture)
 * @self: an overlay split view
 * @enable_show_gesture: whether @self can be opened with a swipe gesture
 *
 * Sets whether @self can be opened with an edge swipe gesture.
 *
 * Only touchscreen swipes are supported.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_enable_show_gesture (AdwOverlaySplitView *self,
                                                gboolean             enable_show_gesture)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  enable_show_gesture = !!enable_show_gesture;

  if (self->enable_show_gesture == enable_show_gesture)
    return;

  self->enable_show_gesture = enable_show_gesture;

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_SHOW_GESTURE]);
}

/**
 * adw_overlay_split_view_get_enable_hide_gesture: (attributes org.gtk.Method.get_property=enable-hide-gesture)
 * @self: an overlay split view
 *
 * Gets whether @self can be closed with a swipe gesture.
 *
 * Returns: `TRUE` if @self can be closed with a swipe gesture
 *
 * Since: 1.4
 */
gboolean
adw_overlay_split_view_get_enable_hide_gesture (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), FALSE);

  return self->enable_hide_gesture;
}

/**
 * adw_overlay_split_view_set_enable_hide_gesture: (attributes org.gtk.Method.set_property=enable-hide-gesture)
 * @self: an overlay split view
 * @enable_hide_gesture: whether @self can be closed with a swipe gesture
 *
 * Sets whether @self can be closed with a swipe gesture.
 *
 * Only touchscreen swipes are supported.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_enable_hide_gesture (AdwOverlaySplitView *self,
                                                gboolean             enable_hide_gesture)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  enable_hide_gesture = !!enable_hide_gesture;

  if (self->enable_hide_gesture == enable_hide_gesture)
    return;

  self->enable_hide_gesture = enable_hide_gesture;

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_HIDE_GESTURE]);
}

/**
 * adw_overlay_split_view_get_min_sidebar_width: (attributes org.gtk.Method.get_property=min-sidebar-width)
 * @self: an overlay split view
 *
 * Gets the minimum sidebar width for @self.
 *
 * Returns: the minimum width
 *
 * Since: 1.4
 */
double
adw_overlay_split_view_get_min_sidebar_width (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), 0.0);

  return self->min_sidebar_width;
}

/**
 * adw_overlay_split_view_set_min_sidebar_width: (attributes org.gtk.Method.set_property=min-sidebar-width)
 * @self: an overlay split view
 * @width: the minimum width
 *
 * Sets the minimum sidebar width for @self.
 *
 * Minimum width is affected by [property@OverlaySplitView:sidebar-width-unit].
 *
 * The sidebar widget can still be allocated with larger width if its own
 * minimum width exceeds it.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_min_sidebar_width (AdwOverlaySplitView *self,
                                              double               width)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->min_sidebar_width, width, DBL_EPSILON))
    return;

  self->min_sidebar_width = width;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MIN_SIDEBAR_WIDTH]);
}

/**
 * adw_overlay_split_view_get_max_sidebar_width: (attributes org.gtk.Method.get_property=max-sidebar-width)
 * @self: an overlay split view
 *
 * Gets the maximum sidebar width for @self.
 *
 * Returns: the maximum width
 *
 * Since: 1.4
 */
double
adw_overlay_split_view_get_max_sidebar_width (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), 0.0);

  return self->max_sidebar_width;
}

/**
 * adw_overlay_split_view_set_max_sidebar_width: (attributes org.gtk.Method.set_property=max-sidebar-width)
 * @self: an overlay split view
 * @width: the maximum width
 *
 * Sets the maximum sidebar width for @self.
 *
 * Maximum width is affected by [property@OverlaySplitView:sidebar-width-unit].
 *
 * The sidebar widget can still be allocated with larger width if its own
 * minimum width exceeds it.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_max_sidebar_width (AdwOverlaySplitView *self,
                                              double               width)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->max_sidebar_width, width, DBL_EPSILON))
    return;

  self->max_sidebar_width = width;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAX_SIDEBAR_WIDTH]);
}

/**
 * adw_overlay_split_view_get_sidebar_width_fraction: (attributes org.gtk.Method.get_property=sidebar-width-fraction)
 * @self: an overlay split view
 *
 * Gets the preferred sidebar width fraction for @self.
 *
 * Returns: the preferred width fraction
 *
 * Since: 1.4
 */
double
adw_overlay_split_view_get_sidebar_width_fraction (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), 0.0);

  return self->sidebar_width_fraction;
}

/**
 * adw_overlay_split_view_set_sidebar_width_fraction: (attributes org.gtk.Method.set_property=sidebar-width-fraction)
 * @self: an overlay split view
 * @fraction: the preferred width fraction
 *
 * Sets the preferred sidebar width as a fraction of the total width of @self.
 *
 * The preferred width is additionally limited by
 * [property@OverlaySplitView:min-sidebar-width] and
 * [property@OverlaySplitView:max-sidebar-width].
 *
 * The sidebar widget can be allocated with larger width if its own minimum
 * width exceeds the preferred width.
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_sidebar_width_fraction (AdwOverlaySplitView *self,
                                                   double               fraction)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->sidebar_width_fraction, fraction, DBL_EPSILON))
    return;

  self->sidebar_width_fraction = fraction;

  if (!self->collapsed)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR_WIDTH_FRACTION]);
}

/**
 * adw_overlay_split_view_get_sidebar_width_unit: (attributes org.gtk.Method.get_property=sidebar-width-unit)
 * @self: an overlay split view
 *
 * Gets the length unit for minimum and maximum sidebar widths.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdwLengthUnit
adw_overlay_split_view_get_sidebar_width_unit (AdwOverlaySplitView *self)
{
  g_return_val_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self), ADW_LENGTH_UNIT_PX);

  return self->sidebar_width_unit;
}

/**
 * adw_overlay_split_view_set_sidebar_width_unit: (attributes org.gtk.Method.set_property=sidebar-width-unit)
 * @self: an overlay split view
 * @unit: the length unit
 *
 * Sets the length unit for minimum and maximum sidebar widths.
 *
 * See [property@OverlaySplitView:min-sidebar-width] and
 * [property@OverlaySplitView:max-sidebar-width].
 *
 * Since: 1.4
 */
void
adw_overlay_split_view_set_sidebar_width_unit (AdwOverlaySplitView *self,
                                               AdwLengthUnit        unit)
{
  g_return_if_fail (ADW_IS_OVERLAY_SPLIT_VIEW (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->sidebar_width_unit)
    return;

  self->sidebar_width_unit = unit;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR_WIDTH_UNIT]);
}
