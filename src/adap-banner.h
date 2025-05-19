/*
 * Copyright (C) 2022 Jamie Murphy <hello@itsjamie.dev>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-enums.h"

G_BEGIN_DECLS

#define ADW_TYPE_BANNER (adw_banner_get_type())

ADW_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdwBanner, adw_banner, ADW, BANNER, GtkWidget)

ADW_AVAILABLE_IN_1_3
GtkWidget *adw_banner_new (const char *title) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_3
const char  *adw_banner_get_title (AdwBanner *self);
ADW_AVAILABLE_IN_1_3
void         adw_banner_set_title (AdwBanner  *self,
                                   const char *title);

ADW_AVAILABLE_IN_1_3
const char  *adw_banner_get_button_label (AdwBanner *self);
ADW_AVAILABLE_IN_1_3
void         adw_banner_set_button_label (AdwBanner  *self,
                                          const char *label);

ADW_AVAILABLE_IN_1_3
gboolean adw_banner_get_revealed (AdwBanner *self);
ADW_AVAILABLE_IN_1_3
void     adw_banner_set_revealed (AdwBanner *self,
                                  gboolean   revealed);

ADW_AVAILABLE_IN_1_3
gboolean adw_banner_get_use_markup (AdwBanner *self);
ADW_AVAILABLE_IN_1_3
void     adw_banner_set_use_markup (AdwBanner *self,
                                    gboolean   use_markup);

G_END_DECLS
