/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-animation.h"
#include "adap-spring-params.h"

G_BEGIN_DECLS

#define ADAP_TYPE_SPRING_ANIMATION (adap_spring_animation_get_type())

ADAP_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdapSpringAnimation, adap_spring_animation, ADAP, SPRING_ANIMATION, AdapAnimation)

ADAP_AVAILABLE_IN_ALL
AdapAnimation *adap_spring_animation_new (GtkWidget          *widget,
                                        double              from,
                                        double              to,
                                        AdapSpringParams    *spring_params,
                                        AdapAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
double adap_spring_animation_get_value_from (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_spring_animation_set_value_from (AdapSpringAnimation *self,
                                            double             value);

ADAP_AVAILABLE_IN_ALL
double adap_spring_animation_get_value_to (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_spring_animation_set_value_to (AdapSpringAnimation *self,
                                          double             value);

ADAP_AVAILABLE_IN_ALL
AdapSpringParams *adap_spring_animation_get_spring_params (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void             adap_spring_animation_set_spring_params (AdapSpringAnimation *self,
                                                         AdapSpringParams    *spring_params);

ADAP_AVAILABLE_IN_ALL
double adap_spring_animation_get_initial_velocity (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_spring_animation_set_initial_velocity (AdapSpringAnimation *self,
                                                  double              velocity);

ADAP_AVAILABLE_IN_ALL
double adap_spring_animation_get_epsilon (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_spring_animation_set_epsilon (AdapSpringAnimation *self,
                                         double              epsilon);

ADAP_AVAILABLE_IN_ALL
gboolean adap_spring_animation_get_clamp (AdapSpringAnimation *self);
ADAP_AVAILABLE_IN_ALL
void     adap_spring_animation_set_clamp (AdapSpringAnimation *self,
                                         gboolean            clamp);

ADAP_AVAILABLE_IN_ALL
guint adap_spring_animation_get_estimated_duration (AdapSpringAnimation *self);

ADAP_AVAILABLE_IN_ALL
double adap_spring_animation_get_velocity (AdapSpringAnimation *self);

ADAP_AVAILABLE_IN_1_3
double adap_spring_animation_calculate_value    (AdapSpringAnimation *self,
                                                guint              time);
ADAP_AVAILABLE_IN_1_3
double adap_spring_animation_calculate_velocity (AdapSpringAnimation *self,
                                                guint              time);

G_END_DECLS
