/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_expander_row_add_remove (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));
  GtkWidget *child;

  g_assert_nonnull (row);

  child = gtk_list_box_row_new ();
  g_assert_nonnull (child);

  adap_expander_row_add_row (row, child);
  adap_expander_row_remove (row, child);

  g_assert_finalize_object (row);
}


static void
test_adap_expander_row_subtitle (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_cmpstr (adap_expander_row_get_subtitle (row), ==, "");

  adap_expander_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (adap_expander_row_get_subtitle (row), ==, "Dummy subtitle");

  adap_preferences_row_set_use_markup (ADAP_PREFERENCES_ROW (row), FALSE);
  adap_expander_row_set_subtitle (row, "Invalid <b>markup");
  g_assert_cmpstr (adap_expander_row_get_subtitle (row), ==, "Invalid <b>markup");

  g_assert_finalize_object (row);
}


static void
test_adap_expander_row_expanded (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_false (adap_expander_row_get_expanded (row));

  adap_expander_row_set_expanded (row, TRUE);
  g_assert_true (adap_expander_row_get_expanded (row));

  adap_expander_row_set_expanded (row, FALSE);
  g_assert_false (adap_expander_row_get_expanded (row));

  g_assert_finalize_object (row);
}


static void
test_adap_expander_row_enable_expansion (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_true (adap_expander_row_get_enable_expansion (row));
  g_assert_false (adap_expander_row_get_expanded (row));

  adap_expander_row_set_expanded (row, TRUE);
  g_assert_true (adap_expander_row_get_expanded (row));

  adap_expander_row_set_enable_expansion (row, FALSE);
  g_assert_false (adap_expander_row_get_enable_expansion (row));
  g_assert_false (adap_expander_row_get_expanded (row));

  adap_expander_row_set_expanded (row, TRUE);
  g_assert_false (adap_expander_row_get_expanded (row));

  adap_expander_row_set_enable_expansion (row, TRUE);
  g_assert_true (adap_expander_row_get_enable_expansion (row));
  g_assert_true (adap_expander_row_get_expanded (row));

  g_assert_finalize_object (row);
}


static void
test_adap_expander_row_show_enable_switch (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_false (adap_expander_row_get_show_enable_switch (row));

  adap_expander_row_set_show_enable_switch (row, TRUE);
  g_assert_true (adap_expander_row_get_show_enable_switch (row));

  adap_expander_row_set_show_enable_switch (row, FALSE);
  g_assert_false (adap_expander_row_get_show_enable_switch (row));

  g_assert_finalize_object (row);
}

static void
test_adap_expander_row_title_lines (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adap_expander_row_get_title_lines (row), ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adap_action_row_set_title_lines: assertion 'title_lines >= 0' failed");
  adap_expander_row_set_title_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adap_expander_row_get_title_lines (row), ==, 0);

  adap_expander_row_set_title_lines (row, 1);
  g_assert_cmpint (adap_expander_row_get_title_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


static void
test_adap_expander_row_subtitle_lines (void)
{
  AdapExpanderRow *row = g_object_ref_sink (ADAP_EXPANDER_ROW (adap_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adap_expander_row_get_subtitle_lines (row), ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adap_action_row_set_subtitle_lines: assertion 'subtitle_lines >= 0' failed");
  adap_expander_row_set_subtitle_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adap_expander_row_get_subtitle_lines (row), ==, 0);

  adap_expander_row_set_subtitle_lines (row, 1);
  g_assert_cmpint (adap_expander_row_get_subtitle_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ExpanderRow/add_remove", test_adap_expander_row_add_remove);
  g_test_add_func("/Adapta/ExpanderRow/subtitle", test_adap_expander_row_subtitle);
  g_test_add_func("/Adapta/ExpanderRow/expanded", test_adap_expander_row_expanded);
  g_test_add_func("/Adapta/ExpanderRow/enable_expansion", test_adap_expander_row_enable_expansion);
  g_test_add_func("/Adapta/ExpanderRow/show_enable_switch", test_adap_expander_row_show_enable_switch);
  g_test_add_func("/Adapta/ExpanderRow/title_lines", test_adap_expander_row_title_lines);
  g_test_add_func("/Adapta/ExpanderRow/subtitle_lines", test_adap_expander_row_subtitle_lines);

  return g_test_run();
}
