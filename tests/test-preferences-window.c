/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
test_adap_preferences_window_add_remove (void)
{
  AdapPreferencesWindow *window = ADAP_PREFERENCES_WINDOW (adap_preferences_window_new ());
  AdapPreferencesPage *page;

  g_assert_nonnull (window);

  page = ADAP_PREFERENCES_PAGE (adap_preferences_page_new ());
  g_assert_nonnull (page);
  adap_preferences_window_add (window, page);

  adap_preferences_window_remove (window, page);

  g_assert_finalize_object (window);
}

static void
test_adap_preferences_window_add_toast (void)
{
  AdapPreferencesWindow *window = ADAP_PREFERENCES_WINDOW (adap_preferences_window_new ());
  AdapToast *toast = adap_toast_new ("Test Notification");

  g_assert_nonnull (window);
  g_assert_nonnull (toast);

  adap_preferences_window_add_toast (window, g_object_ref (toast));

  g_assert_finalize_object (window);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PreferencesWindow/add_remove", test_adap_preferences_window_add_remove);
  g_test_add_func("/Adapta/PreferencesWindow/add_toast", test_adap_preferences_window_add_toast);

  return g_test_run();
}
