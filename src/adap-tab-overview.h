/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_OVERVIEW (adap_tab_overview_get_type())

ADAP_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdapTabOverview, adap_tab_overview, ADAP, TAB_OVERVIEW, GtkWidget)

ADAP_AVAILABLE_IN_1_3
GtkWidget *adap_tab_overview_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_3
AdapTabView *adap_tab_overview_get_view (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void        adap_tab_overview_set_view (AdapTabOverview *self,
                                       AdapTabView     *view);

ADAP_AVAILABLE_IN_1_3
GtkWidget *adap_tab_overview_get_child (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void       adap_tab_overview_set_child (AdapTabOverview *self,
                                       GtkWidget      *child);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_open  (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_open (AdapTabOverview *self,
                                    gboolean        open);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_inverted (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_inverted (AdapTabOverview *self,
                                        gboolean        inverted);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_enable_search (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_enable_search (AdapTabOverview *self,
                                             gboolean        enable_search);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_search_active (AdapTabOverview *self);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_enable_new_tab (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_enable_new_tab (AdapTabOverview *self,
                                              gboolean        enable_new_tab);

ADAP_AVAILABLE_IN_1_3
GMenuModel *adap_tab_overview_get_secondary_menu (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void        adap_tab_overview_set_secondary_menu (AdapTabOverview *self,
                                                 GMenuModel     *secondary_menu);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_show_start_title_buttons (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_show_start_title_buttons (AdapTabOverview *self,
                                                        gboolean        show_start_title_buttons);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_show_end_title_buttons (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_show_end_title_buttons (AdapTabOverview *self,
                                                      gboolean        show_end_title_buttons);

ADAP_AVAILABLE_IN_1_3
void adap_tab_overview_setup_extra_drop_target (AdapTabOverview *self,
                                               GdkDragAction   actions,
                                               GType          *types,
                                               gsize           n_types);

ADAP_AVAILABLE_IN_1_4
GdkDragAction adap_tab_overview_get_extra_drag_preferred_action (AdapTabOverview *self);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_overview_get_extra_drag_preload (AdapTabOverview *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_overview_set_extra_drag_preload (AdapTabOverview *self,
                                                  gboolean        preload);

G_END_DECLS
