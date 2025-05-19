/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_WINDOW_TITLE (adw_window_title_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwWindowTitle, adw_window_title, ADW, WINDOW_TITLE, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_window_title_new (const char *title,
                                 const char *subtitle) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_window_title_get_title (AdwWindowTitle *self);
ADW_AVAILABLE_IN_ALL
void        adw_window_title_set_title (AdwWindowTitle *self,
                                        const char     *title);

ADW_AVAILABLE_IN_ALL
const char *adw_window_title_get_subtitle (AdwWindowTitle *self);
ADW_AVAILABLE_IN_ALL
void        adw_window_title_set_subtitle (AdwWindowTitle *self,
                                           const char     *subtitle);

G_END_DECLS
