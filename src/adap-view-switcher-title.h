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

#define ADW_TYPE_VIEW_SWITCHER_TITLE (adw_view_switcher_title_get_type())

ADW_DEPRECATED_IN_1_4
G_DECLARE_FINAL_TYPE (AdwViewSwitcherTitle, adw_view_switcher_title, ADW, VIEW_SWITCHER_TITLE, GtkWidget)

ADW_DEPRECATED_IN_1_4
GtkWidget *adw_view_switcher_title_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_4
AdwViewStack *adw_view_switcher_title_get_stack (AdwViewSwitcherTitle *self);
ADW_DEPRECATED_IN_1_4
void          adw_view_switcher_title_set_stack (AdwViewSwitcherTitle *self,
                                                 AdwViewStack         *stack);

ADW_DEPRECATED_IN_1_4
const char *adw_view_switcher_title_get_title (AdwViewSwitcherTitle *self);
ADW_DEPRECATED_IN_1_4
void        adw_view_switcher_title_set_title (AdwViewSwitcherTitle *self,
                                               const char           *title);

ADW_DEPRECATED_IN_1_4
const char *adw_view_switcher_title_get_subtitle (AdwViewSwitcherTitle *self);
ADW_DEPRECATED_IN_1_4
void        adw_view_switcher_title_set_subtitle (AdwViewSwitcherTitle *self,
                                                  const char           *subtitle);

ADW_DEPRECATED_IN_1_4
gboolean adw_view_switcher_title_get_view_switcher_enabled (AdwViewSwitcherTitle *self);
ADW_DEPRECATED_IN_1_4
void     adw_view_switcher_title_set_view_switcher_enabled (AdwViewSwitcherTitle *self,
                                                            gboolean              enabled);

ADW_DEPRECATED_IN_1_4
gboolean adw_view_switcher_title_get_title_visible (AdwViewSwitcherTitle *self);

G_END_DECLS
