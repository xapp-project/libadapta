/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_FLOATING_SHEET (adap_floating_sheet_get_type())

G_DECLARE_FINAL_TYPE (AdapFloatingSheet, adap_floating_sheet, ADAP, FLOATING_SHEET, GtkWidget)

GtkWidget *adap_floating_sheet_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adap_floating_sheet_get_child (AdapFloatingSheet *self);
void       adap_floating_sheet_set_child (AdapFloatingSheet *self,
                                         GtkWidget        *child);

gboolean adap_floating_sheet_get_open (AdapFloatingSheet *self);
void     adap_floating_sheet_set_open (AdapFloatingSheet *self,
                                      gboolean          open);

gboolean adap_floating_sheet_get_can_close (AdapFloatingSheet *self);
void     adap_floating_sheet_set_can_close (AdapFloatingSheet *self,
                                           gboolean          can_close);

GtkWidget *adap_floating_sheet_get_sheet_bin (AdapFloatingSheet *self);
G_END_DECLS
