/*
 * Copyright (C) 2023 Purism SPC
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
#include "adap-settings-private.h"

G_BEGIN_DECLS

#define ADAP_TYPE_SETTINGS_IMPL (adap_settings_impl_get_type())

G_DECLARE_DERIVABLE_TYPE (AdapSettingsImpl, adap_settings_impl, ADAP, SETTINGS_IMPL, GObject)

struct _AdapSettingsImplClass
{
  GObjectClass parent_class;
};

gboolean adap_settings_impl_get_has_theme_name (AdapSettingsImpl *self);
gboolean adap_settings_impl_get_has_color_scheme  (AdapSettingsImpl *self);
gboolean adap_settings_impl_get_has_high_contrast (AdapSettingsImpl *self);
void     adap_settings_impl_set_features          (AdapSettingsImpl *self,
                                                  gboolean         has_theme_name,
                                                  gboolean         has_color_scheme,
                                                  gboolean         has_high_contrast);

AdapSystemColorScheme adap_settings_impl_get_color_scheme (AdapSettingsImpl      *self);
void                 adap_settings_impl_set_color_scheme (AdapSettingsImpl      *self,
                                                         AdapSystemColorScheme  color_scheme);

gboolean adap_settings_impl_get_high_contrast (AdapSettingsImpl *self);
void     adap_settings_impl_set_high_contrast (AdapSettingsImpl *self,
                                              gboolean         high_contrast);
void     adap_settings_impl_set_theme_name    (AdapSettingsImpl *self,
                                              const gchar     *theme_name);
const gchar *adap_settings_impl_get_theme_name (AdapSettingsImpl *self);

gboolean adap_get_disable_portal (void);

#ifdef __APPLE__
#define ADAP_TYPE_SETTINGS_IMPL_MACOS (adap_settings_impl_macos_get_type())

G_DECLARE_FINAL_TYPE (AdapSettingsImplMacOS, adap_settings_impl_macos, ADAP, SETTINGS_IMPL_MACOS, AdapSettingsImpl)

AdapSettingsImpl *adap_settings_impl_macos_new (gboolean enable_color_scheme,
                                              gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#elif defined(G_OS_WIN32)
#define ADAP_TYPE_SETTINGS_IMPL_WIN32 (adap_settings_impl_win32_get_type())

G_DECLARE_FINAL_TYPE (AdapSettingsImplWin32, adap_settings_impl_win32, ADAP, SETTINGS_IMPL_WIN32, AdapSettingsImpl)

AdapSettingsImpl *adap_settings_impl_win32_new (gboolean enable_color_scheme,
                                              gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#else
#define ADAP_TYPE_SETTINGS_IMPL_PORTAL (adap_settings_impl_portal_get_type())

G_DECLARE_FINAL_TYPE (AdapSettingsImplPortal, adap_settings_impl_portal, ADAP, SETTINGS_IMPL_PORTAL, AdapSettingsImpl)

AdapSettingsImpl *adap_settings_impl_portal_new (gboolean enable_theme_name,
                                               gboolean enable_color_scheme,
                                               gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;
#endif

#define ADAP_TYPE_SETTINGS_IMPL_GSETTINGS (adap_settings_impl_gsettings_get_type())

G_DECLARE_FINAL_TYPE (AdapSettingsImplGSettings, adap_settings_impl_gsettings, ADAP, SETTINGS_IMPL_GSETTINGS, AdapSettingsImpl)

AdapSettingsImpl *adap_settings_impl_gsettings_new (gboolean enable_theme_name,
                                                  gboolean enable_color_scheme,
                                                  gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;

#define ADAP_TYPE_SETTINGS_IMPL_LEGACY (adap_settings_impl_legacy_get_type())

G_DECLARE_FINAL_TYPE (AdapSettingsImplLegacy, adap_settings_impl_legacy, ADAP, SETTINGS_IMPL_LEGACY, AdapSettingsImpl)

AdapSettingsImpl *adap_settings_impl_legacy_new (gboolean enable_color_scheme,
                                               gboolean enable_high_contrast) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
