/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-floating-sheet-private.h"

#include "adw-animation-target.h"
#include "adw-animation-util.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-spring-animation.h"
#include "adw-widget-utils-private.h"

#include <math.h>

#define MIN_SCALE 0.8

#define HORZ_PADDING_MIN_WIDTH 720
#define HORZ_PADDING_MIN_VALUE 30
#define HORZ_PADDING_TARGET_WIDTH 1440
#define HORZ_PADDING_TARGET_VALUE 120

#define VERT_PADDING_MIN_HEIGHT 720
#define VERT_PADDING_MIN_VALUE 30
#define VERT_PADDING_TARGET_HEIGHT 1440
#define VERT_PADDING_TARGET_VALUE 120

struct _AdwFloatingSheet
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkWidget *sheet_bin;
  GtkWidget *dimming;

  gboolean open;
  gboolean can_close;

  AdwAnimation *open_animation;
  double progress;
};

G_DEFINE_FINAL_TYPE (AdwFloatingSheet, adw_floating_sheet, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_CHILD,
  PROP_OPEN,
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
open_animation_cb (double            value,
                   AdwFloatingSheet *self)
{
  self->progress = value;

  gtk_widget_set_opacity (self->dimming, CLAMP (value, 0, 1));
  gtk_widget_set_opacity (self->sheet_bin, CLAMP (value, 0, 1));
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
open_animation_done_cb (AdwFloatingSheet *self)
{
  if (self->progress < 0.5) {
    gtk_widget_set_child_visible (self->dimming, FALSE);
    gtk_widget_set_child_visible (self->sheet_bin, FALSE);

    g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
  }
}

static void
sheet_close_cb (AdwFloatingSheet *self)
{
  GtkWidget *parent;

  if (!self->can_close) {
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
    return;
  }

  if (self->open) {
    adw_floating_sheet_set_open (self, FALSE);
    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "sheet.close", NULL);
}

static void
adw_floating_sheet_measure (GtkWidget      *widget,
                            GtkOrientation  orientation,
                            int             for_size,
                            int            *minimum,
                            int            *natural,
                            int            *minimum_baseline,
                            int            *natural_baseline)
{
  AdwFloatingSheet *self = ADW_FLOATING_SHEET (widget);
  int dim_min, dim_nat, sheet_min, sheet_nat;

  gtk_widget_measure (self->dimming, orientation, for_size,
                      &dim_min, &dim_nat, NULL, NULL);

  gtk_widget_measure (self->sheet_bin, orientation, for_size,
                      &sheet_min, &sheet_nat, NULL, NULL);

  if (minimum)
    *minimum = MAX (dim_min, sheet_min);
  if (natural)
    *natural = MAX (dim_nat, sheet_nat);
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_floating_sheet_size_allocate (GtkWidget *widget,
                                  int        width,
                                  int        height,
                                  int        baseline)
{
  AdwFloatingSheet *self = ADW_FLOATING_SHEET (widget);
  GskTransform *transform;
  int sheet_x, sheet_y, sheet_min_width, sheet_width, sheet_min_height, sheet_height;
  int horz_padding, vert_padding;
  float scale;

  if (width == 0 && height == 0)
    return;

  gtk_widget_allocate (self->dimming, width, height, baseline, NULL);

  horz_padding = adw_lerp (HORZ_PADDING_MIN_VALUE,
                           HORZ_PADDING_TARGET_VALUE,
                           MAX (0, (width - HORZ_PADDING_MIN_WIDTH) /
                                   (double) (HORZ_PADDING_TARGET_WIDTH -
                                             HORZ_PADDING_MIN_WIDTH)));
  vert_padding = adw_lerp (VERT_PADDING_MIN_VALUE,
                           VERT_PADDING_TARGET_VALUE,
                           MAX (0, (height - VERT_PADDING_MIN_HEIGHT) /
                                   (double) (VERT_PADDING_TARGET_HEIGHT -
                                             VERT_PADDING_MIN_HEIGHT)));

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sheet_min_width, &sheet_width, NULL, NULL);

  sheet_width = MAX (sheet_min_width, MIN (sheet_width, width - horz_padding * 2));

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  sheet_height = MAX (sheet_min_height, MIN (sheet_height, height - vert_padding * 2));

  sheet_x = round ((width - sheet_width) * 0.5);
  sheet_y = round ((height - sheet_height) * 0.5);

  scale = MIN_SCALE + (1 - MIN_SCALE) * self->progress;
  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width / 2.0f, height / 2.0f));
  transform = gsk_transform_scale (transform, scale, scale);
  transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-width / 2.0f, -height / 2.0f));
  transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (sheet_x, sheet_y));
  gtk_widget_allocate (self->sheet_bin, sheet_width, sheet_height, baseline, transform);
}

static void
adw_floating_sheet_dispose (GObject *object)
{
  AdwFloatingSheet *self = ADW_FLOATING_SHEET (object);

  g_clear_pointer (&self->dimming, gtk_widget_unparent);
  g_clear_pointer (&self->sheet_bin, gtk_widget_unparent);
  g_clear_object (&self->open_animation);
  self->child = NULL;

  G_OBJECT_CLASS (adw_floating_sheet_parent_class)->dispose (object);
}

static void
adw_floating_sheet_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwFloatingSheet *self = ADW_FLOATING_SHEET (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_floating_sheet_get_child (self));
    break;
  case PROP_OPEN:
    g_value_set_boolean (value, adw_floating_sheet_get_open (self));
    break;
  case PROP_CAN_CLOSE:
    g_value_set_boolean (value, adw_floating_sheet_get_can_close (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_floating_sheet_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwFloatingSheet *self = ADW_FLOATING_SHEET (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_floating_sheet_set_child (self, g_value_get_object (value));
    break;
  case PROP_OPEN:
    adw_floating_sheet_set_open (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_CLOSE:
    adw_floating_sheet_set_can_close (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_floating_sheet_class_init (AdwFloatingSheetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_floating_sheet_dispose;
  object_class->get_property = adw_floating_sheet_get_property;
  object_class->set_property = adw_floating_sheet_set_property;

  widget_class->contains = adw_widget_contains_passthrough;
  widget_class->measure = adw_floating_sheet_measure;
  widget_class->size_allocate = adw_floating_sheet_size_allocate;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;
  widget_class->focus = adw_widget_focus_child;
  widget_class->grab_focus = adw_widget_grab_focus_child;

  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_OPEN] =
    g_param_spec_boolean ("open", NULL, NULL,
                          FALSE,
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
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSING],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  signals[SIGNAL_CLOSED] =
    g_signal_new ("closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  signals[SIGNAL_CLOSE_ATTEMPT] =
    g_signal_new ("close-attempt",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_ATTEMPT],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_install_action (widget_class, "sheet.close", NULL,
                                   (GtkWidgetActionActivateFunc) sheet_close_cb);

  gtk_widget_class_set_css_name (widget_class, "floating-sheet");
}

static void
adw_floating_sheet_init (AdwFloatingSheet *self)
{
  AdwAnimationTarget *target;

  self->can_close = TRUE;

  self->dimming = g_object_new (GTK_TYPE_WINDOW_HANDLE,
                                "css-name", "dimming",
                                NULL);
  gtk_widget_set_opacity (self->dimming, 0);
  gtk_widget_set_child_visible (self->dimming, FALSE);
  gtk_widget_set_can_target (self->dimming, FALSE);
  gtk_widget_set_parent (self->dimming, GTK_WIDGET (self));

  self->sheet_bin = adw_gizmo_new_with_role ("sheet", GTK_ACCESSIBLE_ROLE_GENERIC,
                                             NULL, NULL, NULL, NULL,
                                             (AdwGizmoFocusFunc) adw_widget_focus_child,
                                             (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child_or_self);
  gtk_widget_set_focusable (self->sheet_bin, TRUE);
  gtk_widget_set_opacity (self->sheet_bin, 0);
  gtk_widget_set_layout_manager (self->sheet_bin, gtk_bin_layout_new ());
  gtk_widget_add_css_class (self->sheet_bin, "background");
  gtk_widget_set_overflow (self->sheet_bin, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_child_visible (self->sheet_bin, FALSE);
  gtk_widget_set_parent (self->sheet_bin, GTK_WIDGET (self));

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) open_animation_cb,
                                              self,
                                              NULL);

  self->open_animation = adw_spring_animation_new (GTK_WIDGET (self),
                                                   0,
                                                   1,
                                                   adw_spring_params_new (0.62, 1, 500),
                                                   target);
  adw_spring_animation_set_epsilon (ADW_SPRING_ANIMATION (self->open_animation), 0.01);
  g_signal_connect_swapped (self->open_animation, "done",
                            G_CALLBACK (open_animation_done_cb), self);
}

GtkWidget *
adw_floating_sheet_new (void)
{
  return g_object_new (ADW_TYPE_FLOATING_SHEET, NULL);
}

GtkWidget *
adw_floating_sheet_get_child (AdwFloatingSheet *self)
{
  g_return_val_if_fail (ADW_IS_FLOATING_SHEET (self), NULL);

  return self->child;
}

void
adw_floating_sheet_set_child (AdwFloatingSheet *self,
                              GtkWidget        *child)
{
  g_return_if_fail (ADW_IS_FLOATING_SHEET (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  if (self->child)
    gtk_widget_unparent (self->child);

  self->child = child;

  if (self->child)
    gtk_widget_set_parent (self->child, self->sheet_bin);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

gboolean
adw_floating_sheet_get_open (AdwFloatingSheet *self)
{
  g_return_val_if_fail (ADW_IS_FLOATING_SHEET (self), FALSE);

  return self->open;
}

void
adw_floating_sheet_set_open (AdwFloatingSheet *self,
                             gboolean          open)
{
  g_return_if_fail (ADW_IS_FLOATING_SHEET (self));

  open = !!open;

  if (self->open == open)
    return;

  self->open = open;

  if (open) {
    gtk_widget_set_child_visible (self->dimming, TRUE);
    gtk_widget_set_child_visible (self->sheet_bin, TRUE);
  }

  gtk_widget_set_can_target (self->dimming, open);
  gtk_widget_set_can_target (self->sheet_bin, open);

  if (!open) {
    g_signal_emit (self, signals[SIGNAL_CLOSING], 0);

    if (self->open != open)
      return;
  }

  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->open_animation),
                                       self->progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->open_animation),
                                     open ? 1 : 0);
  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->open_animation),
                                  !open);
  adw_animation_play (self->open_animation);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_OPEN]);
}

gboolean
adw_floating_sheet_get_can_close (AdwFloatingSheet *self)
{
  g_return_val_if_fail (ADW_IS_FLOATING_SHEET (self), FALSE);

  return self->can_close;
}

void
adw_floating_sheet_set_can_close (AdwFloatingSheet *self,
                                  gboolean          can_close)
{
  g_return_if_fail (ADW_IS_FLOATING_SHEET (self));

  can_close = !!can_close;

  if (self->can_close == can_close)
    return;

  self->can_close = can_close;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_CLOSE]);
}

GtkWidget *
adw_floating_sheet_get_sheet_bin (AdwFloatingSheet *self)
{
  g_return_val_if_fail (ADW_IS_FLOATING_SHEET (self), NULL);

  return self->sheet_bin;
}
