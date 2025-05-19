/*
 * Copyright (C) 2021 Purism SPC
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
test_adap_bin_child (void)
{
  AdapBin *bin = g_object_ref_sink (ADAP_BIN (adap_bin_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (bin);

  g_signal_connect_swapped (bin, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (bin, "child", &widget, NULL);
  g_assert_null (widget);

  adap_bin_set_child (bin, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_bin_set_child (bin, widget);
  g_assert_true (adap_bin_get_child (bin) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bin, "child", NULL, NULL);
  g_assert_null (adap_bin_get_child (bin));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bin);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/Bin/child", test_adap_bin_child);

  return g_test_run ();
}
