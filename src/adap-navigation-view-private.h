/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-navigation-view.h"

G_BEGIN_DECLS

AdapNavigationView *adap_navigation_page_get_child_view (AdapNavigationPage *self);

void adap_navigation_page_showing (AdapNavigationPage *self);
void adap_navigation_page_shown   (AdapNavigationPage *self);
void adap_navigation_page_hiding  (AdapNavigationPage *self);
void adap_navigation_page_hidden  (AdapNavigationPage *self);

void adap_navigation_page_block_signals   (AdapNavigationPage *self);
void adap_navigation_page_unblock_signals (AdapNavigationPage *self);

void adap_navigation_page_add_child_nav_split_view    (AdapNavigationPage *self);
void adap_navigation_page_remove_child_nav_split_view (AdapNavigationPage *self);

G_END_DECLS
