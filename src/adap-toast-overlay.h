/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
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

#define ADW_TYPE_TOAST_OVERLAY (adw_toast_overlay_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwToastOverlay, adw_toast_overlay, ADW, TOAST_OVERLAY, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_toast_overlay_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_toast_overlay_get_child (AdwToastOverlay *self);
ADW_AVAILABLE_IN_ALL
void       adw_toast_overlay_set_child (AdwToastOverlay *self,
                                        GtkWidget       *child);

ADW_AVAILABLE_IN_ALL
void adw_toast_overlay_add_toast (AdwToastOverlay *self,
                                  AdwToast        *toast);

G_END_DECLS
