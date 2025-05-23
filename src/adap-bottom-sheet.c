/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adap-bottom-sheet-private.h"

#include <math.h>

#include "adap-animation-target.h"
#include "adap-animation-util.h"
#include "adap-gizmo-private.h"
#include "adap-marshalers.h"
#include "adap-spring-animation.h"
#include "adap-swipeable.h"
#include "adap-swipe-tracker-private.h"
#include "adap-widget-utils-private.h"

#define TOP_PADDING_MIN_HEIGHT 720
#define TOP_PADDING_MIN_VALUE 30
#define TOP_PADDING_TARGET_HEIGHT 1440
#define TOP_PADDING_TARGET_VALUE 120

struct _AdapBottomSheet
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkWidget *sheet;

  GtkWidget *sheet_bin;
  GtkWidget *dimming;

  GtkWidget *drag_handle;
  GtkWidget *outline;

  gboolean open;

  AdapAnimation *open_animation;
  double progress;
  float align;

  gboolean show_drag_handle;
  gboolean modal;
  gboolean can_close;

  AdapSwipeTracker *swipe_tracker;
  gboolean swipe_active;

  int min_natural_width;
};

static void adap_bottom_sheet_buildable_init (GtkBuildableIface *iface);
static void adap_bottom_sheet_swipeable_init (AdapSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapBottomSheet, adap_bottom_sheet, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_bottom_sheet_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADAP_TYPE_SWIPEABLE, adap_bottom_sheet_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_SHEET,
  PROP_OPEN,
  PROP_ALIGN,
  PROP_SHOW_DRAG_HANDLE,
  PROP_MODAL,
  PROP_CAN_CLOSE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CLOSING,
  SIGNAL_CLOSED,
  SIGNAL_CLOSE_ATTEMPT,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
released_cb (GtkGestureClick *gesture,
             int              n_press,
             double           x,
             double           y,
             AdapBottomSheet  *self)
{
  if (self->swipe_active)
    return;

  if (!self->can_close)
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
  else
    adap_bottom_sheet_set_open (self, FALSE);

  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
open_animation_cb (double          value,
                   AdapBottomSheet *self)
{
  self->progress = value;

  gtk_widget_set_opacity (self->dimming, CLAMP (value, 0, 1));
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
open_animation_done_cb (AdapBottomSheet *self)
{
  if (self->progress < 0.5) {
    gtk_widget_set_child_visible (self->dimming, FALSE);
    gtk_widget_set_child_visible (self->sheet_bin, FALSE);

    g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
  }
}

static void
sheet_close_cb (AdapBottomSheet *self)
{
  GtkWidget *parent;

  if (!self->can_close) {
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
    return;
  }

  if (self->open) {
    adap_bottom_sheet_set_open (self, FALSE);
    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "sheet.close", NULL);
}

static void
measure_sheet (GtkWidget      *widget,
               GtkOrientation  orientation,
               int             for_size,
               int            *minimum,
               int            *natural,
               int            *minimum_baseline,
               int            *natural_baseline)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (gtk_widget_get_parent (widget));
  int sheet_min, sheet_nat, handle_min, handle_nat, outline_min, outline_nat;

  if (self->sheet && gtk_widget_should_layout (self->sheet)) {
    gtk_widget_measure (self->sheet, orientation, for_size,
                        &sheet_min, &sheet_nat, NULL, NULL);
  } else {
    sheet_min = sheet_nat = 0;
  }

  if (self->min_natural_width >= 0)
    sheet_nat = MAX (sheet_nat, self->min_natural_width);

  if (gtk_widget_should_layout (self->drag_handle)) {
    gtk_widget_measure (self->drag_handle, orientation, for_size,
                        &handle_min, &handle_nat, NULL, NULL);
  } else {
    handle_min = handle_nat = 0;
  }

  if (gtk_widget_should_layout (self->outline)) {
    gtk_widget_measure (self->outline, orientation, for_size,
                        &outline_min, &outline_nat, NULL, NULL);
  } else {
    outline_min = outline_nat = 0;
  }

  if (minimum)
    *minimum = MAX (sheet_min, MAX (outline_min, handle_min));
  if (natural)
    *natural = MAX (sheet_nat, MAX (outline_nat, handle_nat));
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_sheet (GtkWidget *widget,
                int        width,
                int        height,
                int        baseline)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (gtk_widget_get_parent (widget));

  if (gtk_widget_should_layout (self->drag_handle)) {
    int handle_width, handle_height, handle_x;
    GskTransform *transform;

    gtk_widget_measure (self->drag_handle, GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &handle_width, NULL, NULL);
    gtk_widget_measure (self->drag_handle, GTK_ORIENTATION_VERTICAL, -1,
                        NULL, &handle_height, NULL, NULL);

    handle_width = MIN (handle_width, width);
    handle_height = MIN (handle_height, height);

    handle_x = round ((width - handle_width) / 2);

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (handle_x, 0));

    gtk_widget_allocate (self->drag_handle, handle_width, handle_height,
                         baseline, transform);
  }

  if (self->sheet && gtk_widget_should_layout (self->sheet))
    gtk_widget_allocate (self->sheet, width, height, baseline, NULL);

  if (gtk_widget_should_layout (self->outline))
    gtk_widget_allocate (self->outline, width, height, baseline, NULL);
}

static void
adap_bottom_sheet_measure (GtkWidget      *widget,
                          GtkOrientation  orientation,
                          int             for_size,
                          int            *minimum,
                          int            *natural,
                          int            *minimum_baseline,
                          int            *natural_baseline)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (widget);
  int child_min, child_nat, dim_min, dim_nat, sheet_min, sheet_nat;

  if (self->child && gtk_widget_should_layout (self->child)) {
    gtk_widget_measure (self->child, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
  } else {
    child_min = child_nat = 0;
  }

  gtk_widget_measure (self->dimming, orientation, for_size,
                      &dim_min, &dim_nat, NULL, NULL);

  gtk_widget_measure (self->sheet_bin, orientation, for_size,
                      &sheet_min, &sheet_nat, NULL, NULL);

  if (minimum)
    *minimum = MAX (child_min, MAX (dim_min, sheet_min));
  if (natural)
    *natural = MAX (child_nat, MAX (dim_nat, sheet_nat));
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adap_bottom_sheet_size_allocate (GtkWidget *widget,
                                int        width,
                                int        height,
                                int        baseline)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (widget);
  GskTransform *transform;
  int sheet_x, sheet_y, sheet_min_width, sheet_width, sheet_min_height, sheet_height;
  int top_padding;
  float align;

  if (width == 0 && height == 0)
    return;

  if (self->child && gtk_widget_should_layout (self->child))
    gtk_widget_allocate (self->child, width, height, baseline, NULL);

  gtk_widget_allocate (self->dimming, width, height, baseline, NULL);

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sheet_min_width, &sheet_width, NULL, NULL);

  sheet_width = MAX (MIN (sheet_width, width), sheet_min_width);

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  top_padding = adap_lerp (TOP_PADDING_MIN_VALUE,
                          TOP_PADDING_TARGET_VALUE,
                          MAX (0, (height - TOP_PADDING_MIN_HEIGHT) /
                                  (double) (TOP_PADDING_TARGET_HEIGHT -
                                            TOP_PADDING_MIN_HEIGHT)));

  sheet_height = MAX (MIN (sheet_height, height - top_padding), sheet_min_height);
  sheet_y = height - round (sheet_height * self->progress);
  sheet_height = MAX (sheet_height, height - sheet_y);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    align = 1 - self->align;
  else
    align = self->align;

  sheet_x = round ((width - sheet_width) * align);

  if (sheet_x == 0)
    gtk_widget_add_css_class (GTK_WIDGET (self->outline), "flush-left");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self->outline), "flush-left");

  if (sheet_x == width - sheet_width)
    gtk_widget_add_css_class (GTK_WIDGET (self->outline), "flush-right");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self->outline), "flush-right");

  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sheet_x, sheet_y));
  gtk_widget_allocate (self->sheet_bin, sheet_width, sheet_height, baseline, transform);
}

static void
adap_bottom_sheet_dispose (GObject *object)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (object);

  g_clear_pointer (&self->child, gtk_widget_unparent);
  g_clear_pointer (&self->dimming, gtk_widget_unparent);
  g_clear_pointer (&self->sheet_bin, gtk_widget_unparent);
  g_clear_object (&self->open_animation);
  g_clear_object (&self->swipe_tracker);
  self->sheet = NULL;
  self->drag_handle = NULL;

  G_OBJECT_CLASS (adap_bottom_sheet_parent_class)->dispose (object);
}

static void
adap_bottom_sheet_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_bottom_sheet_get_child (self));
    break;
  case PROP_SHEET:
    g_value_set_object (value, adap_bottom_sheet_get_sheet (self));
    break;
  case PROP_OPEN:
    g_value_set_boolean (value, adap_bottom_sheet_get_open (self));
    break;
  case PROP_ALIGN:
    g_value_set_float (value, adap_bottom_sheet_get_align (self));
    break;
  case PROP_SHOW_DRAG_HANDLE:
    g_value_set_boolean (value, adap_bottom_sheet_get_show_drag_handle (self));
    break;
  case PROP_MODAL:
    g_value_set_boolean (value, adap_bottom_sheet_get_modal (self));
    break;
  case PROP_CAN_CLOSE:
    g_value_set_boolean (value, adap_bottom_sheet_get_can_close (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_bottom_sheet_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_bottom_sheet_set_child (self, g_value_get_object (value));
    break;
  case PROP_SHEET:
    adap_bottom_sheet_set_sheet (self, g_value_get_object (value));
    break;
  case PROP_OPEN:
    adap_bottom_sheet_set_open (self, g_value_get_boolean (value));
    break;
  case PROP_ALIGN:
    adap_bottom_sheet_set_align (self, g_value_get_float (value));
    break;
  case PROP_SHOW_DRAG_HANDLE:
    adap_bottom_sheet_set_show_drag_handle (self, g_value_get_boolean (value));
    break;
  case PROP_MODAL:
    adap_bottom_sheet_set_modal (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_CLOSE:
    adap_bottom_sheet_set_can_close (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_bottom_sheet_class_init (AdapBottomSheetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_bottom_sheet_dispose;
  object_class->get_property = adap_bottom_sheet_get_property;
  object_class->set_property = adap_bottom_sheet_set_property;

  widget_class->contains = adap_widget_contains_passthrough;
  widget_class->measure = adap_bottom_sheet_measure;
  widget_class->size_allocate = adap_bottom_sheet_size_allocate;
  widget_class->get_request_mode = adap_widget_get_request_mode;
  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->focus = adap_widget_focus_child;
  widget_class->grab_focus = adap_widget_grab_focus_child;

  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SHEET] =
    g_param_spec_object ("sheet", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_OPEN] =
    g_param_spec_boolean ("open", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0, 1, 0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SHOW_DRAG_HANDLE] =
    g_param_spec_boolean ("show-drag-handle", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_MODAL] =
    g_param_spec_boolean ("modal", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CAN_CLOSE] =
    g_param_spec_boolean ("can-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  signals[SIGNAL_CLOSING] =
    g_signal_new ("closing",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSING],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  signals[SIGNAL_CLOSED] =
    g_signal_new ("closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  signals[SIGNAL_CLOSE_ATTEMPT] =
    g_signal_new ("close-attempt",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_ATTEMPT],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  gtk_widget_class_install_action (widget_class, "sheet.close", NULL,
                                   (GtkWidgetActionActivateFunc) sheet_close_cb);

  gtk_widget_class_set_css_name (widget_class, "bottom-sheet");
}

static void
prepare_cb (AdapSwipeTracker        *tracker,
            AdapNavigationDirection  direction,
            AdapBottomSheet         *self)
{
  if (adap_animation_get_state (self->open_animation) == ADAP_ANIMATION_PLAYING &&
      adap_spring_animation_get_value_to (ADAP_SPRING_ANIMATION (self->open_animation)) < 0.5)
    return;

  self->swipe_active = TRUE;
}

static void
begin_swipe_cb (AdapSwipeTracker *tracker,
                AdapBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  adap_animation_pause (self->open_animation);
}

static void
update_swipe_cb (AdapSwipeTracker *tracker,
                 double           progress,
                 AdapBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  open_animation_cb (progress, self);
}

static void
end_swipe_cb (AdapSwipeTracker *tracker,
              double           velocity,
              double           to,
              AdapBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  self->swipe_active = FALSE;

  adap_spring_animation_set_initial_velocity (ADAP_SPRING_ANIMATION (self->open_animation),
                                             velocity);

  if ((to > 0.5) != self->open) {
    adap_bottom_sheet_set_open (self, to > 0.5);
    return;
  }

  adap_spring_animation_set_value_from (ADAP_SPRING_ANIMATION (self->open_animation),
                                       self->progress);
  adap_spring_animation_set_value_to (ADAP_SPRING_ANIMATION (self->open_animation),
                                     to);
  adap_spring_animation_set_clamp (ADAP_SPRING_ANIMATION (self->open_animation),
                                  to < 0.5);
  adap_animation_play (self->open_animation);
}

static void
adap_bottom_sheet_init (AdapBottomSheet *self)
{
  AdapAnimationTarget *target;
  GtkEventController *gesture;

  self->align = 0.5;
  self->show_drag_handle = TRUE;
  self->modal = TRUE;
  self->can_close = TRUE;

  self->dimming = adap_gizmo_new ("dimming", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_opacity (self->dimming, 0);
  gtk_widget_set_child_visible (self->dimming, FALSE);
  gtk_widget_set_can_target (self->dimming, FALSE);
  gtk_widget_set_parent (self->dimming, GTK_WIDGET (self));

  gesture = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture),
                                              GTK_PHASE_CAPTURE);
  g_signal_connect_object (gesture, "released", G_CALLBACK (released_cb), self, 0);
  gtk_widget_add_controller (self->dimming, gesture);

  self->sheet_bin = adap_gizmo_new ("sheet", NULL, NULL, NULL, NULL,
                                   (AdapGizmoFocusFunc) adap_widget_focus_child,
                                   (AdapGizmoGrabFocusFunc) adap_widget_grab_focus_child_or_self);
  gtk_widget_set_focusable (self->sheet_bin, TRUE);
  gtk_widget_set_layout_manager (self->sheet_bin,
                                 gtk_custom_layout_new (adap_widget_get_request_mode,
                                                        measure_sheet,
                                                        allocate_sheet));
  gtk_widget_add_css_class (self->sheet_bin, "background");
  gtk_widget_set_overflow (self->sheet_bin, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_child_visible (self->sheet_bin, FALSE);
  gtk_widget_set_parent (self->sheet_bin, GTK_WIDGET (self));

  self->drag_handle = adap_gizmo_new ("drag-handle", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_parent (self->drag_handle, self->sheet_bin);

  self->outline = adap_gizmo_new ("outline", NULL, NULL, NULL,
                                 (AdapGizmoContainsFunc) adap_widget_contains_passthrough,
                                 NULL, NULL);
  gtk_widget_set_can_target (self->outline, FALSE);
  gtk_widget_set_can_focus (self->outline, FALSE);
  gtk_widget_set_parent (self->outline, self->sheet_bin);

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc) open_animation_cb,
                                              self,
                                              NULL);

  self->open_animation = adap_spring_animation_new (GTK_WIDGET (self),
                                                   0,
                                                   1,
                                                   adap_spring_params_new (0.8, 1, 400),
                                                   target);
  adap_spring_animation_set_epsilon (ADAP_SPRING_ANIMATION (self->open_animation), 0.0001);
  g_signal_connect_swapped (self->open_animation, "done",
                            G_CALLBACK (open_animation_done_cb), self);

  self->swipe_tracker = adap_swipe_tracker_new (ADAP_SWIPEABLE (self));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->swipe_tracker),
                                  GTK_ORIENTATION_VERTICAL);
  adap_swipe_tracker_set_upper_overshoot (self->swipe_tracker, TRUE);
  adap_swipe_tracker_set_allow_window_handle (self->swipe_tracker, TRUE);

  g_signal_connect (self->swipe_tracker, "prepare", G_CALLBACK (prepare_cb), self);
  g_signal_connect (self->swipe_tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self);
}

static void
adap_bottom_sheet_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  if (!g_strcmp0 (type, "sheet"))
    adap_bottom_sheet_set_sheet (ADAP_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adap_bottom_sheet_set_child (ADAP_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_bottom_sheet_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_bottom_sheet_buildable_add_child;
}

static int
get_sheet_width (AdapBottomSheet *self)
{
  int sheet_width, sheet_min_width, width;

  width = gtk_widget_get_width (GTK_WIDGET (self));

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sheet_min_width, &sheet_width, NULL, NULL);

  return MAX (MIN (sheet_width, width), sheet_min_width);
}

static double
adap_bottom_sheet_get_distance (AdapSwipeable *swipeable)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (swipeable);
  int height, sheet_width, sheet_min_height, sheet_height;

  height = gtk_widget_get_height (GTK_WIDGET (self));

  sheet_width = get_sheet_width (self);

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  return MAX (MIN (sheet_height, height), sheet_min_height);
}

static double *
adap_bottom_sheet_get_snap_points (AdapSwipeable *swipeable,
                                  int          *n_snap_points)
{
  double *points;

  points = g_new0 (double, 2);

  if (n_snap_points)
    *n_snap_points = 2;

  points[0] = 0;
  points[1] = 1;

  return points;
}

static double
adap_bottom_sheet_get_progress (AdapSwipeable *swipeable)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (swipeable);

  return self->progress;
}

static double
adap_bottom_sheet_get_cancel_progress (AdapSwipeable *swipeable)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (swipeable);

  return round (self->progress);
}

static void
adap_bottom_sheet_get_swipe_area (AdapSwipeable           *swipeable,
                                 AdapNavigationDirection  navigation_direction,
                                 gboolean                is_drag,
                                 GdkRectangle           *rect)
{
  AdapBottomSheet *self = ADAP_BOTTOM_SHEET (swipeable);
  int width, height, sheet_width, sheet_min_height, sheet_height;
  int sheet_x, sheet_y;
  float align;

  if (!is_drag) {
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;

    return;
  }

  sheet_width = get_sheet_width (self);
  width = gtk_widget_get_width (GTK_WIDGET (self));
  height = gtk_widget_get_height (GTK_WIDGET (self));

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    align = 1 - self->align;
  else
    align = self->align;

  sheet_x = round ((width - sheet_width) * align);

  sheet_height = MAX (MIN (sheet_height, height), sheet_min_height);
  sheet_y = height - round (sheet_height * self->progress);

  rect->x = sheet_x;
  rect->y = sheet_y;
  rect->width = sheet_width;
  rect->height = height - sheet_y;
}

static void
adap_bottom_sheet_swipeable_init (AdapSwipeableInterface *iface)
{
  iface->get_distance = adap_bottom_sheet_get_distance;
  iface->get_snap_points = adap_bottom_sheet_get_snap_points;
  iface->get_progress = adap_bottom_sheet_get_progress;
  iface->get_cancel_progress = adap_bottom_sheet_get_cancel_progress;
  iface->get_swipe_area = adap_bottom_sheet_get_swipe_area;
}

/**
 * adap_bottom_sheet_new:
 *
 * Creates a new `AdapBottomSheet`.
 *
 * Returns: the new created `AdapBottomSheet`
 *
 * Since: 1.5
 */
GtkWidget *
adap_bottom_sheet_new (void)
{
  return g_object_new (ADAP_TYPE_BOTTOM_SHEET, NULL);
}

/**
 * adap_bottom_sheet_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a bottom sheet
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.5
 */
GtkWidget *
adap_bottom_sheet_get_child (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), NULL);

  return self->child;
}

/**
 * adap_bottom_sheet_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a bottom sheet
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.5
 */
void
adap_bottom_sheet_set_child (AdapBottomSheet *self,
                            GtkWidget      *child)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  if (self->child)
    gtk_widget_unparent (self->child);

  self->child = child;

  if (self->child)
    gtk_widget_insert_before (self->child, GTK_WIDGET (self), self->dimming);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

GtkWidget *
adap_bottom_sheet_get_sheet (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), NULL);

  return self->sheet;
}

void
adap_bottom_sheet_set_sheet (AdapBottomSheet *self,
                            GtkWidget      *sheet)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));
  g_return_if_fail (sheet == NULL || GTK_IS_WIDGET (sheet));

  if (sheet)
    g_return_if_fail (gtk_widget_get_parent (sheet) == NULL);

  if (self->sheet == sheet)
    return;

  if (self->sheet)
    gtk_widget_unparent (self->sheet);

  self->sheet = sheet;

  if (self->sheet)
    gtk_widget_insert_before (self->sheet, self->sheet_bin, self->drag_handle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHEET]);
}

gboolean
adap_bottom_sheet_get_open (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), FALSE);

  return self->open;
}

void
adap_bottom_sheet_set_open (AdapBottomSheet *self,
                           gboolean        open)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  open = !!open;

  if (self->open == open)
    return;

  self->open = open;

  if (open) {
    gtk_widget_set_child_visible (self->dimming, self->modal);
    gtk_widget_set_child_visible (self->sheet_bin, TRUE);
  }

  gtk_widget_set_can_target (self->dimming, open);

  if (!open) {
    g_signal_emit (self, signals[SIGNAL_CLOSING], 0);

    if (self->open != open)
      return;
  }

  adap_spring_animation_set_value_from (ADAP_SPRING_ANIMATION (self->open_animation),
                                       self->progress);
  adap_spring_animation_set_value_to (ADAP_SPRING_ANIMATION (self->open_animation),
                                     open ? 1 : 0);
  adap_spring_animation_set_clamp (ADAP_SPRING_ANIMATION (self->open_animation),
                                  !open);
  adap_animation_play (self->open_animation);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_OPEN]);
}

float
adap_bottom_sheet_get_align (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), 0.0f);

  return self->align;
}

void
adap_bottom_sheet_set_align (AdapBottomSheet *self,
                            float           align)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  if (G_APPROX_VALUE (align, self->align, FLT_EPSILON))
    return;

  self->align = align;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}

gboolean
adap_bottom_sheet_get_show_drag_handle (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), FALSE);

  return self->show_drag_handle;
}

void
adap_bottom_sheet_set_show_drag_handle (AdapBottomSheet *self,
                                       gboolean        show_drag_handle)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  show_drag_handle = !!show_drag_handle;

  if (self->show_drag_handle == show_drag_handle)
    return;

  self->show_drag_handle = show_drag_handle;

  gtk_widget_set_visible (self->drag_handle, show_drag_handle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_DRAG_HANDLE]);
}

gboolean
adap_bottom_sheet_get_modal (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), FALSE);

  return self->modal;
}

void
adap_bottom_sheet_set_modal (AdapBottomSheet *self,
                            gboolean        modal)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  modal = !!modal;

  if (self->modal == modal)
    return;

  self->modal = modal;

  if (!G_APPROX_VALUE (self->progress, 0, DBL_EPSILON))
    gtk_widget_set_child_visible (self->dimming, self->modal);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODAL]);
}

gboolean
adap_bottom_sheet_get_can_close (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), FALSE);

  return self->can_close;
}

void
adap_bottom_sheet_set_can_close (AdapBottomSheet *self,
                                gboolean        can_close)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  can_close = !!can_close;

  if (self->can_close == can_close)
    return;

  self->can_close = can_close;

  adap_swipe_tracker_set_enabled (self->swipe_tracker, can_close);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_CLOSE]);
}

void
adap_bottom_sheet_set_min_natural_width (AdapBottomSheet *self,
                                        int             min_natural_width)
{
  g_return_if_fail (ADAP_IS_BOTTOM_SHEET (self));

  self->min_natural_width = min_natural_width;
}

GtkWidget *
adap_bottom_sheet_get_sheet_bin (AdapBottomSheet *self)
{
  g_return_val_if_fail (ADAP_IS_BOTTOM_SHEET (self), NULL);

  return self->sheet_bin;
}
