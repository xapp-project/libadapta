/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_preferences_group_add_remove (void)
{
  AdapPreferencesGroup *group = g_object_ref_sink (ADAP_PREFERENCES_GROUP (adap_preferences_group_new ()));
  AdapPreferencesRow *row;
  GtkWidget *widget;

  g_assert_nonnull (group);

  row = ADAP_PREFERENCES_ROW (adap_preferences_row_new ());
  g_assert_nonnull (row);
  adap_preferences_group_add (group, GTK_WIDGET (row));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  adap_preferences_group_add (group, widget);

  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (GTK_WIDGET (row)), GTK_TYPE_LIST_BOX));
  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (widget), GTK_TYPE_BOX));

  adap_preferences_group_remove (group, GTK_WIDGET (row));
  adap_preferences_group_remove (group, widget);

  g_assert_finalize_object (group);
}


static void
test_adap_preferences_group_title (void)
{
  AdapPreferencesGroup *group = g_object_ref_sink (ADAP_PREFERENCES_GROUP (adap_preferences_group_new ()));

  g_assert_nonnull (group);

  g_assert_cmpstr (adap_preferences_group_get_title (group), ==, "");

  adap_preferences_group_set_title (group, "Dummy title");
  g_assert_cmpstr (adap_preferences_group_get_title (group), ==, "Dummy title");

  adap_preferences_group_set_title (group, NULL);
  g_assert_cmpstr (adap_preferences_group_get_title (group), ==, "");

  g_assert_finalize_object (group);
}


static void
test_adap_preferences_group_description (void)
{
  AdapPreferencesGroup *group = g_object_ref_sink (ADAP_PREFERENCES_GROUP (adap_preferences_group_new ()));

  g_assert_nonnull (group);

  g_assert_cmpstr (adap_preferences_group_get_description (group), ==, "");

  adap_preferences_group_set_description (group, "Dummy description");
  g_assert_cmpstr (adap_preferences_group_get_description (group), ==, "Dummy description");

  adap_preferences_group_set_description (group, NULL);
  g_assert_cmpstr (adap_preferences_group_get_description (group), ==, "");

  g_assert_finalize_object (group);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PreferencesGroup/add_remove", test_adap_preferences_group_add_remove);
  g_test_add_func("/Adapta/PreferencesGroup/title", test_adap_preferences_group_title);
  g_test_add_func("/Adapta/PreferencesGroup/description", test_adap_preferences_group_description);

  return g_test_run();
}
