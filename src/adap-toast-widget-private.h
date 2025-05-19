/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"
#include "adw-toast.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_TOAST_WIDGET (adw_toast_widget_get_type())

G_DECLARE_FINAL_TYPE (AdwToastWidget, adw_toast_widget, ADW, TOAST_WIDGET, GtkWidget)

GtkWidget *adw_toast_widget_new (AdwToast *toast) G_GNUC_WARN_UNUSED_RESULT;

void adw_toast_widget_reset_timeout (AdwToastWidget *self);

gboolean adw_toast_widget_get_button_visible (AdwToastWidget *self);

G_END_DECLS
