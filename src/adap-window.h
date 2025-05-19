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

#define ADAP_TYPE_WINDOW (adap_window_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapWindow, adap_window, ADAP, WINDOW, GtkWindow)

struct _AdapWindowClass
{
  GtkWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_window_get_content (AdapWindow *self);
ADAP_AVAILABLE_IN_ALL
void       adap_window_set_content (AdapWindow *self,
                                   GtkWidget *content);

ADAP_AVAILABLE_IN_1_4
void adap_window_add_breakpoint (AdapWindow     *self,
                                AdapBreakpoint *breakpoint);

ADAP_AVAILABLE_IN_1_4
AdapBreakpoint *adap_window_get_current_breakpoint (AdapWindow *self);

ADAP_AVAILABLE_IN_1_5
GListModel *adap_window_get_dialogs (AdapWindow *self);

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_window_get_visible_dialog (AdapWindow *self);

G_END_DECLS
