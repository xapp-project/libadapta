/*
 * Copyright (C) 2019 Purism SPC
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

G_BEGIN_DECLS

#define ADAP_TYPE_SQUEEZER_PAGE (adap_squeezer_page_get_type ())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapSqueezerPage, adap_squeezer_page, ADAP, SQUEEZER_PAGE, GObject)

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_squeezer_page_get_child (AdapSqueezerPage *self);

ADAP_DEPRECATED_IN_1_4
gboolean adap_squeezer_page_get_enabled (AdapSqueezerPage *self);
ADAP_DEPRECATED_IN_1_4
void     adap_squeezer_page_set_enabled (AdapSqueezerPage *self,
                                        gboolean         enabled);

#define ADAP_TYPE_SQUEEZER (adap_squeezer_get_type ())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapSqueezer, adap_squeezer, ADAP, SQUEEZER, GtkWidget)

typedef enum {
  ADAP_SQUEEZER_TRANSITION_TYPE_NONE,
  ADAP_SQUEEZER_TRANSITION_TYPE_CROSSFADE,
} AdapSqueezerTransitionType;

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_squeezer_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_DEPRECATED_IN_1_4
AdapSqueezerPage *adap_squeezer_add    (AdapSqueezer *self,
                                      GtkWidget   *child);
ADAP_DEPRECATED_IN_1_4
void             adap_squeezer_remove (AdapSqueezer *self,
                                      GtkWidget   *child);

ADAP_DEPRECATED_IN_1_4
AdapSqueezerPage *adap_squeezer_get_page (AdapSqueezer *self,
                                        GtkWidget   *child);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_squeezer_get_visible_child (AdapSqueezer *self);

ADAP_DEPRECATED_IN_1_4
gboolean adap_squeezer_get_homogeneous (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void     adap_squeezer_set_homogeneous (AdapSqueezer *self,
                                       gboolean     homogeneous);

ADAP_DEPRECATED_IN_1_4
AdapFoldThresholdPolicy adap_squeezer_get_switch_threshold_policy (AdapSqueezer             *self);
ADAP_DEPRECATED_IN_1_4
void                   adap_squeezer_set_switch_threshold_policy (AdapSqueezer            *self,
                                                                 AdapFoldThresholdPolicy  policy);

ADAP_DEPRECATED_IN_1_4
gboolean adap_squeezer_get_allow_none (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void     adap_squeezer_set_allow_none (AdapSqueezer *self,
                                      gboolean     allow_none);

ADAP_DEPRECATED_IN_1_4
guint adap_squeezer_get_transition_duration (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void  adap_squeezer_set_transition_duration (AdapSqueezer *self,
                                            guint        duration);

ADAP_DEPRECATED_IN_1_4
AdapSqueezerTransitionType adap_squeezer_get_transition_type (AdapSqueezer               *self);
ADAP_DEPRECATED_IN_1_4
void                      adap_squeezer_set_transition_type (AdapSqueezer               *self,
                                                            AdapSqueezerTransitionType  transition);

ADAP_DEPRECATED_IN_1_4
gboolean adap_squeezer_get_transition_running (AdapSqueezer *self);

ADAP_DEPRECATED_IN_1_4
gboolean adap_squeezer_get_interpolate_size (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void     adap_squeezer_set_interpolate_size (AdapSqueezer *self,
                                            gboolean     interpolate_size);

ADAP_DEPRECATED_IN_1_4
float adap_squeezer_get_xalign (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void  adap_squeezer_set_xalign (AdapSqueezer *self,
                               float        xalign);

ADAP_DEPRECATED_IN_1_4
float adap_squeezer_get_yalign (AdapSqueezer *self);
ADAP_DEPRECATED_IN_1_4
void  adap_squeezer_set_yalign (AdapSqueezer *self,
                               float        yalign);

ADAP_DEPRECATED_IN_1_4
GtkSelectionModel *adap_squeezer_get_pages (AdapSqueezer *self) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
