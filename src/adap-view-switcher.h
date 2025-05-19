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

#include "adap-view-stack.h"

G_BEGIN_DECLS

#define ADAP_TYPE_VIEW_SWITCHER (adap_view_switcher_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapViewSwitcher, adap_view_switcher, ADAP, VIEW_SWITCHER, GtkWidget)

typedef enum {
  ADAP_VIEW_SWITCHER_POLICY_NARROW,
  ADAP_VIEW_SWITCHER_POLICY_WIDE,
} AdapViewSwitcherPolicy;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_view_switcher_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapViewSwitcherPolicy adap_view_switcher_get_policy (AdapViewSwitcher       *self);
ADAP_AVAILABLE_IN_ALL
void                  adap_view_switcher_set_policy (AdapViewSwitcher       *self,
                                                    AdapViewSwitcherPolicy  policy);

ADAP_AVAILABLE_IN_ALL
AdapViewStack *adap_view_switcher_get_stack (AdapViewSwitcher *self);
ADAP_AVAILABLE_IN_ALL
void          adap_view_switcher_set_stack (AdapViewSwitcher *self,
                                           AdapViewStack    *stack);

G_END_DECLS
