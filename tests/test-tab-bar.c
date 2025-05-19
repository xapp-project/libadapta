/*
 * Copyright (C) 2020 Purism SPC
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
test_adap_tab_bar_view (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  AdapTabView *view;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::view", G_CALLBACK (increment), &notified);

  g_object_get (bar, "view", &view, NULL);
  g_assert_null (view);

  adap_tab_bar_set_view (bar, NULL);
  g_assert_cmpint (notified, ==, 0);

  view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  adap_tab_bar_set_view (bar, view);
  g_assert_true (adap_tab_bar_get_view (bar) == view);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "view", NULL, NULL);
  g_assert_null (adap_tab_bar_get_view (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_bar_start_action_widget (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::start-action-widget", G_CALLBACK (increment), &notified);

  g_object_get (bar, "start-action-widget", &widget, NULL);
  g_assert_null (widget);

  adap_tab_bar_set_start_action_widget (bar, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_tab_bar_set_start_action_widget (bar, widget);
  g_assert_true (adap_tab_bar_get_start_action_widget (bar) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "start-action-widget", NULL, NULL);
  g_assert_null (adap_tab_bar_get_start_action_widget (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
}

static void
test_adap_tab_bar_end_action_widget (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::end-action-widget", G_CALLBACK (increment), &notified);

  g_object_get (bar, "end-action-widget", &widget, NULL);
  g_assert_null (widget);

  adap_tab_bar_set_end_action_widget (bar, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_tab_bar_set_end_action_widget (bar, widget);
  g_assert_true (adap_tab_bar_get_end_action_widget (bar) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "end-action-widget", NULL, NULL);
  g_assert_null (adap_tab_bar_get_end_action_widget (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
}

static void
test_adap_tab_bar_autohide (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  gboolean autohide = FALSE;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::autohide", G_CALLBACK (increment), &notified);

  g_object_get (bar, "autohide", &autohide, NULL);
  g_assert_true (autohide);

  adap_tab_bar_set_autohide (bar, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_bar_set_autohide (bar, FALSE);
  g_assert_false (adap_tab_bar_get_autohide (bar));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "autohide", TRUE, NULL);
  g_assert_true (adap_tab_bar_get_autohide (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
}

static void
test_adap_tab_bar_tabs_revealed (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  AdapTabView *view;
  gboolean tabs_revealed = FALSE;
  AdapTabPage *page;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::tabs-revealed", G_CALLBACK (increment), &notified);

  g_object_get (bar, "tabs-revealed", &tabs_revealed, NULL);
  g_assert_false (tabs_revealed);
  g_assert_false (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 0);

  adap_tab_bar_set_autohide (bar, FALSE);
  g_assert_false (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 0);

  view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  adap_tab_bar_set_view (bar, view);
  g_assert_true (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 1);

  adap_tab_bar_set_autohide (bar, TRUE);
  g_assert_false (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 2);

  page = adap_tab_view_append_pinned (view, gtk_button_new ());
  g_assert_true (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 3);

  adap_tab_view_set_page_pinned (view, page, FALSE);
  g_assert_false (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 4);

  adap_tab_view_append (view, gtk_button_new ());
  g_assert_true (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 5);

  adap_tab_view_close_page (view, page);
  g_assert_false (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 6);

  adap_tab_bar_set_autohide (bar, FALSE);
  g_assert_true (adap_tab_bar_get_tabs_revealed (bar));
  g_assert_cmpint (notified, ==, 7);

  g_assert_finalize_object (bar);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_bar_expand_tabs (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  gboolean expand_tabs = FALSE;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::expand-tabs", G_CALLBACK (increment), &notified);

  g_object_get (bar, "expand-tabs", &expand_tabs, NULL);
  g_assert_true (expand_tabs);

  adap_tab_bar_set_expand_tabs (bar, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_bar_set_expand_tabs (bar, FALSE);
  g_assert_false (adap_tab_bar_get_expand_tabs (bar));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "expand-tabs", TRUE, NULL);
  g_assert_true (adap_tab_bar_get_expand_tabs (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
}

static void
test_adap_tab_bar_inverted (void)
{
  AdapTabBar *bar = g_object_ref_sink (ADAP_TAB_BAR (adap_tab_bar_new ()));
  gboolean inverted = FALSE;
  int notified = 0;

  g_assert_nonnull (bar);

  g_signal_connect_swapped (bar, "notify::inverted", G_CALLBACK (increment), &notified);

  g_object_get (bar, "inverted", &inverted, NULL);
  g_assert_false (inverted);

  adap_tab_bar_set_inverted (bar, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_bar_set_inverted (bar, TRUE);
  g_assert_true (adap_tab_bar_get_inverted (bar));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bar, "inverted", FALSE, NULL);
  g_assert_false (adap_tab_bar_get_inverted (bar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bar);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/TabBar/view", test_adap_tab_bar_view);
  g_test_add_func ("/Adapta/TabBar/start_action_widget", test_adap_tab_bar_start_action_widget);
  g_test_add_func ("/Adapta/TabBar/end_action_widget", test_adap_tab_bar_end_action_widget);
  g_test_add_func ("/Adapta/TabBar/autohide", test_adap_tab_bar_autohide);
  g_test_add_func ("/Adapta/TabBar/tabs_revealed", test_adap_tab_bar_tabs_revealed);
  g_test_add_func ("/Adapta/TabBar/expand_tabs", test_adap_tab_bar_expand_tabs);
  g_test_add_func ("/Adapta/TabBar/inverted", test_adap_tab_bar_inverted);

  return g_test_run ();
}
