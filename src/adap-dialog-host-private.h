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

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-dialog.h"

G_BEGIN_DECLS

#define ADW_TYPE_DIALOG_HOST (adw_dialog_host_get_type())

G_DECLARE_FINAL_TYPE (AdwDialogHost, adw_dialog_host, ADW, DIALOG_HOST, GtkWidget)

GtkWidget *adw_dialog_host_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_dialog_host_get_child (AdwDialogHost *self);
void       adw_dialog_host_set_child (AdwDialogHost *self,
                                      GtkWidget     *child);

GListModel *adw_dialog_host_get_dialogs (AdwDialogHost *self);

AdwDialog *adw_dialog_host_get_visible_dialog (AdwDialogHost *self);

void adw_dialog_host_present_dialog (AdwDialogHost *self,
                                     AdwDialog     *dialog);

GtkWidget *adw_dialog_host_get_proxy (AdwDialogHost *self);
void       adw_dialog_host_set_proxy (AdwDialogHost *self,
                                      GtkWidget     *proxy);

AdwDialogHost *adw_dialog_host_get_from_proxy (GtkWidget *widget);

G_END_DECLS
