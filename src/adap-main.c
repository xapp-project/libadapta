/*
 * Copyright (C) 2018-2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "config.h"

#include "adap-main-private.h"

#include "adap-inspector-page-private.h"
#include "adap-style-manager-private.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

static int adap_initialized = FALSE;

/**
 * adap_init:
 *
 * Initializes Libadapta.
 *
 * This function can be used instead of [func@Gtk.init] as it initializes GTK
 * implicitly.
 *
 * There's no need to call this function if you're using [class@Application].
 *
 * If Libadapta has already been initialized, the function will simply return.
 *
 * This makes sure translations, types, themes, and icons for the Adapta
 * library are set up properly.
 */
void
adap_init (void)
{
  if (adap_initialized)
    return;

  gtk_init ();

  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  adap_init_public_types ();

  if (!adap_is_granite_present ()) {
    gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                      "/org/gnome/Adapta/icons");

    adap_style_manager_ensure ();

    if (g_io_extension_point_lookup ("gtk-inspector-page"))
      g_io_extension_point_implement ("gtk-inspector-page",
                                      ADAP_TYPE_INSPECTOR_PAGE,
                                      "libadapta",
                                      10);
  }

  adap_initialized = TRUE;
}

/**
 * adap_is_initialized:
 *
 * Use this function to check if libadapta has been initialized with
 * [func@init].
 *
 * Returns: the initialization status
 */
gboolean
adap_is_initialized (void)
{
  return adap_initialized;
}

/*
 * Some applications, like Epiphany, are used on both GNOME and elementary.
 * Make it possible to integrate those apps with it while still using libadapta.
 */
gboolean
adap_is_granite_present (void)
{
  static int present = -1;

  if (present == -1) {
    GType granite_settings = g_type_from_name ("GraniteSettings");

    present = granite_settings != G_TYPE_INVALID;
  }

  return present;
}
