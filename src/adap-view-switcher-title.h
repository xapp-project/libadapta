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

#define ADAP_TYPE_VIEW_SWITCHER_TITLE (adap_view_switcher_title_get_type())

ADAP_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdapViewSwitcherTitle, adap_view_switcher_title, ADAP, VIEW_SWITCHER_TITLE, GtkWidget)

ADAP_DEPRECATED_IN_1_4
GtkWidget *adap_view_switcher_title_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_DEPRECATED_IN_1_4
AdapViewStack *adap_view_switcher_title_get_stack (AdapViewSwitcherTitle *self);
ADAP_DEPRECATED_IN_1_4
void          adap_view_switcher_title_set_stack (AdapViewSwitcherTitle *self,
                                                 AdapViewStack         *stack);

ADAP_DEPRECATED_IN_1_4
const char *adap_view_switcher_title_get_title (AdapViewSwitcherTitle *self);
ADAP_DEPRECATED_IN_1_4
void        adap_view_switcher_title_set_title (AdapViewSwitcherTitle *self,
                                               const char           *title);

ADAP_DEPRECATED_IN_1_4
const char *adap_view_switcher_title_get_subtitle (AdapViewSwitcherTitle *self);
ADAP_DEPRECATED_IN_1_4
void        adap_view_switcher_title_set_subtitle (AdapViewSwitcherTitle *self,
                                                  const char           *subtitle);

ADAP_DEPRECATED_IN_1_4
gboolean adap_view_switcher_title_get_view_switcher_enabled (AdapViewSwitcherTitle *self);
ADAP_DEPRECATED_IN_1_4
void     adap_view_switcher_title_set_view_switcher_enabled (AdapViewSwitcherTitle *self,
                                                            gboolean              enabled);

ADAP_DEPRECATED_IN_1_4
gboolean adap_view_switcher_title_get_title_visible (AdapViewSwitcherTitle *self);

G_END_DECLS
