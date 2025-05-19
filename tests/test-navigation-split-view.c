/*
 * Copyright (C) 2023 Purism SPC
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
test_adap_navigation_split_view_sidebar (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  AdapNavigationPage *widget = NULL;
  int notified = 0, showing = 0, hiding = 0, shown = 0, hidden = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar", &widget, NULL);
  g_assert_null (widget);

  adap_navigation_split_view_set_sidebar (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Sidebar"));
  g_signal_connect_swapped (widget, "showing", G_CALLBACK (increment), &showing);
  g_signal_connect_swapped (widget, "hiding", G_CALLBACK (increment), &hiding);
  g_signal_connect_swapped (widget, "shown", G_CALLBACK (increment), &shown);
  g_signal_connect_swapped (widget, "hidden", G_CALLBACK (increment), &hidden);

  adap_navigation_split_view_set_sidebar (split_view, widget);
  g_assert_true (adap_navigation_split_view_get_sidebar (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  g_object_set (split_view, "sidebar", NULL, NULL);
  g_assert_null (adap_navigation_split_view_get_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  g_assert_finalize_object (widget);
  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_content (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  AdapNavigationPage *widget = NULL;
  int notified = 0, showing = 0, hiding = 0, shown = 0, hidden = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::content", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "content", &widget, NULL);
  g_assert_null (widget);

  adap_navigation_split_view_set_content (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Content"));
  g_signal_connect_swapped (widget, "showing", G_CALLBACK (increment), &showing);
  g_signal_connect_swapped (widget, "hiding", G_CALLBACK (increment), &hiding);
  g_signal_connect_swapped (widget, "shown", G_CALLBACK (increment), &shown);
  g_signal_connect_swapped (widget, "hidden", G_CALLBACK (increment), &hidden);

  adap_navigation_split_view_set_content (split_view, widget);
  g_assert_true (adap_navigation_split_view_get_content (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  g_object_set (split_view, "content", NULL, NULL);
  g_assert_null (adap_navigation_split_view_get_content (split_view));
  g_assert_cmpint (notified, ==, 2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  g_assert_finalize_object (widget);
  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_collapsed (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  gboolean collapsed;
  int notified = 0;
  int sidebar_showing = 0, sidebar_hiding = 0, sidebar_shown = 0, sidebar_hidden = 0;
  int content_showing = 0, content_hiding = 0, content_shown = 0, content_hidden = 0;
  AdapNavigationPage *sidebar, *content;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::collapsed", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "collapsed", &collapsed, NULL);
  g_assert_false (collapsed);

  adap_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "collapsed", FALSE, NULL);
  g_assert_false (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 2);

  sidebar = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Sidebar"));
  g_signal_connect_swapped (sidebar, "showing", G_CALLBACK (increment), &sidebar_showing);
  g_signal_connect_swapped (sidebar, "hiding", G_CALLBACK (increment), &sidebar_hiding);
  g_signal_connect_swapped (sidebar, "shown", G_CALLBACK (increment), &sidebar_shown);
  g_signal_connect_swapped (sidebar, "hidden", G_CALLBACK (increment), &sidebar_hidden);

  content = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Content"));
  g_signal_connect_swapped (content, "showing", G_CALLBACK (increment), &content_showing);
  g_signal_connect_swapped (content, "hiding", G_CALLBACK (increment), &content_hiding);
  g_signal_connect_swapped (content, "shown", G_CALLBACK (increment), &content_shown);
  g_signal_connect_swapped (content, "hidden", G_CALLBACK (increment), &content_hidden);

  adap_navigation_split_view_set_sidebar (split_view, sidebar);
  g_assert_cmpint (sidebar_showing, ==, 1);
  g_assert_cmpint (sidebar_shown, ==, 1);
  g_assert_cmpint (sidebar_hiding, ==, 0);
  g_assert_cmpint (sidebar_hidden, ==, 0);

  adap_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 3);

  adap_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 4);

  adap_navigation_split_view_set_sidebar (split_view, NULL);
  adap_navigation_split_view_set_content (split_view, content);
  g_assert_cmpint (sidebar_showing, ==, 1);
  g_assert_cmpint (sidebar_shown, ==, 1);
  g_assert_cmpint (sidebar_hiding, ==, 1);
  g_assert_cmpint (sidebar_hidden, ==, 1);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adap_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 5);

  adap_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 6);

  adap_navigation_split_view_set_sidebar (split_view, sidebar);
  g_assert_cmpint (sidebar_showing, ==, 2);
  g_assert_cmpint (sidebar_shown, ==, 2);
  g_assert_cmpint (sidebar_hiding, ==, 1);
  g_assert_cmpint (sidebar_hidden, ==, 1);

  adap_navigation_split_view_set_show_content (split_view, TRUE);

  adap_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 7);
  g_assert_cmpint (sidebar_showing, ==, 2);
  g_assert_cmpint (sidebar_shown, ==, 2);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adap_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 8);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adap_navigation_split_view_set_show_content (split_view, FALSE);

  adap_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 9);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 1);
  g_assert_cmpint (content_hidden, ==, 1);

  adap_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 10);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 2);
  g_assert_cmpint (content_shown, ==, 2);
  g_assert_cmpint (content_hiding, ==, 1);
  g_assert_cmpint (content_hidden, ==, 1);

  g_assert_finalize_object (split_view);
  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (content);
}

static void
test_adap_navigation_split_view_show_content (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  gboolean show_content;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::show-content", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "show-content", &show_content, NULL);
  g_assert_false (show_content);

  adap_navigation_split_view_set_show_content (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  adap_navigation_split_view_set_sidebar (split_view, adap_navigation_page_new (gtk_button_new (), "Sidebar"));

  adap_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 3);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 4);

  adap_navigation_split_view_set_content (split_view, adap_navigation_page_new (gtk_button_new (), "Content"));
  adap_navigation_split_view_set_sidebar (split_view, NULL);

  adap_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 5);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 6);

  adap_navigation_split_view_set_sidebar (split_view, adap_navigation_page_new (gtk_button_new (), "Sidebar"));

  adap_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 7);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 8);

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_min_sidebar_width (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  double width;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::min-sidebar-width", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "min-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 180, DBL_EPSILON));

  adap_navigation_split_view_set_min_sidebar_width (split_view, 180);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_min_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_min_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "min-sidebar-width", 180.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_min_sidebar_width (split_view), 180, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_max_sidebar_width (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  double width;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::max-sidebar-width", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "max-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 280, DBL_EPSILON));

  adap_navigation_split_view_set_max_sidebar_width (split_view, 280);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_max_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_max_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "max-sidebar-width", 280.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_max_sidebar_width (split_view), 280, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_sidebar_width_fraction (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  double fraction;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-fraction", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-width-fraction", &fraction, NULL);
  g_assert_true (G_APPROX_VALUE (fraction, 0.25, DBL_EPSILON));

  adap_navigation_split_view_set_sidebar_width_fraction (split_view, 0.25);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_sidebar_width_fraction (split_view, 0.2);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_sidebar_width_fraction (split_view), 0.2, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-fraction", 0.25, NULL);
  g_assert_true (G_APPROX_VALUE (adap_navigation_split_view_get_sidebar_width_fraction (split_view), 0.25, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_sidebar_width_unit (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  AdapLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-unit", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-width-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADAP_LENGTH_UNIT_SP);

  adap_navigation_split_view_set_sidebar_width_unit (split_view, ADAP_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_split_view_set_sidebar_width_unit (split_view, ADAP_LENGTH_UNIT_PX);
  g_assert_cmpint (adap_navigation_split_view_get_sidebar_width_unit (split_view), ==, ADAP_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-unit", ADAP_LENGTH_UNIT_SP, NULL);
  g_assert_cmpint (adap_navigation_split_view_get_sidebar_width_unit (split_view), ==, ADAP_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_page_tags (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));

  g_assert_nonnull (split_view);

  adap_navigation_split_view_set_sidebar (split_view, adap_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adap_navigation_split_view_set_content (split_view, adap_navigation_page_new_with_tag (gtk_button_new (), "Content", "sidebar"));
  g_test_assert_expected_messages ();

  adap_navigation_split_view_set_content (split_view, adap_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adap_navigation_page_set_tag (adap_navigation_split_view_get_sidebar (split_view), "content");
  g_test_assert_expected_messages ();

  adap_navigation_page_set_tag (adap_navigation_split_view_get_sidebar (split_view), "sidebar");

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adap_navigation_page_set_tag (adap_navigation_split_view_get_content (split_view), "sidebar");
  g_test_assert_expected_messages ();

  g_assert_finalize_object (split_view);
}

static void
test_adap_navigation_split_view_actions (void)
{
  AdapNavigationSplitView *split_view = g_object_ref_sink (ADAP_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_new ()));
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::show-content", G_CALLBACK (increment), &notified);

  adap_navigation_split_view_set_sidebar (split_view, adap_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
  adap_navigation_split_view_set_content (split_view, adap_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in the navigation stack*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");
  g_test_assert_expected_messages ();

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*No page with the tag*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "something");
  g_test_assert_expected_messages ();

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

  g_assert_true (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in the navigation stack*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");
  g_test_assert_expected_messages ();

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.pop", NULL);

  g_assert_false (adap_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/NavigationSplitView/sidebar", test_adap_navigation_split_view_sidebar);
  g_test_add_func ("/Adapta/NavigationSplitView/content", test_adap_navigation_split_view_content);
  g_test_add_func ("/Adapta/NavigationSplitView/collapsed", test_adap_navigation_split_view_collapsed);
  g_test_add_func ("/Adapta/NavigationSplitView/show_content", test_adap_navigation_split_view_show_content);
  g_test_add_func ("/Adapta/NavigationSplitView/min_sidebar_width", test_adap_navigation_split_view_min_sidebar_width);
  g_test_add_func ("/Adapta/NavigationSplitView/max_sidebar_width", test_adap_navigation_split_view_max_sidebar_width);
  g_test_add_func ("/Adapta/NavigationSplitView/sidebar_width_fraction", test_adap_navigation_split_view_sidebar_width_fraction);
  g_test_add_func ("/Adapta/NavigationSplitView/sidebar_width_unit", test_adap_navigation_split_view_sidebar_width_unit);
  g_test_add_func ("/Adapta/NavigationSplitView/page_tags", test_adap_navigation_split_view_page_tags);
  g_test_add_func ("/Adapta/NavigationSplitView/actions", test_adap_navigation_split_view_actions);

  return g_test_run ();
}
