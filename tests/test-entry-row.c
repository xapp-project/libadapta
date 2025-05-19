/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_entry_row_add_remove (void)
{
  AdapEntryRow *row = g_object_ref_sink (ADAP_ENTRY_ROW (adap_entry_row_new ()));
  GtkWidget *prefix, *suffix;

  g_assert_nonnull (row);

  prefix = gtk_check_button_new ();
  g_assert_nonnull (prefix);

  suffix = gtk_check_button_new ();
  g_assert_nonnull (suffix);

  adap_entry_row_add_prefix (row, prefix);
  adap_entry_row_add_suffix (row, suffix);

  adap_entry_row_remove (row, prefix);
  adap_entry_row_remove (row, suffix);

  g_assert_finalize_object (row);
}

static void
test_adap_entry_row_show_apply_button (void)
{
  AdapEntryRow *row = g_object_ref_sink (ADAP_ENTRY_ROW (adap_entry_row_new ()));
  gboolean show_apply_button;
  int notified = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "notify::show-apply-button", G_CALLBACK (increment), &notified);

  g_object_get (row, "show-apply-button", &show_apply_button, NULL);
  g_assert_false (show_apply_button);

  adap_entry_row_set_show_apply_button (row, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_entry_row_set_show_apply_button (row, TRUE);
  g_assert_true (adap_entry_row_get_show_apply_button (row));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (row, "show-apply-button", FALSE, NULL);
  g_assert_false (adap_entry_row_get_show_apply_button (row));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (row);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/EntryRow/add_remove", test_adap_entry_row_add_remove);
  g_test_add_func("/Adapta/EntryRow/show_apply_button", test_adap_entry_row_show_apply_button);

  return g_test_run();
}
