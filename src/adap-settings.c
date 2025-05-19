/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adap-settings-private.h"

#include "adap-settings-impl-private.h"

#include <gtk/gtk.h>

struct _AdapSettings
{
  GObject parent_instance;

  AdapSettingsImpl *platform_impl;
  AdapSettingsImpl *gsettings_impl;
  AdapSettingsImpl *legacy_impl;

  gchar *theme_name;

  AdapSystemColorScheme color_scheme;
  gboolean high_contrast;
  gboolean system_supports_color_schemes;

  gboolean override;
  gboolean system_supports_color_schemes_override;
  AdapSystemColorScheme color_scheme_override;
  gboolean high_contrast_override;
};

G_DEFINE_FINAL_TYPE (AdapSettings, adap_settings, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_THEME_NAME,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_COLOR_SCHEME,
  PROP_HIGH_CONTRAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static AdapSettings *default_instance;

static void
set_theme_name (AdapSettings          *self,
                const gchar          *theme_name)
{
  if (g_strcmp0 (self->theme_name, theme_name) == 0)
    return;

  self->theme_name = g_strdup (theme_name);

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_THEME_NAME]);
}

static void
set_color_scheme (AdapSettings          *self,
                  AdapSystemColorScheme  color_scheme)
{
  if (color_scheme == self->color_scheme)
    return;

  self->color_scheme = color_scheme;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

static void
set_high_contrast (AdapSettings *self,
                   gboolean     high_contrast)
{
  if (high_contrast == self->high_contrast)
    return;
  
  self->high_contrast = high_contrast;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

static void
init_debug (AdapSettings *self,
            gboolean    *found_theme_name,
            gboolean    *found_color_scheme,
            gboolean    *found_high_contrast)
{
  const char *env = g_getenv ("ADAP_DEBUG_HIGH_CONTRAST");
  if (env && *env) {
    if (!g_strcmp0 (env, "1")) {
      *found_high_contrast = TRUE;
      self->high_contrast = TRUE;
    } else if (!g_strcmp0 (env, "0")) {
      *found_high_contrast = TRUE;
      self->high_contrast = FALSE;
    } else {
      g_warning ("Invalid value for ADAP_DEBUG_HIGH_CONTRAST: %s (Expected 0 or 1)", env);
    }
  }

  env = g_getenv ("ADAP_DEBUG_COLOR_SCHEME");
  if (env) {
    if (!g_strcmp0 (env, "default")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADAP_SYSTEM_COLOR_SCHEME_DEFAULT;
    } else if (!g_strcmp0 (env, "prefer-dark")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADAP_SYSTEM_COLOR_SCHEME_PREFER_DARK;
    } else if (!g_strcmp0 (env, "prefer-light")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADAP_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;
    } else {
      g_warning ("Invalid color scheme %s (Expected one of: default, prefer-dark, prefer-light)", env);
    }
  }

  env = g_getenv ("ADAP_DEBUG_THEME_NAME");
  if (env) {
    *found_theme_name = TRUE;
    self->theme_name = g_strdup (env);
  }
}

static void
register_impl (AdapSettings      *self,
               AdapSettingsImpl  *impl,
               gboolean         *found_theme_name,
               gboolean         *found_color_scheme,
               gboolean         *found_high_contrast)
{
  if (adap_settings_impl_get_has_theme_name (impl)) {
    *found_theme_name = TRUE;

    set_theme_name (self, adap_settings_impl_get_theme_name (impl));

    g_signal_connect_swapped (impl, "theme-name-changed",
                              G_CALLBACK (set_theme_name), self);
  }
  if (adap_settings_impl_get_has_color_scheme (impl)) {
    *found_color_scheme = TRUE;

    set_color_scheme (self, adap_settings_impl_get_color_scheme (impl));

    g_signal_connect_swapped (impl, "color-scheme-changed",
                              G_CALLBACK (set_color_scheme), self);
  }

  if (adap_settings_impl_get_has_high_contrast (impl)) {
    *found_high_contrast = TRUE;

    set_high_contrast (self, adap_settings_impl_get_high_contrast (impl));

    g_signal_connect_swapped (impl, "high-contrast-changed",
                              G_CALLBACK (set_high_contrast), self);
  }
}

static void
adap_settings_constructed (GObject *object)
{
  AdapSettings *self = ADAP_SETTINGS (object);
  gboolean found_theme_name = FALSE;
  gboolean found_color_scheme = FALSE;
  gboolean found_high_contrast = FALSE;

  G_OBJECT_CLASS (adap_settings_parent_class)->constructed (object);

  init_debug (self, &found_theme_name, &found_color_scheme, &found_high_contrast);

  if (!found_color_scheme || !found_high_contrast) {
#ifdef __APPLE__
    self->platform_impl = adap_settings_impl_macos_new (!found_color_scheme, !found_high_contrast);
#elif defined(G_OS_WIN32)
    self->platform_impl = adap_settings_impl_win32_new (!found_color_scheme, !found_high_contrast);
#else
    self->platform_impl = adap_settings_impl_portal_new (!found_theme_name, !found_color_scheme, !found_high_contrast);
#endif

    register_impl (self, self->platform_impl, &found_theme_name, &found_color_scheme, &found_high_contrast);
  }

  if (!found_theme_name || !found_color_scheme || !found_high_contrast) {
    self->gsettings_impl = adap_settings_impl_gsettings_new (!found_theme_name, !found_color_scheme, !found_high_contrast);
    register_impl (self, self->gsettings_impl, &found_theme_name, &found_color_scheme, &found_high_contrast);
  }

  if (!found_color_scheme || !found_high_contrast) {
    self->legacy_impl = adap_settings_impl_legacy_new (!found_color_scheme, !found_high_contrast);
    register_impl (self, self->legacy_impl, &found_theme_name, &found_color_scheme, &found_high_contrast);
  }

  self->system_supports_color_schemes = found_color_scheme;
}

static void
adap_settings_dispose (GObject *object)
{
  AdapSettings *self = ADAP_SETTINGS (object);

  g_clear_object (&self->platform_impl);
  g_clear_object (&self->gsettings_impl);
  g_clear_object (&self->legacy_impl);

  G_OBJECT_CLASS (adap_settings_parent_class)->dispose (object);
}

static void
adap_settings_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdapSettings *self = ADAP_SETTINGS (object);

  switch (prop_id) {
  case PROP_THEME_NAME:
    g_value_set_string (value, adap_settings_get_theme_name (self));
    break;

  case PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES:
    g_value_set_boolean (value, adap_settings_get_system_supports_color_schemes (self));
    break;

  case PROP_COLOR_SCHEME:
    g_value_set_enum (value, adap_settings_get_color_scheme (self));
    break;

  case PROP_HIGH_CONTRAST:
    g_value_set_boolean (value, adap_settings_get_high_contrast (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_settings_class_init (AdapSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adap_settings_constructed;
  object_class->dispose = adap_settings_dispose;
  object_class->get_property = adap_settings_get_property;

  props[PROP_THEME_NAME] =
    g_param_spec_string ("theme-name", NULL, NULL,
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES] =
    g_param_spec_boolean ("system-supports-color-schemes", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme", NULL, NULL,
                       ADAP_TYPE_SYSTEM_COLOR_SCHEME,
                       ADAP_SYSTEM_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adap_settings_init (AdapSettings *self)
{
}

AdapSettings *
adap_settings_get_default (void)
{
  if (!default_instance)
    default_instance = g_object_new (ADAP_TYPE_SETTINGS, NULL);

  return default_instance;
}

gboolean
adap_settings_get_system_supports_color_schemes (AdapSettings *self)
{
  g_return_val_if_fail (ADAP_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->system_supports_color_schemes_override;

  return self->system_supports_color_schemes;
}

const gchar *
adap_settings_get_theme_name (AdapSettings *self)
{
  g_return_val_if_fail (ADAP_IS_SETTINGS (self), NULL);

  return self->theme_name;
}

AdapSystemColorScheme
adap_settings_get_color_scheme (AdapSettings *self)
{
  g_return_val_if_fail (ADAP_IS_SETTINGS (self), ADAP_SYSTEM_COLOR_SCHEME_DEFAULT);

  if (self->override)
    return self->color_scheme_override;

  return self->color_scheme;
}

gboolean
adap_settings_get_high_contrast (AdapSettings *self)
{
  g_return_val_if_fail (ADAP_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->high_contrast_override;

  return self->high_contrast;
}

void
adap_settings_start_override (AdapSettings *self)
{
  g_return_if_fail (ADAP_IS_SETTINGS (self));

  if (self->override)
    return;

  self->override = TRUE;

  self->system_supports_color_schemes_override = self->system_supports_color_schemes;
  self->color_scheme_override = self->color_scheme;
  self->high_contrast_override = self->high_contrast;
}

void
adap_settings_end_override (AdapSettings *self)
{
  gboolean notify_system_supports_color_scheme, notify_color_scheme, notify_hc;

  g_return_if_fail (ADAP_IS_SETTINGS (self));

  if (!self->override)
    return;

  notify_system_supports_color_scheme = self->system_supports_color_schemes_override != self->system_supports_color_schemes;
  notify_color_scheme = self->color_scheme_override != self->color_scheme;
  notify_hc = self->high_contrast_override != self->high_contrast;

  self->override = FALSE;
  self->system_supports_color_schemes_override = FALSE;
  self->color_scheme_override = ADAP_SYSTEM_COLOR_SCHEME_DEFAULT;
  self->high_contrast_override = FALSE;

  if (notify_system_supports_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
  if (notify_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
  if (notify_hc)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

void
adap_settings_override_system_supports_color_schemes (AdapSettings *self,
                                                     gboolean     system_supports_color_schemes)
{
  g_return_if_fail (ADAP_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  system_supports_color_schemes = !!system_supports_color_schemes;

  if (system_supports_color_schemes == self->system_supports_color_schemes_override)
    return;

  if (!system_supports_color_schemes)
    adap_settings_override_color_scheme (self, ADAP_SYSTEM_COLOR_SCHEME_DEFAULT);

  self->system_supports_color_schemes_override = system_supports_color_schemes;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
}

void
adap_settings_override_color_scheme (AdapSettings          *self,
                                    AdapSystemColorScheme  color_scheme)
{
  g_return_if_fail (ADAP_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  if (color_scheme == self->color_scheme_override ||
      !self->system_supports_color_schemes_override)
    return;

  self->color_scheme_override = color_scheme;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

void
adap_settings_override_high_contrast (AdapSettings *self,
                                     gboolean     high_contrast)
{
  g_return_if_fail (ADAP_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  high_contrast = !!high_contrast;

  if (high_contrast == self->high_contrast_override)
    return;

  self->high_contrast_override = high_contrast;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}
