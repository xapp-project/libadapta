/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"
#include "adap-toast.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_TOAST_WIDGET (adap_toast_widget_get_type())

G_DECLARE_FINAL_TYPE (AdapToastWidget, adap_toast_widget, ADAP, TOAST_WIDGET, GtkWidget)

GtkWidget *adap_toast_widget_new (AdapToast *toast) G_GNUC_WARN_UNUSED_RESULT;

void adap_toast_widget_reset_timeout (AdapToastWidget *self);

gboolean adap_toast_widget_get_button_visible (AdapToastWidget *self);

G_END_DECLS
