/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>


static void
test_adap_squeezer_homogeneous (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_true (adap_squeezer_get_homogeneous (squeezer));

  adap_squeezer_set_homogeneous (squeezer, FALSE);
  g_assert_false (adap_squeezer_get_homogeneous (squeezer));

  adap_squeezer_set_homogeneous (squeezer, TRUE);
  g_assert_true (adap_squeezer_get_homogeneous (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_allow_none (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adap_squeezer_get_allow_none (squeezer));

  adap_squeezer_set_allow_none (squeezer, TRUE);
  g_assert_true (adap_squeezer_get_allow_none (squeezer));

  adap_squeezer_set_allow_none (squeezer, FALSE);
  g_assert_false (adap_squeezer_get_allow_none (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_transition_duration (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_cmpuint (adap_squeezer_get_transition_duration (squeezer), ==, 200);

  adap_squeezer_set_transition_duration (squeezer, 400);
  g_assert_cmpuint (adap_squeezer_get_transition_duration (squeezer), ==, 400);

  adap_squeezer_set_transition_duration (squeezer, -1);
  g_assert_cmpuint (adap_squeezer_get_transition_duration (squeezer), ==, G_MAXUINT);

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_transition_type (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_cmpuint (adap_squeezer_get_transition_type (squeezer), ==, ADAP_SQUEEZER_TRANSITION_TYPE_NONE);

  adap_squeezer_set_transition_type (squeezer, ADAP_SQUEEZER_TRANSITION_TYPE_CROSSFADE);
  g_assert_cmpuint (adap_squeezer_get_transition_type (squeezer), ==, ADAP_SQUEEZER_TRANSITION_TYPE_CROSSFADE);

  adap_squeezer_set_transition_type (squeezer, ADAP_SQUEEZER_TRANSITION_TYPE_NONE);
  g_assert_cmpuint (adap_squeezer_get_transition_type (squeezer), ==, ADAP_SQUEEZER_TRANSITION_TYPE_NONE);

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_transition_running (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adap_squeezer_get_transition_running (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_show_hide_child (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));
  GtkWidget *child;

  g_assert_nonnull (squeezer);

  g_assert_null (adap_squeezer_get_visible_child (squeezer));

  child = gtk_label_new ("");
  adap_squeezer_add (squeezer, child);
  g_assert (adap_squeezer_get_visible_child (squeezer) == child);

  gtk_widget_set_visible (child, FALSE);
  g_assert_null (adap_squeezer_get_visible_child (squeezer));

  gtk_widget_set_visible (child, TRUE);
  g_assert (adap_squeezer_get_visible_child (squeezer) == child);

  adap_squeezer_remove (squeezer, child);
  g_assert_null (adap_squeezer_get_visible_child (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_interpolate_size (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adap_squeezer_get_interpolate_size (squeezer));

  adap_squeezer_set_interpolate_size (squeezer, TRUE);
  g_assert_true (adap_squeezer_get_interpolate_size (squeezer));

  adap_squeezer_set_interpolate_size (squeezer, FALSE);
  g_assert_false (adap_squeezer_get_interpolate_size (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adap_squeezer_page_enabled (void)
{
  AdapSqueezer *squeezer = g_object_ref_sink (ADAP_SQUEEZER (adap_squeezer_new ()));
  GtkWidget *child;
  AdapSqueezerPage *page;

  g_assert_nonnull (squeezer);

  child = gtk_label_new ("");
  page = adap_squeezer_add (squeezer, child);
  g_assert_true (adap_squeezer_page_get_enabled (page));

  adap_squeezer_page_set_enabled (page, FALSE);
  g_assert_false (adap_squeezer_page_get_enabled (page));

  adap_squeezer_page_set_enabled (page, TRUE);
  g_assert_true (adap_squeezer_page_get_enabled (page));

  g_assert_finalize_object (squeezer);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ViewSwitcher/homogeneous", test_adap_squeezer_homogeneous);
  g_test_add_func("/Adapta/ViewSwitcher/allow_none", test_adap_squeezer_allow_none);
  g_test_add_func("/Adapta/ViewSwitcher/transition_duration", test_adap_squeezer_transition_duration);
  g_test_add_func("/Adapta/ViewSwitcher/transition_type", test_adap_squeezer_transition_type);
  g_test_add_func("/Adapta/ViewSwitcher/transition_running", test_adap_squeezer_transition_running);
  g_test_add_func("/Adapta/ViewSwitcher/show_hide_child", test_adap_squeezer_show_hide_child);
  g_test_add_func("/Adapta/ViewSwitcher/interpolate_size", test_adap_squeezer_interpolate_size);
  g_test_add_func("/Adapta/ViewSwitcher/page_enabled", test_adap_squeezer_page_enabled);

  return g_test_run();
}
