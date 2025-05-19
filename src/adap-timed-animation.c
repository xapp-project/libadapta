/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-timed-animation.h"

#include "adw-animation-private.h"
#include "adw-animation-util.h"

/**
 * AdwTimedAnimation:
 *
 * A time-based [class@Animation].
 *
 * `AdwTimedAnimation` implements a simple animation interpolating the given
 * value from [property@TimedAnimation:value-from] to
 * [property@TimedAnimation:value-to] over
 * [property@TimedAnimation:duration] milliseconds using the curve described by
 * [property@TimedAnimation:easing].
 *
 * If [property@TimedAnimation:reverse] is set to `TRUE`, `AdwTimedAnimation`
 * will instead animate from [property@TimedAnimation:value-to] to
 * [property@TimedAnimation:value-from], and the easing curve will be inverted.
 *
 * The animation can repeat a certain amount of times, or endlessly, depending
 * on the [property@TimedAnimation:repeat-count] value. If
 * [property@TimedAnimation:alternate] is set to `TRUE`, it will also change the
 * direction every other iteration.
 */

struct _AdwTimedAnimation
{
  AdwAnimation parent_instance;

  double value_from;
  double value_to;
  guint duration; /* ms */
  AdwEasing easing;
  guint repeat_count;
  gboolean reverse;
  gboolean alternate;
};

struct _AdwTimedAnimationClass
{
  AdwAnimationClass parent_class;
};

G_DEFINE_FINAL_TYPE (AdwTimedAnimation, adw_timed_animation, ADW_TYPE_ANIMATION)

enum {
  PROP_0,
  PROP_VALUE_FROM,
  PROP_VALUE_TO,
  PROP_DURATION,
  PROP_EASING,
  PROP_REPEAT_COUNT,
  PROP_REVERSE,
  PROP_ALTERNATE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static guint
adw_timed_animation_estimate_duration (AdwAnimation *animation)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);

  if (self->repeat_count == 0)
    return ADW_DURATION_INFINITE;

  return self->duration * self->repeat_count;
}

static double
adw_timed_animation_calculate_value (AdwAnimation *animation,
                                     guint         t)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);

  double value;
  double iteration, progress;
  gboolean reverse = false;

  if (self->duration == 0)
    return self->value_to;

  progress = modf (((double) t / self->duration), &iteration);

  if (self->alternate)
    reverse = ((int) iteration % 2);

  if (self->reverse)
    reverse = !reverse;

  /* When the animation ends, return the exact final value, which depends on the
     direction the animation is going at that moment, having into account that at the
     time of this check we're already on the next iteration. */
  if (t >= adw_timed_animation_estimate_duration (animation))
    return self->alternate == reverse ? self->value_to : self->value_from;

  progress = reverse ? (1 - progress) : progress;

  value = adw_easing_ease (self->easing, progress);

  return adw_lerp (self->value_from, self->value_to, value);
}

static void
adw_timed_animation_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (object);

  switch (prop_id) {
  case PROP_VALUE_FROM:
    g_value_set_double (value, adw_timed_animation_get_value_from (self));
    break;

  case PROP_VALUE_TO:
    g_value_set_double (value, adw_timed_animation_get_value_to (self));
    break;

  case PROP_DURATION:
    g_value_set_uint (value, adw_timed_animation_get_duration (self));
    break;

  case PROP_EASING:
    g_value_set_enum (value, adw_timed_animation_get_easing (self));
    break;

  case PROP_REPEAT_COUNT:
    g_value_set_uint (value, adw_timed_animation_get_repeat_count (self));
    break;

  case PROP_REVERSE:
    g_value_set_boolean (value, adw_timed_animation_get_reverse (self));
    break;

  case PROP_ALTERNATE:
    g_value_set_boolean (value, adw_timed_animation_get_alternate (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_timed_animation_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (object);

  switch (prop_id) {
  case PROP_VALUE_FROM:
    adw_timed_animation_set_value_from (self, g_value_get_double (value));
    break;

  case PROP_VALUE_TO:
    adw_timed_animation_set_value_to (self, g_value_get_double (value));
    break;

  case PROP_DURATION:
    adw_timed_animation_set_duration (self, g_value_get_uint (value));
    break;

  case PROP_EASING:
    adw_timed_animation_set_easing (self, g_value_get_enum (value));
    break;

  case PROP_REPEAT_COUNT:
    adw_timed_animation_set_repeat_count (self, g_value_get_uint (value));
    break;

  case PROP_REVERSE:
    adw_timed_animation_set_reverse (self, g_value_get_boolean (value));
    break;

  case PROP_ALTERNATE:
    adw_timed_animation_set_alternate (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_timed_animation_class_init (AdwTimedAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AdwAnimationClass *animation_class = ADW_ANIMATION_CLASS (klass);

  object_class->set_property = adw_timed_animation_set_property;
  object_class->get_property = adw_timed_animation_get_property;

  animation_class->estimate_duration = adw_timed_animation_estimate_duration;
  animation_class->calculate_value = adw_timed_animation_calculate_value;

  /**
   * AdwTimedAnimation:value-from: (attributes org.gtk.Property.get=adw_timed_animation_get_value_from org.gtk.Property.set=adw_timed_animation_set_value_from)
   *
   * The value to animate from.
   *
   * The animation will start at this value and end at
   * [property@TimedAnimation:value-to].
   *
   * If [property@TimedAnimation:reverse] is `TRUE`, the animation will end at
   * this value instead.
   */
  props[PROP_VALUE_FROM] =
    g_param_spec_double ("value-from", NULL, NULL,
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:value-to: (attributes org.gtk.Property.get=adw_timed_animation_get_value_to org.gtk.Property.set=adw_timed_animation_set_value_to)
   *
   * The value to animate to.
   *
   * The animation will start at [property@TimedAnimation:value-from] and end at
   * this value.
   *
   * If [property@TimedAnimation:reverse] is `TRUE`, the animation will start
   * at this value instead.
   */
  props[PROP_VALUE_TO] =
    g_param_spec_double ("value-to", NULL, NULL,
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:duration: (attributes org.gtk.Property.get=adw_timed_animation_get_duration org.gtk.Property.set=adw_timed_animation_set_duration)
   *
   * Duration of the animation, in milliseconds.
   *
   * Describes how much time the animation will take.
   *
   * If the animation repeats more than once, describes the duration of one
   * iteration.
   */
  props[PROP_DURATION] =
    g_param_spec_uint ("duration", NULL, NULL,
                       0,
                       ADW_DURATION_INFINITE,
                       0,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:easing: (attributes org.gtk.Property.get=adw_timed_animation_get_easing org.gtk.Property.set=adw_timed_animation_set_easing)
   *
   * Easing function used in the animation.
   *
   * Describes the curve the value is interpolated on.
   *
   * See [enum@Easing] for the description of specific easing functions.
   */
  props[PROP_EASING] =
    g_param_spec_enum ("easing", NULL, NULL,
                       ADW_TYPE_EASING,
                       ADW_EASE_OUT_CUBIC,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:repeat-count: (attributes org.gtk.Property.get=adw_timed_animation_get_repeat_count org.gtk.Property.set=adw_timed_animation_set_repeat_count)
   *
   * Number of times the animation will play.
   *
   * If set to 0, the animation will repeat endlessly.
   */
  props[PROP_REPEAT_COUNT] =
    g_param_spec_uint ("repeat-count", NULL, NULL,
                       0,
                       G_MAXUINT,
                       1,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:reverse: (attributes org.gtk.Property.get=adw_timed_animation_get_reverse org.gtk.Property.set=adw_timed_animation_set_reverse)
   *
   * Whether the animation plays backwards.
   */
  props[PROP_REVERSE] =
    g_param_spec_boolean ("reverse", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTimedAnimation:alternate: (attributes org.gtk.Property.get=adw_timed_animation_get_alternate org.gtk.Property.set=adw_timed_animation_set_alternate)
   *
   * Whether the animation changes direction on every iteration.
   */
  props[PROP_ALTERNATE] =
    g_param_spec_boolean ("alternate", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_timed_animation_init (AdwTimedAnimation *self)
{
}

/**
 * adw_timed_animation_new:
 * @widget: a widget to create animation on
 * @from: a value to animate from
 * @to: a value to animate to
 * @duration: a duration for the animation
 * @target: (transfer full): a target value to animate
 *
 * Creates a new `AdwTimedAnimation` on @widget to animate @target from @from
 * to @to.
 *
 * Returns: (transfer none): the newly created animation
 */
AdwAnimation *
adw_timed_animation_new (GtkWidget          *widget,
                         double              from,
                         double              to,
                         guint               duration,
                         AdwAnimationTarget *target)
{
  AdwAnimation *animation;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (ADW_IS_ANIMATION_TARGET (target), NULL);

  animation = g_object_new (ADW_TYPE_TIMED_ANIMATION,
                            "widget", widget,
                            "value-from", from,
                            "value-to", to,
                            "duration", duration,
                            "target", target,
                            NULL);

  g_object_unref (target);

  return animation;
}

/**
 * adw_timed_animation_get_value_from: (attributes org.gtk.Method.get_property=value-from)
 * @self: a timed animation
 *
 * Gets the value @self will animate from.
 *
 * Returns: the value to animate from
 */
double
adw_timed_animation_get_value_from (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0.0);

  return self->value_from;
}

/**
 * adw_timed_animation_set_value_from: (attributes org.gtk.Method.set_property=value-from)
 * @self: a timed animation
 * @value: the value to animate from
 *
 * Sets the value @self will animate from.
 *
 * The animation will start at this value and end at
 * [property@TimedAnimation:value-to].
 *
 * If [property@TimedAnimation:reverse] is `TRUE`, the animation will end at
 * this value instead.
 */
void
adw_timed_animation_set_value_from (AdwTimedAnimation *self,
                                    double             value)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (G_APPROX_VALUE (self->value_from, value, DBL_EPSILON))
    return;

  self->value_from = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_FROM]);
}

/**
 * adw_timed_animation_get_value_to: (attributes org.gtk.Method.get_property=value-to)
 * @self: a timed animation
 *
 * Gets the value @self will animate to.
 *
 * Returns: the value to animate to
 */
double
adw_timed_animation_get_value_to (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0.0);

  return self->value_to;
}

/**
 * adw_timed_animation_set_value_to: (attributes org.gtk.Method.set_property=value-to)
 * @self: a timed animation
 * @value: the value to animate to
 *
 * Sets the value @self will animate to.
 *
 * The animation will start at [property@TimedAnimation:value-from] and end at
 * this value.
 *
 * If [property@TimedAnimation:reverse] is `TRUE`, the animation will start
 * at this value instead.
 */
void
adw_timed_animation_set_value_to (AdwTimedAnimation *self,
                                  double             value)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (G_APPROX_VALUE (self->value_to, value, DBL_EPSILON))
    return;

  self->value_to = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_TO]);
}

/**
 * adw_timed_animation_get_duration: (attributes org.gtk.Method.get_property=duration)
 * @self: a timed animation
 *
 * Gets the duration of @self.
 *
 * Returns: the duration of @self, in milliseconds
 */
guint
adw_timed_animation_get_duration (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0);

  return self->duration;
}

/**
 * adw_timed_animation_set_duration: (attributes org.gtk.Method.set_property=duration)
 * @self: a timed animation
 * @duration: the duration to use, in milliseconds
 *
 * Sets the duration of @self.
 *
 * If the animation repeats more than once, sets the duration of one iteration.
 */
void
adw_timed_animation_set_duration (AdwTimedAnimation *self,
                                  guint              duration)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->duration == duration)
    return;

  self->duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DURATION]);
}

/**
 * adw_timed_animation_get_easing: (attributes org.gtk.Method.get_property=easing)
 * @self: a timed animation
 *
 * Gets the easing function @self uses.
 *
 * Returns: the easing function @self uses
 */
AdwEasing
adw_timed_animation_get_easing (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self),
                        ADW_LINEAR);

  return self->easing;
}

/**
 * adw_timed_animation_set_easing: (attributes org.gtk.Method.set_property=easing)
 * @self: a timed animation
 * @easing: the easing function to use
 *
 * Sets the easing function @self will use.
 *
 * See [enum@Easing] for the description of specific easing functions.
 */
void
adw_timed_animation_set_easing (AdwTimedAnimation *self,
                                AdwEasing          easing)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));
  g_return_if_fail (easing <= ADW_EASE_IN_OUT_BOUNCE);

  if (self->easing == easing)
    return;

  self->easing = easing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EASING]);
}

/**
 * adw_timed_animation_get_repeat_count: (attributes org.gtk.Method.get_property=repeat-count)
 * @self: a timed animation
 *
 * Gets the number of times @self will play.
 *
 * Returns: the number of times @self will play
 */
guint
adw_timed_animation_get_repeat_count (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0);

  return self->repeat_count;
}

/**
 * adw_timed_animation_set_repeat_count: (attributes org.gtk.Method.set_property=repeat-count)
 * @self: a timed animation
 * @repeat_count: the number of times @self will play
 *
 * Sets the number of times @self will play.
 *
 * If set to 0, @self will repeat endlessly.
 */
void
adw_timed_animation_set_repeat_count (AdwTimedAnimation *self,
                                      guint              repeat_count)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->repeat_count == repeat_count)
    return;

  self->repeat_count = repeat_count;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REPEAT_COUNT]);
}

/**
 * adw_timed_animation_get_reverse: (attributes org.gtk.Method.get_property=reverse)
 * @self: a timed animation
 *
 * Gets whether @self plays backwards.
 *
 * Returns: whether @self plays backwards
 */
gboolean
adw_timed_animation_get_reverse (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), FALSE);

  return self->reverse;
}

/**
 * adw_timed_animation_set_reverse: (attributes org.gtk.Method.set_property=reverse)
 * @self: a timed animation
 * @reverse: whether @self plays backwards
 *
 * Sets whether @self plays backwards.
 */
void
adw_timed_animation_set_reverse (AdwTimedAnimation *self,
                                 gboolean           reverse)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->reverse == reverse)
    return;

  self->reverse = reverse;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVERSE]);
}

/**
 * adw_timed_animation_get_alternate: (attributes org.gtk.Method.get_property=alternate)
 * @self: a timed animation
 *
 * Gets whether @self changes direction on every iteration.
 *
 * Returns: whether @self alternates
 */
gboolean
adw_timed_animation_get_alternate (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), FALSE);

  return self->alternate;
}

/**
 * adw_timed_animation_set_alternate: (attributes org.gtk.Method.set_property=alternate)
 * @self: a timed animation
 * @alternate: whether @self alternates
 *
 * Sets whether @self changes direction on every iteration.
 */
void
adw_timed_animation_set_alternate (AdwTimedAnimation *self,
                                   gboolean           alternate)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->alternate == alternate)
    return;

  self->alternate = alternate;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALTERNATE]);
}
