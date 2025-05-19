/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_preferences_row_title (void)
{
  AdapPreferencesRow *row = g_object_ref_sink (ADAP_PREFERENCES_ROW (adap_preferences_row_new ()));

  g_assert_nonnull (row);

  g_assert_cmpstr (adap_preferences_row_get_title (row), ==, "");

  adap_preferences_row_set_title (row, "Dummy title");
  g_assert_cmpstr (adap_preferences_row_get_title (row), ==, "Dummy title");

  adap_preferences_row_set_title (row, NULL);
  g_assert_cmpstr (adap_preferences_row_get_title (row), ==, "");

  adap_preferences_row_set_use_markup (row, FALSE);
  adap_preferences_row_set_title (row, "Invalid <b>markup");
  g_assert_cmpstr (adap_preferences_row_get_title (row), ==, "Invalid <b>markup");

  g_assert_finalize_object (row);
}


static void
test_adap_preferences_row_use_undeline (void)
{
  AdapPreferencesRow *row = g_object_ref_sink (ADAP_PREFERENCES_ROW (adap_preferences_row_new ()));

  g_assert_nonnull (row);

  g_assert_false (adap_preferences_row_get_use_underline (row));

  adap_preferences_row_set_use_underline (row, TRUE);
  g_assert_true (adap_preferences_row_get_use_underline (row));

  adap_preferences_row_set_use_underline (row, FALSE);
  g_assert_false (adap_preferences_row_get_use_underline (row));

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PreferencesRow/title", test_adap_preferences_row_title);
  g_test_add_func("/Adapta/PreferencesRow/use_underline", test_adap_preferences_row_use_undeline);

  return g_test_run();
}
