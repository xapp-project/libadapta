/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-view-switcher.h"

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_BAR (adw_view_switcher_bar_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwViewSwitcherBar, adw_view_switcher_bar, ADW, VIEW_SWITCHER_BAR, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_switcher_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwViewStack *adw_view_switcher_bar_get_stack (AdwViewSwitcherBar *self);
ADW_AVAILABLE_IN_ALL
void          adw_view_switcher_bar_set_stack (AdwViewSwitcherBar *self,
                                               AdwViewStack       *stack);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_switcher_bar_get_reveal (AdwViewSwitcherBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_switcher_bar_set_reveal (AdwViewSwitcherBar *self,
                                           gboolean            reveal);

G_END_DECLS
