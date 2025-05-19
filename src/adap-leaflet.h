/*
 * Copyright (C) 2018 Purism SPC
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
#include "adw-navigation-direction.h"
#include "adw-spring-params.h"

G_BEGIN_DECLS

#define ADW_TYPE_LEAFLET_PAGE (adw_leaflet_page_get_type ())

ADW_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdwLeafletPage, adw_leaflet_page, ADW, LEAFLET_PAGE, GObject)

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_leaflet_page_get_child (AdwLeafletPage *self);

ADW_DEPRECATED_IN_1_4
const char *adw_leaflet_page_get_name (AdwLeafletPage *self);
ADW_DEPRECATED_IN_1_4
void        adw_leaflet_page_set_name (AdwLeafletPage *self,
                                       const char     *name);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_page_get_navigatable (AdwLeafletPage *self);
ADW_DEPRECATED_IN_1_4
void     adw_leaflet_page_set_navigatable (AdwLeafletPage *self,
                                           gboolean        navigatable);

#define ADW_TYPE_LEAFLET (adw_leaflet_get_type())

ADW_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdwLeaflet, adw_leaflet, ADW, LEAFLET, GtkWidget)

typedef enum {
  ADW_LEAFLET_TRANSITION_TYPE_OVER,
  ADW_LEAFLET_TRANSITION_TYPE_UNDER,
  ADW_LEAFLET_TRANSITION_TYPE_SLIDE,
} AdwLeafletTransitionType;

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_leaflet_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_4
AdwLeafletPage *adw_leaflet_append  (AdwLeaflet *self,
                                     GtkWidget  *child);
ADW_DEPRECATED_IN_1_4
AdwLeafletPage *adw_leaflet_prepend (AdwLeaflet *self,
                                     GtkWidget  *child);

ADW_DEPRECATED_IN_1_4
AdwLeafletPage *adw_leaflet_insert_child_after  (AdwLeaflet *self,
                                                 GtkWidget  *child,
                                                 GtkWidget  *sibling);
ADW_DEPRECATED_IN_1_4
void            adw_leaflet_reorder_child_after (AdwLeaflet *self,
                                                  GtkWidget *child,
                                                  GtkWidget *sibling);

ADW_DEPRECATED_IN_1_4
void adw_leaflet_remove (AdwLeaflet *self,
                         GtkWidget  *child);

ADW_DEPRECATED_IN_1_4
AdwLeafletPage *adw_leaflet_get_page (AdwLeaflet *self,
                                      GtkWidget  *child);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_can_unfold (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void     adw_leaflet_set_can_unfold (AdwLeaflet *self,
                                     gboolean    can_unfold);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_folded (AdwLeaflet *self);

ADW_DEPRECATED_IN_1_4
AdwFoldThresholdPolicy adw_leaflet_get_fold_threshold_policy (AdwLeaflet             *self);
ADW_DEPRECATED_IN_1_4
void                   adw_leaflet_set_fold_threshold_policy (AdwLeaflet             *self,
                                                              AdwFoldThresholdPolicy  policy);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_homogeneous (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void     adw_leaflet_set_homogeneous (AdwLeaflet *self,
                                      gboolean    homogeneous);

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_leaflet_get_visible_child (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void       adw_leaflet_set_visible_child (AdwLeaflet *self,
                                          GtkWidget  *visible_child);

ADW_DEPRECATED_IN_1_4
const char *adw_leaflet_get_visible_child_name (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void        adw_leaflet_set_visible_child_name (AdwLeaflet *self,
                                                const char *name);

ADW_DEPRECATED_IN_1_4
AdwLeafletTransitionType adw_leaflet_get_transition_type (AdwLeaflet               *self);
ADW_DEPRECATED_IN_1_4
void                     adw_leaflet_set_transition_type (AdwLeaflet               *self,
                                                          AdwLeafletTransitionType  transition);

ADW_DEPRECATED_IN_1_4
guint adw_leaflet_get_mode_transition_duration (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void  adw_leaflet_set_mode_transition_duration (AdwLeaflet *self,
                                                guint       duration);

ADW_DEPRECATED_IN_1_4
AdwSpringParams *adw_leaflet_get_child_transition_params (AdwLeaflet      *self);
ADW_DEPRECATED_IN_1_4
void             adw_leaflet_set_child_transition_params (AdwLeaflet      *self,
                                                          AdwSpringParams *params);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_child_transition_running (AdwLeaflet *self);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_can_navigate_back (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void     adw_leaflet_set_can_navigate_back (AdwLeaflet *self,
                                            gboolean    can_navigate_back);

ADW_DEPRECATED_IN_1_4
gboolean adw_leaflet_get_can_navigate_forward (AdwLeaflet *self);
ADW_DEPRECATED_IN_1_4
void     adw_leaflet_set_can_navigate_forward (AdwLeaflet *self,
                                               gboolean    can_navigate_forward);

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_leaflet_get_adjacent_child (AdwLeaflet             *self,
                                           AdwNavigationDirection  direction);
ADW_DEPRECATED_IN_1_4
gboolean   adw_leaflet_navigate           (AdwLeaflet             *self,
                                           AdwNavigationDirection  direction);

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_leaflet_get_child_by_name (AdwLeaflet *self,
                                          const char *name);

ADW_DEPRECATED_IN_1_4
GtkSelectionModel *adw_leaflet_get_pages (AdwLeaflet *self) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
