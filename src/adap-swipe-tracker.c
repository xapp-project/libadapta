/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-swipe-tracker-private.h"

#include "adw-marshalers.h"
#include "adw-navigation-direction.h"

#include <math.h>

#define TOUCHPAD_BASE_DISTANCE_H 400
#define TOUCHPAD_BASE_DISTANCE_V 300
#define EVENT_HISTORY_THRESHOLD_MS 150
#define MIN_ANIMATION_DURATION 100
#define MAX_ANIMATION_DURATION 400
#define VELOCITY_THRESHOLD_TOUCH 0.3
#define VELOCITY_THRESHOLD_TOUCHPAD 0.6
#define DECELERATION_TOUCH 0.998
#define DECELERATION_TOUCHPAD 0.997
#define VELOCITY_CURVE_THRESHOLD 2
#define DECELERATION_PARABOLA_MULTIPLIER 0.35
#define DURATION_MULTIPLIER 3
#define ANIMATION_BASE_VELOCITY 0.002
#define DRAG_THRESHOLD_DISTANCE 16
#define EPSILON 0.005
#define OVERSHOOT_DISTANCE_MULTIPLIER 0.1

#define SIGN(x) ((x) > 0.0 ? 1.0 : ((x) < 0.0 ? -1.0 : 0.0))

/**
 * AdwSwipeTracker:
 *
 * A swipe tracker used in [class@Carousel], [class@NavigationView] and
 * [class@OverlaySplitView].
 *
 * The `AdwSwipeTracker` object can be used for implementing widgets with swipe
 * gestures. It supports touch-based swipes, pointer dragging, and touchpad
 * scrolling.
 *
 * The widgets will probably want to expose the [property@SwipeTracker:enabled]
 * property. If they expect to use horizontal orientation,
 * [property@SwipeTracker:reversed] can be used for supporting RTL text
 * direction.
 */

typedef enum {
  ADW_SWIPE_TRACKER_STATE_NONE,
  ADW_SWIPE_TRACKER_STATE_PENDING,
  ADW_SWIPE_TRACKER_STATE_SCROLLING,
  ADW_SWIPE_TRACKER_STATE_FINISHING,
  ADW_SWIPE_TRACKER_STATE_REJECTED,
} AdwSwipeTrackerState;

typedef struct {
  double delta;
  guint32 time;
} EventHistoryRecord;

struct _AdwSwipeTracker
{
  GObject parent_instance;

  AdwSwipeable *swipeable;
  gboolean enabled;
  gboolean reversed;
  gboolean allow_mouse_drag;
  gboolean allow_long_swipes;
  GtkOrientation orientation;
  gboolean lower_overshoot;
  gboolean upper_overshoot;
  gboolean allow_window_handle;

  double pointer_x;
  double pointer_y;

  GArray *event_history;

  double initial_progress;
  double progress;
  gboolean cancelled;

  double prev_offset;

  AdwSwipeTrackerState state;

  GtkEventController *motion_controller;
  GtkEventController *scroll_controller;
  GtkGesture *touch_gesture;
  GtkGesture *touch_gesture_capture;

  gboolean is_window_handle;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSwipeTracker, adw_swipe_tracker, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_SWIPEABLE,
  PROP_ENABLED,
  PROP_REVERSED,
  PROP_ALLOW_MOUSE_DRAG,
  PROP_ALLOW_LONG_SWIPES,
  PROP_LOWER_OVERSHOOT,
  PROP_UPPER_OVERSHOOT,
  PROP_ALLOW_WINDOW_HANDLE,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ALLOW_WINDOW_HANDLE + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PREPARE,
  SIGNAL_BEGIN_SWIPE,
  SIGNAL_UPDATE_SWIPE,
  SIGNAL_END_SWIPE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
swipeable_notify_cb (AdwSwipeTracker *self)
{
  self->motion_controller = NULL;
  self->scroll_controller = NULL;
  self->touch_gesture = NULL;
  self->touch_gesture_capture = NULL;
  self->swipeable = NULL;
}

static void
set_swipeable (AdwSwipeTracker *self,
               AdwSwipeable    *swipeable)
{
  if (self->swipeable == swipeable)
    return;

  if (self->swipeable)
    g_object_weak_unref (G_OBJECT (self->swipeable),
                         (GWeakNotify) swipeable_notify_cb,
                         self);

  self->swipeable = swipeable;

  if (self->swipeable)
    g_object_weak_ref (G_OBJECT (self->swipeable),
                       (GWeakNotify) swipeable_notify_cb,
                       self);
}

static void
reset (AdwSwipeTracker *self)
{
  self->state = ADW_SWIPE_TRACKER_STATE_NONE;

  self->prev_offset = 0;

  self->initial_progress = 0;
  self->progress = 0;

  g_array_remove_range (self->event_history, 0, self->event_history->len);

  self->cancelled = FALSE;
}

static void
get_range (AdwSwipeTracker *self,
           double          *first,
           double          *last)
{
  double *points;
  int n;

  points = adw_swipeable_get_snap_points (self->swipeable, &n);

  *first = points[0];
  *last = points[n - 1];

  g_free (points);
}

static void
gesture_prepare (AdwSwipeTracker        *self,
                 AdwNavigationDirection  direction)
{
  if (self->state != ADW_SWIPE_TRACKER_STATE_NONE)
    return;

  g_signal_emit (self, signals[SIGNAL_PREPARE], 0, direction);

  self->initial_progress = adw_swipeable_get_progress (self->swipeable);
  self->progress = self->initial_progress;
  self->state = ADW_SWIPE_TRACKER_STATE_PENDING;
}

static int
find_closest_point (double *points,
                    int     n,
                    double  pos)
{
  guint i, min = 0;

  for (i = 1; i < n; i++)
    if (ABS (points[i] - pos) < ABS (points[min] - pos))
      min = i;

  return min;
}

static int
find_next_point (double *points,
                 int     n,
                 double  pos)
{
  guint i;

  for (i = 0; i < n; i++)
    if (G_APPROX_VALUE (points[i], pos, DBL_EPSILON) || points[i] > pos)
      return i;

  return -1;
}

static int
find_previous_point (double *points,
                     int     n,
                     double  pos)
{
  int i;

  for (i = n - 1; i >= 0; i--)
    if (G_APPROX_VALUE (points[i], pos, DBL_EPSILON) || points[i] < pos)
      return i;

  return -1;
}

static void
get_bounds (AdwSwipeTracker *self,
            double          *points,
            int              n,
            double           pos,
            double          *lower,
            double          *upper)
{
  int prev, next;
  int closest = find_closest_point (points, n, pos);

  if (ABS (points[closest] - pos) < EPSILON) {
    prev = next = closest;
  } else {
    prev = find_previous_point (points, n, pos);
    next = find_next_point (points, n, pos);
  }

  *lower = points[MAX (prev - 1, 0)];
  *upper = points[MIN (next + 1, n - 1)];
}

static double
adjust_for_overshoot (AdwSwipeTracker *self,
                      double           amount)
{
  double d = adw_swipeable_get_distance (self->swipeable) * OVERSHOOT_DISTANCE_MULTIPLIER;

  return (1 - 1 / (1 + amount * d)) / d;
}

static void
trim_history (AdwSwipeTracker *self,
              guint32          current_time)
{
  guint32 threshold_time = current_time - EVENT_HISTORY_THRESHOLD_MS;
  guint i;

  for (i = 0; i < self->event_history->len; i++) {
    guint32 time = g_array_index (self->event_history,
                                  EventHistoryRecord, i).time;

    if (time >= threshold_time)
      break;
  }

  if (i > 0)
    g_array_remove_range (self->event_history, 0, i);
}

static void
append_to_history (AdwSwipeTracker *self,
                   double           delta,
                   guint32          time)
{
  EventHistoryRecord record;

  trim_history (self, time);

  record.delta = delta;
  record.time = time;

  g_array_append_val (self->event_history, record);
}

static double
calculate_velocity (AdwSwipeTracker *self)
{
  double total_delta = 0, velocity, lower, upper;
  double *points;
  guint32 first_time = 0, last_time = 0;
  guint i;
  int n;

  for (i = 0; i < self->event_history->len; i++) {
    EventHistoryRecord *r =
      &g_array_index (self->event_history, EventHistoryRecord, i);

    if (i == 0)
      first_time = r->time;
    else
      total_delta += r->delta;

    last_time = r->time;
  }

  if (first_time == last_time)
    return 0;

  velocity = total_delta / (last_time - first_time);

  /* Overshoot */

  points = adw_swipeable_get_snap_points (self->swipeable, &n);

  if (!self->allow_long_swipes)
    get_bounds (self, points, n, self->initial_progress, &lower, &upper);
  else
    get_range (self, &lower, &upper);

  if (self->progress <= lower) {
    if (self->lower_overshoot && self->progress > lower)
      velocity *= adjust_for_overshoot (self, lower - self->progress) / (lower - self->progress);
    else if (velocity < 0)
      velocity = 0;
  }

  if (self->progress >= upper) {
    if (self->upper_overshoot && self->progress < upper)
      velocity *= adjust_for_overshoot (self, self->progress - upper) / (self->progress - upper);
    else if (velocity > 0)
      velocity = 0;
  }

  return velocity;
}

static void
gesture_begin (AdwSwipeTracker *self)
{
  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING)
    return;

  self->state = ADW_SWIPE_TRACKER_STATE_SCROLLING;

  g_signal_emit (self, signals[SIGNAL_BEGIN_SWIPE], 0);
}

static int
find_point_for_projection (AdwSwipeTracker *self,
                           double          *points,
                           int              n,
                           double           pos,
                           double           velocity)
{
  int initial = find_closest_point (points, n, self->initial_progress);
  int prev = find_previous_point (points, n, pos);
  int next = find_next_point (points, n, pos);

  if ((velocity > 0 ? prev : next) == initial)
    return velocity > 0 ? next : prev;

  return find_closest_point (points, n, pos);
}

static void
gesture_update (AdwSwipeTracker *self,
                double           delta,
                guint32          time)
{
  double lower, upper;
  double progress;

  if (self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  if (!self->allow_long_swipes) {
    double *points;
    int n;

    points = adw_swipeable_get_snap_points (self->swipeable, &n);
    get_bounds (self, points, n, self->initial_progress, &lower, &upper);

    g_free (points);
  } else {
    get_range (self, &lower, &upper);
  }

  progress = self->progress + delta;

  self->progress = progress;

  if (progress < lower) {
    if (self->lower_overshoot)
      progress = lower - adjust_for_overshoot (self, lower - progress);
    else
      progress = self->progress = lower;
  }

  if (progress > upper) {
    if (self->upper_overshoot)
      progress = upper + adjust_for_overshoot (self, progress - upper);
    else
      progress = self->progress = upper;
  }

  g_signal_emit (self, signals[SIGNAL_UPDATE_SWIPE], 0, progress);
}

static double
get_end_progress (AdwSwipeTracker *self,
                  double           velocity,
                  gboolean         is_touchpad)
{
  double pos, decel, slope;
  double *points;
  int n;
  double lower, upper;

  if (self->cancelled)
    return adw_swipeable_get_cancel_progress (self->swipeable);

  points = adw_swipeable_get_snap_points (self->swipeable, &n);

  if (!self->allow_long_swipes)
    get_bounds (self, points, n, self->initial_progress, &lower, &upper);
  else
    get_range (self, &lower, &upper);

  if (ABS (velocity) < (is_touchpad ? VELOCITY_THRESHOLD_TOUCHPAD : VELOCITY_THRESHOLD_TOUCH)) {
    pos = points[find_closest_point (points, n, self->progress)];
    pos = CLAMP (pos, lower, upper);

    g_free (points);

    return pos;
  }

  decel = is_touchpad ? DECELERATION_TOUCHPAD : DECELERATION_TOUCH;
  slope = decel / (1.0 - decel) / 1000.0;

  if (ABS (velocity) > VELOCITY_CURVE_THRESHOLD) {
    const double c = slope / 2 / DECELERATION_PARABOLA_MULTIPLIER;
    const double x = ABS (velocity) - VELOCITY_CURVE_THRESHOLD + c;

    pos = DECELERATION_PARABOLA_MULTIPLIER * x * x
        - DECELERATION_PARABOLA_MULTIPLIER * c * c
        + slope * VELOCITY_CURVE_THRESHOLD;
  } else {
    pos = ABS (velocity) * slope;
  }

  pos = (pos * SIGN (velocity)) + self->progress;

  pos = CLAMP (pos, lower, upper);
  pos = points[find_point_for_projection (self, points, n, pos, velocity)];

  g_free (points);

  return pos;
}

static void
gesture_end (AdwSwipeTracker *self,
             double           distance,
             guint32          time,
             gboolean         is_touchpad)
{
  double end_progress, velocity;

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE)
    return;

  trim_history (self, time);

  velocity = calculate_velocity (self);
  end_progress = get_end_progress (self, velocity, is_touchpad);

  g_signal_emit (self, signals[SIGNAL_END_SWIPE], 0, velocity, end_progress);

  if (!self->cancelled)
    self->state = ADW_SWIPE_TRACKER_STATE_FINISHING;

  reset (self);
}

static void
gesture_cancel (AdwSwipeTracker *self,
                double           distance,
                guint32          time,
                gboolean         is_touchpad)
{
  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING &&
      self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    reset (self);

    return;
  }

  self->cancelled = TRUE;
  gesture_end (self, distance, time, is_touchpad);
}

static gboolean
has_window_handle (AdwSwipeTracker *self,
                   GtkWidget       *widget)
{
  GtkWidget *parent = widget;
  gboolean found_window_handle = FALSE;

  while (parent && parent != GTK_WIDGET (self->swipeable)) {
    found_window_handle |= GTK_IS_WINDOW_HANDLE (parent);

    parent = gtk_widget_get_parent (parent);
  }

  return found_window_handle;
}

static gboolean
should_suppress_drag (AdwSwipeTracker *self,
                      GtkWidget       *widget)
{
  return !self->allow_window_handle && has_window_handle (self, widget);
}

static gboolean
should_force_drag (AdwSwipeTracker *self,
                   GtkWidget       *widget)
{
  return self->allow_window_handle && has_window_handle (self, widget);
}

static inline gboolean
is_in_swipe_area (AdwSwipeTracker        *self,
                  double                  x,
                  double                  y,
                  AdwNavigationDirection  direction,
                  gboolean                is_drag)
{
  GdkRectangle rect;

  adw_swipeable_get_swipe_area (self->swipeable, direction, is_drag, &rect);

  return rect.width > 0 && rect.height > 0 &&
         (G_APPROX_VALUE (x, rect.x, DBL_EPSILON) || x > rect.x) &&
         x < rect.x + rect.width &&
         (G_APPROX_VALUE (y, rect.y, DBL_EPSILON) || y > rect.y) &&
         y < rect.y + rect.height;
}

static void
drag_capture_begin_cb (AdwSwipeTracker *self,
                       double           start_x,
                       double           start_y,
                       GtkGestureDrag  *gesture)
{
  GtkWidget *widget;

  if (self->state != ADW_SWIPE_TRACKER_STATE_NONE) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  widget = gtk_widget_pick (GTK_WIDGET (self->swipeable),
                          start_x,
                          start_y,
                          GTK_PICK_DEFAULT);

  if (should_force_drag (self, widget)) {
    self->is_window_handle = TRUE;
    return;
  }

  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
  self->is_window_handle = FALSE;
}

static void
drag_begin_cb (AdwSwipeTracker *self,
               double           start_x,
               double           start_y,
               GtkGestureDrag  *gesture)
{
  GtkWidget *widget;

  if (self->state != ADW_SWIPE_TRACKER_STATE_NONE) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  widget = gtk_widget_pick (GTK_WIDGET (self->swipeable),
                          start_x,
                          start_y,
                          GTK_PICK_DEFAULT);

  self->is_window_handle = FALSE;

  if (should_suppress_drag (self, widget)) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gtk_gesture_set_state (self->touch_gesture_capture, GTK_EVENT_SEQUENCE_DENIED);
}

static void
drag_update_cb (AdwSwipeTracker *self,
                double           offset_x,
                double           offset_y,
                GtkGestureDrag  *gesture)
{
  double offset, distance, delta;
  gboolean is_vertical, is_offset_vertical;
  guint32 time;

  distance = adw_swipeable_get_distance (self->swipeable);

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  offset = is_vertical ? offset_y : offset_x;

  if (!self->reversed)
    offset = -offset;

  delta = offset - self->prev_offset;
  self->prev_offset = offset;

  is_offset_vertical = (ABS (offset_y) > ABS (offset_x));

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  append_to_history (self, delta, time);

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE) {
    if (is_vertical == is_offset_vertical)
      gesture_prepare (self, offset > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK);
    else
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_PENDING) {
    double drag_distance, threshold;
    double first_point, last_point;

    get_range (self, &first_point, &last_point);

    drag_distance = sqrt (offset_x * offset_x + offset_y * offset_y);

    if (self->is_window_handle) {
      GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (self->swipeable));
      int dnd_threshold;

      g_object_get (settings, "gtk-dnd-drag-threshold", &dnd_threshold, NULL);

      threshold = dnd_threshold;
    } else {
      threshold = DRAG_THRESHOLD_DISTANCE;
    }

    if (G_APPROX_VALUE (drag_distance, threshold, DBL_EPSILON) ||
        drag_distance > threshold) {
      double start_x, start_y;
      AdwNavigationDirection direction;
      gboolean is_overshooting_lower, is_overshooting_upper;

      gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);
      direction = offset > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK;

      if (!is_in_swipe_area (self, start_x, start_y, direction, TRUE) &&
          !is_in_swipe_area (self, start_x + offset_x, start_y + offset_y, direction, TRUE))
        return;

      if (is_vertical != is_offset_vertical) {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
        return;
      }

      if (G_APPROX_VALUE (first_point, last_point, DBL_EPSILON)) {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
        return;
      }

      is_overshooting_lower =
        offset < 0 &&
        (G_APPROX_VALUE (self->progress, first_point, DBL_EPSILON) ||
         self->progress < first_point);

      is_overshooting_upper =
        offset > 0 &&
        (G_APPROX_VALUE (self->progress, last_point, DBL_EPSILON) ||
         self->progress > last_point);

      if ((!self->lower_overshoot && is_overshooting_lower) ||
          (!self->upper_overshoot && is_overshooting_upper)) {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
        return;
      }

      gesture_begin (self);

      self->prev_offset = offset;
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
    }
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_SCROLLING)
    gesture_update (self, delta / distance, time);
}

static void
drag_end_cb (AdwSwipeTracker *self,
             double           offset_x,
             double           offset_y,
             GtkGestureDrag  *gesture)
{
  double distance;
  guint32 time;

  distance = adw_swipeable_get_distance (self->swipeable);

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);

    reset (self);
    return;
  }

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  if (self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_cancel (self, distance, time, FALSE);
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gesture_end (self, distance, time, FALSE);
}

static void
drag_cancel_cb (AdwSwipeTracker  *self,
                GdkEventSequence *sequence,
                GtkGesture       *gesture)
{
  guint32 time;
  double distance;

  distance = adw_swipeable_get_distance (self->swipeable);

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  gesture_cancel (self, distance, time, FALSE);
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static gboolean
handle_scroll_event (AdwSwipeTracker *self,
                     GdkEvent        *event)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  double dx, dy, delta, distance;
  gboolean is_vertical;
  guint32 time;

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  distance = is_vertical ? TOUCHPAD_BASE_DISTANCE_V : TOUCHPAD_BASE_DISTANCE_H;

  if (!event || gdk_event_get_event_type (event) != GDK_SCROLL)
    return GDK_EVENT_PROPAGATE;

  if (gdk_scroll_event_get_direction (event) != GDK_SCROLL_SMOOTH)
    return GDK_EVENT_PROPAGATE;

  source_device = gdk_event_get_device (event);
  input_source = gdk_device_get_source (source_device);
  if (input_source != GDK_SOURCE_TOUCHPAD)
    return GDK_EVENT_PROPAGATE;

  gdk_scroll_event_get_deltas (event, &dx, &dy);
  delta = is_vertical ? dy : dx;
  if (self->reversed)
    delta = -delta;

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    if (gdk_scroll_event_is_stop (event))
      reset (self);

    return GDK_EVENT_PROPAGATE;
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE) {
    AdwNavigationDirection direction;

    if (gdk_scroll_event_is_stop (event))
      return GDK_EVENT_PROPAGATE;

    direction = delta > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK;

    if (!is_in_swipe_area (self, self->pointer_x, self->pointer_y, direction, FALSE)) {
      self->state = ADW_SWIPE_TRACKER_STATE_REJECTED;
      return GDK_EVENT_PROPAGATE;
    }

    gesture_prepare (self, delta > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK);
  }

  time = gdk_event_get_time (event);

  if (self->state == ADW_SWIPE_TRACKER_STATE_PENDING) {
    double first_point, last_point;

    get_range (self, &first_point, &last_point);

    append_to_history (self, delta, time);

    if (G_APPROX_VALUE (first_point, last_point, DBL_EPSILON)) {
      gesture_cancel (self, distance, time, TRUE);
    } else {
      gboolean is_overshooting_lower =
        delta < 0 &&
        (G_APPROX_VALUE (self->progress, first_point, DBL_EPSILON) ||
         self->progress < first_point);

      gboolean is_overshooting_upper =
        delta > 0 &&
        (G_APPROX_VALUE (self->progress, last_point, DBL_EPSILON) ||
         self->progress > last_point);

      if ((!self->lower_overshoot && is_overshooting_lower) ||
          (!self->upper_overshoot && is_overshooting_upper))
        gesture_cancel (self, distance, time, TRUE);
      else
        gesture_begin (self);
    }
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    if (gdk_scroll_event_is_stop (event)) {
      gesture_end (self, distance, time, TRUE);
    } else {
      append_to_history (self, delta, time);

      gesture_update (self, delta / distance, time);
      return GDK_EVENT_STOP;
    }
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_FINISHING)
    reset (self);

  return GDK_EVENT_PROPAGATE;
}

static void
scroll_begin_cb (AdwSwipeTracker          *self,
                 GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static gboolean
scroll_cb (AdwSwipeTracker          *self,
           double                    dx,
           double                    dy,
           GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  return handle_scroll_event (self, event);
}

static void
scroll_end_cb (AdwSwipeTracker          *self,
               GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static void
motion_cb (AdwSwipeTracker          *self,
           double                    x,
           double                    y,
           GtkEventControllerMotion *controller)
{
  self->pointer_x = x;
  self->pointer_y = y;
}

static void
update_controllers (AdwSwipeTracker *self)
{
  GtkEventControllerScrollFlags flags;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    flags = GTK_EVENT_CONTROLLER_SCROLL_HORIZONTAL;
  else
    flags = GTK_EVENT_CONTROLLER_SCROLL_VERTICAL;

  if (self->scroll_controller) {
    gtk_event_controller_scroll_set_flags (GTK_EVENT_CONTROLLER_SCROLL (self->scroll_controller), flags);
    gtk_event_controller_set_propagation_phase (self->scroll_controller,
                                                self->enabled ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);
  }

  if (self->motion_controller)
    gtk_event_controller_set_propagation_phase (self->motion_controller,
                                                self->enabled ? GTK_PHASE_CAPTURE : GTK_PHASE_NONE);

  if (self->touch_gesture)
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (self->touch_gesture),
                                                self->enabled ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);

  if (self->touch_gesture_capture)
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (self->touch_gesture_capture),
                                                self->enabled ? GTK_PHASE_CAPTURE : GTK_PHASE_NONE);
}

static void
set_orientation (AdwSwipeTracker *self,
                 GtkOrientation   orientation)
{

  if (orientation == self->orientation)
    return;

  self->orientation = orientation;
  update_controllers (self);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_swipe_tracker_constructed (GObject *object)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);
  GtkEventController *controller;

  g_assert (self->swipeable);

  g_signal_connect_object (self->swipeable, "unrealize", G_CALLBACK (reset), self, G_CONNECT_SWAPPED);

  controller = gtk_event_controller_motion_new ();
  gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
  g_signal_connect_object (controller, "motion", G_CALLBACK (motion_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->motion_controller = controller;

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  g_signal_connect_object (controller, "drag-begin", G_CALLBACK (drag_capture_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-update", G_CALLBACK (drag_update_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-end", G_CALLBACK (drag_end_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "cancel", G_CALLBACK (drag_cancel_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->touch_gesture_capture = GTK_GESTURE (controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  g_signal_connect_object (controller, "drag-begin", G_CALLBACK (drag_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-update", G_CALLBACK (drag_update_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-end", G_CALLBACK (drag_end_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "cancel", G_CALLBACK (drag_cancel_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->touch_gesture = GTK_GESTURE (controller);

  g_object_bind_property (self, "allow-mouse-drag",
                          self->touch_gesture, "touch-only",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  g_object_bind_property (self, "allow-mouse-drag",
                          self->touch_gesture_capture, "touch-only",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  controller = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_NONE);
  g_signal_connect_object (controller, "scroll-begin", G_CALLBACK (scroll_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "scroll", G_CALLBACK (scroll_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "scroll-end", G_CALLBACK (scroll_end_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->scroll_controller = controller;

  update_controllers (self);

  G_OBJECT_CLASS (adw_swipe_tracker_parent_class)->constructed (object);
}

static void
adw_swipe_tracker_dispose (GObject *object)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  if (self->touch_gesture) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable),
                                  GTK_EVENT_CONTROLLER (self->touch_gesture));
    self->touch_gesture = NULL;
  }

  if (self->touch_gesture_capture) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable),
                                  GTK_EVENT_CONTROLLER (self->touch_gesture_capture));
    self->touch_gesture_capture = NULL;
  }

  if (self->motion_controller) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable), self->motion_controller);
    self->motion_controller = NULL;
  }

  if (self->scroll_controller) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable), self->scroll_controller);
    self->scroll_controller = NULL;
  }

  set_swipeable (self, NULL);

  G_OBJECT_CLASS (adw_swipe_tracker_parent_class)->dispose (object);
}

static void
adw_swipe_tracker_finalize (GObject *object)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  g_array_free (self->event_history, TRUE);

  G_OBJECT_CLASS (adw_swipe_tracker_parent_class)->finalize (object);
}

static void
adw_swipe_tracker_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_SWIPEABLE:
    g_value_set_object (value, adw_swipe_tracker_get_swipeable (self));
    break;

  case PROP_ENABLED:
    g_value_set_boolean (value, adw_swipe_tracker_get_enabled (self));
    break;

  case PROP_REVERSED:
    g_value_set_boolean (value, adw_swipe_tracker_get_reversed (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, adw_swipe_tracker_get_allow_mouse_drag (self));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    g_value_set_boolean (value, adw_swipe_tracker_get_allow_long_swipes (self));
    break;

  case PROP_LOWER_OVERSHOOT:
    g_value_set_boolean (value, adw_swipe_tracker_get_lower_overshoot (self));
    break;

  case PROP_UPPER_OVERSHOOT:
    g_value_set_boolean (value, adw_swipe_tracker_get_upper_overshoot (self));
    break;

  case PROP_ALLOW_WINDOW_HANDLE:
    g_value_set_boolean (value, adw_swipe_tracker_get_allow_window_handle (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_swipe_tracker_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_SWIPEABLE:
    set_swipeable (self, g_value_get_object (value));
    break;

  case PROP_ENABLED:
    adw_swipe_tracker_set_enabled (self, g_value_get_boolean (value));
    break;

  case PROP_REVERSED:
    adw_swipe_tracker_set_reversed (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    adw_swipe_tracker_set_allow_mouse_drag (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    adw_swipe_tracker_set_allow_long_swipes (self, g_value_get_boolean (value));
    break;

  case PROP_LOWER_OVERSHOOT:
    adw_swipe_tracker_set_lower_overshoot (self, g_value_get_boolean (value));
    break;

  case PROP_UPPER_OVERSHOOT:
    adw_swipe_tracker_set_upper_overshoot (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_WINDOW_HANDLE:
    adw_swipe_tracker_set_allow_window_handle (self, g_value_get_boolean (value));
    break;

  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_swipe_tracker_class_init (AdwSwipeTrackerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_swipe_tracker_constructed;
  object_class->dispose = adw_swipe_tracker_dispose;
  object_class->finalize = adw_swipe_tracker_finalize;
  object_class->get_property = adw_swipe_tracker_get_property;
  object_class->set_property = adw_swipe_tracker_set_property;

  /**
   * AdwSwipeTracker:swipeable: (attributes org.gtk.Property.get=adw_swipe_tracker_get_swipeable)
   *
   * The widget the swipe tracker is attached to.
   */
  props[PROP_SWIPEABLE] =
    g_param_spec_object ("swipeable", NULL, NULL,
                         ADW_TYPE_SWIPEABLE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSwipeTracker:enabled: (attributes org.gtk.Property.get=adw_swipe_tracker_get_enabled org.gtk.Property.set=adw_swipe_tracker_set_enabled)
   *
   * Whether the swipe tracker is enabled.
   *
   * When it's not enabled, no events will be processed. Usually widgets will
   * want to expose this via a property.
   */
  props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:reversed: (attributes org.gtk.Property.get=adw_swipe_tracker_get_reversed org.gtk.Property.set=adw_swipe_tracker_set_reversed)
   *
   * Whether to reverse the swipe direction.
   *
   * If the swipe tracker is horizontal, it can be used for supporting RTL text
   * direction.
   */
  props[PROP_REVERSED] =
    g_param_spec_boolean ("reversed", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:allow-mouse-drag: (attributes org.gtk.Property.get=adw_swipe_tracker_get_allow_mouse_drag org.gtk.Property.set=adw_swipe_tracker_set_allow_mouse_drag)
   *
   * Whether to allow dragging with mouse pointer.
   */
  props[PROP_ALLOW_MOUSE_DRAG] =
    g_param_spec_boolean ("allow-mouse-drag", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:allow-long-swipes: (attributes org.gtk.Property.get=adw_swipe_tracker_get_allow_long_swipes org.gtk.Property.set=adw_swipe_tracker_set_allow_long_swipes)
   *
   * Whether to allow swiping for more than one snap point at a time.
   *
   * If the value is `FALSE`, each swipe can only move to the adjacent snap
   * points.
   */
  props[PROP_ALLOW_LONG_SWIPES] =
    g_param_spec_boolean ("allow-long-swipes", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:lower-overshoot: (attributes org.gtk.Property.get=adw_swipe_tracker_get_lower_overshoot org.gtk.Property.set=adw_swipe_tracker_set_lower_overshoot)
   *
   * Whether to allow swiping past the first available snap point.
   *
   * Since: 1.4
   */
  props[PROP_LOWER_OVERSHOOT] =
    g_param_spec_boolean ("lower-overshoot", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:upper-overshoot: (attributes org.gtk.Property.get=adw_swipe_tracker_get_upper_overshoot org.gtk.Property.set=adw_swipe_tracker_set_upper_overshoot)
   *
   * Whether to allow swiping past the last available snap point.
   *
   * Since: 1.4
   */
  props[PROP_UPPER_OVERSHOOT] =
    g_param_spec_boolean ("upper-overshoot", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:allow-window-handle: (attributes org.gtk.Property.get=adw_swipe_tracker_get_allow_window_handle org.gtk.Property.set=adw_swipe_tracker_set_allow_window_handle)
   *
   * Whether to allow touchscreen swiping from `GtkWindowHandle`.
   *
   * This will make dragging the window impossible.
   *
   * Since: 1.5
   */
  props[PROP_ALLOW_WINDOW_HANDLE] =
    g_param_spec_boolean ("allow-window-handle", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwSwipeTracker::prepare:
   * @self: a swipe tracker
   * @direction: the direction of the swipe
   *
   * This signal is emitted when a possible swipe is detected.
   *
   * The @direction value can be used to restrict the swipe to a certain
   * direction.
   */
  signals[SIGNAL_PREPARE] =
    g_signal_new ("prepare",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__ENUM,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_NAVIGATION_DIRECTION);
  g_signal_set_va_marshaller (signals[SIGNAL_PREPARE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__ENUMv);

  /**
   * AdwSwipeTracker::begin-swipe:
   *
   * This signal is emitted right before a swipe will be started, after the
   * drag threshold has been passed.
   */
  signals[SIGNAL_BEGIN_SWIPE] =
    g_signal_new ("begin-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_BEGIN_SWIPE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwSwipeTracker::update-swipe:
   * @self: a swipe tracker
   * @progress: the current animation progress value
   *
   * This signal is emitted every time the progress value changes.
   */
  signals[SIGNAL_UPDATE_SWIPE] =
    g_signal_new ("update-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__DOUBLE,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_DOUBLE);
  g_signal_set_va_marshaller (signals[SIGNAL_UPDATE_SWIPE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__DOUBLEv);

  /**
   * AdwSwipeTracker::end-swipe:
   * @self: a swipe tracker
   * @velocity: the velocity of the swipe
   * @to: the progress value to animate to
   *
   * This signal is emitted as soon as the gesture has stopped.
   *
   * The user is expected to animate the deceleration from the current progress
   * value to @to with an animation using @velocity as the initial velocity,
   * provided in pixels per second. [class@SpringAnimation] is usually a good
   * fit for this.
   */
  signals[SIGNAL_END_SWIPE] =
    g_signal_new ("end-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__DOUBLE_DOUBLE,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_DOUBLE, G_TYPE_DOUBLE);
  g_signal_set_va_marshaller (signals[SIGNAL_END_SWIPE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__DOUBLE_DOUBLEv);
}

static void
adw_swipe_tracker_init (AdwSwipeTracker *self)
{
  self->event_history = g_array_new (FALSE, FALSE, sizeof (EventHistoryRecord));
  reset (self);

  self->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->enabled = TRUE;
}

/**
 * adw_swipe_tracker_new:
 * @swipeable: a widget to add the tracker on
 *
 * Creates a new `AdwSwipeTracker` for @widget.
 *
 * Returns: the newly created `AdwSwipeTracker`
 */
AdwSwipeTracker *
adw_swipe_tracker_new (AdwSwipeable *swipeable)
{
  g_return_val_if_fail (ADW_IS_SWIPEABLE (swipeable), NULL);

  return g_object_new (ADW_TYPE_SWIPE_TRACKER,
                       "swipeable", swipeable,
                       NULL);
}

/**
 * adw_swipe_tracker_get_swipeable: (attributes org.gtk.Method.get_property=swipeable)
 * @self: a swipe tracker
 *
 * Get the widget @self is attached to.
 *
 * Returns: (transfer none): the swipeable widget
 */
AdwSwipeable *
adw_swipe_tracker_get_swipeable (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), NULL);

  return self->swipeable;
}

/**
 * adw_swipe_tracker_get_enabled: (attributes org.gtk.Method.get_property=enabled)
 * @self: a swipe tracker
 *
 * Gets whether @self is enabled.
 *
 * Returns: whether @self is enabled
 */
gboolean
adw_swipe_tracker_get_enabled (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->enabled;
}

/**
 * adw_swipe_tracker_set_enabled: (attributes org.gtk.Method.set_property=enabled)
 * @self: a swipe tracker
 * @enabled: whether @self is enabled
 *
 * Sets whether @self is enabled.
 *
 * When it's not enabled, no events will be processed. Usually widgets will want
 * to expose this via a property.
 */
void
adw_swipe_tracker_set_enabled (AdwSwipeTracker *self,
                               gboolean         enabled)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  enabled = !!enabled;

  if (self->enabled == enabled)
    return;

  self->enabled = enabled;

  if (!enabled && self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    reset (self);

  update_controllers (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLED]);
}

/**
 * adw_swipe_tracker_get_reversed: (attributes org.gtk.Method.get_property=reversed)
 * @self: a swipe tracker
 *
 * Gets whether @self is reversing the swipe direction.
 *
 * Returns: whether the direction is reversed
 */
gboolean
adw_swipe_tracker_get_reversed (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->reversed;
}

/**
 * adw_swipe_tracker_set_reversed: (attributes org.gtk.Method.set_property=reversed)
 * @self: a swipe tracker
 * @reversed: whether to reverse the swipe direction
 *
 * Sets whether to reverse the swipe direction.
 *
 * If the swipe tracker is horizontal, it can be used for supporting RTL text
 * direction.
 */
void
adw_swipe_tracker_set_reversed (AdwSwipeTracker *self,
                                gboolean         reversed)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  reversed = !!reversed;

  if (self->reversed == reversed)
    return;

  self->reversed = reversed;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVERSED]);
}

/**
 * adw_swipe_tracker_get_allow_mouse_drag: (attributes org.gtk.Method.get_property=allow-mouse-drag)
 * @self: a swipe tracker
 *
 * Gets whether @self can be dragged with mouse pointer.
 *
 * Returns: whether mouse dragging is allowed
 */
gboolean
adw_swipe_tracker_get_allow_mouse_drag (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_mouse_drag;
}

/**
 * adw_swipe_tracker_set_allow_mouse_drag: (attributes org.gtk.Method.set_property=allow-mouse-drag)
 * @self: a swipe tracker
 * @allow_mouse_drag: whether to allow mouse dragging
 *
 * Sets whether @self can be dragged with mouse pointer.
 */
void
adw_swipe_tracker_set_allow_mouse_drag (AdwSwipeTracker *self,
                                        gboolean         allow_mouse_drag)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  allow_mouse_drag = !!allow_mouse_drag;

  if (self->allow_mouse_drag == allow_mouse_drag)
    return;

  self->allow_mouse_drag = allow_mouse_drag;

  update_controllers (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_MOUSE_DRAG]);
}

/**
 * adw_swipe_tracker_get_allow_long_swipes: (attributes org.gtk.Method.get_property=allow-long-swipes)
 * @self: a swipe tracker
 *
 * Gets whether to allow swiping for more than one snap point at a time.
 *
 * Returns: whether long swipes are allowed
 */
gboolean
adw_swipe_tracker_get_allow_long_swipes (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_long_swipes;
}

/**
 * adw_swipe_tracker_set_allow_long_swipes: (attributes org.gtk.Method.set_property=allow-long-swipes)
 * @self: a swipe tracker
 * @allow_long_swipes: whether to allow long swipes
 *
 * Sets whether to allow swiping for more than one snap point at a time.
 *
 * If the value is `FALSE`, each swipe can only move to the adjacent snap
 * points.
 */
void
adw_swipe_tracker_set_allow_long_swipes (AdwSwipeTracker *self,
                                         gboolean         allow_long_swipes)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  allow_long_swipes = !!allow_long_swipes;

  if (self->allow_long_swipes == allow_long_swipes)
    return;

  self->allow_long_swipes = allow_long_swipes;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_LONG_SWIPES]);
}

/**
 * adw_swipe_tracker_get_lower_overshoot: (attributes org.gtk.Method.get_property=lower-overshoot)
 * @self: a swipe tracker
 *
 * Gets whether to allow swiping past the first available snap point.
 *
 * Returns: whether to allow swiping past the first available snap point
 *
 * Since: 1.4
 */
gboolean
adw_swipe_tracker_get_lower_overshoot (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->lower_overshoot;
}

/**
 * adw_swipe_tracker_set_lower_overshoot: (attributes org.gtk.Method.set_property=lower-overshoot)
 * @self: a swipe tracker
 * @overshoot: whether to allow swiping past the first available snap point
 *
 * Sets whether to allow swiping past the first available snap point.
 *
 * Since: 1.4
 */
void
adw_swipe_tracker_set_lower_overshoot (AdwSwipeTracker *self,
                                       gboolean         overshoot)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  overshoot = !!overshoot;

  if (self->lower_overshoot == overshoot)
    return;

  self->lower_overshoot = overshoot;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LOWER_OVERSHOOT]);
}

/**
 * adw_swipe_tracker_get_upper_overshoot: (attributes org.gtk.Method.get_property=upper-overshoot)
 * @self: a swipe tracker
 *
 * Gets whether to allow swiping past the last available snap point.
 *
 * Returns: whether to allow swiping past the last available snap point
 *
 * Since: 1.4
 */
gboolean
adw_swipe_tracker_get_upper_overshoot (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->upper_overshoot;
}

/**
 * adw_swipe_tracker_set_upper_overshoot: (attributes org.gtk.Method.set_property=upper-overshoot)
 * @self: a swipe tracker
 * @overshoot: whether to allow swiping past the last available snap point
 *
 * Sets whether to allow swiping past the last available snap point.
 *
 * Since: 1.4
 */
void
adw_swipe_tracker_set_upper_overshoot (AdwSwipeTracker *self,
                                       gboolean         overshoot)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  overshoot = !!overshoot;

  if (self->upper_overshoot == overshoot)
    return;

  self->upper_overshoot = overshoot;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UPPER_OVERSHOOT]);
}

/**
 * adw_swipe_tracker_get_allow_window_handle: (attributes org.gtk.Method.get_property=allow-window-handle)
 * @self: a swipe tracker
 *
 * Gets whether to allow touchscreen swiping from `GtkWindowHandle`.
 *
 * Returns: whether swiping from window handles is allowed
 *
 * Since: 1.5
 */
gboolean
adw_swipe_tracker_get_allow_window_handle (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_window_handle;
}

/**
 * adw_swipe_tracker_set_allow_window_handle: (attributes org.gtk.Method.set_property=allow-window-handle)
 * @self: a swipe tracker
 * @allow_window_handle: whether to allow swiping from window handles
 *
 * Sets whether to allow touchscreen swiping from `GtkWindowHandle`.
 *
 * Setting it to `TRUE` will make dragging the window impossible.
 *
 * Since: 1.5
 */
void
adw_swipe_tracker_set_allow_window_handle (AdwSwipeTracker *self,
                                           gboolean         allow_window_handle)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  allow_window_handle = !!allow_window_handle;

  if (self->allow_window_handle == allow_window_handle)
    return;

  self->allow_window_handle = allow_window_handle;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_WINDOW_HANDLE]);
}

/**
 * adw_swipe_tracker_shift_position:
 * @self: a swipe tracker
 * @delta: the position delta
 *
 * Moves the current progress value by @delta.
 *
 * This can be used to adjust the current position if snap points move during
 * the gesture.
 */
void
adw_swipe_tracker_shift_position (AdwSwipeTracker *self,
                                  double           delta)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING &&
      self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  self->progress += delta;
  self->initial_progress += delta;
}

void
adw_swipe_tracker_reset (AdwSwipeTracker *self)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  if (self->touch_gesture_capture)
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (self->touch_gesture_capture));

  if (self->touch_gesture)
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (self->touch_gesture));

  if (self->scroll_controller)
    gtk_event_controller_reset (self->scroll_controller);
}

