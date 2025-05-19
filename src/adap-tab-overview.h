/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB_OVERVIEW (adw_tab_overview_get_type())

ADW_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdwTabOverview, adw_tab_overview, ADW, TAB_OVERVIEW, GtkWidget)

ADW_AVAILABLE_IN_1_3
GtkWidget *adw_tab_overview_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_3
AdwTabView *adw_tab_overview_get_view (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void        adw_tab_overview_set_view (AdwTabOverview *self,
                                       AdwTabView     *view);

ADW_AVAILABLE_IN_1_3
GtkWidget *adw_tab_overview_get_child (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void       adw_tab_overview_set_child (AdwTabOverview *self,
                                       GtkWidget      *child);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_open  (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_open (AdwTabOverview *self,
                                    gboolean        open);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_inverted (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_inverted (AdwTabOverview *self,
                                        gboolean        inverted);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_enable_search (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_enable_search (AdwTabOverview *self,
                                             gboolean        enable_search);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_search_active (AdwTabOverview *self);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_enable_new_tab (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_enable_new_tab (AdwTabOverview *self,
                                              gboolean        enable_new_tab);

ADW_AVAILABLE_IN_1_3
GMenuModel *adw_tab_overview_get_secondary_menu (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void        adw_tab_overview_set_secondary_menu (AdwTabOverview *self,
                                                 GMenuModel     *secondary_menu);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_show_start_title_buttons (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_show_start_title_buttons (AdwTabOverview *self,
                                                        gboolean        show_start_title_buttons);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_show_end_title_buttons (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_show_end_title_buttons (AdwTabOverview *self,
                                                      gboolean        show_end_title_buttons);

ADW_AVAILABLE_IN_1_3
void adw_tab_overview_setup_extra_drop_target (AdwTabOverview *self,
                                               GdkDragAction   actions,
                                               GType          *types,
                                               gsize           n_types);

ADW_AVAILABLE_IN_1_4
GdkDragAction adw_tab_overview_get_extra_drag_preferred_action (AdwTabOverview *self);

ADW_AVAILABLE_IN_1_3
gboolean adw_tab_overview_get_extra_drag_preload (AdwTabOverview *self);
ADW_AVAILABLE_IN_1_3
void     adw_tab_overview_set_extra_drag_preload (AdwTabOverview *self,
                                                  gboolean        preload);

G_END_DECLS
