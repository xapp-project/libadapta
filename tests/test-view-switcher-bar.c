/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_view_switcher_bar_stack (void)
{
  AdapViewSwitcherBar *bar = g_object_ref_sink (ADAP_VIEW_SWITCHER_BAR (adap_view_switcher_bar_new ()));
  AdapViewStack *stack = g_object_ref_sink (ADAP_VIEW_STACK (adap_view_stack_new ()));

  g_assert_nonnull (bar);
  g_assert_nonnull (stack);

  g_assert_null (adap_view_switcher_bar_get_stack (bar));

  adap_view_switcher_bar_set_stack (bar, stack);
  g_assert (adap_view_switcher_bar_get_stack (bar) == stack);

  adap_view_switcher_bar_set_stack (bar, NULL);
  g_assert_null (adap_view_switcher_bar_get_stack (bar));

  g_assert_finalize_object (bar);
  g_assert_finalize_object (stack);
}


static void
test_adap_view_switcher_bar_reveal (void)
{
  AdapViewSwitcherBar *bar = g_object_ref_sink (ADAP_VIEW_SWITCHER_BAR (adap_view_switcher_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_false (adap_view_switcher_bar_get_reveal (bar));

  adap_view_switcher_bar_set_reveal (bar, TRUE);
  g_assert_true (adap_view_switcher_bar_get_reveal (bar));

  adap_view_switcher_bar_set_reveal (bar, FALSE);
  g_assert_false (adap_view_switcher_bar_get_reveal (bar));

  g_assert_finalize_object (bar);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ViewSwitcherBar/stack", test_adap_view_switcher_bar_stack);
  g_test_add_func("/Adapta/ViewSwitcherBar/reveal", test_adap_view_switcher_bar_reveal);

  return g_test_run();
}
