/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-enums.h"
#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_BAR (adap_tab_bar_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapTabBar, adap_tab_bar, ADAP, TAB_BAR, GtkWidget)

ADAP_AVAILABLE_IN_ALL
AdapTabBar *adap_tab_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapTabView *adap_tab_bar_get_view (AdapTabBar  *self);
ADAP_AVAILABLE_IN_ALL
void        adap_tab_bar_set_view (AdapTabBar  *self,
                                  AdapTabView *view);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_tab_bar_get_start_action_widget (AdapTabBar *self);
ADAP_AVAILABLE_IN_ALL
void       adap_tab_bar_set_start_action_widget (AdapTabBar *self,
                                                GtkWidget *widget);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_tab_bar_get_end_action_widget (AdapTabBar *self);
ADAP_AVAILABLE_IN_ALL
void       adap_tab_bar_set_end_action_widget (AdapTabBar *self,
                                              GtkWidget *widget);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_bar_get_autohide (AdapTabBar *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_bar_set_autohide (AdapTabBar *self,
                                   gboolean   autohide);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_bar_get_tabs_revealed (AdapTabBar *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_bar_get_expand_tabs (AdapTabBar *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_bar_set_expand_tabs (AdapTabBar *self,
                                      gboolean   expand_tabs);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_bar_get_inverted (AdapTabBar *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_bar_set_inverted (AdapTabBar *self,
                                   gboolean   inverted);

ADAP_AVAILABLE_IN_ALL
void adap_tab_bar_setup_extra_drop_target (AdapTabBar     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

ADAP_AVAILABLE_IN_1_4
GdkDragAction adap_tab_bar_get_extra_drag_preferred_action (AdapTabBar *self);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_bar_get_extra_drag_preload (AdapTabBar *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_bar_set_extra_drag_preload (AdapTabBar *self,
                                             gboolean   preload);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_bar_get_is_overflowing (AdapTabBar *self);

G_END_DECLS
