/*
 * Copyright (C) 2019-2020 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-private.h"

#include "adw-animation-target-private.h"
#include "adw-animation-util.h"
#include "adw-marshalers.h"

/**
 * AdwAnimation:
 *
 * A base class for animations.
 *
 * `AdwAnimation` represents an animation on a widget. It has a target that
 * provides a value to animate, and a state indicating whether the
 * animation hasn't been started yet, is playing, paused or finished.
 *
 * Currently there are two concrete animation types:
 * [class@TimedAnimation] and [class@SpringAnimation].
 *
 * `AdwAnimation` will automatically skip the animation if
 * [property@Animation:widget] is unmapped, or if
 * [property@Gtk.Settings:gtk-enable-animations] is `FALSE`.
 *
 * The [signal@Animation::done] signal can be used to perform an action after
 * the animation ends, for example hiding a widget after animating its
 * [property@Gtk.Widget:opacity] to 0.
 *
 * `AdwAnimation` will be kept alive while the animation is playing. As such,
 * it's safe to create an animation, start it and immediately unref it:
 * A fire-and-forget animation:
 *
 * ```c
 * static void
 * animation_cb (double    value,
 *               MyObject *self)
 * {
 *   // Do something with @value
 * }
 *
 * static void
 * my_object_animate (MyObject *self)
 * {
 *   AdwAnimationTarget *target =
 *     adw_callback_animation_target_new ((AdwAnimationTargetFunc) animation_cb,
 *                                        self, NULL);
 *   g_autoptr (AdwAnimation) animation =
 *     adw_timed_animation_new (widget, 0, 1, 250, target);
 *
 *   adw_animation_play (animation);
 * }
 * ```
 *
 * If there's a chance the previous animation for the same target hasn't yet
 * finished, the previous animation should be stopped first, or the existing
 * `AdwAnimation` object can be reused.
 */

/**
 * AdwAnimationState:
 * @ADW_ANIMATION_IDLE: The animation hasn't started yet.
 * @ADW_ANIMATION_PAUSED: The animation has been paused.
 * @ADW_ANIMATION_PLAYING: The animation is currently playing.
 * @ADW_ANIMATION_FINISHED: The animation has finished.
 *
 * Describes the possible states of an [class@Animation].
 *
 * The state can be controlled with [method@Animation.play],
 * [method@Animation.pause], [method@Animation.resume],
 * [method@Animation.reset] and [method@Animation.skip].
 */

typedef struct
{
  GtkWidget *widget;

  double value;

  gint64 start_time; /* ms */
  gint64 paused_time;
  guint tick_cb_id;
  gulong unmap_cb_id;

  AdwAnimationTarget *target;
  gpointer user_data;

  AdwAnimationState state;

  gboolean follow_enable_animations_setting;
} AdwAnimationPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (AdwAnimation, adw_animation, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_WIDGET,
  PROP_TARGET,
  PROP_VALUE,
  PROP_STATE,
  PROP_FOLLOW_ENABLE_ANIMATIONS_SETTING,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_DONE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
widget_notify_cb (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->widget = NULL;
}

static void
set_widget (AdwAnimation *self,
            GtkWidget    *widget)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->widget == widget)
    return;

  if (priv->widget)
    g_object_weak_unref (G_OBJECT (priv->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);

  priv->widget = widget;

  if (priv->widget)
    g_object_weak_ref (G_OBJECT (priv->widget),
                       (GWeakNotify) widget_notify_cb,
                       self);
}

static void
set_value (AdwAnimation *self,
           guint         t)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->value = ADW_ANIMATION_GET_CLASS (self)->calculate_value (self, t);

  adw_animation_target_set_value (priv->target, priv->value);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static void
stop_animation (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->tick_cb_id) {
    gtk_widget_remove_tick_callback (priv->widget, priv->tick_cb_id);
    priv->tick_cb_id = 0;
  }

  if (priv->unmap_cb_id) {
    g_signal_handler_disconnect (priv->widget, priv->unmap_cb_id);
    priv->unmap_cb_id = 0;
  }
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         AdwAnimation  *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000; /* ms */
  guint duration = ADW_ANIMATION_GET_CLASS (self)->estimate_duration (self);
  guint t = (guint) (frame_time - priv->start_time);

  if (t >= duration && duration != ADW_DURATION_INFINITE) {
    adw_animation_skip (self);

    return G_SOURCE_REMOVE;
  }

  set_value (self, t);

  return G_SOURCE_CONTINUE;
}

static guint
adw_animation_estimate_duration (AdwAnimation *animation)
{
  g_assert_not_reached ();
}

static double
adw_animation_calculate_value (AdwAnimation *animation,
                               guint         t)
{
  g_assert_not_reached ();
}

static void
play (AdwAnimation *self)
{

  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_PLAYING) {
    g_critical ("Trying to play animation %p, but it's already playing", self);

    return;
  }

  priv->state = ADW_ANIMATION_PLAYING;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  if ((priv->follow_enable_animations_setting &&
       !adw_get_enable_animations (priv->widget)) ||
      !gtk_widget_get_mapped (priv->widget)) {
    adw_animation_skip (g_object_ref (self));

    return;
  }

  priv->start_time += gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (priv->widget)) / 1000;
  priv->start_time -= priv->paused_time;

  if (priv->tick_cb_id)
    return;

  priv->unmap_cb_id =
    g_signal_connect_swapped (priv->widget, "unmap",
                              G_CALLBACK (adw_animation_skip), self);
  priv->tick_cb_id = gtk_widget_add_tick_callback (priv->widget, (GtkTickCallback) tick_cb, self, NULL);

  g_object_ref (self);
}

static void
adw_animation_constructed (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  G_OBJECT_CLASS (adw_animation_parent_class)->constructed (object);

  priv->value = ADW_ANIMATION_GET_CLASS (self)->calculate_value (self, 0);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static void
adw_animation_dispose (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_PLAYING)
    adw_animation_skip (self);

  g_clear_object (&priv->target);

  set_widget (self, NULL);

  G_OBJECT_CLASS (adw_animation_parent_class)->dispose (object);
}

static void
adw_animation_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwAnimation *self = ADW_ANIMATION (object);

  switch (prop_id) {
  case PROP_WIDGET:
    g_value_set_object (value, adw_animation_get_widget (self));
    break;

  case PROP_TARGET:
    g_value_set_object (value, adw_animation_get_target (self));
    break;

  case PROP_VALUE:
    g_value_set_double (value, adw_animation_get_value (self));
    break;

  case PROP_STATE:
    g_value_set_enum (value, adw_animation_get_state (self));
    break;

  case PROP_FOLLOW_ENABLE_ANIMATIONS_SETTING:
    g_value_set_boolean (value, adw_animation_get_follow_enable_animations_setting (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_animation_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwAnimation *self = ADW_ANIMATION (object);

  switch (prop_id) {
  case PROP_WIDGET:
    set_widget (self, g_value_get_object (value));
    break;

  case PROP_TARGET:
    adw_animation_set_target (ADW_ANIMATION (self), g_value_get_object (value));
    break;

  case PROP_FOLLOW_ENABLE_ANIMATIONS_SETTING:
    adw_animation_set_follow_enable_animations_setting (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_animation_class_init (AdwAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_animation_constructed;
  object_class->dispose = adw_animation_dispose;
  object_class->set_property = adw_animation_set_property;
  object_class->get_property = adw_animation_get_property;

  klass->estimate_duration = adw_animation_estimate_duration;
  klass->calculate_value = adw_animation_calculate_value;

  /**
   * AdwAnimation:widget: (attributes org.gtk.Property.get=adw_animation_get_widget)
   *
   * The animation widget.
   *
   * It provides the frame clock for the animation. It's not strictly necessary
   * for this widget to be same as the one being animated.
   *
   * The widget must be mapped in order for the animation to work. If it's not
   * mapped, or if it gets unmapped during an ongoing animation, the animation
   * will be automatically skipped.
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwAnimation:target: (attributes org.gtk.Property.get=adw_animation_get_target org.gtk.Property.set=adw_animation_set_target)
   *
   * The target to animate.
   */
  props[PROP_TARGET] =
    g_param_spec_object ("target", NULL, NULL,
                         ADW_TYPE_ANIMATION_TARGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAnimation:value: (attributes org.gtk.Property.get=adw_animation_get_value)
   *
   * The current value of the animation.
   */
  props[PROP_VALUE] =
    g_param_spec_double ("value", NULL, NULL,
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwAnimation:state: (attributes org.gtk.Property.get=adw_animation_get_state)
   *
   * The animation state.
   *
   * The state indicates whether the animation is currently playing, paused,
   * finished or hasn't been started yet.
   */
  props[PROP_STATE] =
    g_param_spec_enum ("state", NULL, NULL,
                       ADW_TYPE_ANIMATION_STATE,
                       ADW_ANIMATION_IDLE,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwAnimation:follow-enable-animations-setting: (attributes org.gtk.Property.get=adw_animation_get_follow_enable_animations_setting org.gtk.Property.set=adw_animation_set_follow_enable_animations_setting)
   *
   * Whether to skip the animation when animations are globally disabled.
   *
   * The default behavior is to skip the animation. Set to `FALSE` to disable
   * this behavior.
   *
   * This can be useful for cases where animation is essential, like spinners,
   * or in demo applications. Most other animations should keep it enabled.
   *
   * See [property@Gtk.Settings:gtk-enable-animations].
   *
   * Since: 1.3
   */
  props[PROP_FOLLOW_ENABLE_ANIMATIONS_SETTING] =
    g_param_spec_boolean ("follow-enable-animations-setting", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwAnimation::done:
   *
   * This signal is emitted when the animation has been completed, either on its
   * own or via calling [method@Animation.skip].
   */
  signals[SIGNAL_DONE] =
    g_signal_new ("done",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_DONE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);
}

static void
adw_animation_init (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->state = ADW_ANIMATION_IDLE;
  priv->follow_enable_animations_setting = TRUE;
}

/**
 * adw_animation_get_widget: (attributes org.gtk.Method.get_property=widget)
 * @self: an animation
 *
 * Gets the widget @self was created for.
 *
 * It provides the frame clock for the animation. It's not strictly necessary
 * for this widget to be same as the one being animated.
 *
 * The widget must be mapped in order for the animation to work. If it's not
 * mapped, or if it gets unmapped during an ongoing animation, the animation
 * will be automatically skipped.
 *
 * Returns: (transfer none): the animation widget
 */
GtkWidget *
adw_animation_get_widget (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->widget;
}

/**
 * adw_animation_get_target: (attributes org.gtk.Method.get_property=target)
 * @self: an animation
 *
 * Gets the target @self animates.
 *
 * Returns: (transfer none): the animation target
 */
AdwAnimationTarget *
adw_animation_get_target (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->target;
}

/**
 * adw_animation_set_target: (attributes org.gtk.Method.set_property=target)
 * @self: an animation
 * @target: an animation target
 *
 * Sets the target @self animates to @target.
 */
void
adw_animation_set_target (AdwAnimation       *self,
                          AdwAnimationTarget *target)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));
  g_return_if_fail (ADW_IS_ANIMATION_TARGET (target));

  priv = adw_animation_get_instance_private (self);

  if (target == priv->target)
    return;

  g_set_object (&priv->target, target);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TARGET]);
}

/**
 * adw_animation_get_value: (attributes org.gtk.Method.get_property=value)
 * @self: an animation
 *
 * Gets the current value of @self.
 *
 * Returns: the current value
 */
double
adw_animation_get_value (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0.0);

  priv = adw_animation_get_instance_private (self);

  return priv->value;
}

/**
 * adw_animation_get_state: (attributes org.gtk.Method.get_property=state)
 * @self: an animation
 *
 * Gets the current value of @self.
 *
 * The state indicates whether @self is currently playing, paused, finished or
 * hasn't been started yet.
 *
 * Returns: the animation value
 */
AdwAnimationState
adw_animation_get_state (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), ADW_ANIMATION_IDLE);

  priv = adw_animation_get_instance_private (self);

  return priv->state;
}

/**
 * adw_animation_play:
 * @self: an animation
 *
 * Starts the animation for @self.
 *
 * If the animation is playing, paused or has been completed, restarts it from
 * the beginning. This allows to easily play an animation regardless of whether
 * it's already playing or not.
 *
 * Sets [property@Animation:state] to `ADW_ANIMATION_PLAYING`.
 *
 * The animation will be automatically skipped if [property@Animation:widget] is
 * unmapped, or if [property@Gtk.Settings:gtk-enable-animations] is `FALSE`.
 *
 * As such, it's not guaranteed that the animation will actually run. For
 * example, when using [func@GLib.idle_add] and starting an animation
 * immediately afterwards, it's entirely possible that the idle callback will
 * run after the animation has already finished, and not while it's playing.
 */
void
adw_animation_play (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_IDLE) {
    priv->state = ADW_ANIMATION_IDLE;
    priv->start_time = 0;
    priv->paused_time = 0;
  }

  play (self);
}

/**
 * adw_animation_pause:
 * @self: an animation
 *
 * Pauses a playing animation for @self.
 *
 * Does nothing if the current state of @self isn't `ADW_ANIMATION_PLAYING`.
 *
 * Sets [property@Animation:state] to `ADW_ANIMATION_PAUSED`.
 */
void
adw_animation_pause (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_PLAYING)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  priv->state = ADW_ANIMATION_PAUSED;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  priv->paused_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (priv->widget)) / 1000;

  g_object_thaw_notify (G_OBJECT (self));

  g_object_unref (self);
}

/**
 * adw_animation_resume:
 * @self: an animation
 *
 * Resumes a paused animation for @self.
 *
 * This function must only be used if the animation has been paused with
 * [method@Animation.pause].
 *
 * Sets [property@Animation:state] to `ADW_ANIMATION_PLAYING`.
 */
void
adw_animation_resume (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_PAUSED) {
    g_critical ("Trying to resume animation %p, but it's not paused", self);

    return;
  }

  play (self);
}

/**
 * adw_animation_skip:
 * @self: an animation
 *
 * Skips the animation for @self.
 *
 * If the animation hasn't been started yet, is playing, or is paused, instantly
 * skips the animation to the end and causes [signal@Animation::done] to be
 * emitted.
 *
 * Sets [property@Animation:state] to `ADW_ANIMATION_FINISHED`.
 */
void
adw_animation_skip (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;
  gboolean was_playing;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_FINISHED)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  was_playing = priv->state == ADW_ANIMATION_PLAYING;

  priv->state = ADW_ANIMATION_FINISHED;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  set_value (self, ADW_ANIMATION_GET_CLASS (self)->estimate_duration (self));

  priv->start_time = 0;
  priv->paused_time = 0;

  g_object_thaw_notify (G_OBJECT (self));

  g_signal_emit (self, signals[SIGNAL_DONE], 0);

  if (was_playing)
    g_object_unref (self);
}

/**
 * adw_animation_reset:
 * @self: an animation
 *
 * Resets the animation for @self.
 *
 * Sets [property@Animation:state] to `ADW_ANIMATION_IDLE`.
 */
void
adw_animation_reset (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;
  gboolean was_playing;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_IDLE)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  was_playing = priv->state == ADW_ANIMATION_PLAYING;

  priv->state = ADW_ANIMATION_IDLE;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  set_value (self, 0);
  priv->start_time = 0;
  priv->paused_time = 0;

  g_object_thaw_notify (G_OBJECT (self));

  if (was_playing)
    g_object_unref (self);
}

/**
 * adw_animation_get_follow_enable_animations_setting: (attributes org.gtk.Method.get_property=follow-enable-animations-setting)
 * @self: an animation
 *
 * Gets whether @self should be skipped when animations are globally disabled.
 *
 * Returns: whether to follow the global setting
 *
 * Since: 1.3
 */
gboolean
adw_animation_get_follow_enable_animations_setting (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), FALSE);

  priv = adw_animation_get_instance_private (self);

  return priv->follow_enable_animations_setting;
}

/**
 * adw_animation_set_follow_enable_animations_setting: (attributes org.gtk.Method.set_property=follow-enable-animations-setting)
 * @self: an animation
 * @setting: whether to follow the global setting
 *
 * Sets whether to skip @self when animations are globally disabled.
 *
 * The default behavior is to skip the animation. Set to `FALSE` to disable this
 * behavior.
 *
 * This can be useful for cases where animation is essential, like spinners, or
 * in demo applications. Most other animations should keep it enabled.
 *
 * See [property@Gtk.Settings:gtk-enable-animations].
 *
 * Since: 1.3
 */
void
adw_animation_set_follow_enable_animations_setting (AdwAnimation *self,
                                                    gboolean      setting)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  setting = !!setting;

  if (setting == priv->follow_enable_animations_setting)
    return;

  priv->follow_enable_animations_setting = setting;

  if (setting &&
      !adw_get_enable_animations (priv->widget) &&
      priv->state != ADW_ANIMATION_IDLE)
    adw_animation_skip (g_object_ref (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLLOW_ENABLE_ANIMATIONS_SETTING]);
}
