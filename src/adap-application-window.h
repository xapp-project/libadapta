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

#define ADW_TYPE_APPLICATION_WINDOW (adw_application_window_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwApplicationWindow, adw_application_window, ADW, APPLICATION_WINDOW, GtkApplicationWindow)

struct _AdwApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_application_window_new (GtkApplication *app) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void       adw_application_window_set_content (AdwApplicationWindow *self,
                                               GtkWidget            *content);
ADW_AVAILABLE_IN_ALL
GtkWidget *adw_application_window_get_content (AdwApplicationWindow *self);

ADW_AVAILABLE_IN_1_4
void adw_application_window_add_breakpoint (AdwApplicationWindow *self,
                                            AdwBreakpoint        *breakpoint);

ADW_AVAILABLE_IN_1_4
AdwBreakpoint *adw_application_window_get_current_breakpoint (AdwApplicationWindow *self);

ADW_AVAILABLE_IN_1_5
GListModel *adw_application_window_get_dialogs (AdwApplicationWindow *self);

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_application_window_get_visible_dialog (AdwApplicationWindow *self);

G_END_DECLS
