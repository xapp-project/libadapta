/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-navigation-view.h"

G_BEGIN_DECLS

AdwNavigationView *adw_navigation_page_get_child_view (AdwNavigationPage *self);

void adw_navigation_page_showing (AdwNavigationPage *self);
void adw_navigation_page_shown   (AdwNavigationPage *self);
void adw_navigation_page_hiding  (AdwNavigationPage *self);
void adw_navigation_page_hidden  (AdwNavigationPage *self);

void adw_navigation_page_block_signals   (AdwNavigationPage *self);
void adw_navigation_page_unblock_signals (AdwNavigationPage *self);

void adw_navigation_page_add_child_nav_split_view    (AdwNavigationPage *self);
void adw_navigation_page_remove_child_nav_split_view (AdwNavigationPage *self);

G_END_DECLS
