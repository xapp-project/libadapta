/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_BUTTON_CONTENT (adw_button_content_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwButtonContent, adw_button_content, ADW, BUTTON_CONTENT, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_button_content_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_button_content_get_label (AdwButtonContent  *self);
ADW_AVAILABLE_IN_ALL
void        adw_button_content_set_label (AdwButtonContent *self,
                                          const char       *label);

ADW_AVAILABLE_IN_ALL
const char *adw_button_content_get_icon_name (AdwButtonContent  *self);
ADW_AVAILABLE_IN_ALL
void        adw_button_content_set_icon_name (AdwButtonContent *self,
                                              const char       *icon_name);

ADW_AVAILABLE_IN_ALL
gboolean adw_button_content_get_use_underline (AdwButtonContent *self);
ADW_AVAILABLE_IN_ALL
void     adw_button_content_set_use_underline (AdwButtonContent *self,
                                               gboolean          use_underline);

ADW_AVAILABLE_IN_1_4
gboolean adw_button_content_get_can_shrink (AdwButtonContent *self);
ADW_AVAILABLE_IN_1_4
void     adw_button_content_set_can_shrink (AdwButtonContent *self,
                                            gboolean          can_shrink);

G_END_DECLS
