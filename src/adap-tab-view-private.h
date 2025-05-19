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

#include "adap-tab-view.h"

G_BEGIN_DECLS

GdkPaintable *adap_tab_page_get_paintable (AdapTabPage *self);

gboolean adap_tab_view_select_first_page (AdapTabView *self);
gboolean adap_tab_view_select_last_page  (AdapTabView *self);

void adap_tab_view_detach_page (AdapTabView *self,
                               AdapTabPage *page);
void adap_tab_view_attach_page (AdapTabView *self,
                               AdapTabPage *page,
                               int         position);

AdapTabView *adap_tab_view_create_window (AdapTabView *self) G_GNUC_WARN_UNUSED_RESULT;

void adap_tab_view_open_overview (AdapTabView *self);
void adap_tab_view_close_overview (AdapTabView *self);

G_END_DECLS
