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

#include "adap-tab-bar.h"

#include "adap-tab-box-private.h"

G_BEGIN_DECLS

gboolean adap_tab_bar_tabs_have_visible_focus (AdapTabBar *self);

AdapTabBox *adap_tab_bar_get_tab_box        (AdapTabBar *self);
AdapTabBox *adap_tab_bar_get_pinned_tab_box (AdapTabBar *self);

G_END_DECLS
