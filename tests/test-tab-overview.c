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
test_adap_tab_overview_view (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  AdapTabView *view;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::view", G_CALLBACK (increment), &notified);

  g_object_get (overview, "view", &view, NULL);
  g_assert_null (view);

  adap_tab_overview_set_view (overview, NULL);
  g_assert_cmpint (notified, ==, 0);

  view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  adap_tab_overview_set_view (overview, view);
  g_assert_true (adap_tab_overview_get_view (overview) == view);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "view", NULL, NULL);
  g_assert_null (adap_tab_overview_get_view (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_overview_child (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (overview, "child", &widget, NULL);
  g_assert_null (widget);

  adap_tab_overview_set_child (overview, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_tab_overview_set_child (overview, widget);
  g_assert_true (adap_tab_overview_get_child (overview) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "child", NULL, NULL);
  g_assert_null (adap_tab_overview_get_child (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_open (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  AdapTabView *view = ADAP_TAB_VIEW (adap_tab_view_new ());
  gboolean open = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);
  g_assert_nonnull (view);

  adap_tab_view_add_page (view, gtk_button_new (), NULL);

  adap_tab_overview_set_child (overview, GTK_WIDGET (view));
  adap_tab_overview_set_view (overview, g_object_ref (view));

  g_signal_connect_swapped (overview, "notify::open", G_CALLBACK (increment), &notified);

  g_object_get (overview, "open", &open, NULL);
  g_assert_false (open);

  adap_tab_overview_set_open (overview, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_open (overview, TRUE);
  g_assert_true (adap_tab_overview_get_open (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "open", FALSE, NULL);
  g_assert_false (adap_tab_overview_get_open (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_overview_inverted (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  gboolean inverted = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::inverted", G_CALLBACK (increment), &notified);

  g_object_get (overview, "inverted", &inverted, NULL);
  g_assert_false (inverted);

  adap_tab_overview_set_inverted (overview, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_inverted (overview, TRUE);
  g_assert_true (adap_tab_overview_get_inverted (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "inverted", FALSE, NULL);
  g_assert_false (adap_tab_overview_get_inverted (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_enable_search (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  gboolean enable_search = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::enable-search", G_CALLBACK (increment), &notified);

  g_object_get (overview, "enable-search", &enable_search, NULL);
  g_assert_true (enable_search);

  adap_tab_overview_set_enable_search (overview, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_enable_search (overview, FALSE);
  g_assert_false (adap_tab_overview_get_enable_search (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "enable-search", TRUE, NULL);
  g_assert_true (adap_tab_overview_get_enable_search (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_enable_new_tab (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  gboolean enable_new_tab = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::enable-new-tab", G_CALLBACK (increment), &notified);

  g_object_get (overview, "enable-new-tab", &enable_new_tab, NULL);
  g_assert_false (enable_new_tab);

  adap_tab_overview_set_enable_new_tab (overview, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_enable_new_tab (overview, TRUE);
  g_assert_true (adap_tab_overview_get_enable_new_tab (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "enable-new-tab", FALSE, NULL);
  g_assert_false (adap_tab_overview_get_enable_new_tab (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_show_start_title_buttons (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  gboolean show_start_title_buttons = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::show-start-title-buttons", G_CALLBACK (increment), &notified);

  g_object_get (overview, "show-start-title-buttons", &show_start_title_buttons, NULL);
  g_assert_true (show_start_title_buttons);

  adap_tab_overview_set_show_start_title_buttons (overview, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_show_start_title_buttons (overview, FALSE);
  g_assert_false (adap_tab_overview_get_show_start_title_buttons (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "show-start-title-buttons", TRUE, NULL);
  g_assert_true (adap_tab_overview_get_show_start_title_buttons (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_show_end_title_buttons (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  gboolean show_end_title_buttons = FALSE;
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::show-end-title-buttons", G_CALLBACK (increment), &notified);

  g_object_get (overview, "show-end-title-buttons", &show_end_title_buttons, NULL);
  g_assert_true (show_end_title_buttons);

  adap_tab_overview_set_show_end_title_buttons (overview, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_show_end_title_buttons (overview, FALSE);
  g_assert_false (adap_tab_overview_get_show_end_title_buttons (overview));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "show-end-title-buttons", TRUE, NULL);
  g_assert_true (adap_tab_overview_get_show_end_title_buttons (overview));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
}

static void
test_adap_tab_overview_secondary_menu (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  GMenuModel *model;
  GMenuModel *model1 = G_MENU_MODEL (g_menu_new ());
  GMenuModel *model2 = G_MENU_MODEL (g_menu_new ());
  int notified = 0;

  g_assert_nonnull (overview);

  g_signal_connect_swapped (overview, "notify::secondary-menu", G_CALLBACK (increment), &notified);

  g_object_get (overview, "secondary-menu", &model, NULL);
  g_assert_null (model);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_overview_set_secondary_menu (overview, model1);
  g_assert_true (adap_tab_overview_get_secondary_menu (overview) == model1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (overview, "secondary-menu", model2, NULL);
  g_assert_true (adap_tab_overview_get_secondary_menu (overview) == model2);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (overview);
  g_assert_finalize_object (model1);
  g_assert_finalize_object (model2);
}

static void
test_adap_tab_overview_actions (void)
{
  AdapTabOverview *overview = g_object_ref_sink (ADAP_TAB_OVERVIEW (adap_tab_overview_new ()));
  AdapTabView *view = ADAP_TAB_VIEW (adap_tab_view_new ());

  g_assert_nonnull (overview);
  g_assert_nonnull (view);

  adap_tab_view_add_page (view, gtk_button_new (), NULL);

  adap_tab_overview_set_child (overview, GTK_WIDGET (view));
  adap_tab_overview_set_view (overview, g_object_ref (view));

  gtk_widget_activate_action (GTK_WIDGET (overview), "overview.open", NULL);

  g_assert_true (adap_tab_overview_get_open (overview));

  gtk_widget_activate_action (GTK_WIDGET (overview), "overview.close", NULL);

  g_assert_false (adap_tab_overview_get_open (overview));

  g_assert_finalize_object (overview);
  g_assert_finalize_object (view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/TabOverview/view", test_adap_tab_overview_view);
  g_test_add_func ("/Adapta/TabOverview/child", test_adap_tab_overview_child);
  g_test_add_func ("/Adapta/TabOverview/open", test_adap_tab_overview_open);
  g_test_add_func ("/Adapta/TabOverview/inverted", test_adap_tab_overview_inverted);
  g_test_add_func ("/Adapta/TabOverview/enable_search", test_adap_tab_overview_enable_search);
  g_test_add_func ("/Adapta/TabOverview/enable_new_tab", test_adap_tab_overview_enable_new_tab);
  g_test_add_func ("/Adapta/TabOverview/secondary_menu", test_adap_tab_overview_secondary_menu);
  g_test_add_func ("/Adapta/TabOverview/show_start_title_buttons", test_adap_tab_overview_show_start_title_buttons);
  g_test_add_func ("/Adapta/TabOverview/show_end_title_buttons", test_adap_tab_overview_show_end_title_buttons);
  g_test_add_func ("/Adapta/TabOverview/actions", test_adap_tab_overview_actions);

  return g_test_run ();
}
