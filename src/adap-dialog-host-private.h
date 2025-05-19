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

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-dialog.h"

G_BEGIN_DECLS

#define ADAP_TYPE_DIALOG_HOST (adap_dialog_host_get_type())

G_DECLARE_FINAL_TYPE (AdapDialogHost, adap_dialog_host, ADAP, DIALOG_HOST, GtkWidget)

GtkWidget *adap_dialog_host_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adap_dialog_host_get_child (AdapDialogHost *self);
void       adap_dialog_host_set_child (AdapDialogHost *self,
                                      GtkWidget     *child);

GListModel *adap_dialog_host_get_dialogs (AdapDialogHost *self);

AdapDialog *adap_dialog_host_get_visible_dialog (AdapDialogHost *self);

void adap_dialog_host_present_dialog (AdapDialogHost *self,
                                     AdapDialog     *dialog);

GtkWidget *adap_dialog_host_get_proxy (AdapDialogHost *self);
void       adap_dialog_host_set_proxy (AdapDialogHost *self,
                                      GtkWidget     *proxy);

AdapDialogHost *adap_dialog_host_get_from_proxy (GtkWidget *widget);

G_END_DECLS
