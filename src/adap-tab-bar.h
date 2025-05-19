/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-enums.h"
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB_BAR (adw_tab_bar_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwTabBar, adw_tab_bar, ADW, TAB_BAR, GtkWidget)

ADW_AVAILABLE_IN_ALL
AdwTabBar *adw_tab_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwTabView *adw_tab_bar_get_view (AdwTabBar  *self);
ADW_AVAILABLE_IN_ALL
void        adw_tab_bar_set_view (AdwTabBar  *self,
                                  AdwTabView *view);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_tab_bar_get_start_action_widget (AdwTabBar *self);
ADW_AVAILABLE_IN_ALL
void       adw_tab_bar_set_start_action_widget (AdwTabBar *self,
                                                GtkWidget *widget);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_tab_bar_get_end_action_widget (AdwTabBar *self);
ADW_AVAILABLE_IN_ALL
void       adw_tab_bar_set_end_action_widget (AdwTabBar *self,
                                              GtkWidget *widget);

ADW_AVAILABLE_IN_ALL
gboolean adw_tab_bar_get_autohide (AdwTabBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_tab_bar_set_autohide (AdwTabBar *self,
                                   gboolean   autohide);

ADW_AVAILABLE_IN_ALL
gboolean adw_tab_bar_get_tabs_revealed (AdwTabBar *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_tab_bar_get_expand_tabs (AdwTabBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_tab_bar_set_expand_tabs (AdwTabBar *self,
                                      gboolean   expand_tabs);

ADW_AVAILABLE_IN_ALL
gboolean adw_tab_bar_get_inverted (AdwTabBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_tab_bar_set_inverted (AdwTabBar *self,
                                   gboolean   inverted);

ADW_AVAILABLE_IN_ALL
void adw_tab_bar_setup_extra_drop_target (AdwTabBar     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

ADW_AVAILABLE_IN_1_4
GdkDragAction adw_tab_bar_get_extra_drag_preferred_action (AdwTabBar *self);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_bar_get_extra_drag_preload (AdwTabBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_bar_set_extra_drag_preload (AdwTabBar *self,
                                             gboolean   preload);

ADW_AVAILABLE_IN_ALL
gboolean adw_tab_bar_get_is_overflowing (AdwTabBar *self);

G_END_DECLS
