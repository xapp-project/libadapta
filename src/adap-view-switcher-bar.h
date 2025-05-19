/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-view-switcher.h"

G_BEGIN_DECLS

#define ADAP_TYPE_VIEW_SWITCHER_BAR (adap_view_switcher_bar_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapViewSwitcherBar, adap_view_switcher_bar, ADAP, VIEW_SWITCHER_BAR, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_view_switcher_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapViewStack *adap_view_switcher_bar_get_stack (AdapViewSwitcherBar *self);
ADAP_AVAILABLE_IN_ALL
void          adap_view_switcher_bar_set_stack (AdapViewSwitcherBar *self,
                                               AdapViewStack       *stack);

ADAP_AVAILABLE_IN_ALL
gboolean adap_view_switcher_bar_get_reveal (AdapViewSwitcherBar *self);
ADAP_AVAILABLE_IN_ALL
void     adap_view_switcher_bar_set_reveal (AdapViewSwitcherBar *self,
                                           gboolean            reveal);

G_END_DECLS
