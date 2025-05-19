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

#define ADAP_TYPE_TAB (adap_tab_get_type())

G_DECLARE_FINAL_TYPE (AdapTab, adap_tab, ADAP, TAB, GtkWidget)

AdapTab *adap_tab_new (AdapTabView *view,
                     gboolean    pinned) G_GNUC_WARN_UNUSED_RESULT;

AdapTabPage *adap_tab_get_page (AdapTab     *self);
void        adap_tab_set_page (AdapTab     *self,
                              AdapTabPage *page);

gboolean adap_tab_get_dragging (AdapTab   *self);
void     adap_tab_set_dragging (AdapTab   *self,
                               gboolean  dragging);

gboolean adap_tab_get_inverted (AdapTab   *self);
void     adap_tab_set_inverted (AdapTab   *self,
                               gboolean  inverted);

void adap_tab_set_fully_visible (AdapTab   *self,
                                gboolean  fully_visible);

void adap_tab_setup_extra_drop_target (AdapTab        *self,
                                      GdkDragAction  actions,
                                      GType         *types,
                                      gsize          n_types);

void adap_tab_set_extra_drag_preload (AdapTab   *self,
                                     gboolean  preload);

gboolean adap_tab_can_click_at (AdapTab *self,
                               float   x,
                               float   y);

G_END_DECLS
