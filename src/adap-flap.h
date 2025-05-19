/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-enums.h"
#include "adap-fold-threshold-policy.h"
#include "adap-spring-params.h"

G_BEGIN_DECLS

#define ADAP_TYPE_FLAP (adap_flap_get_type ())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapFlap, adap_flap, ADAP, FLAP, GtkWidget)

typedef enum {
  ADAP_FLAP_FOLD_POLICY_NEVER,
  ADAP_FLAP_FOLD_POLICY_ALWAYS,
  ADAP_FLAP_FOLD_POLICY_AUTO,
} AdapFlapFoldPolicy;

typedef enum {
  ADAP_FLAP_TRANSITION_TYPE_OVER,
  ADAP_FLAP_TRANSITION_TYPE_UNDER,
  ADAP_FLAP_TRANSITION_TYPE_SLIDE,
} AdapFlapTransitionType;

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_flap_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_flap_get_content (AdapFlap   *self);
ADAP_DEPRECATED_IN_1_4
void       adap_flap_set_content (AdapFlap   *self,
                                 GtkWidget *content);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_flap_get_flap (AdapFlap   *self);
ADAP_DEPRECATED_IN_1_4
void       adap_flap_set_flap (AdapFlap   *self,
                              GtkWidget *flap);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_flap_get_separator (AdapFlap   *self);
ADAP_DEPRECATED_IN_1_4
void       adap_flap_set_separator (AdapFlap   *self,
                                   GtkWidget *separator);

ADAP_DEPRECATED_IN_1_4
GtkPackType adap_flap_get_flap_position (AdapFlap     *self);
ADAP_DEPRECATED_IN_1_4
void        adap_flap_set_flap_position (AdapFlap     *self,
                                        GtkPackType  position);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_reveal_flap (AdapFlap  *self);
ADAP_DEPRECATED_IN_1_4
void     adap_flap_set_reveal_flap (AdapFlap  *self,
                                   gboolean  reveal_flap);

ADAP_DEPRECATED_IN_1_4
AdapSpringParams *adap_flap_get_reveal_params (AdapFlap         *self);
ADAP_DEPRECATED_IN_1_4
void             adap_flap_set_reveal_params (AdapFlap         *self,
                                             AdapSpringParams *params);

ADAP_DEPRECATED_IN_1_4
double adap_flap_get_reveal_progress (AdapFlap *self);

ADAP_DEPRECATED_IN_1_4
AdapFlapFoldPolicy adap_flap_get_fold_policy (AdapFlap           *self);
ADAP_DEPRECATED_IN_1_4
void              adap_flap_set_fold_policy (AdapFlap           *self,
                                            AdapFlapFoldPolicy  policy);

ADAP_DEPRECATED_IN_1_4
AdapFoldThresholdPolicy adap_flap_get_fold_threshold_policy (AdapFlap                *self);
ADAP_DEPRECATED_IN_1_4
void                   adap_flap_set_fold_threshold_policy (AdapFlap                *self,
                                                           AdapFoldThresholdPolicy  policy);

ADAP_DEPRECATED_IN_1_4
guint adap_flap_get_fold_duration (AdapFlap *self);
ADAP_DEPRECATED_IN_1_4
void  adap_flap_set_fold_duration (AdapFlap *self,
                                  guint    duration);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_folded (AdapFlap *self);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_locked (AdapFlap *self);
ADAP_DEPRECATED_IN_1_4
void     adap_flap_set_locked (AdapFlap  *self,
                              gboolean  locked);

ADAP_DEPRECATED_IN_1_4
AdapFlapTransitionType adap_flap_get_transition_type (AdapFlap               *self);
ADAP_DEPRECATED_IN_1_4
void                  adap_flap_set_transition_type (AdapFlap               *self,
                                                    AdapFlapTransitionType  transition_type);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_modal (AdapFlap  *self);
ADAP_DEPRECATED_IN_1_4
void     adap_flap_set_modal (AdapFlap  *self,
                             gboolean  modal);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_swipe_to_open (AdapFlap  *self);
ADAP_DEPRECATED_IN_1_4
void     adap_flap_set_swipe_to_open (AdapFlap  *self,
                                     gboolean  swipe_to_open);

ADAP_DEPRECATED_IN_1_4
gboolean adap_flap_get_swipe_to_close (AdapFlap  *self);
ADAP_DEPRECATED_IN_1_4
void     adap_flap_set_swipe_to_close (AdapFlap  *self,
                                      gboolean  swipe_to_close);

G_END_DECLS
