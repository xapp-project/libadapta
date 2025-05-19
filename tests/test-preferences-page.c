/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_preferences_page_add_remove (void)
{
  AdapPreferencesPage *page = g_object_ref_sink (ADAP_PREFERENCES_PAGE (adap_preferences_page_new ()));
  AdapPreferencesGroup *group;

  g_assert_nonnull (page);

  group = ADAP_PREFERENCES_GROUP (adap_preferences_group_new ());
  g_assert_nonnull (group);
  adap_preferences_page_add (page, group);

  adap_preferences_page_remove (page, group);

  g_assert_finalize_object (page);
}


static void
test_adap_preferences_page_icon_name (void)
{
  AdapPreferencesPage *page = g_object_ref_sink (ADAP_PREFERENCES_PAGE (adap_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_null (adap_preferences_page_get_icon_name (page));

  adap_preferences_page_set_icon_name (page, "dummy-icon-name");
  g_assert_cmpstr (adap_preferences_page_get_icon_name (page), ==, "dummy-icon-name");

  adap_preferences_page_set_icon_name (page, NULL);
  g_assert_null (adap_preferences_page_get_icon_name (page));

  g_assert_finalize_object (page);
}


static void
test_adap_preferences_page_title (void)
{
  AdapPreferencesPage *page = g_object_ref_sink (ADAP_PREFERENCES_PAGE (adap_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_cmpstr (adap_preferences_page_get_title (page), ==, "");

  adap_preferences_page_set_title (page, "Dummy title");
  g_assert_cmpstr (adap_preferences_page_get_title (page), ==, "Dummy title");

  adap_preferences_page_set_title (page, NULL);
  g_assert_cmpstr (adap_preferences_page_get_title (page), ==, "");

  g_assert_finalize_object (page);
}


static void
test_adap_preferences_page_description (void)
{
  AdapPreferencesPage *page = g_object_ref_sink (ADAP_PREFERENCES_PAGE (adap_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_cmpstr (adap_preferences_page_get_description (page), ==, "");

  adap_preferences_page_set_description (page, "Dummy description");
  g_assert_cmpstr (adap_preferences_page_get_description (page), ==, "Dummy description");

  adap_preferences_page_set_description (page, NULL);
  g_assert_cmpstr (adap_preferences_page_get_description (page), ==, "");

  g_assert_finalize_object (page);
}


static void
test_adap_preferences_page_use_underline (void)
{
  AdapPreferencesPage *page = g_object_ref_sink (ADAP_PREFERENCES_PAGE (adap_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_false (adap_preferences_page_get_use_underline (page));

  adap_preferences_page_set_use_underline (page, TRUE);
  g_assert_true (adap_preferences_page_get_use_underline (page));

  adap_preferences_page_set_use_underline (page, FALSE);
  g_assert_false (adap_preferences_page_get_use_underline (page));

  g_assert_finalize_object (page);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PreferencesPage/add_remove", test_adap_preferences_page_add_remove);
  g_test_add_func("/Adapta/PreferencesPage/icon_name", test_adap_preferences_page_icon_name);
  g_test_add_func("/Adapta/PreferencesPage/title", test_adap_preferences_page_title);
  g_test_add_func("/Adapta/PreferencesPage/description", test_adap_preferences_page_description);
  g_test_add_func("/Adapta/PreferencesPage/use_underline", test_adap_preferences_page_use_underline);

  return g_test_run();
}
