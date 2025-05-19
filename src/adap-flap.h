/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-enums.h"
#include "adw-fold-threshold-policy.h"
#include "adw-spring-params.h"

G_BEGIN_DECLS

#define ADW_TYPE_FLAP (adw_flap_get_type ())

ADW_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdwFlap, adw_flap, ADW, FLAP, GtkWidget)

typedef enum {
  ADW_FLAP_FOLD_POLICY_NEVER,
  ADW_FLAP_FOLD_POLICY_ALWAYS,
  ADW_FLAP_FOLD_POLICY_AUTO,
} AdwFlapFoldPolicy;

typedef enum {
  ADW_FLAP_TRANSITION_TYPE_OVER,
  ADW_FLAP_TRANSITION_TYPE_UNDER,
  ADW_FLAP_TRANSITION_TYPE_SLIDE,
} AdwFlapTransitionType;

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_flap_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_flap_get_content (AdwFlap   *self);
ADW_DEPRECATED_IN_1_4
void       adw_flap_set_content (AdwFlap   *self,
                                 GtkWidget *content);

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_flap_get_flap (AdwFlap   *self);
ADW_DEPRECATED_IN_1_4
void       adw_flap_set_flap (AdwFlap   *self,
                              GtkWidget *flap);

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_flap_get_separator (AdwFlap   *self);
ADW_DEPRECATED_IN_1_4
void       adw_flap_set_separator (AdwFlap   *self,
                                   GtkWidget *separator);

ADW_DEPRECATED_IN_1_4
GtkPackType adw_flap_get_flap_position (AdwFlap     *self);
ADW_DEPRECATED_IN_1_4
void        adw_flap_set_flap_position (AdwFlap     *self,
                                        GtkPackType  position);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_reveal_flap (AdwFlap  *self);
ADW_DEPRECATED_IN_1_4
void     adw_flap_set_reveal_flap (AdwFlap  *self,
                                   gboolean  reveal_flap);

ADW_DEPRECATED_IN_1_4
AdwSpringParams *adw_flap_get_reveal_params (AdwFlap         *self);
ADW_DEPRECATED_IN_1_4
void             adw_flap_set_reveal_params (AdwFlap         *self,
                                             AdwSpringParams *params);

ADW_DEPRECATED_IN_1_4
double adw_flap_get_reveal_progress (AdwFlap *self);

ADW_DEPRECATED_IN_1_4
AdwFlapFoldPolicy adw_flap_get_fold_policy (AdwFlap           *self);
ADW_DEPRECATED_IN_1_4
void              adw_flap_set_fold_policy (AdwFlap           *self,
                                            AdwFlapFoldPolicy  policy);

ADW_DEPRECATED_IN_1_4
AdwFoldThresholdPolicy adw_flap_get_fold_threshold_policy (AdwFlap                *self);
ADW_DEPRECATED_IN_1_4
void                   adw_flap_set_fold_threshold_policy (AdwFlap                *self,
                                                           AdwFoldThresholdPolicy  policy);

ADW_DEPRECATED_IN_1_4
guint adw_flap_get_fold_duration (AdwFlap *self);
ADW_DEPRECATED_IN_1_4
void  adw_flap_set_fold_duration (AdwFlap *self,
                                  guint    duration);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_folded (AdwFlap *self);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_locked (AdwFlap *self);
ADW_DEPRECATED_IN_1_4
void     adw_flap_set_locked (AdwFlap  *self,
                              gboolean  locked);

ADW_DEPRECATED_IN_1_4
AdwFlapTransitionType adw_flap_get_transition_type (AdwFlap               *self);
ADW_DEPRECATED_IN_1_4
void                  adw_flap_set_transition_type (AdwFlap               *self,
                                                    AdwFlapTransitionType  transition_type);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_modal (AdwFlap  *self);
ADW_DEPRECATED_IN_1_4
void     adw_flap_set_modal (AdwFlap  *self,
                             gboolean  modal);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_swipe_to_open (AdwFlap  *self);
ADW_DEPRECATED_IN_1_4
void     adw_flap_set_swipe_to_open (AdwFlap  *self,
                                     gboolean  swipe_to_open);

ADW_DEPRECATED_IN_1_4
gboolean adw_flap_get_swipe_to_close (AdwFlap  *self);
ADW_DEPRECATED_IN_1_4
void     adw_flap_set_swipe_to_close (AdwFlap  *self,
                                      gboolean  swipe_to_close);

G_END_DECLS
