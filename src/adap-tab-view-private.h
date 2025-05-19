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

#include "adw-tab-view.h"

G_BEGIN_DECLS

GdkPaintable *adw_tab_page_get_paintable (AdwTabPage *self);

gboolean adw_tab_view_select_first_page (AdwTabView *self);
gboolean adw_tab_view_select_last_page  (AdwTabView *self);

void adw_tab_view_detach_page (AdwTabView *self,
                               AdwTabPage *page);
void adw_tab_view_attach_page (AdwTabView *self,
                               AdwTabPage *page,
                               int         position);

AdwTabView *adw_tab_view_create_window (AdwTabView *self) G_GNUC_WARN_UNUSED_RESULT;

void adw_tab_view_open_overview (AdwTabView *self);
void adw_tab_view_close_overview (AdwTabView *self);

G_END_DECLS
