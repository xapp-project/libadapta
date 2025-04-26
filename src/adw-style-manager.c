/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-style-manager-private.h"

#include "adw-main-private.h"
#include "adw-settings-private.h"
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#define SWITCH_DURATION 250

/**
 * AdwColorScheme:
 * @ADW_COLOR_SCHEME_DEFAULT: Inherit the parent color-scheme. When set on the
 *   `AdwStyleManager` returned by [func@StyleManager.get_default], it's
 *   equivalent to `ADW_COLOR_SCHEME_PREFER_LIGHT`.
 * @ADW_COLOR_SCHEME_FORCE_LIGHT: Always use light appearance.
 * @ADW_COLOR_SCHEME_PREFER_LIGHT: Use light appearance unless the system
 *   prefers dark colors.
 * @ADW_COLOR_SCHEME_PREFER_DARK: Use dark appearance unless the system prefers
 *   prefers light colors.
 * @ADW_COLOR_SCHEME_FORCE_DARK: Always use dark appearance.
 *
 * Application color schemes for [property@StyleManager:color-scheme].
 */

/**
 * AdwStyleManager:
 *
 * A class for managing application-wide styling.
 *
 * `AdwStyleManager` provides a way to query and influence the application
 * styles, such as whether to use dark or high contrast appearance.
 *
 * It allows to set the color scheme via the
 * [property@StyleManager:color-scheme] property, and to query the current
 * appearance, as well as whether a system-wide color scheme preference exists.
 */

struct _AdwStyleManager
{
  GObject parent_instance;

  GdkDisplay *display;
  AdwSettings *settings;
  GtkCssProvider *provider;
  GtkCssProvider *colors_provider;

  AdwColorScheme color_scheme;
  gboolean dark;
  gboolean setting_dark;

  GtkCssProvider *animations_provider;
  guint animation_timeout_id;

  gchar *real_theme_name;
};

G_DEFINE_FINAL_TYPE (AdwStyleManager, adw_style_manager, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_DISPLAY,
  PROP_COLOR_SCHEME,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_DARK,
  PROP_HIGH_CONTRAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static GHashTable *display_style_managers = NULL;
static AdwStyleManager *default_instance = NULL;

static void
debug_theme_valist (const gchar *format,
                    va_list      args)
{
  static gsize init = 0;
  static gboolean debug = FALSE;

  if (g_once_init_enter (&init))
    {
      debug = !!g_getenv ("ADW_DEBUG_THEMES");
      g_once_init_leave (&init, 1);
    }

  if (debug)
    g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, args);
}

static void
debug_theme (const gchar *format,
             ...)
{
  va_list args;
  va_start (args, format);
  debug_theme_valist (format, args);
  va_end (args);
}

static void
warn_prefer_dark_theme (AdwStyleManager *self)
{
  if (self->setting_dark)
    return;

  g_warning ("Using GtkSettings:gtk-application-prefer-dark-theme with "
             "libadwaita is unsupported. Please use "
             "AdwStyleManager:color-scheme instead.");
}

static void
unregister_display (GdkDisplay *display)
{
  g_assert (g_hash_table_contains (display_style_managers, display));

  g_hash_table_remove (display_style_managers, display);
}

static void
register_display (GdkDisplayManager *display_manager,
                  GdkDisplay        *display)
{
  AdwStyleManager *style_manager;

  style_manager = g_object_new (ADW_TYPE_STYLE_MANAGER,
                                "display", display,
                                NULL);

  g_assert (!g_hash_table_contains (display_style_managers, display));

  g_hash_table_insert (display_style_managers, display, style_manager);

  g_signal_connect (display,
                    "closed",
                    G_CALLBACK (unregister_display),
                    NULL);
}

static void
enable_animations_cb (AdwStyleManager *self)
{
  gtk_style_context_remove_provider_for_display (self->display,
                                                 GTK_STYLE_PROVIDER (self->animations_provider));

  self->animation_timeout_id = 0;
}

static gboolean
find_theme_dir_each (const gchar  *dir,
                     const gchar  *subdir,
                     const gchar  *name,
                     gboolean      hc,
                     gboolean      dark,
                     gchar       **found_theme_path,
                     gchar       **found_base_path,
                     gchar       **found_colors_path)
{
  const gchar *base_file;
  const gchar *color_file;

  g_autofree gchar *top_theme_dir = NULL;
  g_autofree gchar *parent_dir = NULL;
  g_autofree gchar *base_path = NULL;
  g_autofree gchar *colors_path = NULL;
  g_autofree gchar *version_dir = NULL;

  g_clear_pointer (found_theme_path, g_free);
  g_clear_pointer (found_base_path, g_free);
  g_clear_pointer (found_colors_path, g_free);

  if (hc)
    base_file = "base-hc.css";
  else
    base_file = "base.css";

  if (dark)
    color_file = "defaults-dark.css";
  else
    color_file = "defaults-light.css";

  top_theme_dir = g_build_filename (dir, subdir, NULL);
  parent_dir = g_build_filename (top_theme_dir, name, NULL);

  debug_theme ("Looking for theme '%s' in '%s'", name, top_theme_dir);

  if (!g_file_test (parent_dir, G_FILE_TEST_EXISTS))
    {
      return FALSE;
    }

  debug_theme ("Found theme directory '%s'.", parent_dir);

  version_dir = g_strdup_printf ("libadwaita-%d.%d", ADW_MAJOR_VERSION, ADW_MINOR_VERSION);
  base_path = g_build_filename (parent_dir, version_dir, base_file, NULL);
  colors_path = g_build_filename (parent_dir, version_dir, color_file, NULL);

  if (g_file_test (base_path, G_FILE_TEST_EXISTS))
    {
      *found_base_path = g_strdup (base_path);
    }
  else
    {
      // If the user wants high-contrast and the current theme doesn't support it,
      // the default theme should be used.
      if (hc)
        {
          debug_theme ("No high-contrast variant found, skipping this theme.");
          return FALSE;
        }
    }

  if (g_file_test (colors_path, G_FILE_TEST_EXISTS))
    {
      *found_colors_path = g_strdup (colors_path);
    }
  else
    {
      // If the user wants the dark variant and the current theme doesn't support it, check
      // for its normal variant.
      if (dark)
        {
          debug_theme ("No dark variant found, trying light variant instead.");
          return find_theme_dir_each (dir, subdir, name, hc, FALSE, found_theme_path, found_base_path, found_colors_path);
        }
    }

  if (*found_base_path && *found_colors_path)
    {
      *found_theme_path = g_strdup (parent_dir);
      return TRUE;
    }

  return FALSE;
}

static gboolean
find_theme_dir (const gchar  *name,
                gboolean      hc,
                gboolean      dark,
                gchar       **found_theme_path,
                gchar       **found_base_path,
                gchar       **found_colors_path)
{
  const char *const *dirs;
  int i;
  /* First look in the user's data directory */
  if (find_theme_dir_each (g_get_user_data_dir (), "themes", name, hc, dark, found_theme_path, found_base_path, found_colors_path))
    return TRUE;

  /* Next look in the user's home directory */
  if (find_theme_dir_each (g_get_home_dir (), ".themes", name, hc, dark, found_theme_path, found_base_path, found_colors_path))
    return TRUE;

  /* Look in system data directories */
  dirs = g_get_system_data_dirs ();
  for (i = 0; dirs[i]; i++)
    {
      if (find_theme_dir_each (dirs[i], "themes", name, hc, dark, found_theme_path, found_base_path, found_colors_path))
        return TRUE;
    }

  return FALSE;
}

static void
update_stylesheet (AdwStyleManager *self)
{
  GtkSettings *gtk_settings;

  if (!self->display)
    return;

  gtk_settings = gtk_settings_get_for_display (self->display);

  if (self->animation_timeout_id)
    g_clear_handle_id (&self->animation_timeout_id, g_source_remove);

  gtk_style_context_add_provider_for_display (self->display,
                                              GTK_STYLE_PROVIDER (self->animations_provider),
                                              10000);

  self->setting_dark = TRUE;

  g_object_set (gtk_settings,
                "gtk-application-prefer-dark-theme", self->dark,
                NULL);

  self->setting_dark = FALSE;

  gchar *found_theme_path = NULL;
  gchar *found_base_path = NULL;
  gchar *found_colors_path = NULL;

  if (find_theme_dir (self->real_theme_name,
                      adw_settings_get_high_contrast (self->settings),
                      self->dark,
                      &found_theme_path,
                      &found_base_path,
                      &found_colors_path))
    {
      debug_theme ("Using theme '%s' found in %s.", self->real_theme_name, found_theme_path);

      if (self->provider)
        gtk_css_provider_load_from_path (self->provider, found_base_path);
      if (self->colors_provider)
        gtk_css_provider_load_from_path (self->colors_provider, found_colors_path);

      g_free (found_theme_path);
      g_free (found_base_path);
      g_free (found_colors_path);
    }
  else
    {
      debug_theme ("No libadwaita support in system theme, using default style.");
      if (self->provider) {
        if (adw_settings_get_high_contrast (self->settings))
          gtk_css_provider_load_from_resource (self->provider,
                                               "/org/gnome/Adwaita/styles/base-hc.css");
        else
          gtk_css_provider_load_from_resource (self->provider,
                                               "/org/gnome/Adwaita/styles/base.css");
      }

      if (self->colors_provider) {
        if (self->dark)
          gtk_css_provider_load_from_resource (self->colors_provider,
                                               "/org/gnome/Adwaita/styles/defaults-dark.css");
        else
          gtk_css_provider_load_from_resource (self->colors_provider,
                                               "/org/gnome/Adwaita/styles/defaults-light.css");
      }
    }

  self->animation_timeout_id =
    g_timeout_add_once (SWITCH_DURATION,
                        (GSourceOnceFunc) enable_animations_cb,
                        self);
}

static gboolean
get_is_dark (AdwStyleManager *self)
{
  AdwSystemColorScheme system_scheme = adw_settings_get_color_scheme (self->settings);

  switch (self->color_scheme) {
  case ADW_COLOR_SCHEME_DEFAULT:
    if (self->display)
      return get_is_dark (default_instance);
    return (system_scheme == ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  case ADW_COLOR_SCHEME_FORCE_LIGHT:
    return FALSE;
  case ADW_COLOR_SCHEME_PREFER_LIGHT:
    return system_scheme == ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
  case ADW_COLOR_SCHEME_PREFER_DARK:
    return system_scheme != ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;
  case ADW_COLOR_SCHEME_FORCE_DARK:
    return TRUE;
  default:
    g_assert_not_reached ();
  }
}

static void
update_dark (AdwStyleManager *self)
{
  gboolean dark = get_is_dark (self);

  if (dark == self->dark)
    return;

  self->dark = dark;

  update_stylesheet (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DARK]);
}

static void
notify_system_supports_color_schemes_cb (AdwStyleManager *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
}

static void
notify_high_contrast_cb (AdwStyleManager *self)
{
  update_stylesheet (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

static void
adw_style_manager_constructed (GObject *object)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  G_OBJECT_CLASS (adw_style_manager_parent_class)->constructed (object);

  if (self->display) {
    GtkSettings *settings = gtk_settings_get_for_display (self->display);
    gboolean prefer_dark_theme;

    g_object_get (settings,
                  "gtk-application-prefer-dark-theme", &prefer_dark_theme,
                  "gtk-theme-name", &self->real_theme_name,
                  NULL);

    if (prefer_dark_theme)
      warn_prefer_dark_theme (self);

    g_signal_connect_object (settings,
                             "notify::gtk-application-prefer-dark-theme",
                             G_CALLBACK (warn_prefer_dark_theme),
                             self,
                             G_CONNECT_SWAPPED);

    if (!adw_is_granite_present () && !g_getenv ("GTK_THEME")) {
      g_object_set (gtk_settings_get_for_display (self->display),
                    "gtk-theme-name", "Adwaita-empty",
                    NULL);

      self->provider = gtk_css_provider_new ();
      gtk_style_context_add_provider_for_display (self->display,
                                                  GTK_STYLE_PROVIDER (self->provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_THEME);

      self->colors_provider = gtk_css_provider_new ();
      gtk_style_context_add_provider_for_display (self->display,
                                                  GTK_STYLE_PROVIDER (self->colors_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_THEME);
    }

    self->animations_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_string (self->animations_provider,
                                       "* { transition: none; }");
  }

  self->settings = adw_settings_get_default ();

  g_signal_connect_object (self->settings,
                           "notify::system-supports-color-schemes",
                           G_CALLBACK (notify_system_supports_color_schemes_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::color-scheme",
                           G_CALLBACK (update_dark),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::high-contrast",
                           G_CALLBACK (notify_high_contrast_cb),
                           self,
                           G_CONNECT_SWAPPED);

  update_dark (self);
  update_stylesheet (self);
}

static void
adw_style_manager_dispose (GObject *object)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  g_clear_handle_id (&self->animation_timeout_id, g_source_remove);
  g_clear_object (&self->provider);
  g_clear_object (&self->colors_provider);
  g_clear_object (&self->animations_provider);
  g_clear_pointer (&self->real_theme_name, g_free);

  G_OBJECT_CLASS (adw_style_manager_parent_class)->dispose (object);
}

static void
adw_style_manager_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    g_value_set_object (value, adw_style_manager_get_display (self));
    break;

  case PROP_COLOR_SCHEME:
    g_value_set_enum (value, adw_style_manager_get_color_scheme (self));
    break;

  case PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES:
    g_value_set_boolean (value, adw_style_manager_get_system_supports_color_schemes (self));
    break;

  case PROP_DARK:
    g_value_set_boolean (value, adw_style_manager_get_dark (self));
    break;

  case PROP_HIGH_CONTRAST:
    g_value_set_boolean (value, adw_style_manager_get_high_contrast (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_manager_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    self->display = g_value_get_object (value);
    break;

  case PROP_COLOR_SCHEME:
    adw_style_manager_set_color_scheme (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_manager_class_init (AdwStyleManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_style_manager_constructed;
  object_class->dispose = adw_style_manager_dispose;
  object_class->get_property = adw_style_manager_get_property;
  object_class->set_property = adw_style_manager_set_property;

  /**
   * AdwStyleManager:display: (attributes org.gtk.Property.get=adw_style_manager_get_display)
   *
   * The display the style manager is associated with.
   *
   * The display will be `NULL` for the style manager returned by
   * [func@StyleManager.get_default].
   */
  props[PROP_DISPLAY] =
    g_param_spec_object ("display", NULL, NULL,
                         GDK_TYPE_DISPLAY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:color-scheme: (attributes org.gtk.Property.get=adw_style_manager_get_color_scheme org.gtk.Property.set=adw_style_manager_set_color_scheme)
   *
   * The requested application color scheme.
   *
   * The effective appearance will be decided based on the application color
   * scheme and the system preferred color scheme. The
   * [property@StyleManager:dark] property can be used to query the current
   * effective appearance.
   *
   * The `ADW_COLOR_SCHEME_PREFER_LIGHT` color scheme results in the application
   * using light appearance unless the system prefers dark colors. This is the
   * default value.
   *
   * The `ADW_COLOR_SCHEME_PREFER_DARK` color scheme results in the application
   * using dark appearance, but can still switch to the light appearance if the
   * system can prefers it, for example, when the high contrast preference is
   * enabled.
   *
   * The `ADW_COLOR_SCHEME_FORCE_LIGHT` and `ADW_COLOR_SCHEME_FORCE_DARK` values
   * ignore the system preference entirely. They are useful if the application
   * wants to match its UI to its content or to provide a separate color scheme
   * switcher.
   *
   * If a per-[class@Gdk.Display] style manager has its color scheme set to
   * `ADW_COLOR_SCHEME_DEFAULT`, it will inherit the color scheme from the
   * default style manager.
   *
   * For the default style manager, `ADW_COLOR_SCHEME_DEFAULT` is equivalent to
   * `ADW_COLOR_SCHEME_PREFER_LIGHT`.
   *
   * The [property@StyleManager:system-supports-color-schemes] property can be
   * used to check if the current environment provides a color scheme
   * preference.
   */
  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme", NULL, NULL,
                       ADW_TYPE_COLOR_SCHEME,
                       ADW_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStyleManager:system-supports-color-schemes: (attributes org.gtk.Property.get=adw_style_manager_get_system_supports_color_schemes)
   *
   * Whether the system supports color schemes.
   *
   * This property can be used to check if the current environment provides a
   * color scheme preference. For example, applications might want to show a
   * separate appearance switcher if it's set to `FALSE`.
   *
   * See [property@StyleManager:color-scheme].
   */
  props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES] =
    g_param_spec_boolean ("system-supports-color-schemes", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:dark: (attributes org.gtk.Property.get=adw_style_manager_get_dark)
   *
   * Whether the application is using dark appearance.
   *
   * This property can be used to query the current appearance, as requested via
   * [property@StyleManager:color-scheme].
   */
  props[PROP_DARK] =
    g_param_spec_boolean ("dark", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:high-contrast: (attributes org.gtk.Property.get=adw_style_manager_get_high_contrast)
   *
   * Whether the application is using high contrast appearance.
   *
   * This cannot be overridden by applications.
   */
  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_style_manager_init (AdwStyleManager *self)
{
  self->color_scheme = ADW_COLOR_SCHEME_DEFAULT;
}

void
adw_style_manager_ensure (void)
{
  GdkDisplayManager *display_manager = gdk_display_manager_get ();
  GSList *displays;
  GSList *l;

  if (display_style_managers)
    return;

  default_instance = g_object_new (ADW_TYPE_STYLE_MANAGER, NULL);
  display_style_managers = g_hash_table_new_full (g_direct_hash,
                                                  g_direct_equal,
                                                  NULL,
                                                  g_object_unref);

  displays = gdk_display_manager_list_displays (display_manager);

  for (l = displays; l; l = l->next)
    register_display (display_manager, l->data);

  g_signal_connect (display_manager,
                    "display-opened",
                    G_CALLBACK (register_display),
                    NULL);

  g_slist_free (displays);
}

/**
 * adw_style_manager_get_default:
 *
 * Gets the default `AdwStyleManager` instance.
 *
 * It manages all [class@Gdk.Display] instances unless the style manager for
 * that display has an override.
 *
 * See [func@StyleManager.get_for_display].
 *
 * Returns: (transfer none): the default style manager
 */
AdwStyleManager *
adw_style_manager_get_default (void)
{
  if (!default_instance)
    adw_style_manager_ensure ();

  return default_instance;
}

/**
 * adw_style_manager_get_for_display:
 * @display: a `GdkDisplay`
 *
 * Gets the `AdwStyleManager` instance managing @display.
 *
 * It can be used to override styles for that specific display instead of the
 * whole application.
 *
 * Most applications should use [func@StyleManager.get_default] instead.
 *
 * Returns: (transfer none): the style manager for @display
 */
AdwStyleManager *
adw_style_manager_get_for_display (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  if (!display_style_managers)
    adw_style_manager_ensure ();

  g_return_val_if_fail (g_hash_table_contains (display_style_managers, display), NULL);

  return g_hash_table_lookup (display_style_managers, display);
}

/**
 * adw_style_manager_get_display: (attributes org.gtk.Method.get_property=display)
 * @self: a style manager
 *
 * Gets the display the style manager is associated with.
 *
 * The display will be `NULL` for the style manager returned by
 * [func@StyleManager.get_default].
 *
 * Returns: (transfer none) (nullable): the display
 */
GdkDisplay *
adw_style_manager_get_display (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), NULL);

  return self->display;
}

/**
 * adw_style_manager_get_color_scheme: (attributes org.gtk.Method.get_property=color-scheme)
 * @self: a style manager
 *
 * Gets the requested application color scheme.
 *
 * Returns: the color scheme
 */
AdwColorScheme
adw_style_manager_get_color_scheme (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), ADW_COLOR_SCHEME_DEFAULT);

  return self->color_scheme;
}

/**
 * adw_style_manager_set_color_scheme: (attributes org.gtk.Method.set_property=color-scheme)
 * @self: a style manager
 * @color_scheme: the color scheme
 *
 * Sets the requested application color scheme.
 *
 * The effective appearance will be decided based on the application color
 * scheme and the system preferred color scheme. The
 * [property@StyleManager:dark] property can be used to query the current
 * effective appearance.
 *
 * The `ADW_COLOR_SCHEME_PREFER_LIGHT` color scheme results in the application
 * using light appearance unless the system prefers dark colors. This is the
 * default value.
 *
 * The `ADW_COLOR_SCHEME_PREFER_DARK` color scheme results in the application
 * using dark appearance, but can still switch to the light appearance if the
 * system can prefers it, for example, when the high contrast preference is
 * enabled.
 *
 * The `ADW_COLOR_SCHEME_FORCE_LIGHT` and `ADW_COLOR_SCHEME_FORCE_DARK` values
 * ignore the system preference entirely. They are useful if the application
 * wants to match its UI to its content or to provide a separate color scheme
 * switcher.
 *
 * If a per-[class@Gdk.Display] style manager has its color scheme set to
 * `ADW_COLOR_SCHEME_DEFAULT`, it will inherit the color scheme from the
 * default style manager.
 *
 * For the default style manager, `ADW_COLOR_SCHEME_DEFAULT` is equivalent to
 * `ADW_COLOR_SCHEME_PREFER_LIGHT`.
 *
 * The [property@StyleManager:system-supports-color-schemes] property can be
 * used to check if the current environment provides a color scheme
 * preference.
 */
void
adw_style_manager_set_color_scheme (AdwStyleManager *self,
                                    AdwColorScheme   color_scheme)
{
  g_return_if_fail (ADW_IS_STYLE_MANAGER (self));

  if (color_scheme == self->color_scheme)
    return;

  self->color_scheme = color_scheme;

  g_object_freeze_notify (G_OBJECT (self));

  update_dark (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);

  g_object_thaw_notify (G_OBJECT (self));

  if (!self->display) {
    GHashTableIter iter;
    AdwStyleManager *manager;

    g_hash_table_iter_init (&iter, display_style_managers);

    while (g_hash_table_iter_next (&iter, NULL, (gpointer) &manager))
      if (manager->color_scheme == ADW_COLOR_SCHEME_DEFAULT)
        update_dark (manager);
  }
}

/**
 * adw_style_manager_get_system_supports_color_schemes: (attributes org.gtk.Method.get_property=system-supports-color-schemes)
 * @self: a style manager
 *
 * Gets whether the system supports color schemes.
 *
 * This can be used to check if the current environment provides a color scheme
 * preference. For example, applications might want to show a separate
 * appearance switcher if it's set to `FALSE`.
 *
 * Returns: whether the system supports color schemes
 */
gboolean
adw_style_manager_get_system_supports_color_schemes (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return adw_settings_get_system_supports_color_schemes (self->settings);
}

/**
 * adw_style_manager_get_dark: (attributes org.gtk.Method.get_property=dark)
 * @self: a style manager
 *
 * Gets whether the application is using dark appearance.
 *
 * This can be used to query the current appearance, as requested via
 * [property@StyleManager:color-scheme].
 *
 * Returns: whether the application is using dark appearance
 */
gboolean
adw_style_manager_get_dark (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return self->dark;
}

/**
 * adw_style_manager_get_high_contrast: (attributes org.gtk.Method.get_property=high-contrast)
 * @self: a style manager
 *
 * Gets whether the application is using high contrast appearance.
 *
 * This cannot be overridden by applications.
 *
 * Returns: whether the application is using high contrast appearance
 */
gboolean
adw_style_manager_get_high_contrast (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return adw_settings_get_high_contrast (self->settings);
}
