/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-dialog.h"

G_BEGIN_DECLS

void adap_dialog_set_shadowed (AdapDialog *self,
                              gboolean   shadowed);

void adap_dialog_set_callbacks (AdapDialog *self,
                               GFunc      closing_callback,
                               GFunc      remove_callback,
                               gpointer   user_data);

gboolean adap_dialog_get_closing (AdapDialog *self);
void     adap_dialog_set_closing (AdapDialog *self,
                                 gboolean   closing);

GtkWidget *adap_dialog_get_window (AdapDialog *self);

G_END_DECLS
