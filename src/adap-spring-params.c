/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-spring-params.h"

#include <math.h>

G_DEFINE_BOXED_TYPE (AdwSpringParams, adw_spring_params,
                     adw_spring_params_ref, adw_spring_params_unref)

/**
 * AdwSpringParams:
 *
 * Physical parameters of a spring for [class@SpringAnimation].
 *
 * Any spring can be described by three parameters: mass, stiffness and damping.
 *
 * An undamped spring will produce an oscillatory motion which will go on
 * forever.
 *
 * The frequency and amplitude of the oscillations will be determined by the
 * stiffness (how "strong" the spring is) and its mass (how much "inertia" it
 * has).
 *
 * If damping is larger than 0, the amplitude of that oscillating motion will
 * exponientally decrease over time. If that damping is strong enough that the
 * spring can't complete a full oscillation, it's called an overdamped spring.
 *
 * If we the spring can oscillate, it's called an underdamped spring.
 *
 * The value between these two behaviors is called critical damping; a
 * critically damped spring will comes to rest in the minimum possible time
 * without producing oscillations.
 *
 * The damping can be replaced by damping ratio, which produces the following
 * springs:
 *
 * * 0: an undamped spring.
 * * Between 0 and 1: an underdamped spring.
 * * 1: a critically damped spring.
 * * Larger than 1: an overdamped spring.
 *
 * As such
 */

struct _AdwSpringParams
{
  gatomicrefcount ref_count;

  double damping;
  double mass;
  double stiffness;
};

/**
 * adw_spring_params_new:
 * @damping_ratio: the damping ratio of the spring
 * @mass: the mass of the spring
 * @stiffness: the stiffness of the spring
 *
 * Creates a new `AdwSpringParams` from @mass, @stiffness and @damping_ratio.
 *
 * The damping value is calculated from @damping_ratio and the other two
 * parameters.
 *
 * * If @damping_ratio is 0, the spring will not be damped and will oscillate
 *   endlessly.
 * * If @damping_ratio is between 0 and 1, the spring is underdamped and will
 *   always overshoot.
 * * If @damping_ratio is 1, the spring is critically damped and will reach its
 *   resting position the quickest way possible.
 * * If @damping_ratio is larger than 1, the spring is overdamped and will reach
 *   its resting position faster than it can complete an oscillation.
 *
 * [ctor@SpringParams.new_full] allows to pass a raw damping value instead.
 *
 * Returns: (transfer full): the newly created spring parameters
 */
AdwSpringParams *
adw_spring_params_new (double damping_ratio,
                       double mass,
                       double stiffness)
{
  double critical_damping, damping;

  g_return_val_if_fail (G_APPROX_VALUE (damping_ratio, 0.0, DBL_EPSILON) || damping_ratio > 0.0, NULL);

  critical_damping = 2 * sqrt (mass * stiffness);
  damping = damping_ratio * critical_damping;

  return adw_spring_params_new_full (damping, mass, stiffness);
}

/**
 * adw_spring_params_new_full:
 * @damping: the damping of the spring
 * @mass: the mass of the spring
 * @stiffness: the stiffness of the spring
 *
 * Creates a new `AdwSpringParams` from @mass, @stiffness and @damping.
 *
 * See [ctor@SpringParams.new] for a simplified constructor using damping ratio
 * instead of @damping.
 *
 * Returns: (transfer full): the newly created spring parameters
 */
AdwSpringParams *
adw_spring_params_new_full (double damping,
                            double mass,
                            double stiffness)
{
  AdwSpringParams *self;

  g_return_val_if_fail (G_APPROX_VALUE (damping, 0.0, DBL_EPSILON) || damping > 0.0, NULL);
  g_return_val_if_fail (mass > 0.0, NULL);
  g_return_val_if_fail (stiffness > 0.0, NULL);

  self = g_new0 (AdwSpringParams, 1);

  g_atomic_ref_count_init (&self->ref_count);

  self->damping = damping;
  self->mass = mass;
  self->stiffness = stiffness;

  return self;
}

/**
 * adw_spring_params_ref:
 * @self: spring params
 *
 * Increases the reference count of @self.
 *
 * Returns: (transfer full): @self
 */
AdwSpringParams *
adw_spring_params_ref (AdwSpringParams *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  g_atomic_ref_count_inc (&self->ref_count);

  return self;
}

/**
 * adw_spring_params_unref:
 * @self: spring params
 *
 * Decreases the reference count of @self.
 *
 * If the last reference is dropped, the structure is freed.
 */
void
adw_spring_params_unref (AdwSpringParams *self)
{
  g_return_if_fail (self != NULL);

  if (g_atomic_ref_count_dec (&self->ref_count))
    g_free (self);
}

/**
 * adw_spring_params_get_damping:
 * @self: spring params
 *
 * Gets the damping of @self.
 *
 * Returns: the damping
 */
double
adw_spring_params_get_damping (AdwSpringParams *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->damping;
}

/**
 * adw_spring_params_get_damping_ratio:
 * @self: spring params
 *
 * Gets the damping ratio of @self.
 *
 * Returns: the damping ratio
 */
double
adw_spring_params_get_damping_ratio (AdwSpringParams *self)
{
  double critical_damping;

  g_return_val_if_fail (self != NULL, 0.0);

  critical_damping = 2 * sqrt (self->mass * self->stiffness);

  return self->damping / critical_damping;
}

/**
 * adw_spring_params_get_mass:
 * @self: spring params
 *
 * Gets the mass of @self.
 *
 * Returns: the mass
 */
double
adw_spring_params_get_mass (AdwSpringParams *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->mass;
}

/**
 * adw_spring_params_get_stiffness:
 * @self: spring params
 *
 * Gets the stiffness of @self.
 *
 * Returns: the stiffness
 */
double
adw_spring_params_get_stiffness (AdwSpringParams *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->stiffness;
}
