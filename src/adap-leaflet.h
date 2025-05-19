/*
 * Copyright (C) 2018 Purism SPC
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
#include "adap-navigation-direction.h"
#include "adap-spring-params.h"

G_BEGIN_DECLS

#define ADAP_TYPE_LEAFLET_PAGE (adap_leaflet_page_get_type ())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapLeafletPage, adap_leaflet_page, ADAP, LEAFLET_PAGE, GObject)

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_leaflet_page_get_child (AdapLeafletPage *self);

ADAP_DEPRECATED_IN_1_4
const char *adap_leaflet_page_get_name (AdapLeafletPage *self);
ADAP_DEPRECATED_IN_1_4
void        adap_leaflet_page_set_name (AdapLeafletPage *self,
                                       const char     *name);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_page_get_navigatable (AdapLeafletPage *self);
ADAP_DEPRECATED_IN_1_4
void     adap_leaflet_page_set_navigatable (AdapLeafletPage *self,
                                           gboolean        navigatable);

#define ADAP_TYPE_LEAFLET (adap_leaflet_get_type())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapLeaflet, adap_leaflet, ADAP, LEAFLET, GtkWidget)

typedef enum {
  ADAP_LEAFLET_TRANSITION_TYPE_OVER,
  ADAP_LEAFLET_TRANSITION_TYPE_UNDER,
  ADAP_LEAFLET_TRANSITION_TYPE_SLIDE,
} AdapLeafletTransitionType;

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_leaflet_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_DEPRECATED_IN_1_4
AdapLeafletPage *adap_leaflet_append  (AdapLeaflet *self,
                                     GtkWidget  *child);
ADAP_DEPRECATED_IN_1_4
AdapLeafletPage *adap_leaflet_prepend (AdapLeaflet *self,
                                     GtkWidget  *child);

ADAP_DEPRECATED_IN_1_4
AdapLeafletPage *adap_leaflet_insert_child_after  (AdapLeaflet *self,
                                                 GtkWidget  *child,
                                                 GtkWidget  *sibling);
ADAP_DEPRECATED_IN_1_4
void            adap_leaflet_reorder_child_after (AdapLeaflet *self,
                                                  GtkWidget *child,
                                                  GtkWidget *sibling);

ADAP_DEPRECATED_IN_1_4
void adap_leaflet_remove (AdapLeaflet *self,
                         GtkWidget  *child);

ADAP_DEPRECATED_IN_1_4
AdapLeafletPage *adap_leaflet_get_page (AdapLeaflet *self,
                                      GtkWidget  *child);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_can_unfold (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void     adap_leaflet_set_can_unfold (AdapLeaflet *self,
                                     gboolean    can_unfold);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_folded (AdapLeaflet *self);

ADAP_DEPRECATED_IN_1_4
AdapFoldThresholdPolicy adap_leaflet_get_fold_threshold_policy (AdapLeaflet             *self);
ADAP_DEPRECATED_IN_1_4
void                   adap_leaflet_set_fold_threshold_policy (AdapLeaflet             *self,
                                                              AdapFoldThresholdPolicy  policy);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_homogeneous (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void     adap_leaflet_set_homogeneous (AdapLeaflet *self,
                                      gboolean    homogeneous);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_leaflet_get_visible_child (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void       adap_leaflet_set_visible_child (AdapLeaflet *self,
                                          GtkWidget  *visible_child);

ADAP_DEPRECATED_IN_1_4
const char *adap_leaflet_get_visible_child_name (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void        adap_leaflet_set_visible_child_name (AdapLeaflet *self,
                                                const char *name);

ADAP_DEPRECATED_IN_1_4
AdapLeafletTransitionType adap_leaflet_get_transition_type (AdapLeaflet               *self);
ADAP_DEPRECATED_IN_1_4
void                     adap_leaflet_set_transition_type (AdapLeaflet               *self,
                                                          AdapLeafletTransitionType  transition);

ADAP_DEPRECATED_IN_1_4
guint adap_leaflet_get_mode_transition_duration (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void  adap_leaflet_set_mode_transition_duration (AdapLeaflet *self,
                                                guint       duration);

ADAP_DEPRECATED_IN_1_4
AdapSpringParams *adap_leaflet_get_child_transition_params (AdapLeaflet      *self);
ADAP_DEPRECATED_IN_1_4
void             adap_leaflet_set_child_transition_params (AdapLeaflet      *self,
                                                          AdapSpringParams *params);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_child_transition_running (AdapLeaflet *self);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_can_navigate_back (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void     adap_leaflet_set_can_navigate_back (AdapLeaflet *self,
                                            gboolean    can_navigate_back);

ADAP_DEPRECATED_IN_1_4
gboolean adap_leaflet_get_can_navigate_forward (AdapLeaflet *self);
ADAP_DEPRECATED_IN_1_4
void     adap_leaflet_set_can_navigate_forward (AdapLeaflet *self,
                                               gboolean    can_navigate_forward);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_leaflet_get_adjacent_child (AdapLeaflet             *self,
                                           AdapNavigationDirection  direction);
ADAP_DEPRECATED_IN_1_4
gboolean   adap_leaflet_navigate           (AdapLeaflet             *self,
                                           AdapNavigationDirection  direction);

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_leaflet_get_child_by_name (AdapLeaflet *self,
                                          const char *name);

ADAP_DEPRECATED_IN_1_4
GtkSelectionModel *adap_leaflet_get_pages (AdapLeaflet *self) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
