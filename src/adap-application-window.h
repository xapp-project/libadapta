/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-breakpoint.h"
#include "adap-dialog.h"

G_BEGIN_DECLS

#define ADAP_TYPE_APPLICATION_WINDOW (adap_application_window_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapApplicationWindow, adap_application_window, ADAP, APPLICATION_WINDOW, GtkApplicationWindow)

struct _AdapApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_application_window_new (GtkApplication *app) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void       adap_application_window_set_content (AdapApplicationWindow *self,
                                               GtkWidget            *content);
ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_application_window_get_content (AdapApplicationWindow *self);

ADAP_AVAILABLE_IN_1_4
void adap_application_window_add_breakpoint (AdapApplicationWindow *self,
                                            AdapBreakpoint        *breakpoint);

ADAP_AVAILABLE_IN_1_4
AdapBreakpoint *adap_application_window_get_current_breakpoint (AdapApplicationWindow *self);

ADAP_AVAILABLE_IN_1_5
GListModel *adap_application_window_get_dialogs (AdapApplicationWindow *self);

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_application_window_get_visible_dialog (AdapApplicationWindow *self);

G_END_DECLS
