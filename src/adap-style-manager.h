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

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_COLOR_SCHEME_DEFAULT,
  ADAP_COLOR_SCHEME_FORCE_LIGHT,
  ADAP_COLOR_SCHEME_PREFER_LIGHT,
  ADAP_COLOR_SCHEME_PREFER_DARK,
  ADAP_COLOR_SCHEME_FORCE_DARK,
} AdapColorScheme;

#define ADAP_TYPE_STYLE_MANAGER (adap_style_manager_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapStyleManager, adap_style_manager, ADAP, STYLE_MANAGER, GObject)

ADAP_AVAILABLE_IN_ALL
AdapStyleManager *adap_style_manager_get_default (void);
ADAP_AVAILABLE_IN_ALL
AdapStyleManager *adap_style_manager_get_for_display (GdkDisplay *display);

ADAP_AVAILABLE_IN_ALL
GdkDisplay *adap_style_manager_get_display (AdapStyleManager *self);

ADAP_AVAILABLE_IN_ALL
AdapColorScheme adap_style_manager_get_color_scheme (AdapStyleManager *self);
ADAP_AVAILABLE_IN_ALL
void           adap_style_manager_set_color_scheme (AdapStyleManager *self,
                                                   AdapColorScheme   color_scheme);

ADAP_AVAILABLE_IN_ALL
gboolean adap_style_manager_get_system_supports_color_schemes (AdapStyleManager *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_style_manager_get_dark          (AdapStyleManager *self);
ADAP_AVAILABLE_IN_ALL
gboolean adap_style_manager_get_high_contrast (AdapStyleManager *self);

G_END_DECLS
