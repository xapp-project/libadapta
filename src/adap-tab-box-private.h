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

#include <gtk/gtk.h>
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB_BOX (adw_tab_box_get_type())

G_DECLARE_FINAL_TYPE (AdwTabBox, adw_tab_box, ADW, TAB_BOX, GtkWidget)

void adw_tab_box_set_view (AdwTabBox  *self,
                           AdwTabView *view);

void adw_tab_box_attach_page (AdwTabBox  *self,
                              AdwTabPage *page,
                              int         position);
void adw_tab_box_detach_page (AdwTabBox  *self,
                              AdwTabPage *page);
void adw_tab_box_select_page (AdwTabBox  *self,
                              AdwTabPage *page);

void adw_tab_box_try_focus_selected_tab (AdwTabBox  *self);
gboolean adw_tab_box_is_page_focused    (AdwTabBox  *self,
                                         AdwTabPage *page);

void adw_tab_box_setup_extra_drop_target (AdwTabBox     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

gboolean adw_tab_box_get_extra_drag_preload (AdwTabBox *self);
void     adw_tab_box_set_extra_drag_preload (AdwTabBox *self,
                                             gboolean   preload);

gboolean adw_tab_box_get_expand_tabs (AdwTabBox *self);
void     adw_tab_box_set_expand_tabs (AdwTabBox *self,
                                      gboolean   expand_tabs);

gboolean adw_tab_box_get_inverted (AdwTabBox *self);
void     adw_tab_box_set_inverted (AdwTabBox *self,
                                   gboolean   inverted);

G_END_DECLS
