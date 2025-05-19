/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

static void
test_adap_password_entry_row_new (void)
{
  GtkWidget *row = g_object_ref_sink (adap_password_entry_row_new ());
  g_assert_nonnull (row);

  g_assert_finalize_object (row);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PasswordEntryRow/new", test_adap_password_entry_row_new);

  return g_test_run();
}
