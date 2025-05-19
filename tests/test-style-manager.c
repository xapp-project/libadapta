/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>
#include "adap-settings-private.h"

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_style_manager_color_scheme (void)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();
  AdapColorScheme color_scheme;
  int notified = 0;

  g_signal_connect_swapped (manager, "notify::color-scheme", G_CALLBACK (increment), &notified);

  g_object_get (manager, "color-scheme", &color_scheme, NULL);
  g_assert_cmpint (color_scheme, ==, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (notified, ==, 0);

  adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (notified, ==, 0);

  adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_PREFER_DARK);
  g_object_get (manager, "color-scheme", &color_scheme, NULL);
  g_assert_cmpint (color_scheme, ==, ADAP_COLOR_SCHEME_PREFER_DARK);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (manager, "color-scheme", ADAP_COLOR_SCHEME_PREFER_LIGHT, NULL);
  g_assert_cmpint (adap_style_manager_get_color_scheme (manager), ==, ADAP_COLOR_SCHEME_PREFER_LIGHT);
  g_assert_cmpint (notified, ==, 2);

  g_signal_handlers_disconnect_by_func (manager, increment, &notified);
  adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_DEFAULT);
}

static void
test_dark (AdapStyleManager *manager,
           gboolean         initial_dark,
           ...)
{
  va_list args;
  AdapColorScheme scheme;
  gboolean last_dark = initial_dark;
  int notified = 0, expected_notified = 0;

  g_signal_connect_swapped (manager, "notify::dark", G_CALLBACK (increment), &notified);

  va_start (args, initial_dark);

  for (scheme = ADAP_COLOR_SCHEME_DEFAULT; scheme <= ADAP_COLOR_SCHEME_FORCE_DARK; scheme++) {
    gboolean dark = va_arg (args, gboolean);

    adap_style_manager_set_color_scheme (manager, scheme);

    if (dark)
      g_assert_true (adap_style_manager_get_dark (manager));
    else
      g_assert_false (adap_style_manager_get_dark (manager));

    if (dark != last_dark)
      expected_notified++;

    g_assert_cmpint (notified, ==, expected_notified);

    last_dark = dark;
  }

  va_end (args);

  g_signal_handlers_disconnect_by_func (manager, increment, &notified);
}

static void
test_adap_style_manager_dark (void)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();
  AdapSettings *settings = adap_settings_get_default ();

  adap_settings_start_override (settings);
  adap_settings_override_system_supports_color_schemes (settings, TRUE);
  adap_settings_override_color_scheme (settings, ADAP_SYSTEM_COLOR_SCHEME_PREFER_LIGHT);

  test_dark (manager, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);

  adap_settings_override_color_scheme (settings, ADAP_SYSTEM_COLOR_SCHEME_DEFAULT);
  test_dark (manager, TRUE,  FALSE, FALSE, FALSE, TRUE, TRUE);

  adap_settings_override_color_scheme (settings, ADAP_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  test_dark (manager, TRUE,  TRUE, FALSE, TRUE, TRUE, TRUE);

  adap_settings_end_override (settings);

  adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_DEFAULT);
}

static void
test_adap_style_manager_high_contrast (void)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();
  AdapSettings *settings = adap_settings_get_default ();
  int notified = 0;

  adap_settings_start_override (settings);
  adap_settings_override_high_contrast (settings, FALSE);

  g_signal_connect_swapped (manager, "notify::high-contrast", G_CALLBACK (increment), &notified);

  g_assert_false (adap_style_manager_get_high_contrast (manager));

  adap_settings_override_high_contrast (settings, FALSE);
  g_assert_false (adap_style_manager_get_high_contrast (manager));
  g_assert_cmpint (notified, ==, 0);

  adap_settings_override_high_contrast (settings, TRUE);
  g_assert_true (adap_style_manager_get_high_contrast (manager));
  g_assert_cmpint (notified, ==, 1);

  adap_settings_end_override (settings);

  g_signal_handlers_disconnect_by_func (manager, increment, &notified);
}

static void
test_adap_style_manager_system_supports_color_schemes (void)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();
  AdapSettings *settings = adap_settings_get_default ();
  int notified = 0;

  adap_settings_start_override (settings);
  adap_settings_override_system_supports_color_schemes (settings, FALSE);

  g_signal_connect_swapped (manager, "notify::system-supports-color-schemes", G_CALLBACK (increment), &notified);

  g_assert_false (adap_style_manager_get_system_supports_color_schemes (manager));

  adap_settings_override_system_supports_color_schemes (settings, FALSE);
  g_assert_false (adap_style_manager_get_system_supports_color_schemes (manager));
  g_assert_cmpint (notified, ==, 0);

  adap_settings_override_system_supports_color_schemes (settings, TRUE);
  g_assert_true (adap_style_manager_get_system_supports_color_schemes (manager));
  g_assert_cmpint (notified, ==, 1);

  adap_settings_end_override (settings);

  g_signal_handlers_disconnect_by_func (manager, increment, &notified);
}

static void
test_adap_style_manager_inheritance (void)
{
  AdapStyleManager *default_manager = adap_style_manager_get_default ();
  AdapStyleManager *display_manager = adap_style_manager_get_for_display (gdk_display_get_default ());
  AdapSettings *settings = adap_settings_get_default ();

  adap_settings_start_override (settings);
  adap_settings_override_system_supports_color_schemes (settings, TRUE);
  adap_settings_override_color_scheme (settings, ADAP_SYSTEM_COLOR_SCHEME_DEFAULT);

  g_assert_cmpint (adap_style_manager_get_color_scheme (default_manager), ==, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (adap_style_manager_get_color_scheme (display_manager), ==, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_false (adap_style_manager_get_dark (default_manager));
  g_assert_false (adap_style_manager_get_dark (display_manager));

  adap_style_manager_set_color_scheme (default_manager, ADAP_COLOR_SCHEME_PREFER_DARK);

  g_assert_cmpint (adap_style_manager_get_color_scheme (display_manager), ==, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_true (adap_style_manager_get_dark (default_manager));
  g_assert_true (adap_style_manager_get_dark (display_manager));

  adap_style_manager_set_color_scheme (display_manager, ADAP_COLOR_SCHEME_PREFER_LIGHT);
  g_assert_true (adap_style_manager_get_dark (default_manager));
  g_assert_false (adap_style_manager_get_dark (display_manager));

  adap_settings_override_color_scheme (settings, ADAP_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  g_assert_true (adap_style_manager_get_dark (default_manager));
  g_assert_true (adap_style_manager_get_dark (display_manager));

  adap_style_manager_set_color_scheme (default_manager, ADAP_COLOR_SCHEME_FORCE_LIGHT);
  g_assert_false (adap_style_manager_get_dark (default_manager));
  g_assert_true (adap_style_manager_get_dark (display_manager));

  adap_style_manager_set_color_scheme (display_manager, ADAP_COLOR_SCHEME_DEFAULT);
  g_assert_false (adap_style_manager_get_dark (default_manager));
  g_assert_false (adap_style_manager_get_dark (display_manager));

  adap_settings_end_override (settings);
  adap_style_manager_set_color_scheme (default_manager, ADAP_COLOR_SCHEME_DEFAULT);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/StyleManager/color_scheme", test_adap_style_manager_color_scheme);
  g_test_add_func("/Adapta/StyleManager/dark", test_adap_style_manager_dark);
  g_test_add_func("/Adapta/StyleManager/high_contrast", test_adap_style_manager_high_contrast);
  g_test_add_func("/Adapta/StyleManager/system_supports_color_schemes", test_adap_style_manager_system_supports_color_schemes);
  g_test_add_func("/Adapta/StyleManager/inheritance", test_adap_style_manager_inheritance);

  return g_test_run();
}
