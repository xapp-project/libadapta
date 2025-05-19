/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-dialog.h"

G_BEGIN_DECLS

void adw_dialog_set_shadowed (AdwDialog *self,
                              gboolean   shadowed);

void adw_dialog_set_callbacks (AdwDialog *self,
                               GFunc      closing_callback,
                               GFunc      remove_callback,
                               gpointer   user_data);

gboolean adw_dialog_get_closing (AdwDialog *self);
void     adw_dialog_set_closing (AdwDialog *self,
                                 gboolean   closing);

GtkWidget *adw_dialog_get_window (AdwDialog *self);

G_END_DECLS
