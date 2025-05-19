/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-animation.h"
#include "adw-spring-params.h"

G_BEGIN_DECLS

#define ADW_TYPE_SPRING_ANIMATION (adw_spring_animation_get_type())

ADW_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdwSpringAnimation, adw_spring_animation, ADW, SPRING_ANIMATION, AdwAnimation)

ADW_AVAILABLE_IN_ALL
AdwAnimation *adw_spring_animation_new (GtkWidget          *widget,
                                        double              from,
                                        double              to,
                                        AdwSpringParams    *spring_params,
                                        AdwAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
double adw_spring_animation_get_value_from (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_spring_animation_set_value_from (AdwSpringAnimation *self,
                                            double             value);

ADW_AVAILABLE_IN_ALL
double adw_spring_animation_get_value_to (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_spring_animation_set_value_to (AdwSpringAnimation *self,
                                          double             value);

ADW_AVAILABLE_IN_ALL
AdwSpringParams *adw_spring_animation_get_spring_params (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void             adw_spring_animation_set_spring_params (AdwSpringAnimation *self,
                                                         AdwSpringParams    *spring_params);

ADW_AVAILABLE_IN_ALL
double adw_spring_animation_get_initial_velocity (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_spring_animation_set_initial_velocity (AdwSpringAnimation *self,
                                                  double              velocity);

ADW_AVAILABLE_IN_ALL
double adw_spring_animation_get_epsilon (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_spring_animation_set_epsilon (AdwSpringAnimation *self,
                                         double              epsilon);

ADW_AVAILABLE_IN_ALL
gboolean adw_spring_animation_get_clamp (AdwSpringAnimation *self);
ADW_AVAILABLE_IN_ALL
void     adw_spring_animation_set_clamp (AdwSpringAnimation *self,
                                         gboolean            clamp);

ADW_AVAILABLE_IN_ALL
guint adw_spring_animation_get_estimated_duration (AdwSpringAnimation *self);

ADW_AVAILABLE_IN_ALL
double adw_spring_animation_get_velocity (AdwSpringAnimation *self);

ADW_AVAILABLE_IN_1_3
double adw_spring_animation_calculate_value    (AdwSpringAnimation *self,
                                                guint              time);
ADW_AVAILABLE_IN_1_3
double adw_spring_animation_calculate_velocity (AdwSpringAnimation *self,
                                                guint              time);

G_END_DECLS
