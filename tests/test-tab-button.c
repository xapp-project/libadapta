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
test_adap_tab_button_view (void)
{
  AdapTabButton *button = g_object_ref_sink (ADAP_TAB_BUTTON (adap_tab_button_new ()));
  AdapTabView *view;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::view", G_CALLBACK (increment), &notified);

  g_object_get (button, "view", &view, NULL);
  g_assert_null (view);

  adap_tab_button_set_view (button, NULL);
  g_assert_cmpint (notified, ==, 0);

  view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  adap_tab_button_set_view (button, view);
  g_assert_true (adap_tab_button_get_view (button) == view);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "view", NULL, NULL);
  g_assert_null (adap_tab_button_get_view (button));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (button);
  g_assert_finalize_object (view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/TabButton/view", test_adap_tab_button_view);

  return g_test_run ();
}
