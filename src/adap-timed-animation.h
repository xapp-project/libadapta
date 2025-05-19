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
#include "adw-easing.h"

G_BEGIN_DECLS

#define ADW_TYPE_TIMED_ANIMATION (adw_timed_animation_get_type())

ADW_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdwTimedAnimation, adw_timed_animation, ADW, TIMED_ANIMATION, AdwAnimation)

ADW_AVAILABLE_IN_ALL
AdwAnimation *adw_timed_animation_new (GtkWidget          *widget,
                                       double              from,
                                       double              to,
                                       guint               duration,
                                       AdwAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
double adw_timed_animation_get_value_from (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_timed_animation_set_value_from (AdwTimedAnimation *self,
                                           double             value);

ADW_AVAILABLE_IN_ALL
double adw_timed_animation_get_value_to (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void   adw_timed_animation_set_value_to (AdwTimedAnimation *self,
                                         double             value);

ADW_AVAILABLE_IN_ALL
guint adw_timed_animation_get_duration (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void  adw_timed_animation_set_duration (AdwTimedAnimation *self,
                                        guint              duration);

ADW_AVAILABLE_IN_ALL
AdwEasing adw_timed_animation_get_easing (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void      adw_timed_animation_set_easing (AdwTimedAnimation *self,
                                          AdwEasing          easing);

ADW_AVAILABLE_IN_ALL
guint adw_timed_animation_get_repeat_count (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void  adw_timed_animation_set_repeat_count (AdwTimedAnimation *self,
                                            guint              repeat_count);

ADW_AVAILABLE_IN_ALL
gboolean adw_timed_animation_get_reverse (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void     adw_timed_animation_set_reverse (AdwTimedAnimation *self,
                                          gboolean           reverse);

ADW_AVAILABLE_IN_ALL
gboolean adw_timed_animation_get_alternate (AdwTimedAnimation *self);
ADW_AVAILABLE_IN_ALL
void     adw_timed_animation_set_alternate (AdwTimedAnimation *self,
                                            gboolean           alternate);

G_END_DECLS
