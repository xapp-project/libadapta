/*
 * Copyright (C) 2023 Purism SPC
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
#include "adw-settings-private.h"

G_BEGIN_DECLS

#define ADW_TYPE_SETTINGS_IMPL (adw_settings_impl_get_type())

G_DECLARE_DERIVABLE_TYPE (AdwSettingsImpl, adw_settings_impl, ADW, SETTINGS_IMPL, GObject)

struct _AdwSettingsImplClass
{
  GObjectClass parent_class;
};

gboolean adw_settings_impl_get_has_theme_name (AdwSettingsImpl *self);
gboolean adw_settings_impl_get_has_color_scheme  (AdwSettingsImpl *self);
gboolean adw_settings_impl_get_has_high_contrast (AdwSettingsImpl *self);
void     adw_settings_impl_set_features          (AdwSettingsImpl *self,
                                                  gboolean         has_theme_name,
                                                  gboolean         has_color_scheme,
                                                  gboolean         has_high_contrast);

AdwSystemColorScheme adw_settings_impl_get_color_scheme (AdwSettingsImpl      *self);
void                 adw_settings_impl_set_color_scheme (AdwSettingsImpl      *self,
                                                         AdwSystemColorScheme  color_scheme);

gboolean adw_settings_impl_get_high_contrast (AdwSettingsImpl *self);
void     adw_settings_impl_set_high_contrast (AdwSettingsImpl *self,
                                              gboolean         high_contrast);
void     adw_settings_impl_set_theme_name    (AdwSettingsImpl *self,
                                              const gchar     *theme_name);
const gchar *adw_settings_impl_get_theme_name (AdwSettingsImpl *self);

gboolean adw_get_disable_portal (void);

#ifdef __APPLE__
#define ADW_TYPE_SETTINGS_IMPL_MACOS (adw_settings_impl_macos_get_type())

G_DECLARE_FINAL_TYPE (AdwSettingsImplMacOS, adw_settings_impl_macos, ADW, SETTINGS_IMPL_MACOS, AdwSettingsImpl)

AdwSettingsImpl *adw_settings_impl_macos_new (gboolean enable_color_scheme,
                                              gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#elif defined(G_OS_WIN32)
#define ADW_TYPE_SETTINGS_IMPL_WIN32 (adw_settings_impl_win32_get_type())

G_DECLARE_FINAL_TYPE (AdwSettingsImplWin32, adw_settings_impl_win32, ADW, SETTINGS_IMPL_WIN32, AdwSettingsImpl)

AdwSettingsImpl *adw_settings_impl_win32_new (gboolean enable_color_scheme,
                                              gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#else
#define ADW_TYPE_SETTINGS_IMPL_PORTAL (adw_settings_impl_portal_get_type())

G_DECLARE_FINAL_TYPE (AdwSettingsImplPortal, adw_settings_impl_portal, ADW, SETTINGS_IMPL_PORTAL, AdwSettingsImpl)

AdwSettingsImpl *adw_settings_impl_portal_new (gboolean enable_theme_name,
                                               gboolean enable_color_scheme,
                                               gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#endif

#define ADW_TYPE_SETTINGS_IMPL_GSETTINGS (adw_settings_impl_gsettings_get_type())

G_DECLARE_FINAL_TYPE (AdwSettingsImplGSettings, adw_settings_impl_gsettings, ADW, SETTINGS_IMPL_GSETTINGS, AdwSettingsImpl)

AdwSettingsImpl *adw_settings_impl_gsettings_new (gboolean enable_theme_name,
                                                  gboolean enable_color_scheme,
                                                  gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;

#define ADW_TYPE_SETTINGS_IMPL_LEGACY (adw_settings_impl_legacy_get_type())

G_DECLARE_FINAL_TYPE (AdwSettingsImplLegacy, adw_settings_impl_legacy, ADW, SETTINGS_IMPL_LEGACY, AdwSettingsImpl)

AdwSettingsImpl *adw_settings_impl_legacy_new (gboolean enable_color_scheme,
                                               gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
