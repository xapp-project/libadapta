/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adap-tab-overview-private.h"

#include "adap-animation-util.h"
#include "adap-bin.h"
#include "adap-header-bar.h"
#include "adap-marshalers.h"
#include "adap-style-manager.h"
#include "adap-tab-grid-private.h"
#include "adap-tab-thumbnail-private.h"
#include "adap-tab-view-private.h"
#include "adap-timed-animation.h"
#include "adap-widget-utils-private.h"
#include "adap-window-title.h"

#define SCROLL_ANIMATION_DURATION 200
#define TRANSITION_DURATION 400
#define THUMBNAIL_BORDER_RADIUS 12
#define WINDOW_BORDER_RADIUS 12

/**
 * AdapTabOverview:
 *
 * A tab overview for [class@TabView].
 *
 * <picture>
 *   <source srcset="tab-overview-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="tab-overview.png" alt="tab-overview">
 * </picture>
 *
 * `AdapTabOverview` is a widget that can display tabs from an `AdapTabView` in a
 * grid.
 *
 * `AdapTabOverview` shows a thumbnail for each tab. By default thumbnails are
 * static for all pages except the selected one. They can be made always live
 * by setting [property@TabPage:live-thumbnail] to `TRUE`, or refreshed with
 * [method@TabPage.invalidate_thumbnail] or
 * [method@TabView.invalidate_thumbnails] otherwise.
 *
 * If the pages are too tall or too wide, the thumbnails will be cropped; use
 * [property@TabPage:thumbnail-xalign] and [property@TabPage:thumbnail-yalign] to
 * control which part of the page should be visible in this case.
 *
 * Pinned tabs are shown as smaller cards without thumbnails above the other
 * tabs. Unlike in [class@TabBar], they still have titles, as well as an unpin
 * button.
 *
 * `AdapTabOverview` provides search in open tabs. It searches in tab titles and
 * tooltips, as well as [property@TabPage:keyword].
 *
 * If [property@TabOverview:enable-new-tab] is set to `TRUE`, a new tab button
 * will be shown. Connect to the [signal@TabOverview::create-tab] signal to use
 * it.
 *
 * [property@TabOverview:secondary-menu] can be used to provide a secondary menu
 * for the overview. Use it to add extra actions, e.g. to open a new window or
 * undo closed tab.
 *
 * `AdapTabOverview` is intended to be used as the direct child of the window,
 * with the rest of the window contents set as the [property@TabOverview:child].
 * The child is expected to contain an [class@TabView].
 *
 * `AdapTabOverview` shows window buttons by default. They can be disabled by
 * setting [property@TabOverview:show-start-title-buttons] and/or
 * [property@TabOverview:show-start-title-buttons] and/or
 * [property@TabOverview:show-end-title-buttons] to `FALSE`.
 *
 * If search and window buttons are disabled, and secondary menu is not set, the
 * header bar will be hidden.
 *
 * ## Actions
 *
 * `AdapTabOverview` defines the `overview.open` and `overview.close` actions for
 * opening and closing itself. They can be convenient when used together with
 * [class@TabButton].
 *
 * ## CSS nodes
 *
 * `AdapTabOverview` has a single CSS node with name `taboverview`.
 *
 * Since: 1.3
 */

struct _AdapTabOverview
{
  GtkWidget parent_instance;

  GtkWidget *overview;
  GtkWidget *empty_state;
  GtkWidget *search_empty_state;
  GtkWidget *scrollable;
  GtkWidget *child_bin;
  GtkWidget *header_bar;
  GtkWidget *title;
  GtkWidget *new_tab_button;
  GtkWidget *search_button;
  GtkWidget *search_bar;
  GtkWidget *search_entry;
  GtkWidget *secondary_menu_button;
  AdapTabView *view;

  AdapTabGrid *grid;
  AdapTabGrid *pinned_grid;

  gboolean enable_search;
  gboolean enable_new_tab;
  gboolean search_active;

  gboolean is_open;
  AdapAnimation *open_animation;
  double progress;
  gboolean animating;

  AdapTabThumbnail *transition_thumbnail;
  GtkWidget *transition_picture;
  gboolean transition_pinned;

  GdkDragAction extra_drag_preferred_action;

  GtkWidget *last_focus;
};

static void adap_tab_overview_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapTabOverview, adap_tab_overview, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_tab_overview_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_VIEW,
  PROP_CHILD,
  PROP_OPEN,
  PROP_INVERTED,
  PROP_ENABLE_SEARCH,
  PROP_SEARCH_ACTIVE,
  PROP_ENABLE_NEW_TAB,
  PROP_SECONDARY_MENU,
  PROP_SHOW_START_TITLE_BUTTONS,
  PROP_SHOW_END_TITLE_BUTTONS,
  PROP_EXTRA_DRAG_PREFERRED_ACTION,
  PROP_EXTRA_DRAG_PRELOAD,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CREATE_TAB,
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_EXTRA_DRAG_VALUE,
  SIGNAL_LAST_SIGNAL,
};

typedef enum {
  ANIMATION_NONE,
  ANIMATION_IN,
  ANIMATION_OUT,
} AnimationDirection;

static guint signals[SIGNAL_LAST_SIGNAL];

#define ADAP_TYPE_TAB_OVERVIEW_SCROLLABLE (adap_tab_overview_scrollable_get_type ())

G_DECLARE_FINAL_TYPE (AdapTabOverviewScrollable, adap_tab_overview_scrollable, ADAP, TAB_OVERVIEW_SCROLLABLE, GtkWidget)

struct _AdapTabOverviewScrollable
{
  GtkWidget parent_instance;

  GtkWidget *grid;
  GtkWidget *pinned_grid;
  GtkWidget *overview;
  GtkWidget *new_button;
  GtkWidget *search_entry;

  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;
  GtkScrollablePolicy hscroll_policy;
  GtkScrollablePolicy vscroll_policy;

  AdapAnimation *scroll_animation;
  AdapTabGrid *scroll_animation_grid;
  gboolean scroll_animation_done;
  double scroll_animation_from;
  double scroll_animation_offset;
  gboolean block_scrolling;
  double adjustment_prev_value;

  int grid_pos;
  int pinned_grid_pos;

  gboolean hovering;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapTabOverviewScrollable, adap_tab_overview_scrollable, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

enum {
  SCROLLABLE_PROP_0,
  SCROLLABLE_PROP_GRID,
  SCROLLABLE_PROP_PINNED_GRID,
  SCROLLABLE_PROP_OVERVIEW,
  SCROLLABLE_PROP_NEW_BUTTON,
  /* GtkScrollable */
  SCROLLABLE_PROP_HADJUSTMENT,
  SCROLLABLE_PROP_VADJUSTMENT,
  SCROLLABLE_PROP_HSCROLL_POLICY,
  SCROLLABLE_PROP_VSCROLL_POLICY,
  LAST_SCROLLABLE_PROP = SCROLLABLE_PROP_HADJUSTMENT
};

static GParamSpec *scrollable_props[LAST_SCROLLABLE_PROP];

static void
vadjustment_value_changed_cb (AdapTabOverviewScrollable *self)
{
  double value = gtk_adjustment_get_value (self->vadjustment);

  adap_tab_grid_adjustment_shifted (ADAP_TAB_GRID (self->grid),
                                   value - self->adjustment_prev_value);

  self->adjustment_prev_value = value;

  if (self->block_scrolling)
    return;

  adap_animation_pause (self->scroll_animation);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
vadjustment_weak_notify (gpointer  data,
                         GObject  *object)
{
  AdapTabOverviewScrollable *self = data;

  self->vadjustment = NULL;
}

static void
set_vadjustment (AdapTabOverviewScrollable *self,
                 GtkAdjustment            *adjustment)
{
  if (self->vadjustment) {
    g_signal_handlers_disconnect_by_func (self->vadjustment, vadjustment_value_changed_cb, self);

    g_object_weak_unref (G_OBJECT (self->vadjustment), vadjustment_weak_notify, self);
  }

  self->vadjustment = adjustment;

  if (self->vadjustment) {
    g_object_weak_ref (G_OBJECT (self->vadjustment), vadjustment_weak_notify, self);

    g_signal_connect_swapped (self->vadjustment, "value-changed",
                              G_CALLBACK (vadjustment_value_changed_cb), self);
  }
}

static inline int
get_grid_offset (AdapTabOverviewScrollable *self,
                 AdapTabGrid               *grid)
{
  if (grid == ADAP_TAB_GRID (self->grid))
    return self->grid_pos;

  if (grid == ADAP_TAB_GRID (self->pinned_grid))
    return self->pinned_grid_pos;

  g_assert_not_reached ();
}

static double
get_scroll_animation_value (AdapTabOverviewScrollable *self,
                            double                    final_upper)
{
  double to, value;
  double scrolled_y;

  g_assert (self->scroll_animation);

  if (adap_animation_get_state (self->scroll_animation) != ADAP_ANIMATION_PLAYING &&
      adap_animation_get_state (self->scroll_animation) != ADAP_ANIMATION_FINISHED)
    return gtk_adjustment_get_value (self->vadjustment);

  to = self->scroll_animation_offset;

  scrolled_y = adap_tab_grid_get_scrolled_tab_y (self->scroll_animation_grid);
  if (!isnan (scrolled_y)) {
    double page_size = gtk_adjustment_get_page_size (self->vadjustment);

    to += scrolled_y + get_grid_offset (self, self->scroll_animation_grid);
    to = CLAMP (to, 0, final_upper - page_size);
  }

  value = adap_animation_get_value (self->scroll_animation);

  return round (adap_lerp (self->scroll_animation_from, to, value));
}

static void
scroll_animation_cb (double                    value,
                     AdapTabOverviewScrollable *self)
{
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
scroll_animation_done_cb (AdapTabOverviewScrollable *self)
{
  self->scroll_animation_done = TRUE;
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
stop_kinetic_scrolling (AdapTabOverviewScrollable *self)
{
  GtkWidget *window = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_SCROLLED_WINDOW);

  g_assert (window);

  /* HACK: Need to cancel kinetic scrolling. If only the built-in adjustment
   * animation API was public, we wouldn't have to do any of this... */
  gtk_scrolled_window_set_kinetic_scrolling (GTK_SCROLLED_WINDOW (window), FALSE);
  gtk_scrolled_window_set_kinetic_scrolling (GTK_SCROLLED_WINDOW (window), TRUE);
}

static void
animate_scroll (AdapTabOverviewScrollable *self,
                AdapTabGrid               *grid,
                double                    offset,
                guint                     duration)
{
  stop_kinetic_scrolling (self);

  self->scroll_animation_done = FALSE;
  self->scroll_animation_grid = grid;
  self->scroll_animation_from = gtk_adjustment_get_value (self->vadjustment);
  self->scroll_animation_offset = offset;

  adap_timed_animation_set_duration (ADAP_TIMED_ANIMATION (self->scroll_animation),
                                    duration);
  adap_animation_play (self->scroll_animation);
}

static void
scroll_relative_cb (AdapTabOverviewScrollable *self,
                    double                    delta,
                    guint                     duration,
                    AdapTabGrid               *grid)
{
  double current_value = gtk_adjustment_get_value (self->vadjustment);

  if (adap_animation_get_state (self->scroll_animation) == ADAP_ANIMATION_PLAYING) {
    double tab_y = adap_tab_grid_get_scrolled_tab_y (self->scroll_animation_grid);

    current_value = self->scroll_animation_offset;

    if (!isnan (tab_y))
      current_value += tab_y + get_grid_offset (self, self->scroll_animation_grid);
  }

  animate_scroll (self, grid, current_value + delta, duration);
}

static void
scroll_to_tab_cb (AdapTabOverviewScrollable *self,
                  double                    offset,
                  guint                     duration,
                  AdapTabGrid               *grid)
{
  animate_scroll (self, grid, offset, duration);
}

static void
set_grid (AdapTabOverviewScrollable  *self,
          GtkWidget                **field,
          AdapTabGrid                *grid)
{
  if (*field) {
    g_signal_handlers_disconnect_by_func (*field, scroll_relative_cb, self);
    g_signal_handlers_disconnect_by_func (*field, scroll_to_tab_cb, self);

    gtk_widget_unparent (*field);
  }

  *field = GTK_WIDGET (grid);

  if (*field) {
    gtk_widget_set_parent (*field, GTK_WIDGET (self));

    g_signal_connect_swapped (*field, "scroll-relative",
                              G_CALLBACK (scroll_relative_cb), self);
    g_signal_connect_swapped (*field, "scroll-to-tab",
                              G_CALLBACK (scroll_to_tab_cb), self);
  }
}

static void
set_hovering (AdapTabOverviewScrollable *self,
              gboolean                  hovering)
{
  self->hovering = hovering;

  adap_tab_grid_set_hovering (ADAP_TAB_GRID (self->grid), hovering);
  adap_tab_grid_set_hovering (ADAP_TAB_GRID (self->pinned_grid), hovering);
}

static void
motion_cb (AdapTabOverviewScrollable *self,
           double                    x,
           double                    y,
           GtkEventController       *controller)
{
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  if (input_source == GDK_SOURCE_TOUCHSCREEN)
    return;

  if (self->hovering)
    return;

  set_hovering (self, TRUE);
}

static void
leave_cb (AdapTabOverviewScrollable *self,
          GtkEventController       *controller)
{
  set_hovering (self, FALSE);
}

static void
adap_tab_overview_scrollable_unmap (GtkWidget *widget)
{
  AdapTabOverviewScrollable *self = ADAP_TAB_OVERVIEW_SCROLLABLE (widget);

  set_hovering (self, FALSE);

  GTK_WIDGET_CLASS (adap_tab_overview_scrollable_parent_class)->unmap (widget);
}

static void
adap_tab_overview_scrollable_measure (GtkWidget      *widget,
                                     GtkOrientation  orientation,
                                     int             for_size,
                                     int            *minimum,
                                     int            *natural,
                                     int            *minimum_baseline,
                                     int            *natural_baseline)
{
  int min = 0, nat = 0;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    int child_min, child_nat;

    gtk_widget_measure (child, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      min = MAX (min, child_min);
      nat = MAX (nat, child_nat);
    } else {
      min += child_min;
      nat += child_nat;
    }
  }

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adap_tab_overview_scrollable_size_allocate (GtkWidget *widget,
                                           int        width,
                                           int        height,
                                           int        baseline)
{
  AdapTabOverviewScrollable *self = ADAP_TAB_OVERVIEW_SCROLLABLE (widget);
  double value;
  int grid_height, pinned_height, new_button_height;
  int final_grid_height, final_pinned_height;

  gtk_widget_measure (self->grid, GTK_ORIENTATION_VERTICAL, width,
                      &grid_height, NULL, NULL, NULL);
  gtk_widget_measure (self->pinned_grid, GTK_ORIENTATION_VERTICAL, width,
                      &pinned_height, NULL, NULL, NULL);

  final_grid_height = adap_tab_grid_measure_height_final (ADAP_TAB_GRID (self->grid), width);
  final_pinned_height = adap_tab_grid_measure_height_final (ADAP_TAB_GRID (self->pinned_grid), width);

  if (gtk_widget_should_layout (self->new_button))
    gtk_widget_measure (self->new_button, GTK_ORIENTATION_VERTICAL, width,
                        &new_button_height, NULL, NULL, NULL);
  else
    new_button_height = 0;

  self->pinned_grid_pos = 0;
  self->grid_pos = self->pinned_grid_pos + pinned_height;

  grid_height = MAX (grid_height, height - new_button_height - self->grid_pos);

  value = get_scroll_animation_value (self,
                                      final_grid_height +
                                      final_pinned_height +
                                      new_button_height);

  self->block_scrolling = TRUE;
  gtk_adjustment_configure (self->vadjustment,
                            value,
                            0,
                            self->grid_pos + grid_height + new_button_height,
                            height * 0.1,
                            height * 0.9,
                            height);
  self->block_scrolling = FALSE;

  /* The value may have changed during gtk_adjustment_configure() */
  value = floor (gtk_adjustment_get_value (self->vadjustment));

  adap_tab_grid_set_visible_range (ADAP_TAB_GRID (self->pinned_grid),
                                  CLAMP (value - self->pinned_grid_pos, 0, pinned_height),
                                  CLAMP (value - self->pinned_grid_pos + height - new_button_height, 0, pinned_height),
                                  height - new_button_height,
                                  0,
                                  CLAMP (self->pinned_grid_pos + pinned_height - height + new_button_height - value, 0, new_button_height));
  adap_tab_grid_set_visible_range (ADAP_TAB_GRID (self->grid),
                                  CLAMP (value - self->grid_pos, 0, grid_height),
                                  CLAMP (value - self->grid_pos + height - new_button_height, 0, grid_height),
                                  height - new_button_height,
                                  0,
                                  CLAMP (self->grid_pos + grid_height - height + new_button_height - value, 0, new_button_height));

  if (self->scroll_animation_done) {
    g_clear_pointer (&self->scroll_animation_grid, adap_tab_grid_reset_scrolled_tab);
    self->scroll_animation_done = FALSE;
    adap_animation_reset (self->scroll_animation);
  }

  gtk_widget_allocate (self->pinned_grid, width, pinned_height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, self->pinned_grid_pos - value)));
  gtk_widget_allocate (self->grid, width, grid_height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, self->grid_pos - value)));
}

static void
adap_tab_overview_scrollable_dispose (GObject *object)
{
  AdapTabOverviewScrollable *self = ADAP_TAB_OVERVIEW_SCROLLABLE (object);

  g_clear_object (&self->scroll_animation);

  set_vadjustment (self, NULL);

  g_clear_pointer (&self->grid, gtk_widget_unparent);
  g_clear_pointer (&self->pinned_grid, gtk_widget_unparent);

  self->overview = NULL;
  self->new_button = NULL;

  G_OBJECT_CLASS (adap_tab_overview_scrollable_parent_class)->dispose (object);
}

static void
adap_tab_overview_scrollable_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  AdapTabOverviewScrollable *self = ADAP_TAB_OVERVIEW_SCROLLABLE (object);

  switch (prop_id) {
  case SCROLLABLE_PROP_GRID:
    g_value_set_object (value, self->grid);
    break;
  case SCROLLABLE_PROP_PINNED_GRID:
    g_value_set_object (value, self->pinned_grid);
    break;
  case SCROLLABLE_PROP_OVERVIEW:
    g_value_set_object (value, self->overview);
    break;
  case SCROLLABLE_PROP_NEW_BUTTON:
    g_value_set_object (value, self->new_button);
    break;
  case SCROLLABLE_PROP_HADJUSTMENT:
    g_value_set_object (value, self->hadjustment);
    break;
  case SCROLLABLE_PROP_VADJUSTMENT:
    g_value_set_object (value, self->vadjustment);
    break;
  case SCROLLABLE_PROP_HSCROLL_POLICY:
    g_value_set_enum (value, self->hscroll_policy);
    break;
  case SCROLLABLE_PROP_VSCROLL_POLICY:
    g_value_set_enum (value, self->vscroll_policy);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_overview_scrollable_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  AdapTabOverviewScrollable *self = ADAP_TAB_OVERVIEW_SCROLLABLE (object);

  switch (prop_id) {
  case SCROLLABLE_PROP_GRID:
    set_grid (self, &self->grid, g_value_get_object (value));
    break;
  case SCROLLABLE_PROP_PINNED_GRID:
    set_grid (self, &self->pinned_grid, g_value_get_object (value));
    break;
  case SCROLLABLE_PROP_OVERVIEW:
    self->overview = g_value_get_object (value);
    break;
  case SCROLLABLE_PROP_NEW_BUTTON:
    self->new_button = g_value_get_object (value);
    break;
  case SCROLLABLE_PROP_HADJUSTMENT:
    self->hadjustment = g_value_get_object (value);
    break;
  case SCROLLABLE_PROP_VADJUSTMENT:
    set_vadjustment (self, g_value_get_object (value));
    break;
  case SCROLLABLE_PROP_HSCROLL_POLICY:
    self->hscroll_policy = g_value_get_enum (value);
    break;
  case SCROLLABLE_PROP_VSCROLL_POLICY:
    self->vscroll_policy = g_value_get_enum (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_overview_scrollable_class_init (AdapTabOverviewScrollableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_tab_overview_scrollable_dispose;
  object_class->get_property = adap_tab_overview_scrollable_get_property;
  object_class->set_property = adap_tab_overview_scrollable_set_property;

  widget_class->unmap = adap_tab_overview_scrollable_unmap;
  widget_class->measure = adap_tab_overview_scrollable_measure;
  widget_class->size_allocate = adap_tab_overview_scrollable_size_allocate;

  scrollable_props[SCROLLABLE_PROP_GRID] =
    g_param_spec_object ("grid", NULL, NULL,
                         ADAP_TYPE_TAB_GRID,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  scrollable_props[SCROLLABLE_PROP_PINNED_GRID] =
    g_param_spec_object ("pinned-grid", NULL, NULL,
                         ADAP_TYPE_TAB_GRID,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  scrollable_props[SCROLLABLE_PROP_OVERVIEW] =
    g_param_spec_object ("overview", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  scrollable_props[SCROLLABLE_PROP_NEW_BUTTON] =
    g_param_spec_object ("new-button", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_SCROLLABLE_PROP, scrollable_props);

  g_object_class_override_property (object_class,
                                    SCROLLABLE_PROP_HADJUSTMENT,
                                    "hadjustment");
  g_object_class_override_property (object_class,
                                    SCROLLABLE_PROP_VADJUSTMENT,
                                    "vadjustment");
  g_object_class_override_property (object_class,
                                    SCROLLABLE_PROP_HSCROLL_POLICY,
                                    "hscroll-policy");
  g_object_class_override_property (object_class,
                                    SCROLLABLE_PROP_VSCROLL_POLICY,
                                    "vscroll-policy");
}

static void
adap_tab_overview_scrollable_init (AdapTabOverviewScrollable *self)
{
  GtkEventController *controller;
  AdapAnimationTarget *target;

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  controller = gtk_event_controller_motion_new ();
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (leave_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  /* The actual update will be done in size_allocate(). After the animation
   * finishes, don't remove it right away, it will be done in size-allocate as
   * well after one last update, so that we don't miss the last frame.
   */
  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc)
                                              scroll_animation_cb,
                                              self, NULL);
  self->scroll_animation =
    adap_timed_animation_new (GTK_WIDGET (self), 0, 1,
                             SCROLL_ANIMATION_DURATION, target);
  g_signal_connect_swapped (self->scroll_animation, "done",
                            G_CALLBACK (scroll_animation_done_cb), self);
}

static void
set_extra_drag_preferred_action (AdapTabOverview *self,
                                 GdkDragAction   preferred_action)
{
  self->extra_drag_preferred_action = preferred_action;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PREFERRED_ACTION]);
}


static gboolean
extra_drag_drop_cb (AdapTabOverview *self,
                    AdapTabPage     *page,
                    GValue         *value,
                    GdkDragAction   preferred_action)
{
  gboolean ret = GDK_EVENT_PROPAGATE;

  set_extra_drag_preferred_action (self, preferred_action);
  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, page, value, &ret);
  set_extra_drag_preferred_action (self, 0);

  return ret;
}

static GdkDragAction
extra_drag_value_cb (AdapTabOverview *self,
                     AdapTabPage     *page,
                     GValue         *value)
{
  GdkDragAction preferred_action;

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, page, value, &preferred_action);

  return preferred_action;
}

static GdkDragAction
extra_drag_value_notify (AdapTabOverview *self,
                         GValue         *value)
{
  return GDK_ACTION_ALL;
}

static void
empty_changed_cb (AdapTabOverview *self)
{
  gboolean empty =
    adap_tab_grid_get_empty (self->grid) &&
    adap_tab_grid_get_empty (self->pinned_grid);

  gtk_widget_set_visible (self->empty_state, empty && !self->search_active);
  gtk_widget_set_visible (self->search_empty_state, empty && self->search_active);
}

static void
update_actions (AdapTabOverview *self)
{
  gboolean has_view = self->view != NULL;
  gboolean has_pages = has_view && adap_tab_view_get_n_pages (self->view) > 0;

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "overview.open",
                                 !self->is_open && has_view);
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "overview.close",
                                 self->is_open && has_view && has_pages);
}

static void
update_header_bar (AdapTabOverview *self)
{
  gtk_widget_set_visible (self->header_bar,
                          self->enable_search ||
                          adap_tab_overview_get_secondary_menu (self) ||
                          adap_tab_overview_get_show_start_title_buttons (self) ||
                          adap_tab_overview_get_show_end_title_buttons (self));
}

static void
update_new_tab_button (AdapTabOverview *self)
{
  gtk_widget_set_visible (self->new_tab_button,
                          self->enable_new_tab && !self->search_active);
  gtk_widget_queue_resize (self->scrollable);
}

static void
set_search_active (AdapTabOverview *self,
                   gboolean        search_active)
{
  if (search_active == self->search_active)
    return;

  self->search_active = search_active;

  update_new_tab_button (self);
  empty_changed_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SEARCH_ACTIVE]);
}

static void
search_changed_cb (AdapTabOverview *self)
{
  const char *text = gtk_editable_get_text (GTK_EDITABLE (self->search_entry));

  adap_tab_grid_set_search_terms (self->grid, text);
  adap_tab_grid_set_search_terms (self->pinned_grid, text);

  set_search_active (self, text && *text);
}

static void
stop_search_cb (AdapTabOverview *self)
{
  gtk_editable_set_text (GTK_EDITABLE (self->search_entry), "");

  adap_tab_grid_set_search_terms (self->grid, "");
  adap_tab_grid_set_search_terms (self->pinned_grid, "");

  set_search_active (self, FALSE);
}

static AdapTabPage *
create_tab (AdapTabOverview *self)
{
  AdapTabPage *new_page = NULL;

  g_signal_emit (self, signals[SIGNAL_CREATE_TAB], 0, &new_page);

  if (!new_page) {
    g_critical ("AdapTabOverview::create-tab handler must not return NULL");

    return NULL;
  }

  return new_page;
}

static void
new_tab_clicked_cb (AdapTabOverview *self)
{
  AdapTabPage *new_page = create_tab (self);
  GtkWidget *child;

  if (!new_page)
    return;

  child = adap_tab_page_get_child (new_page);

  adap_tab_view_set_selected_page (self->view, new_page);
  adap_tab_overview_set_open (self, FALSE);

  gtk_widget_grab_focus (child);
}

static void
view_destroy_cb (AdapTabOverview *self)
{
  adap_tab_overview_set_view (self, NULL);
}

static void
notify_selected_page_cb (AdapTabOverview *self)
{
  AdapTabPage *page = adap_tab_view_get_selected_page (self->view);

  if (!page)
    return;

  if (adap_tab_page_get_pinned (page)) {
    adap_tab_grid_select_page (self->pinned_grid, page);
    adap_tab_grid_select_page (self->grid, page);
  } else {
    adap_tab_grid_select_page (self->grid, page);
    adap_tab_grid_select_page (self->pinned_grid, page);
  }
}

static void
notify_n_pages_cb (AdapTabOverview *self)
{
  guint n_pages;
  char *title_str;

  if (!self->view) {
    adap_window_title_set_title (ADAP_WINDOW_TITLE (self->title), "");
    return;
  }

  n_pages = adap_tab_view_get_n_pages (self->view);

  /* Translators: Tab overview title, %u is the number of open tabs */
  title_str = g_strdup_printf (dngettext (GETTEXT_PACKAGE, "%u Tab", "%u Tabs", n_pages), n_pages);

  adap_window_title_set_title (ADAP_WINDOW_TITLE (self->title), title_str);

  g_free (title_str);
}

static void
notify_pinned_cb (AdapTabPage     *page,
                  GParamSpec     *pspec,
                  AdapTabOverview *self)
{
  AdapTabGrid *from, *to;

  if (adap_tab_page_get_pinned (page)) {
    from = self->grid;
    to = self->pinned_grid;
  } else {
    from = self->pinned_grid;
    to = self->grid;
  }

  adap_tab_grid_detach_page (from, page);
  adap_tab_grid_attach_page (to, page, adap_tab_view_get_n_pinned_pages (self->view));

  adap_tab_grid_scroll_to_page (to, page, TRUE);

  adap_tab_grid_focus_page (to, page);
}

static void
page_attached_cb (AdapTabOverview *self,
                  AdapTabPage     *page,
                  int             position)
{
  g_signal_connect_object (page, "notify::pinned",
                           G_CALLBACK (notify_pinned_cb), self,
                           0);
  update_actions (self);
}

static void
page_detached_cb (AdapTabOverview *self,
                  AdapTabPage     *page,
                  int             position)
{
  g_signal_handlers_disconnect_by_func (page, notify_pinned_cb, self);
  update_actions (self);
}

static void
open_animation_value_cb (double          value,
                         AdapTabOverview *self)
{
  self->progress = value;
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
set_overview_visible (AdapTabOverview     *self,
                      gboolean            visible,
                      AnimationDirection  direction)
{
  gboolean animating = direction != ANIMATION_NONE;

  gtk_widget_set_child_visible (self->overview, visible || animating);
  gtk_widget_set_can_target (self->overview, visible);
  gtk_widget_set_can_focus (self->overview, visible);
  gtk_widget_set_can_target (self->child_bin, !visible && !animating);
  gtk_widget_set_can_focus (self->child_bin, !visible && direction != ANIMATION_IN);

  if (visible || animating)
    gtk_widget_add_css_class (self->child_bin, "background");
  else
    gtk_widget_remove_css_class (self->child_bin, "background");
}

static void
open_animation_done_cb (AdapTabOverview *self)
{
  if (self->transition_picture) {
    g_clear_object (&self->transition_picture);

    adap_tab_thumbnail_fade_in (self->transition_thumbnail);
    self->transition_thumbnail = NULL;
  }

  set_overview_visible (self, self->is_open, ANIMATION_NONE);

  if (!self->is_open) {
    adap_tab_view_close_overview (self->view);

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (self->search_bar), FALSE);

    if (self->last_focus) {
      gtk_widget_grab_focus (self->last_focus);

      g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                    (gpointer *) &self->last_focus);
      self->last_focus = NULL;
    }
  }

  self->animating = FALSE;
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static inline double
inverse_lerp (double a,
              double b,
              double t)
{
  return (t - a) / (b - a);
}

static void
calculate_bounds (AdapTabOverview  *self,
                  graphene_rect_t *bounds,
                  graphene_rect_t *transition_bounds,
                  graphene_rect_t *clip_bounds,
                  graphene_size_t *clip_scale)
{
  GtkWidget *widget = GTK_WIDGET (self);
  graphene_rect_t view_bounds, thumbnail_bounds;
  double view_ratio, thumb_ratio;
  AdapTabPage *page = adap_tab_view_get_selected_page (self->view);

  if (!gtk_widget_compute_bounds (GTK_WIDGET (self->view), widget, &view_bounds))
    g_error ("AdapTabView %p must be inside its AdapTabOverview %p", self->view, self);

  if (!gtk_widget_compute_bounds (self->transition_picture, widget, &thumbnail_bounds))
    graphene_rect_init (&thumbnail_bounds, 0, 0, 0, 0);

  graphene_rect_init (bounds, 0, 0,
                      gtk_widget_get_width (widget),
                      gtk_widget_get_height (widget));

  view_ratio = view_bounds.size.width / view_bounds.size.height;
  thumb_ratio = thumbnail_bounds.size.width / thumbnail_bounds.size.height;

  if (view_ratio > thumb_ratio) {
    double new_width = view_bounds.size.height * thumb_ratio;
    double xalign = adap_tab_page_get_thumbnail_xalign (page);

    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
      xalign = 1 - xalign;

    view_bounds.origin.x += (float) (view_bounds.size.width - new_width) * xalign;
    view_bounds.size.width = new_width;
  } else if (view_ratio < thumb_ratio) {
    double new_height = view_bounds.size.width / thumb_ratio;
    double yalign = adap_tab_page_get_thumbnail_yalign (page);

    view_bounds.origin.y += (float) (view_bounds.size.height - new_height) * yalign;
    view_bounds.size.height = new_height;
  }

  graphene_rect_interpolate (bounds, &view_bounds,
                             self->progress, clip_bounds);

  graphene_size_init (clip_scale,
                      adap_lerp (1, thumbnail_bounds.size.width / view_bounds.size.width, self->progress),
                      adap_lerp (1, thumbnail_bounds.size.height / view_bounds.size.height, self->progress));

  graphene_size_init (&transition_bounds->size,
                      clip_bounds->size.width * clip_scale->width,
                      clip_bounds->size.height * clip_scale->height);
  graphene_point_init (&transition_bounds->origin,
                       adap_lerp (0, thumbnail_bounds.origin.x,
                                 inverse_lerp (bounds->size.width,
                                               thumbnail_bounds.size.width,
                                               transition_bounds->size.width)),
                       adap_lerp (0, thumbnail_bounds.origin.y,
                                 inverse_lerp (bounds->size.height,
                                               thumbnail_bounds.size.height,
                                               transition_bounds->size.height)));
}

static void
should_round_corners (AdapTabOverview *self,
                      gboolean       *round_top_left,
                      gboolean       *round_top_right,
                      gboolean       *round_bottom_left,
                      gboolean       *round_bottom_right)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  graphene_rect_t bounds;
  GdkSurface *surface;
  GdkToplevelState state;
  gboolean top_left = TRUE;
  gboolean top_right = TRUE;
  gboolean bottom_left = TRUE;
  gboolean bottom_right = TRUE;

  *round_top_left = FALSE;
  *round_top_right = FALSE;
  *round_bottom_left = FALSE;
  *round_bottom_right = FALSE;

  if (!root || !GTK_IS_WINDOW (root) || !gtk_widget_get_realized (GTK_WIDGET (root)))
    return;

  surface = gtk_native_get_surface (GTK_NATIVE (root));
  state = gdk_toplevel_get_state (GDK_TOPLEVEL (surface));

  if ((state & (GDK_TOPLEVEL_STATE_FULLSCREEN |
                GDK_TOPLEVEL_STATE_MAXIMIZED |
                GDK_TOPLEVEL_STATE_TILED |
                GDK_TOPLEVEL_STATE_TOP_TILED |
                GDK_TOPLEVEL_STATE_RIGHT_TILED |
                GDK_TOPLEVEL_STATE_BOTTOM_TILED |
                GDK_TOPLEVEL_STATE_LEFT_TILED)) > 0)
    return;

  if (!gtk_widget_has_css_class (GTK_WIDGET (root), "csd") ||
      gtk_widget_has_css_class (GTK_WIDGET (root), "solid-csd"))
    return;

  if (!gtk_widget_compute_bounds (GTK_WIDGET (self->child_bin), GTK_WIDGET (root), &bounds))
    return;

  if (bounds.origin.x > 0) {
    top_left = FALSE;
    bottom_left = FALSE;
  }

  if (bounds.origin.x + bounds.size.width < gtk_widget_get_width (GTK_WIDGET (root))) {
    top_right = FALSE;
    bottom_right = FALSE;
  }

  if (bounds.origin.y > 0) {
    top_left = FALSE;
    top_right = FALSE;
  }

  if (bounds.origin.y + bounds.size.height < gtk_widget_get_height (GTK_WIDGET (root))) {
    bottom_left = FALSE;
    bottom_right = FALSE;
  }

  *round_top_left = top_left;
  *round_top_right = top_right;
  *round_bottom_left = bottom_left;
  *round_bottom_right = bottom_right;
}

static void
adap_tab_overview_snapshot (GtkWidget   *widget,
                           GtkSnapshot *snapshot)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (widget);
  graphene_rect_t bounds, transition_bounds, clip_bounds;
  graphene_size_t clip_scale, corner_size, window_corner_size;
  GskRoundedRect transition_rect;
  gboolean round_top_left, round_top_right;
  gboolean round_bottom_left, round_bottom_right;
  GdkRGBA rgba;
  GdkDisplay *display;
  AdapStyleManager *style_manager;
  gboolean hc;

  if (!self->animating) {
    if (self->is_open) {
      GtkSnapshot *child_snapshot;

      gtk_widget_snapshot_child (widget, self->overview, snapshot);

      /* We don't want to actually draw the child, but we do need it
       * to redraw so that it can be displayed by the paintables */
      child_snapshot = gtk_snapshot_new ();

      gtk_widget_snapshot_child (widget, self->child_bin, child_snapshot);

      g_object_unref (child_snapshot);
    } else {
      gtk_widget_snapshot_child (widget, self->child_bin, snapshot);
    }

    return;
  }

  calculate_bounds (self, &bounds, &transition_bounds, &clip_bounds, &clip_scale);
  should_round_corners (self, &round_top_left, &round_top_right,
                        &round_bottom_left, &round_bottom_right);

  graphene_size_init (&corner_size,
                      adap_lerp (0, THUMBNAIL_BORDER_RADIUS, self->progress),
                      adap_lerp (0, THUMBNAIL_BORDER_RADIUS, self->progress));

  graphene_size_init (&window_corner_size,
                      adap_lerp (WINDOW_BORDER_RADIUS,
                                THUMBNAIL_BORDER_RADIUS, self->progress),
                      adap_lerp (WINDOW_BORDER_RADIUS,
                                THUMBNAIL_BORDER_RADIUS, self->progress));

  gsk_rounded_rect_init (&transition_rect, &transition_bounds,
                         round_top_left     ? &window_corner_size : &corner_size,
                         round_top_right    ? &window_corner_size : &corner_size,
                         round_bottom_right ? &window_corner_size : &corner_size,
                         round_bottom_left  ? &window_corner_size : &corner_size);

  display = gtk_widget_get_display (widget);
  style_manager = adap_style_manager_get_for_display (display);
  hc = adap_style_manager_get_high_contrast (style_manager);

  /* Draw overview */
  gtk_widget_snapshot_child (widget, self->overview, snapshot);

  /* Draw dim layer */
  if (!adap_widget_lookup_color (widget, "shade_color", &rgba))
    rgba.alpha = 0;

  rgba.alpha *= 1 - self->progress;

  gtk_snapshot_append_color (snapshot, &rgba, &bounds);

  /* Draw the transition thumbnail. Unfortunately, since GTK widgets have
   * integer sizes, we can't use a real widget for this and have to custom
   * draw it instead. We also want to interpolate border-radius. */
  gtk_snapshot_push_rounded_clip (snapshot, &transition_rect);

  if (self->transition_pinned)
    gtk_snapshot_push_cross_fade (snapshot, adap_easing_ease (ADAP_EASE_IN_EXPO, self->progress));

  gtk_snapshot_translate (snapshot, &transition_bounds.origin);
  gtk_snapshot_scale (snapshot, clip_scale.width, clip_scale.height);
  gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (-clip_bounds.origin.x,
                                                          -clip_bounds.origin.y));
  gtk_widget_snapshot_child (widget, self->child_bin, snapshot);

  if (self->transition_pinned) {
    if (!adap_widget_lookup_color (self->transition_picture,
                                  "thumbnail_bg_color", &rgba))
      rgba.red = rgba.green = rgba.blue = rgba.alpha = 1;

    gtk_snapshot_pop (snapshot);
    gtk_snapshot_append_color (snapshot, &rgba, &bounds);
    gtk_snapshot_pop (snapshot);
  }

  gtk_snapshot_pop (snapshot);

  /* Draw outer outline */
  if (hc) {
    rgba.red = rgba.green = rgba.blue = 0;
    rgba.alpha = 0.5;
  } else {
    if (!adap_widget_lookup_color (widget, "shade_color", &rgba))
      rgba.alpha = 0;
  }

  rgba.alpha *= adap_easing_ease (ADAP_EASE_OUT_EXPO, self->progress);

  gtk_snapshot_append_outset_shadow (snapshot, &transition_rect,
                                     &rgba, 0, 0, 1, 0);

  /* Draw inner outline */
  if (!self->transition_pinned || hc) {
    /* Keep in sync with $window_outline_color */
    rgba.red = rgba.green = rgba.blue = 1;
    rgba.alpha = hc ? 0.3 : 0.07;

    rgba.alpha *= adap_easing_ease (ADAP_EASE_OUT_EXPO, self->progress);

    gtk_snapshot_append_inset_shadow (snapshot, &transition_rect,
                                      &rgba, 0, 0, 1, 0);
  }
}

static gboolean
adap_tab_overview_focus (GtkWidget        *widget,
                        GtkDirectionType  direction)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (widget);
  GtkWidget *focus;

  if (!self->is_open)
    return GTK_WIDGET_CLASS (adap_tab_overview_parent_class)->focus (widget, direction);

  focus = gtk_root_get_focus (gtk_widget_get_root (widget));
  if (!focus)
    return GTK_WIDGET_CLASS (adap_tab_overview_parent_class)->focus (widget, direction);

  if (direction != GTK_DIR_UP && direction != GTK_DIR_DOWN)
    return GTK_WIDGET_CLASS (adap_tab_overview_parent_class)->focus (widget, direction);

  if (direction == GTK_DIR_DOWN) {
    if ((focus == self->search_button ||
         gtk_widget_is_ancestor (focus, self->search_button)) &&
        !gtk_search_bar_get_search_mode (GTK_SEARCH_BAR (self->search_bar))) {
      return adap_tab_grid_focus_first_row (self->pinned_grid, 0) ||
             adap_tab_grid_focus_first_row (self->grid, 0);
    }

    if ((focus == self->secondary_menu_button ||
         gtk_widget_is_ancestor (focus, self->secondary_menu_button)) &&
        !gtk_search_bar_get_search_mode (GTK_SEARCH_BAR (self->search_bar))) {
      return adap_tab_grid_focus_first_row (self->pinned_grid, -1) ||
             adap_tab_grid_focus_first_row (self->grid, -1);
    }

    if ((focus == self->search_bar ||
         gtk_widget_is_ancestor (focus, self->search_bar))) {
      return adap_tab_grid_focus_first_row (self->pinned_grid, 0) ||
             adap_tab_grid_focus_first_row (self->grid, 0);
    }

    if ((focus == self->new_tab_button ||
         gtk_widget_is_ancestor (focus, self->new_tab_button)))
      return GDK_EVENT_PROPAGATE;

    if (gtk_widget_is_ancestor (focus, GTK_WIDGET (self->grid))) {
      return gtk_widget_child_focus (GTK_WIDGET (self->grid), direction) ||
             gtk_widget_grab_focus (self->new_tab_button);
    }

    if (gtk_widget_is_ancestor (focus, GTK_WIDGET (self->pinned_grid)) &&
        adap_tab_grid_get_empty (self->grid)) {
      return gtk_widget_child_focus (GTK_WIDGET (self->pinned_grid), direction) ||
             gtk_widget_grab_focus (self->new_tab_button);
    }
  }

  if (direction == GTK_DIR_UP &&
      (focus == self->new_tab_button ||
       gtk_widget_is_ancestor (focus, self->new_tab_button))) {
    return adap_tab_grid_focus_last_row (self->grid, -1) ||
           adap_tab_grid_focus_last_row (self->pinned_grid, -1);
  }

  return adap_widget_focus_child (widget, direction);
}

static void
adap_tab_overview_dispose (GObject *object)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (object);

  if (self->last_focus) {
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);
    self->last_focus = NULL;
  }

  adap_tab_overview_set_view (self, NULL);

  g_clear_object (&self->open_animation);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADAP_TYPE_TAB_OVERVIEW);

  G_OBJECT_CLASS (adap_tab_overview_parent_class)->dispose (object);
}

static void
adap_tab_overview_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, adap_tab_overview_get_view (self));
    break;
  case PROP_CHILD:
    g_value_set_object (value, adap_tab_overview_get_child (self));
    break;
  case PROP_OPEN:
    g_value_set_boolean (value, adap_tab_overview_get_open (self));
    break;
  case PROP_INVERTED:
    g_value_set_boolean (value, adap_tab_overview_get_inverted (self));
    break;
  case PROP_ENABLE_SEARCH:
    g_value_set_boolean (value, adap_tab_overview_get_enable_search (self));
    break;
  case PROP_SEARCH_ACTIVE:
    g_value_set_boolean (value, adap_tab_overview_get_search_active (self));
    break;
  case PROP_ENABLE_NEW_TAB:
    g_value_set_boolean (value, adap_tab_overview_get_enable_new_tab (self));
    break;
  case PROP_SECONDARY_MENU:
    g_value_set_object (value, adap_tab_overview_get_secondary_menu (self));
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    g_value_set_boolean (value, adap_tab_overview_get_show_start_title_buttons (self));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    g_value_set_boolean (value, adap_tab_overview_get_show_end_title_buttons (self));
    break;
  case PROP_EXTRA_DRAG_PREFERRED_ACTION:
    g_value_set_flags (value, adap_tab_overview_get_extra_drag_preferred_action (self));
    break;
  case PROP_EXTRA_DRAG_PRELOAD:
    g_value_set_boolean (value, adap_tab_overview_get_extra_drag_preload (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_overview_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (object);

  switch (prop_id) {
  case PROP_VIEW:
    adap_tab_overview_set_view (self, g_value_get_object (value));
    break;
  case PROP_CHILD:
    adap_tab_overview_set_child (self, g_value_get_object (value));
    break;
  case PROP_OPEN:
    adap_tab_overview_set_open (self, g_value_get_boolean (value));
    break;
  case PROP_INVERTED:
    adap_tab_overview_set_inverted (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_SEARCH:
    adap_tab_overview_set_enable_search (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_NEW_TAB:
    adap_tab_overview_set_enable_new_tab (self, g_value_get_boolean (value));
    break;
  case PROP_SECONDARY_MENU:
    adap_tab_overview_set_secondary_menu (self, g_value_get_object (value));
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    adap_tab_overview_set_show_start_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    adap_tab_overview_set_show_end_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_EXTRA_DRAG_PRELOAD:
    adap_tab_overview_set_extra_drag_preload (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
overview_open_cb (AdapTabOverview *self)
{
  adap_tab_overview_set_open (self, TRUE);
}

static void
overview_close_cb (AdapTabOverview *self)
{
  adap_tab_overview_set_open (self, FALSE);
}

static gboolean
escape_cb (AdapTabOverview *self)
{
  if (!self->is_open)
    return GDK_EVENT_PROPAGATE;

  if (gtk_search_bar_get_search_mode (GTK_SEARCH_BAR (self->search_bar))) {
    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (self->search_bar), FALSE);
    return GDK_EVENT_STOP;
  }

  if (!adap_tab_view_get_n_pages (self->view)) {
    AdapTabPage *page = create_tab (self);

    if (!page)
      return GDK_EVENT_PROPAGATE;
  }

  adap_tab_overview_set_open (self, FALSE);

  return GDK_EVENT_STOP;
}

static gboolean
object_handled_accumulator (GSignalInvocationHint *ihint,
                            GValue                *return_accu,
                            const GValue          *handler_return,
                            gpointer               data)
{
  GObject *object = g_value_get_object (handler_return);

  g_value_set_object (return_accu, object);

  return !object;
}

static void
adap_tab_overview_class_init (AdapTabOverviewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_tab_overview_dispose;
  object_class->get_property = adap_tab_overview_get_property;
  object_class->set_property = adap_tab_overview_set_property;

  widget_class->snapshot = adap_tab_overview_snapshot;
  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->focus = adap_tab_overview_focus;

  /**
   * AdapTabOverview:view: (attributes org.gtk.Property.get=adap_tab_overview_get_view org.gtk.Property.set=adap_tab_overview_set_view)
   *
   * The tab view the overview controls.
   *
   * The view must be inside the tab overview, see [property@TabOverview:child].
   *
   * Since: 1.3
   */
  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         ADAP_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:child: (attributes org.gtk.Property.get=adap_tab_overview_get_child org.gtk.Property.set=adap_tab_overview_set_child)
   *
   * The child widget.
   *
   * Since: 1.3
   */
  props[PROP_CHILD] =
      g_param_spec_object ("child", NULL, NULL,
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:open: (attributes org.gtk.Property.get=adap_tab_overview_get_open org.gtk.Property.set=adap_tab_overview_set_open)
   *
   * Whether the overview is open.
   *
   * Since: 1.3
   */
  props[PROP_OPEN] =
    g_param_spec_boolean ("open", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:inverted: (attributes org.gtk.Property.get=adap_tab_overview_get_inverted org.gtk.Property.set=adap_tab_overview_set_inverted)
   *
   * Whether thumbnails use inverted layout.
   *
   * If set to `TRUE`, thumbnails will have the close or unpin buttons at the
   * beginning and the indicator at the end rather than the other way around.
   *
   * Since: 1.3
   */
  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:enable-search: (attributes org.gtk.Property.get=adap_tab_overview_get_enable_search org.gtk.Property.set=adap_tab_overview_set_enable_search)
   *
   * Whether to enable search in tabs.
   *
   * Search matches tab titles and tooltips, as well as keywords, set via
   * [property@TabPage:keyword]. Use keywords to search in e.g. page URLs in a
   * web browser.
   *
   * During search, tab reordering and drag-n-drop are disabled.
   *
   * Use [property@TabOverview:search-active] to check out if search is
   * currently active.
   *
   * Since: 1.3
   */
  props[PROP_ENABLE_SEARCH] =
    g_param_spec_boolean ("enable-search", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:search-active: (attributes org.gtk.Property.get=adap_tab_overview_get_search_active)
   *
   * Whether search is currently active.
   *
   * See [property@TabOverview:enable-search].
   *
   * Since: 1.3
   */
  props[PROP_SEARCH_ACTIVE] =
    g_param_spec_boolean ("search-active", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdapTabOverview:enable-new-tab: (attributes org.gtk.Property.get=adap_tab_overview_get_enable_new_tab org.gtk.Property.set=adap_tab_overview_set_enable_new_tab)
   *
   * Whether to enable new tab button.
   *
   * Connect to the [signal@TabOverview::create-tab] signal to use it.
   *
   * Since: 1.3
   */
  props[PROP_ENABLE_NEW_TAB] =
    g_param_spec_boolean ("enable-new-tab", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:secondary-menu: (attributes org.gtk.Property.get=adap_tab_overview_get_secondary_menu org.gtk.Property.set=adap_tab_overview_set_secondary_menu)
   *
   * The secondary menu model.
   *
   * Use it to add extra actions, e.g. to open a new window or undo closed tab.
   *
   * Since: 1.3
   */
  props[PROP_SECONDARY_MENU] =
    g_param_spec_object ("secondary-menu", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:show-start-title-buttons: (attributes org.gtk.Property.get=adap_tab_overview_get_show_start_title_buttons org.gtk.Property.set=adap_tab_overview_set_show_start_title_buttons)
   *
   * Whether to show start title buttons in the overview's header bar.
   *
   * See [property@HeaderBar:show-end-title-buttons] for the other side.
   *
   * Since: 1.3
   */
  props[PROP_SHOW_START_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-start-title-buttons", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:show-end-title-buttons: (attributes org.gtk.Property.get=adap_tab_overview_get_show_end_title_buttons org.gtk.Property.set=adap_tab_overview_set_show_end_title_buttons)
   *
   * Whether to show end title buttons in the overview's header bar.
   *
   * See [property@HeaderBar:show-start-title-buttons] for the other side.
   *
   * Since: 1.3
   */
  props[PROP_SHOW_END_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-end-title-buttons", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:extra-drag-preferred-action: (attributes org.gtk.Property.get=adap_tab_overview_get_extra_drag_preferred_action)
   *
   * The unique action on the `current-drop` of the
   * [signal@TabOverview::extra-drag-drop].
   *
   * This property should only be used during a
   * [signal@TabOverview::extra-drag-drop] and is always a subset of what was
   * originally passed to [method@TabOverview.setup_extra_drop_target].
   *
   * Since: 1.4
   */
  props[PROP_EXTRA_DRAG_PREFERRED_ACTION] =
    g_param_spec_flags ("extra-drag-preferred-action", NULL, NULL,
                       GDK_TYPE_DRAG_ACTION, 0,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapTabOverview:extra-drag-preload: (attributes org.gtk.Property.get=adap_tab_overview_get_extra_drag_preload org.gtk.Property.set=adap_tab_overview_set_extra_drag_preload)
   *
   * Whether the drop data should be preloaded on hover.
   *
   * See [property@Gtk.DropTarget:preload].
   *
   * Since: 1.3
   */
  props[PROP_EXTRA_DRAG_PRELOAD] =
    g_param_spec_boolean ("extra-drag-preload", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdapTabOverview::create-tab:
   * @self: a tab overview
   *
   * Emitted when a tab needs to be created.
   *
   * This can happen after the new tab button has been pressed, see
   * [property@TabOverview:enable-new-tab].
   *
   * The signal handler is expected to create a new page in the corresponding
   * [class@TabView] and return it.
   *
   * Returns: (transfer none): the newly created page
   *
   * Since: 1.3
   */
  signals[SIGNAL_CREATE_TAB] =
    g_signal_new ("create-tab",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  object_handled_accumulator,
                  NULL,
                  adap_marshal_OBJECT__VOID,
                  ADAP_TYPE_TAB_PAGE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CREATE_TAB],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_OBJECT__VOIDv);

  /**
   * AdapTabOverview::extra-drag-drop:
   * @self: a tab overview
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when content is dropped onto a tab.
   *
   * The content must be of one of the types set up via
   * [method@TabOverview.setup_extra_drop_target].
   *
   * See [signal@Gtk.DropTarget::drop].
   *
   * Returns: whether the drop was accepted for @page
   *
   * Since: 1.3
   */
  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  G_TYPE_BOOLEAN,
                  2,
                  ADAP_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  /**
   * AdapTabOverview::extra-drag-value:
   * @self: a tab overview
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when the dropped content is preloaded.
   *
   * In order for data to be preloaded, [property@TabOverview:extra-drag-preload]
   * must be set to `TRUE`.
   *
   * The content must be of one of the types set up via
   * [method@TabOverview.setup_extra_drop_target].
   *
   * See [property@Gtk.DropTarget:value].
   *
   * Returns: the preferred action for the drop on @page
   *
   * Since: 1.3
   */
  signals[SIGNAL_EXTRA_DRAG_VALUE] =
    g_signal_new ("extra-drag-value",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  GDK_TYPE_DRAG_ACTION,
                  2,
                  ADAP_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  gtk_widget_class_install_action (widget_class, "overview.open", NULL,
                                   (GtkWidgetActionActivateFunc) overview_open_cb);
  gtk_widget_class_install_action (widget_class, "overview.close", NULL,
                                   (GtkWidgetActionActivateFunc) overview_close_cb);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0,
                                (GtkShortcutFunc) escape_cb, NULL);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-tab-overview.ui");

  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, overview);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, empty_state);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, search_empty_state);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, scrollable);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, child_bin);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, header_bar);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, title);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, new_tab_button);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, search_button);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, search_bar);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, search_entry);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, secondary_menu_button);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, grid);
  gtk_widget_class_bind_template_child (widget_class, AdapTabOverview, pinned_grid);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_drop_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_value_cb);
  gtk_widget_class_bind_template_callback (widget_class, empty_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_search_cb);
  gtk_widget_class_bind_template_callback (widget_class, new_tab_clicked_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "taboverview");

  g_signal_override_class_handler ("extra-drag-value", G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (extra_drag_value_notify));

  g_type_ensure (ADAP_TYPE_TAB_GRID);
  g_type_ensure (ADAP_TYPE_TAB_OVERVIEW_SCROLLABLE);
}

static void
adap_tab_overview_init (AdapTabOverview *self)
{
  AdapAnimationTarget *target;

  self->enable_search = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_child_visible (self->overview, FALSE);

  gtk_search_bar_connect_entry (GTK_SEARCH_BAR (self->search_bar),
                                GTK_EDITABLE (self->search_entry));

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc) open_animation_value_cb,
                                              self, NULL);

  self->open_animation =
    adap_timed_animation_new (GTK_WIDGET (self),
                             0, 0,
                             TRANSITION_DURATION,
                             target);

  g_signal_connect_swapped (self->open_animation, "done",
                            G_CALLBACK (open_animation_done_cb), self);
}

static void
adap_tab_overview_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  AdapTabOverview *self = ADAP_TAB_OVERVIEW (buildable);

  if (!self->overview)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (GTK_IS_WIDGET (child))
    adap_tab_overview_set_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_tab_overview_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_tab_overview_buildable_add_child;
}

/**
 * adap_tab_overview_new:
 *
 * Creates a new `AdapTabOverview`.
 *
 * Returns: the newly created `AdapTabOverview`
 *
 * Since: 1.3
 */
GtkWidget *
adap_tab_overview_new (void)
{
  return g_object_new (ADAP_TYPE_TAB_OVERVIEW, NULL);
}

/**
 * adap_tab_overview_get_view: (attributes org.gtk.Method.get_property=view)
 * @self: a tab overview
 *
 * Gets the tab view @self controls.
 *
 * Returns: (transfer none) (nullable): the tab view
 *
 * Since: 1.3
 */
AdapTabView *
adap_tab_overview_get_view (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), NULL);

  return self->view;
}

/**
 * adap_tab_overview_set_view: (attributes org.gtk.Method.set_property=view)
 * @self: a tab overview
 * @view: (nullable): a tab view
 *
 * Sets the tab view to control.
 *
 * The view must be inside @self, see [property@TabOverview:child].
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_view (AdapTabOverview *self,
                           AdapTabView     *view)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));
  g_return_if_fail (view == NULL || ADAP_IS_TAB_VIEW (view));

  if (self->view == view)
    return;

  if (self->view) {
    int i, n;

    g_signal_handlers_disconnect_by_func (self->view, notify_selected_page_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, notify_n_pages_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_attached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_detached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, view_destroy_cb, self);

    n = adap_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_detached_cb (self, adap_tab_view_get_nth_page (self->view, i), i);

    adap_tab_grid_set_view (self->grid, NULL);
    adap_tab_grid_set_view (self->pinned_grid, NULL);

    notify_n_pages_cb (self);
  }

  g_set_object (&self->view, view);

  if (self->view) {
    int i, n;

    adap_tab_grid_set_view (self->grid, view);
    adap_tab_grid_set_view (self->pinned_grid, view);

    g_signal_connect_object (self->view, "notify::selected-page",
                             G_CALLBACK (notify_selected_page_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::n-pages",
                             G_CALLBACK (notify_n_pages_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-attached",
                             G_CALLBACK (page_attached_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-detached",
                             G_CALLBACK (page_detached_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "destroy",
                             G_CALLBACK (view_destroy_cb), self,
                             G_CONNECT_SWAPPED);

    n = adap_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_attached_cb (self, adap_tab_view_get_nth_page (self->view, i), i);

    notify_n_pages_cb (self);
  }

  update_actions (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}

/**
 * adap_tab_overview_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a `AdapTabOveview`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.3
 */
GtkWidget *
adap_tab_overview_get_child (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), NULL);

  return adap_bin_get_child (ADAP_BIN (self->child_bin));
}

/**
 * adap_tab_overview_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a tab overview
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_child (AdapTabOverview *self,
                            GtkWidget      *child)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (child == adap_tab_overview_get_child (self))
    return;

  adap_bin_set_child (ADAP_BIN (self->child_bin), child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adap_tab_overview_get_open: (attributes org.gtk.Method.get_property=open)
 * @self: a tab overview
 *
 * Gets whether @self is open.
 *
 * Returns: whether the overview is open
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_open (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return self->is_open;
}

/**
 * adap_tab_overview_set_open: (attributes org.gtk.Method.set_property=open)
 * @self: a tab overview
 * @open: whether the overview is open
 *
 * Sets whether the to open @self.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_open (AdapTabOverview *self,
                           gboolean        open)
{
  AdapTabPage *selected_page;
  AdapTabGrid *grid;

  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  open = !!open;

  if (self->is_open == open)
    return;

  if (open && !self->view) {
    g_warning ("Trying to open AdapTabOverview %p, but it doesn't have a view set", self);
    return;
  }

  if (!adap_tab_view_get_n_pages (self->view)) {
    if (open)
      g_warning ("Trying to open AdapTabOverview %p with no pages in its AdapTabView", self);
    else
      g_warning ("Trying to close AdapTabOverview %p with no pages in its AdapTabView", self);

    return;
  }

  selected_page = adap_tab_view_get_selected_page (self->view);

  self->transition_pinned = adap_tab_page_get_pinned (selected_page);

  if (self->transition_pinned)
    grid = self->pinned_grid;
  else
    grid = self->grid;

  if (self->transition_thumbnail &&
      self->transition_thumbnail != adap_tab_grid_get_transition_thumbnail (grid))
    adap_animation_skip (self->open_animation);

  self->is_open = open;

  update_actions (self);

  if (open) {
    GtkWidget *focus = NULL;

    if (gtk_widget_get_root (GTK_WIDGET (self)))
      focus = gtk_root_get_focus (gtk_widget_get_root (GTK_WIDGET (self)));

    if (focus && gtk_widget_is_ancestor (focus, self->child_bin)) {
      if (self->last_focus)
        g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                      (gpointer *)& self->last_focus);

      self->last_focus = focus;

      g_object_add_weak_pointer (G_OBJECT (self->last_focus),
                                 (gpointer *) &self->last_focus);
    }

    adap_tab_view_open_overview (self->view);

    set_overview_visible (self, self->is_open, ANIMATION_IN);

    adap_tab_grid_try_focus_selected_tab (grid, FALSE);
  } else {
    set_overview_visible (self, self->is_open, ANIMATION_OUT);
  }

  if (self->transition_picture)
    adap_tab_thumbnail_fade_in (self->transition_thumbnail);

  self->transition_thumbnail = adap_tab_grid_get_transition_thumbnail (grid);
  self->transition_picture = g_object_ref (adap_tab_thumbnail_get_thumbnail (self->transition_thumbnail));
  adap_tab_thumbnail_fade_out (self->transition_thumbnail);

  adap_timed_animation_set_value_from (ADAP_TIMED_ANIMATION (self->open_animation),
                                      self->progress);
  adap_timed_animation_set_value_to (ADAP_TIMED_ANIMATION (self->open_animation),
                                    open ? 1 : 0);

  self->animating = TRUE;
  adap_animation_play (self->open_animation);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_OPEN]);
}

/**
 * adap_tab_overview_get_inverted: (attributes org.gtk.Method.get_property=inverted)
 * @self: a tab overview
 *
 * Gets whether thumbnails use inverted layout.
 *
 * Returns: whether thumbnails use inverted layout
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_inverted (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return adap_tab_grid_get_inverted (self->grid);
}

/**
 * adap_tab_overview_set_inverted: (attributes org.gtk.Method.set_property=inverted)
 * @self: a tab overview
 * @inverted: whether thumbnails use inverted layout
 *
 * Sets whether thumbnails use inverted layout.
 *
 * If set to `TRUE`, thumbnails will have the close or unpin button at the
 * beginning and the indicator at the end rather than the other way around.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_inverted (AdapTabOverview *self,
                               gboolean        inverted)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  inverted = !!inverted;

  if (adap_tab_overview_get_inverted (self) == inverted)
    return;

  adap_tab_grid_set_inverted (self->grid, inverted);
  adap_tab_grid_set_inverted (self->pinned_grid, inverted);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

/**
 * adap_tab_overview_get_enable_search: (attributes org.gtk.Method.get_property=enable-search)
 * @self: a tab overview
 *
 * Gets whether search in tabs is enabled for @self.
 *
 * Returns: whether search is enabled
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_enable_search (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return self->enable_search;
}

/**
 * adap_tab_overview_set_enable_search: (attributes org.gtk.Method.set_property=enable-search)
 * @self: a tab overview
 * @enable_search: whether to enable search
 *
 * Sets whether to enable search in tabs for @self.
 *
 * Search matches tab titles and tooltips, as well as keywords, set via
 * [property@TabPage:keyword]. Use keywords to search in e.g. page URLs in a web
 * browser.
 *
 * During search, tab reordering and drag-n-drop are disabled.
 *
 * Use [property@TabOverview:search-active] to check out if search is currently
 * active.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_enable_search (AdapTabOverview *self,
                                    gboolean        enable_search)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  enable_search = !!enable_search;

  if (self->enable_search == enable_search)
    return;

  self->enable_search = enable_search;

  if (!enable_search)
    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (self->search_bar), FALSE);

  gtk_search_bar_set_key_capture_widget (GTK_SEARCH_BAR (self->search_bar),
                                         enable_search ? self->overview : NULL);
  gtk_widget_set_visible (self->search_button, enable_search);
  update_header_bar (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_SEARCH]);
}

/**
 * adap_tab_overview_get_search_active: (attributes org.gtk.Method.get_property=search-active)
 * @self: a tab overview
 *
 * Gets whether search is currently active for @self.
 *
 * See [property@TabOverview:enable-search].
 *
 * Returns: whether search is active
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_search_active (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return self->search_active;
}

/**
 * adap_tab_overview_get_enable_new_tab: (attributes org.gtk.Method.get_property=enable-new-tab)
 * @self: a tab overview
 *
 * Gets whether to new tab button is enabled for @self.
 *
 * Returns: whether new tab button is enabled
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_enable_new_tab (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return self->enable_new_tab;
}

/**
 * adap_tab_overview_set_enable_new_tab: (attributes org.gtk.Method.set_property=enable-new-tab)
 * @self: a tab overview
 * @enable_new_tab: whether to enable new tab button
 *
 * Sets whether to enable new tab button for @self.
 *
 * Connect to the [signal@TabOverview::create-tab] signal to use it.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_enable_new_tab (AdapTabOverview *self,
                                     gboolean        enable_new_tab)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  enable_new_tab = !!enable_new_tab;

  if (self->enable_new_tab == enable_new_tab)
    return;

  self->enable_new_tab = enable_new_tab;

  update_new_tab_button (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_NEW_TAB]);
}

/**
 * adap_tab_overview_get_secondary_menu: (attributes org.gtk.Method.get_property=secondary-menu)
 * @self: a tab overview
 *
 * Gets the secondary menu model for @self.
 *
 * Returns: (transfer none) (nullable): the secondary menu model
 *
 * Since: 1.3
 */
GMenuModel *
adap_tab_overview_get_secondary_menu (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), NULL);

  return gtk_menu_button_get_menu_model (GTK_MENU_BUTTON (self->secondary_menu_button));
}

/**
 * adap_tab_overview_set_secondary_menu: (attributes org.gtk.Method.set_property=secondary-menu)
 * @self: a tab overview
 * @secondary_menu: (nullable): a menu model
 *
 * Sets the secondary menu model for @self.
 *
 * Use it to add extra actions, e.g. to open a new window or undo closed tab.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_secondary_menu (AdapTabOverview *self,
                                     GMenuModel     *secondary_menu)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));
  g_return_if_fail (secondary_menu == NULL || G_IS_MENU_MODEL (secondary_menu));

  if (secondary_menu == adap_tab_overview_get_secondary_menu (self))
    return;

  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->secondary_menu_button),
                                  secondary_menu);
  gtk_widget_set_visible (self->secondary_menu_button, !!secondary_menu);
  update_header_bar (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SECONDARY_MENU]);
}

/**
 * adap_tab_overview_get_show_start_title_buttons: (attributes org.gtk.Method.get_property=show-start-title-buttons)
 * @self: a tab overview
 *
 * Gets whether start title buttons are shown in @self's header bar.
 *
 * Returns: whether start title buttons are shown
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_show_start_title_buttons (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return adap_header_bar_get_show_start_title_buttons (ADAP_HEADER_BAR (self->header_bar));
}

/**
 * adap_tab_overview_set_show_start_title_buttons: (attributes org.gtk.Method.set_property=show-start-title-buttons)
 * @self: a tab overview
 * @show_start_title_buttons: whether to show start title buttons
 *
 * Sets whether to show start title buttons in @self's header bar.
 *
 * See [property@HeaderBar:show-end-title-buttons] for the other side.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_show_start_title_buttons (AdapTabOverview *self,
                                               gboolean        show_start_title_buttons)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  show_start_title_buttons = !!show_start_title_buttons;

  if (adap_tab_overview_get_show_start_title_buttons (self) == show_start_title_buttons)
    return;

  adap_header_bar_set_show_start_title_buttons (ADAP_HEADER_BAR (self->header_bar),
                                               show_start_title_buttons);

  update_header_bar (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_START_TITLE_BUTTONS]);
}

/**
 * adap_tab_overview_get_show_end_title_buttons: (attributes org.gtk.Method.get_property=show-end-title-buttons)
 * @self: a tab overview
 *
 * Gets whether end title buttons are shown in @self's header bar.
 *
 * Returns: whether end title buttons are shown
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_show_end_title_buttons (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return adap_header_bar_get_show_end_title_buttons (ADAP_HEADER_BAR (self->header_bar));
}

/**
 * adap_tab_overview_set_show_end_title_buttons: (attributes org.gtk.Method.set_property=show-end-title-buttons)
 * @self: a tab overview
 * @show_end_title_buttons: whether to show end title buttons
 *
 * Sets whether to show end title buttons in @self's header bar.
 *
 * See [property@HeaderBar:show-start-title-buttons] for the other side.
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_show_end_title_buttons (AdapTabOverview *self,
                                             gboolean        show_end_title_buttons)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  show_end_title_buttons = !!show_end_title_buttons;

  if (adap_tab_overview_get_show_end_title_buttons (self) == show_end_title_buttons)
    return;

  adap_header_bar_set_show_end_title_buttons (ADAP_HEADER_BAR (self->header_bar),
                                             show_end_title_buttons);

  update_header_bar (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_END_TITLE_BUTTONS]);
}

/**
 * adap_tab_overview_setup_extra_drop_target:
 * @self: a tab overview
 * @actions: the supported actions
 * @types: (nullable) (transfer none) (array length=n_types):
 *   all supported `GType`s that can be dropped
 * @n_types: number of @types
 *
 * Sets the supported types for this drop target.
 *
 * Sets up an extra drop target on tabs.
 *
 * This allows to drag arbitrary content onto tabs, for example URLs in a web
 * browser.
 *
 * If a tab is hovered for a certain period of time while dragging the content,
 * it will be automatically selected.
 *
 * The [signal@TabOverview::extra-drag-drop] signal can be used to handle the
 * drop.
 *
 * Since: 1.3
 */
void
adap_tab_overview_setup_extra_drop_target (AdapTabOverview *self,
                                          GdkDragAction   actions,
                                          GType          *types,
                                          gsize           n_types)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  adap_tab_grid_setup_extra_drop_target (self->grid, actions, types, n_types);
  adap_tab_grid_setup_extra_drop_target (self->pinned_grid, actions, types, n_types);
}

AdapTabGrid *
adap_tab_overview_get_tab_grid (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), NULL);

  return self->grid;
}

AdapTabGrid *
adap_tab_overview_get_pinned_tab_grid (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), NULL);

  return self->pinned_grid;
}

/**
 * adap_tab_overview_get_extra_drag_preferred_action:
 * @self: a tab overview
 *
 * Gets the current action during a drop on the extra_drop_target.
 *
 * Returns: the drag action of the current drop.
 *
 * Since: 1.4
 */
GdkDragAction
adap_tab_overview_get_extra_drag_preferred_action (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), 0);

  return self->extra_drag_preferred_action;
}

/**
 * adap_tab_overview_get_extra_drag_preload: (attributes org.gtk.Method.get_property=extra-drag-preload)
 * @self: a tab overview
 *
 * Gets whether drop data should be preloaded on hover.
 *
 * Returns: whether drop data should be preloaded on hover
 *
 * Since: 1.3
 */
gboolean
adap_tab_overview_get_extra_drag_preload (AdapTabOverview *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_OVERVIEW (self), FALSE);

  return adap_tab_grid_get_extra_drag_preload (self->grid);
}

/**
 * adap_tab_overview_set_extra_drag_preload: (attributes org.gtk.Method.set_property=extra-drag-preload)
 * @self: a tab overview
 * @preload: whether to preload drop data
 *
 * Sets whether drop data should be preloaded on hover.
 *
 * See [property@Gtk.DropTarget:preload].
 *
 * Since: 1.3
 */
void
adap_tab_overview_set_extra_drag_preload (AdapTabOverview *self,
                                         gboolean        preload)
{
  g_return_if_fail (ADAP_IS_TAB_OVERVIEW (self));

  if (adap_tab_overview_get_extra_drag_preload (self) == preload)
    return;

  adap_tab_grid_set_extra_drag_preload (self->grid, preload);
  adap_tab_grid_set_extra_drag_preload (self->pinned_grid, preload);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PRELOAD]);
}
