/*
 * Copyright (C) 2019 Purism SPC
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

#include "adw-animation-target.h"
#include "adw-enums.h"

G_BEGIN_DECLS

/**
 * ADW_DURATION_INFINITE:
 *
 * Indicates an [class@Animation] with an infinite duration.
 *
 * This value is mostly used internally.
 */

#define ADW_DURATION_INFINITE ((guint) 0xffffffff)

#define ADW_TYPE_ANIMATION (adw_animation_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwAnimation, adw_animation, ADW, ANIMATION, GObject)

typedef enum {
  ADW_ANIMATION_IDLE,
  ADW_ANIMATION_PAUSED,
  ADW_ANIMATION_PLAYING,
  ADW_ANIMATION_FINISHED,
} AdwAnimationState;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_animation_get_widget (AdwAnimation *self);

ADW_AVAILABLE_IN_ALL
AdwAnimationTarget *adw_animation_get_target (AdwAnimation       *self);
ADW_AVAILABLE_IN_ALL
void                adw_animation_set_target (AdwAnimation       *self,
                                              AdwAnimationTarget *target);

ADW_AVAILABLE_IN_ALL
double adw_animation_get_value (AdwAnimation *self);

ADW_AVAILABLE_IN_ALL
AdwAnimationState adw_animation_get_state (AdwAnimation *self);

ADW_AVAILABLE_IN_ALL
void adw_animation_play   (AdwAnimation *self);
ADW_AVAILABLE_IN_ALL
void adw_animation_pause  (AdwAnimation *self);
ADW_AVAILABLE_IN_ALL
void adw_animation_resume (AdwAnimation *self);
ADW_AVAILABLE_IN_ALL
void adw_animation_reset  (AdwAnimation *self);
ADW_AVAILABLE_IN_ALL
void adw_animation_skip   (AdwAnimation *self);

ADW_AVAILABLE_IN_1_3
gboolean adw_animation_get_follow_enable_animations_setting (AdwAnimation *self);
ADW_AVAILABLE_IN_1_3
void     adw_animation_set_follow_enable_animations_setting (AdwAnimation *self,
                                                             gboolean      setting);

G_END_DECLS
