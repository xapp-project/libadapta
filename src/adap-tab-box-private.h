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

#include <gtk/gtk.h>
#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_BOX (adap_tab_box_get_type())

G_DECLARE_FINAL_TYPE (AdapTabBox, adap_tab_box, ADAP, TAB_BOX, GtkWidget)

void adap_tab_box_set_view (AdapTabBox  *self,
                           AdapTabView *view);

void adap_tab_box_attach_page (AdapTabBox  *self,
                              AdapTabPage *page,
                              int         position);
void adap_tab_box_detach_page (AdapTabBox  *self,
                              AdapTabPage *page);
void adap_tab_box_select_page (AdapTabBox  *self,
                              AdapTabPage *page);

void adap_tab_box_try_focus_selected_tab (AdapTabBox  *self);
gboolean adap_tab_box_is_page_focused    (AdapTabBox  *self,
                                         AdapTabPage *page);

void adap_tab_box_setup_extra_drop_target (AdapTabBox     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

gboolean adap_tab_box_get_extra_drag_preload (AdapTabBox *self);
void     adap_tab_box_set_extra_drag_preload (AdapTabBox *self,
                                             gboolean   preload);

gboolean adap_tab_box_get_expand_tabs (AdapTabBox *self);
void     adap_tab_box_set_expand_tabs (AdapTabBox *self,
                                      gboolean   expand_tabs);

gboolean adap_tab_box_get_inverted (AdapTabBox *self);
void     adap_tab_box_set_inverted (AdapTabBox *self,
                                   gboolean   inverted);

G_END_DECLS
