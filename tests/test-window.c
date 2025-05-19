/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_window_new (void)
{
  GtkWidget *window = adap_window_new ();
  g_assert_nonnull (window);

  g_assert_finalize_object (window);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/Window/new", test_adap_window_new);

  return g_test_run();
}
