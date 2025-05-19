/*
 * Copyright (C) 2021 Nahuel Gomez Castro
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-application.h"
#include "adap-main-private.h"

/**
 * AdapApplication:
 *
 * A base class for Adapta applications.
 *
 * `AdapApplication` handles library initialization by calling [func@init] in the
 * default [signal@Gio.Application::startup] signal handler, in turn chaining up
 * as required by [class@Gtk.Application]. Therefore, any subclass of
 * `AdapApplication` should always chain up its `startup` handler before using
 * any Adapta or GTK API.
 *
 * ## Automatic Resources
 *
 * `AdapApplication` will automatically load stylesheets located in the
 * application's resource base path (see
 * [method@Gio.Application.set_resource_base_path], if they're present.
 *
 * They can be used to add custom styles to the application, as follows:
 *
 * - `style.css` contains styles that are always present.
 *
 * - `style-dark.css` contains styles only used when
 * [property@StyleManager:dark] is `TRUE`.
 *
 * - `style-hc.css` contains styles used when the system high contrast
 *   preference is enabled.
 *
 * - `style-hc-dark.css` contains styles used when the system high contrast
 *   preference is enabled and [property@StyleManager:dark] is `TRUE`.
 */

typedef struct
{
  GtkStyleProvider *base_style_provider;
  GtkStyleProvider *dark_style_provider;
  GtkStyleProvider *hc_style_provider;
  GtkStyleProvider *hc_dark_style_provider;
} AdapApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdapApplication, adap_application, GTK_TYPE_APPLICATION)

enum {
  PROP_0,
  PROP_STYLE_MANAGER,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static inline void
style_provider_set_enabled (GtkStyleProvider *provider,
                            gboolean          enabled)
{
  if (enabled)
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                provider,
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  else
    gtk_style_context_remove_provider_for_display (gdk_display_get_default (),
                                                   provider);
}

static void
update_stylesheet (AdapApplication *self)
{
  AdapApplicationPrivate *priv = adap_application_get_instance_private (self);
  AdapStyleManager *manager = adap_style_manager_get_default ();
  gboolean is_dark, is_hc;

  is_dark = adap_style_manager_get_dark (manager);
  is_hc = adap_style_manager_get_high_contrast (manager);

  if (priv->dark_style_provider)
    style_provider_set_enabled (priv->dark_style_provider, is_dark);

  if (priv->hc_style_provider)
    style_provider_set_enabled (priv->hc_style_provider, is_hc);

  if (priv->hc_dark_style_provider)
    style_provider_set_enabled (priv->hc_dark_style_provider, is_hc && is_dark);
}

static void
init_provider_from_file (GtkStyleProvider **provider,
                         GFile             *file)
{
  if (!g_file_query_exists (file, NULL)) {
    g_clear_object (&file);
    return;
  }

  *provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
  gtk_css_provider_load_from_file (GTK_CSS_PROVIDER (*provider), file);

  g_clear_object (&file);
}

static void
init_providers (AdapApplication *self)
{
  AdapApplicationPrivate *priv = adap_application_get_instance_private (self);
  const char *base_path;
  char *base_uri;
  GFile *base_file;

  base_path = g_application_get_resource_base_path (G_APPLICATION (self));

  if (base_path == NULL)
    return;

  base_uri = g_strconcat ("resource://", base_path, NULL);
  base_file = g_file_new_for_uri (base_uri);

  if (!adap_is_granite_present ()) {
    init_provider_from_file (&priv->base_style_provider,
                             g_file_get_child (base_file, "style.css"));
    init_provider_from_file (&priv->dark_style_provider,
                             g_file_get_child (base_file, "style-dark.css"));
    init_provider_from_file (&priv->hc_style_provider,
                             g_file_get_child (base_file, "style-hc.css"));
    init_provider_from_file (&priv->hc_dark_style_provider,
                             g_file_get_child (base_file, "style-hc-dark.css"));
  }

  g_object_unref (base_file);
  g_free (base_uri);
}

static void
init_styling (AdapApplication *self)
{
  AdapApplicationPrivate *priv = adap_application_get_instance_private (self);

  GdkDisplay *display = gdk_display_get_default ();

  if (display == NULL)
    return;

  if (priv->base_style_provider != NULL)
    gtk_style_context_add_provider_for_display (display,
                                                priv->base_style_provider,
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  /* If gdk_display_get_default() worked, it means that
   * gtk_settings adap_style_manager_get_default() won't return NULL, so we don't
   * need to check it separately */
  g_signal_connect_object (adap_style_manager_get_default (),
                           "notify::dark",
                           G_CALLBACK (update_stylesheet),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (adap_style_manager_get_default (),
                           "notify::high-contrast",
                           G_CALLBACK (update_stylesheet),
                           self,
                           G_CONNECT_SWAPPED);

  update_stylesheet (self);
}

static void
adap_application_startup (GApplication *application)
{
  AdapApplication *self = ADAP_APPLICATION (application);

  G_APPLICATION_CLASS (adap_application_parent_class)->startup (application);

  adap_init ();

  init_providers (self);
  init_styling (self);
}

static void
adap_application_dispose (GObject *object)
{
  AdapApplication *self = ADAP_APPLICATION (object);
  AdapApplicationPrivate *priv = adap_application_get_instance_private (self);

  g_clear_object (&priv->base_style_provider);
  g_clear_object (&priv->dark_style_provider);
  g_clear_object (&priv->hc_style_provider);
  g_clear_object (&priv->hc_dark_style_provider);

  G_OBJECT_CLASS (adap_application_parent_class)->dispose (object);
}

static void
adap_application_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdapApplication *self = ADAP_APPLICATION (object);

  switch (prop_id) {
  case PROP_STYLE_MANAGER:
    g_value_set_object (value, adap_application_get_style_manager (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_application_class_init (AdapApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  object_class->dispose = adap_application_dispose;
  object_class->get_property = adap_application_get_property;

  application_class->startup = adap_application_startup;

  /**
   * AdapApplication:style-manager: (attributes org.gtk.Property.get=adap_application_get_style_manager)
   *
   * The style manager for this application.
   *
   * This is a convenience property allowing to access `AdapStyleManager` through
   * property bindings or expressions.
   */
  props[PROP_STYLE_MANAGER] =
    g_param_spec_object ("style-manager", NULL, NULL,
                         ADAP_TYPE_STYLE_MANAGER,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adap_application_init (AdapApplication *self)
{
}

/**
 * adap_application_new:
 * @application_id: (nullable): The application ID
 * @flags: The application flags
 *
 * Creates a new `AdapApplication`.
 *
 * If `application_id` is not `NULL`, then it must be valid. See
 * [func@Gio.Application.id_is_valid].
 *
 * If no application ID is given then some features (most notably application
 * uniqueness) will be disabled.
 *
 * Returns: the newly created `AdapApplication`
 */
AdapApplication *
adap_application_new (const char        *application_id,
                     GApplicationFlags  flags)
{
  return g_object_new (ADAP_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

/**
 * adap_application_get_style_manager: (attributes org.gtk.Method.get_property=style-manager)
 * @self: an application
 *
 * Gets the style manager for @self.
 *
 * This is a convenience property allowing to access `AdapStyleManager` through
 * property bindings or expressions.
 *
 * Returns: (transfer none): the style manager
 */
AdapStyleManager *
adap_application_get_style_manager (AdapApplication *self)
{
  g_return_val_if_fail (ADAP_IS_APPLICATION (self), NULL);

  return adap_style_manager_get_default ();
}
