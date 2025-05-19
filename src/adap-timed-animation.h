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
#include "adap-easing.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TIMED_ANIMATION (adap_timed_animation_get_type())

ADAP_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdapTimedAnimation, adap_timed_animation, ADAP, TIMED_ANIMATION, AdapAnimation)

ADAP_AVAILABLE_IN_ALL
AdapAnimation *adap_timed_animation_new (GtkWidget          *widget,
                                       double              from,
                                       double              to,
                                       guint               duration,
                                       AdapAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
double adap_timed_animation_get_value_from (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_timed_animation_set_value_from (AdapTimedAnimation *self,
                                           double             value);

ADAP_AVAILABLE_IN_ALL
double adap_timed_animation_get_value_to (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void   adap_timed_animation_set_value_to (AdapTimedAnimation *self,
                                         double             value);

ADAP_AVAILABLE_IN_ALL
guint adap_timed_animation_get_duration (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void  adap_timed_animation_set_duration (AdapTimedAnimation *self,
                                        guint              duration);

ADAP_AVAILABLE_IN_ALL
AdapEasing adap_timed_animation_get_easing (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void      adap_timed_animation_set_easing (AdapTimedAnimation *self,
                                          AdapEasing          easing);

ADAP_AVAILABLE_IN_ALL
guint adap_timed_animation_get_repeat_count (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void  adap_timed_animation_set_repeat_count (AdapTimedAnimation *self,
                                            guint              repeat_count);

ADAP_AVAILABLE_IN_ALL
gboolean adap_timed_animation_get_reverse (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void     adap_timed_animation_set_reverse (AdapTimedAnimation *self,
                                          gboolean           reverse);

ADAP_AVAILABLE_IN_ALL
gboolean adap_timed_animation_get_alternate (AdapTimedAnimation *self);
ADAP_AVAILABLE_IN_ALL
void     adap_timed_animation_set_alternate (AdapTimedAnimation *self,
                                            gboolean           alternate);

G_END_DECLS
