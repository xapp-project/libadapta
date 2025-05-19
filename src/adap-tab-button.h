/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB_BUTTON (adw_tab_button_get_type())

ADW_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdwTabButton, adw_tab_button, ADW, TAB_BUTTON, GtkWidget)

ADW_AVAILABLE_IN_1_3
GtkWidget *adw_tab_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_3
AdwTabView *adw_tab_button_get_view (AdwTabButton *self);
ADW_AVAILABLE_IN_1_3
void        adw_tab_button_set_view (AdwTabButton *self,
                                     AdwTabView   *view);

G_END_DECLS
