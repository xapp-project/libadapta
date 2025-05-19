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
test_adap_overlay_split_view_sidebar (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar", &widget, NULL);
  g_assert_null (widget);

  adap_overlay_split_view_set_sidebar (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_overlay_split_view_set_sidebar (split_view, widget);
  g_assert_true (adap_overlay_split_view_get_sidebar (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar", NULL, NULL);
  g_assert_null (adap_overlay_split_view_get_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_content (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::content", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "content", &widget, NULL);
  g_assert_null (widget);

  adap_overlay_split_view_set_content (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_overlay_split_view_set_content (split_view, widget);
  g_assert_true (adap_overlay_split_view_get_content (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "content", NULL, NULL);
  g_assert_null (adap_overlay_split_view_get_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_collapsed (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  gboolean collapsed;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::collapsed", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "collapsed", &collapsed, NULL);
  g_assert_false (collapsed);

  adap_overlay_split_view_set_collapsed (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "collapsed", FALSE, NULL);
  g_assert_false (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 2);

  adap_overlay_split_view_set_sidebar (split_view, gtk_button_new ());

  adap_overlay_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 3);

  adap_overlay_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 4);

  adap_overlay_split_view_set_sidebar (split_view, NULL);
  adap_overlay_split_view_set_content (split_view, gtk_button_new ());

  adap_overlay_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 5);

  adap_overlay_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 6);

  adap_overlay_split_view_set_sidebar (split_view, gtk_button_new ());

  adap_overlay_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 7);

  adap_overlay_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 8);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_sidebar_position (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  GtkPackType position;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-position", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-position", &position, NULL);
  g_assert_cmpint (position, ==, GTK_PACK_START);

  adap_overlay_split_view_set_sidebar_position (split_view, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_sidebar_position (split_view, GTK_PACK_END);
  g_assert_cmpint (adap_overlay_split_view_get_sidebar_position (split_view), ==, GTK_PACK_END);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-position", GTK_PACK_START, NULL);
  g_assert_cmpint (adap_overlay_split_view_get_sidebar_position (split_view), ==, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_show_sidebar (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  gboolean show_sidebar;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::show-sidebar", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "show-sidebar", &show_sidebar, NULL);
  g_assert_true (show_sidebar);

  adap_overlay_split_view_set_show_sidebar (split_view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_show_sidebar (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_show_sidebar (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "show-sidebar", TRUE, NULL);
  g_assert_true (adap_overlay_split_view_get_show_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_pin_sidebar (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  gboolean pin_sidebar;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::pin-sidebar", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "pin-sidebar", &pin_sidebar, NULL);
  g_assert_false (pin_sidebar);

  adap_overlay_split_view_set_pin_sidebar (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_pin_sidebar (split_view, TRUE);
  g_assert_true (adap_overlay_split_view_get_pin_sidebar (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "pin-sidebar", FALSE, NULL);
  g_assert_false (adap_overlay_split_view_get_pin_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_enable_show_gesture (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  gboolean enable_show_gesture;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::enable-show-gesture", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "enable-show-gesture", &enable_show_gesture, NULL);
  g_assert_true (enable_show_gesture);

  adap_overlay_split_view_set_enable_show_gesture (split_view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_enable_show_gesture (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_enable_show_gesture (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "enable-show-gesture", TRUE, NULL);
  g_assert_true (adap_overlay_split_view_get_enable_show_gesture (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_enable_hide_gesture (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  gboolean enable_hide_gesture;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::enable-hide-gesture", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "enable-hide-gesture", &enable_hide_gesture, NULL);
  g_assert_true (enable_hide_gesture);

  adap_overlay_split_view_set_enable_hide_gesture (split_view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_enable_hide_gesture (split_view, FALSE);
  g_assert_false (adap_overlay_split_view_get_enable_hide_gesture (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "enable-hide-gesture", TRUE, NULL);
  g_assert_true (adap_overlay_split_view_get_enable_hide_gesture (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_min_sidebar_width (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  double width;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::min-sidebar-width", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "min-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 180, DBL_EPSILON));

  adap_overlay_split_view_set_min_sidebar_width (split_view, 180);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_min_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_min_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "min-sidebar-width", 180.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_min_sidebar_width (split_view), 180, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_max_sidebar_width (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  double width;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::max-sidebar-width", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "max-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 280, DBL_EPSILON));

  adap_overlay_split_view_set_max_sidebar_width (split_view, 280);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_max_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_max_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "max-sidebar-width", 280.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_max_sidebar_width (split_view), 280, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_sidebar_width_fraction (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  double fraction;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-fraction", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-width-fraction", &fraction, NULL);
  g_assert_true (G_APPROX_VALUE (fraction, 0.25, DBL_EPSILON));

  adap_overlay_split_view_set_sidebar_width_fraction (split_view, 0.25);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_sidebar_width_fraction (split_view, 0.2);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_sidebar_width_fraction (split_view), 0.2, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-fraction", 0.25, NULL);
  g_assert_true (G_APPROX_VALUE (adap_overlay_split_view_get_sidebar_width_fraction (split_view), 0.25, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adap_overlay_split_view_sidebar_width_unit (void)
{
  AdapOverlaySplitView *split_view = g_object_ref_sink (ADAP_OVERLAY_SPLIT_VIEW (adap_overlay_split_view_new ()));
  AdapLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-unit", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-width-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADAP_LENGTH_UNIT_SP);

  adap_overlay_split_view_set_sidebar_width_unit (split_view, ADAP_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 0);

  adap_overlay_split_view_set_sidebar_width_unit (split_view, ADAP_LENGTH_UNIT_PX);
  g_assert_cmpint (adap_overlay_split_view_get_sidebar_width_unit (split_view), ==, ADAP_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-unit", ADAP_LENGTH_UNIT_SP, NULL);
  g_assert_cmpint (adap_overlay_split_view_get_sidebar_width_unit (split_view), ==, ADAP_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/OverlaySplitView/sidebar", test_adap_overlay_split_view_sidebar);
  g_test_add_func ("/Adapta/OverlaySplitView/content", test_adap_overlay_split_view_content);
  g_test_add_func ("/Adapta/OverlaySplitView/collapsed", test_adap_overlay_split_view_collapsed);
  g_test_add_func ("/Adapta/OverlaySplitView/sidebar_position", test_adap_overlay_split_view_sidebar_position);
  g_test_add_func ("/Adapta/OverlaySplitView/show_sidebar", test_adap_overlay_split_view_show_sidebar);
  g_test_add_func ("/Adapta/OverlaySplitView/pin_sidebar", test_adap_overlay_split_view_pin_sidebar);
  g_test_add_func ("/Adapta/OverlaySplitView/enable_show_gesture", test_adap_overlay_split_view_enable_show_gesture);
  g_test_add_func ("/Adapta/OverlaySplitView/enable_hide_gesture", test_adap_overlay_split_view_enable_hide_gesture);
  g_test_add_func ("/Adapta/OverlaySplitView/min_sidebar_width", test_adap_overlay_split_view_min_sidebar_width);
  g_test_add_func ("/Adapta/OverlaySplitView/max_sidebar_width", test_adap_overlay_split_view_max_sidebar_width);
  g_test_add_func ("/Adapta/OverlaySplitView/sidebar_width_fraction", test_adap_overlay_split_view_sidebar_width_fraction);
  g_test_add_func ("/Adapta/OverlaySplitView/sidebar_width_unit", test_adap_overlay_split_view_sidebar_width_unit);

  return g_test_run ();
}
