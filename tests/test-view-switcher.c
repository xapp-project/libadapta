/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_view_switcher_policy (void)
{
  AdapViewSwitcher *view_switcher = g_object_ref_sink (ADAP_VIEW_SWITCHER (adap_view_switcher_new ()));

  g_assert_nonnull (view_switcher);

  g_assert_cmpint (adap_view_switcher_get_policy (view_switcher), ==, ADAP_VIEW_SWITCHER_POLICY_NARROW);

  adap_view_switcher_set_policy (view_switcher, ADAP_VIEW_SWITCHER_POLICY_WIDE);
  g_assert_cmpint (adap_view_switcher_get_policy (view_switcher), ==, ADAP_VIEW_SWITCHER_POLICY_WIDE);

  adap_view_switcher_set_policy (view_switcher, ADAP_VIEW_SWITCHER_POLICY_NARROW);
  g_assert_cmpint (adap_view_switcher_get_policy (view_switcher), ==, ADAP_VIEW_SWITCHER_POLICY_NARROW);

  g_assert_finalize_object (view_switcher);
}


static void
test_adap_view_switcher_stack (void)
{
  AdapViewSwitcher *view_switcher = g_object_ref_sink (ADAP_VIEW_SWITCHER (adap_view_switcher_new ()));
  AdapViewStack *stack = g_object_ref_sink (ADAP_VIEW_STACK (adap_view_stack_new ()));

  g_assert_nonnull (view_switcher);
  g_assert_nonnull (stack);

  g_assert_null (adap_view_switcher_get_stack (view_switcher));

  adap_view_switcher_set_stack (view_switcher, stack);
  g_assert (adap_view_switcher_get_stack (view_switcher) == stack);

  adap_view_switcher_set_stack (view_switcher, NULL);
  g_assert_null (adap_view_switcher_get_stack (view_switcher));

  g_assert_finalize_object (view_switcher);
  g_assert_finalize_object (stack);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ViewSwitcher/policy", test_adap_view_switcher_policy);
  g_test_add_func("/Adapta/ViewSwitcher/stack", test_adap_view_switcher_stack);

  return g_test_run();
}
