/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_BUTTON (adap_tab_button_get_type())

ADAP_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdapTabButton, adap_tab_button, ADAP, TAB_BUTTON, GtkWidget)

ADAP_AVAILABLE_IN_1_3
GtkWidget *adap_tab_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_3
AdapTabView *adap_tab_button_get_view (AdapTabButton *self);
ADAP_AVAILABLE_IN_1_3
void        adap_tab_button_set_view (AdapTabButton *self,
                                     AdapTabView   *view);

G_END_DECLS
