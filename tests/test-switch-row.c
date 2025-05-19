/*
 * Copyright (C) 2023 Joshua Lee <lee.son.wai@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-late
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_switch_row_active (void)
{
  AdapSwitchRow *row = g_object_ref_sink (ADAP_SWITCH_ROW (adap_switch_row_new ()));
  gboolean is_active;
  int notified = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "notify::active", G_CALLBACK (increment), &notified);

  is_active = adap_switch_row_get_active (row);
  g_assert_false (is_active);

  adap_switch_row_set_active (row, !adap_switch_row_get_active (row));
  g_assert_cmpint (notified, ==, 1);

  is_active = adap_switch_row_get_active (row);
  g_assert_true (is_active);

  g_assert_finalize_object (G_OBJECT (row));
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/SwitchRow/active", test_adap_switch_row_active);

  return g_test_run ();
}
