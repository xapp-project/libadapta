/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-breakpoint.h"
#include "adw-dialog.h"

G_BEGIN_DECLS

#define ADW_TYPE_WINDOW (adw_window_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwWindow, adw_window, ADW, WINDOW, GtkWindow)

struct _AdwWindowClass
{
  GtkWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_window_get_content (AdwWindow *self);
ADW_AVAILABLE_IN_ALL
void       adw_window_set_content (AdwWindow *self,
                                   GtkWidget *content);

ADW_AVAILABLE_IN_1_4
void adw_window_add_breakpoint (AdwWindow     *self,
                                AdwBreakpoint *breakpoint);

ADW_AVAILABLE_IN_1_4
AdwBreakpoint *adw_window_get_current_breakpoint (AdwWindow *self);

ADW_AVAILABLE_IN_1_5
GListModel *adw_window_get_dialogs (AdwWindow *self);

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_window_get_visible_dialog (AdwWindow *self);

G_END_DECLS
