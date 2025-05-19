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
test_adap_toolbar_view_content (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::content", G_CALLBACK (increment), &notified);

  g_object_get (view, "content", &widget, NULL);
  g_assert_null (widget);

  adap_toolbar_view_set_content (view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (gtk_button_new ());
  adap_toolbar_view_set_content (view, widget);
  g_assert_true (adap_toolbar_view_get_content (view) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "content", NULL, NULL);
  g_assert_null (adap_toolbar_view_get_content (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
  g_assert_finalize_object (widget);
}

static void
test_adap_toolbar_view_add_remove (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  GtkWidget *header_bar = g_object_ref_sink (gtk_header_bar_new ());
  GtkWidget *search_bar = g_object_ref_sink (gtk_search_bar_new ());
  GtkWidget *action_bar = g_object_ref_sink (gtk_action_bar_new ());
  GtkWidget *content = g_object_ref_sink (gtk_button_new ());

  g_assert_nonnull (view);

  adap_toolbar_view_add_top_bar (view, header_bar);
  adap_toolbar_view_add_top_bar (view, search_bar);
  adap_toolbar_view_add_bottom_bar (view, action_bar);
  adap_toolbar_view_set_content (view, content);

  adap_toolbar_view_remove (view, header_bar);
  adap_toolbar_view_remove (view, search_bar);
  adap_toolbar_view_remove (view, action_bar);
  adap_toolbar_view_remove (view, content);

  g_assert_finalize_object (view);
  g_assert_finalize_object (header_bar);
  g_assert_finalize_object (search_bar);
  g_assert_finalize_object (action_bar);
  g_assert_finalize_object (content);
}

static void
test_adap_toolbar_view_top_bar_style (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  AdapToolbarStyle style = ADAP_TOOLBAR_FLAT;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::top-bar-style", G_CALLBACK (increment), &notified);

  g_object_get (view, "top-bar-style", &style, NULL);
  g_assert_cmpint (style, ==, ADAP_TOOLBAR_FLAT);

  adap_toolbar_view_set_top_bar_style (view, ADAP_TOOLBAR_FLAT);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_top_bar_style (view, ADAP_TOOLBAR_RAISED);
  g_assert_cmpint (adap_toolbar_view_get_top_bar_style (view), ==, ADAP_TOOLBAR_RAISED);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "top-bar-style", ADAP_TOOLBAR_FLAT, NULL);
  g_assert_cmpint (adap_toolbar_view_get_top_bar_style (view), ==, ADAP_TOOLBAR_FLAT);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_toolbar_view_bottom_bar_style (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  AdapToolbarStyle style = ADAP_TOOLBAR_FLAT;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::bottom-bar-style", G_CALLBACK (increment), &notified);

  g_object_get (view, "bottom-bar-style", &style, NULL);
  g_assert_cmpint (style, ==, ADAP_TOOLBAR_FLAT);

  adap_toolbar_view_set_bottom_bar_style (view, ADAP_TOOLBAR_FLAT);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_bottom_bar_style (view, ADAP_TOOLBAR_RAISED);
  g_assert_cmpint (adap_toolbar_view_get_bottom_bar_style (view), ==, ADAP_TOOLBAR_RAISED);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "bottom-bar-style", ADAP_TOOLBAR_FLAT, NULL);
  g_assert_cmpint (adap_toolbar_view_get_bottom_bar_style (view), ==, ADAP_TOOLBAR_FLAT);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_toolbar_view_reveal_top_bars (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  gboolean reveal = FALSE;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::reveal-top-bars", G_CALLBACK (increment), &notified);

  g_object_get (view, "reveal-top-bars", &reveal, NULL);
  g_assert_true (reveal);

  adap_toolbar_view_set_reveal_top_bars (view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_reveal_top_bars (view, FALSE);
  g_assert_false (adap_toolbar_view_get_reveal_top_bars (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "reveal-top-bars", TRUE, NULL);
  g_assert_true (adap_toolbar_view_get_reveal_top_bars (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_toolbar_view_reveal_bottom_bars (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  gboolean reveal = FALSE;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::reveal-bottom-bars", G_CALLBACK (increment), &notified);

  g_object_get (view, "reveal-bottom-bars", &reveal, NULL);
  g_assert_true (reveal);

  adap_toolbar_view_set_reveal_bottom_bars (view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_reveal_bottom_bars (view, FALSE);
  g_assert_false (adap_toolbar_view_get_reveal_bottom_bars (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "reveal-bottom-bars", TRUE, NULL);
  g_assert_true (adap_toolbar_view_get_reveal_bottom_bars (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_toolbar_view_extend_content_to_top_edge (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  gboolean extend = FALSE;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::extend-content-to-top-edge", G_CALLBACK (increment), &notified);

  g_object_get (view, "extend-content-to-top-edge", &extend, NULL);
  g_assert_false (extend);

  adap_toolbar_view_set_extend_content_to_top_edge (view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_extend_content_to_top_edge (view, TRUE);
  g_assert_true (adap_toolbar_view_get_extend_content_to_top_edge (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "extend-content-to-top-edge", FALSE, NULL);
  g_assert_false (adap_toolbar_view_get_extend_content_to_top_edge (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_toolbar_view_extend_content_to_bottom_edge (void)
{
  AdapToolbarView *view = g_object_ref_sink (ADAP_TOOLBAR_VIEW (adap_toolbar_view_new ()));
  gboolean extend = FALSE;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::extend-content-to-bottom-edge", G_CALLBACK (increment), &notified);

  g_object_get (view, "extend-content-to-bottom-edge", &extend, NULL);
  g_assert_false (extend);

  adap_toolbar_view_set_extend_content_to_bottom_edge (view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_toolbar_view_set_extend_content_to_bottom_edge (view, TRUE);
  g_assert_true (adap_toolbar_view_get_extend_content_to_bottom_edge (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "extend-content-to-bottom-edge", FALSE, NULL);
  g_assert_false (adap_toolbar_view_get_extend_content_to_bottom_edge (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/ToolbarView/content", test_adap_toolbar_view_content);
  g_test_add_func ("/Adapta/ToolbarView/add_remove", test_adap_toolbar_view_add_remove);
  g_test_add_func ("/Adapta/ToolbarView/top_bar_style", test_adap_toolbar_view_top_bar_style);
  g_test_add_func ("/Adapta/ToolbarView/bottom_bar_style", test_adap_toolbar_view_bottom_bar_style);
  g_test_add_func ("/Adapta/ToolbarView/reveal_top_bars", test_adap_toolbar_view_reveal_top_bars);
  g_test_add_func ("/Adapta/ToolbarView/reveal_bottom_bars", test_adap_toolbar_view_reveal_bottom_bars);
  g_test_add_func ("/Adapta/ToolbarView/extend_content_to_top_edge", test_adap_toolbar_view_extend_content_to_top_edge);
  g_test_add_func ("/Adapta/ToolbarView/extend_content_to_bottom_edge", test_adap_toolbar_view_extend_content_to_bottom_edge);

  return g_test_run ();
}
