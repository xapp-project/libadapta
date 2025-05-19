/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_action_row_add_remove (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  GtkWidget *prefix, *suffix;

  g_assert_nonnull (row);

  prefix = gtk_check_button_new ();
  g_assert_nonnull (prefix);

  suffix = gtk_check_button_new ();
  g_assert_nonnull (suffix);

  adap_action_row_add_prefix (row, prefix);
  adap_action_row_add_suffix (row, suffix);

  adap_action_row_remove (row, prefix);
  adap_action_row_remove (row, suffix);

  g_assert_finalize_object (row);
}


static void
test_adap_action_row_subtitle (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (adap_action_row_get_subtitle (row), ==, "");

  adap_action_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (adap_action_row_get_subtitle (row), ==, "Dummy subtitle");

  adap_preferences_row_set_use_markup (ADAP_PREFERENCES_ROW (row), FALSE);
  adap_action_row_set_subtitle (row, "Invalid <b>markup");
  g_assert_cmpstr (adap_action_row_get_subtitle (row), ==, "Invalid <b>markup");

  g_assert_finalize_object (row);
}


static void
test_adap_action_row_title_lines (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adap_action_row_get_title_lines (row), ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adap_action_row_set_title_lines: assertion 'title_lines >= 0' failed");
  adap_action_row_set_title_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adap_action_row_get_title_lines (row), ==, 0);

  adap_action_row_set_title_lines (row, 1);
  g_assert_cmpint (adap_action_row_get_title_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


static void
test_adap_action_row_subtitle_lines (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adap_action_row_get_subtitle_lines (row), ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adap_action_row_set_subtitle_lines: assertion 'subtitle_lines >= 0' failed");
  adap_action_row_set_subtitle_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adap_action_row_get_subtitle_lines (row), ==, 0);

  adap_action_row_set_subtitle_lines (row, 1);
  g_assert_cmpint (adap_action_row_get_subtitle_lines (row), ==, 1);

  g_assert_finalize_object (row);
}

static void
test_adap_action_row_subtitle_selectable (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adap_action_row_get_subtitle_selectable (row));

  adap_action_row_set_subtitle_selectable (row, TRUE);
  g_assert_true (adap_action_row_get_subtitle_selectable (row));

  adap_action_row_set_subtitle_selectable (row, FALSE);
  g_assert_false (adap_action_row_get_subtitle_selectable (row));

  g_assert_finalize_object (row);
}


static void
test_adap_action_row_activate (void)
{
  AdapActionRow *row = g_object_ref_sink (ADAP_ACTION_ROW (adap_action_row_new ()));
  int activated = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "activated", G_CALLBACK (increment), &activated);

  adap_action_row_activate (row);
  g_assert_cmpint (activated, ==, 1);

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ActionRow/add_remove", test_adap_action_row_add_remove);
  g_test_add_func("/Adapta/ActionRow/subtitle", test_adap_action_row_subtitle);
  g_test_add_func("/Adapta/ActionRow/title_lines", test_adap_action_row_title_lines);
  g_test_add_func("/Adapta/ActionRow/subtitle_lines", test_adap_action_row_subtitle_lines);
  g_test_add_func("/Adapta/ActionRow/subtitle_selectable", test_adap_action_row_subtitle_selectable);
  g_test_add_func("/Adapta/ActionRow/activate", test_adap_action_row_activate);

  return g_test_run();
}
