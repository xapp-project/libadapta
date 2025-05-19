/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_FLOATING_SHEET (adw_floating_sheet_get_type())

G_DECLARE_FINAL_TYPE (AdwFloatingSheet, adw_floating_sheet, ADW, FLOATING_SHEET, GtkWidget)

GtkWidget *adw_floating_sheet_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_floating_sheet_get_child (AdwFloatingSheet *self);
void       adw_floating_sheet_set_child (AdwFloatingSheet *self,
                                         GtkWidget        *child);

gboolean adw_floating_sheet_get_open (AdwFloatingSheet *self);
void     adw_floating_sheet_set_open (AdwFloatingSheet *self,
                                      gboolean          open);

gboolean adw_floating_sheet_get_can_close (AdwFloatingSheet *self);
void     adw_floating_sheet_set_can_close (AdwFloatingSheet *self,
                                           gboolean          can_close);

GtkWidget *adw_floating_sheet_get_sheet_bin (AdwFloatingSheet *self);
G_END_DECLS
