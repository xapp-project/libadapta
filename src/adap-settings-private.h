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

#include <glib-object.h>
#include "adw-enums-private.h"

G_BEGIN_DECLS

typedef enum {
  ADW_SYSTEM_COLOR_SCHEME_DEFAULT,
  ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK,
  ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT,
} AdwSystemColorScheme;

#define ADW_TYPE_SETTINGS (adw_settings_get_type())

G_DECLARE_FINAL_TYPE (AdwSettings, adw_settings, ADW, SETTINGS, GObject)

ADW_AVAILABLE_IN_ALL
AdwSettings *adw_settings_get_default (void);

ADW_AVAILABLE_IN_ALL
const gchar *adw_settings_get_theme_name (AdwSettings *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_settings_get_system_supports_color_schemes (AdwSettings *self);

ADW_AVAILABLE_IN_ALL
AdwSystemColorScheme adw_settings_get_color_scheme (AdwSettings *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_settings_get_high_contrast (AdwSettings *self);

ADW_AVAILABLE_IN_ALL
void adw_settings_start_override (AdwSettings *self);
ADW_AVAILABLE_IN_ALL
void adw_settings_end_override   (AdwSettings *self);

ADW_AVAILABLE_IN_ALL
void adw_settings_override_system_supports_color_schemes (AdwSettings *self,
                                                          gboolean     system_supports_color_schemes);

ADW_AVAILABLE_IN_ALL
void adw_settings_override_color_scheme (AdwSettings          *self,
                                         AdwSystemColorScheme  color_scheme);

ADW_AVAILABLE_IN_ALL
void adw_settings_override_high_contrast (AdwSettings *self,
                                          gboolean     high_contrast);

G_END_DECLS
