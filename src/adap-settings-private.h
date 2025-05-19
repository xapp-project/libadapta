/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <glib-object.h>
#include "adap-enums-private.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_SYSTEM_COLOR_SCHEME_DEFAULT,
  ADAP_SYSTEM_COLOR_SCHEME_PREFER_DARK,
  ADAP_SYSTEM_COLOR_SCHEME_PREFER_LIGHT,
} AdapSystemColorScheme;

#define ADAP_TYPE_SETTINGS (adap_settings_get_type())

G_DECLARE_FINAL_TYPE (AdapSettings, adap_settings, ADAP, SETTINGS, GObject)

ADAP_AVAILABLE_IN_ALL
AdapSettings *adap_settings_get_default (void);

ADAP_AVAILABLE_IN_ALL
const gchar *adap_settings_get_theme_name (AdapSettings *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_settings_get_system_supports_color_schemes (AdapSettings *self);

ADAP_AVAILABLE_IN_ALL
AdapSystemColorScheme adap_settings_get_color_scheme (AdapSettings *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_settings_get_high_contrast (AdapSettings *self);

ADAP_AVAILABLE_IN_ALL
void adap_settings_start_override (AdapSettings *self);
ADAP_AVAILABLE_IN_ALL
void adap_settings_end_override   (AdapSettings *self);

ADAP_AVAILABLE_IN_ALL
void adap_settings_override_system_supports_color_schemes (AdapSettings *self,
                                                          gboolean     system_supports_color_schemes);

ADAP_AVAILABLE_IN_ALL
void adap_settings_override_color_scheme (AdapSettings          *self,
                                         AdapSystemColorScheme  color_scheme);

ADAP_AVAILABLE_IN_ALL
void adap_settings_override_high_contrast (AdapSettings *self,
                                          gboolean     high_contrast);

G_END_DECLS
