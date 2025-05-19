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

#include "adw-view-stack.h"

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER (adw_view_switcher_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwViewSwitcher, adw_view_switcher, ADW, VIEW_SWITCHER, GtkWidget)

typedef enum {
  ADW_VIEW_SWITCHER_POLICY_NARROW,
  ADW_VIEW_SWITCHER_POLICY_WIDE,
} AdwViewSwitcherPolicy;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_switcher_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwViewSwitcherPolicy adw_view_switcher_get_policy (AdwViewSwitcher       *self);
ADW_AVAILABLE_IN_ALL
void                  adw_view_switcher_set_policy (AdwViewSwitcher       *self,
                                                    AdwViewSwitcherPolicy  policy);

ADW_AVAILABLE_IN_ALL
AdwViewStack *adw_view_switcher_get_stack (AdwViewSwitcher *self);
ADW_AVAILABLE_IN_ALL
void          adw_view_switcher_set_stack (AdwViewSwitcher *self,
                                           AdwViewStack    *stack);

G_END_DECLS
