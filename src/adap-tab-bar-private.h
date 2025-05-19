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

#include "adw-tab-bar.h"

#include "adw-tab-box-private.h"

G_BEGIN_DECLS

gboolean adw_tab_bar_tabs_have_visible_focus (AdwTabBar *self);

AdwTabBox *adw_tab_bar_get_tab_box        (AdwTabBar *self);
AdwTabBox *adw_tab_bar_get_pinned_tab_box (AdwTabBar *self);

G_END_DECLS
