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
test_adap_window_title_title (void)
{
  AdapWindowTitle *window_title = g_object_ref_sink (ADAP_WINDOW_TITLE (adap_window_title_new ("Some title", NULL)));
  char *title;
  int notified = 0;

  g_assert_nonnull (window_title);

  g_signal_connect_swapped (window_title, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (window_title, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Some title");

  adap_window_title_set_title (window_title, "Some title");
  g_assert_cmpint (notified, ==, 0);

  adap_window_title_set_title (window_title, "Another title");
  g_assert_cmpstr (adap_window_title_get_title (window_title), ==, "Another title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (window_title, "title", "Yet another title", NULL);
  g_assert_cmpstr (adap_window_title_get_title (window_title), ==, "Yet another title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (window_title);
}

static void
test_adap_window_title_subtitle (void)
{
  AdapWindowTitle *window_title = g_object_ref_sink (ADAP_WINDOW_TITLE (adap_window_title_new (NULL, "Some subtitle")));
  char *subtitle;
  int notified = 0;

  g_assert_nonnull (window_title);

  g_signal_connect_swapped (window_title, "notify::subtitle", G_CALLBACK (increment), &notified);

  g_object_get (window_title, "subtitle", &subtitle, NULL);
  g_assert_cmpstr (subtitle, ==, "Some subtitle");

  adap_window_title_set_subtitle (window_title, "Some subtitle");
  g_assert_cmpint (notified, ==, 0);

  adap_window_title_set_subtitle (window_title, "Another subtitle");
  g_assert_cmpstr (adap_window_title_get_subtitle (window_title), ==, "Another subtitle");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (window_title, "subtitle", "Yet another subtitle", NULL);
  g_assert_cmpstr (adap_window_title_get_subtitle (window_title), ==, "Yet another subtitle");
  g_assert_cmpint (notified, ==, 2);

  g_free (subtitle);
  g_assert_finalize_object (window_title);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/WindowTitle/title", test_adap_window_title_title);
  g_test_add_func ("/Adapta/WindowTitle/subtitle", test_adap_window_title_subtitle);

  return g_test_run ();
}
