/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
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

#define ADAP_TYPE_TOAST_OVERLAY (adap_toast_overlay_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapToastOverlay, adap_toast_overlay, ADAP, TOAST_OVERLAY, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_toast_overlay_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_toast_overlay_get_child (AdapToastOverlay *self);
ADAP_AVAILABLE_IN_ALL
void       adap_toast_overlay_set_child (AdapToastOverlay *self,
                                        GtkWidget       *child);

ADAP_AVAILABLE_IN_ALL
void adap_toast_overlay_add_toast (AdapToastOverlay *self,
                                  AdapToast        *toast);

G_END_DECLS
