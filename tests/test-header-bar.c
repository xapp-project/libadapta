/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_header_bar_pack (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));
  GtkWidget *widget;

  g_assert_nonnull (bar);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  adap_header_bar_pack_start (bar, widget);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  adap_header_bar_pack_end (bar, widget);

  g_assert_finalize_object (bar);
}


static void
test_adap_header_bar_title_widget (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));
  GtkWidget *widget;

  g_assert_nonnull (bar);

  g_assert_null (adap_header_bar_get_title_widget (bar));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  adap_header_bar_set_title_widget (bar, widget);
  g_assert (adap_header_bar_get_title_widget  (bar) == widget);

  adap_header_bar_set_title_widget (bar, NULL);
  g_assert_null (adap_header_bar_get_title_widget (bar));

  g_assert_finalize_object (bar);
}


static void
test_adap_header_bar_show_start_title_buttons (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_true (adap_header_bar_get_show_start_title_buttons (bar));

  adap_header_bar_set_show_start_title_buttons (bar, FALSE);
  g_assert_false (adap_header_bar_get_show_start_title_buttons (bar));

  adap_header_bar_set_show_start_title_buttons (bar, TRUE);
  g_assert_true (adap_header_bar_get_show_start_title_buttons (bar));

  g_assert_finalize_object (bar);
}


static void
test_adap_header_bar_show_end_title_buttons (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_true (adap_header_bar_get_show_end_title_buttons (bar));

  adap_header_bar_set_show_end_title_buttons (bar, FALSE);
  g_assert_false (adap_header_bar_get_show_end_title_buttons (bar));

  adap_header_bar_set_show_end_title_buttons (bar, TRUE);
  g_assert_true (adap_header_bar_get_show_end_title_buttons (bar));

  g_assert_finalize_object (bar);
}


static void
test_adap_header_bar_decoration_layout (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_null (adap_header_bar_get_decoration_layout (bar));

  adap_header_bar_set_decoration_layout (bar, ":");
  g_assert_cmpstr (adap_header_bar_get_decoration_layout (bar), ==, ":");

  adap_header_bar_set_decoration_layout (bar, NULL);
  g_assert_null (adap_header_bar_get_decoration_layout (bar));

  g_assert_finalize_object (bar);
}


static void
test_adap_header_bar_centering_policy (void)
{
  AdapHeaderBar *bar = g_object_ref_sink (ADAP_HEADER_BAR (adap_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_cmpint (adap_header_bar_get_centering_policy (bar), ==, ADAP_CENTERING_POLICY_LOOSE);

  adap_header_bar_set_centering_policy (bar, ADAP_CENTERING_POLICY_STRICT);
  g_assert_cmpint (adap_header_bar_get_centering_policy (bar), ==, ADAP_CENTERING_POLICY_STRICT);

  adap_header_bar_set_centering_policy (bar, ADAP_CENTERING_POLICY_LOOSE);
  g_assert_cmpint (adap_header_bar_get_centering_policy (bar), ==, ADAP_CENTERING_POLICY_LOOSE);

  g_assert_finalize_object (bar);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/HeaderBar/pack", test_adap_header_bar_pack);
  g_test_add_func("/Adapta/HeaderBar/title_widget", test_adap_header_bar_title_widget);
  g_test_add_func("/Adapta/HeaderBar/show_start_title_buttons", test_adap_header_bar_show_start_title_buttons);
  g_test_add_func("/Adapta/HeaderBar/show_end_title_buttons", test_adap_header_bar_show_end_title_buttons);
  g_test_add_func("/Adapta/HeaderBar/decoration_layout", test_adap_header_bar_decoration_layout);
  g_test_add_func("/Adapta/HeaderBar/centering_policy", test_adap_header_bar_centering_policy);

  return g_test_run();
}
