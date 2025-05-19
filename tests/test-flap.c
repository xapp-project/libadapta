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
test_adap_flap_flap (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::flap", G_CALLBACK (increment), &notified);

  g_object_get (flap, "flap", &widget, NULL);
  g_assert_null (widget);

  adap_flap_set_flap (flap, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_flap_set_flap (flap, widget);
  g_assert_true (adap_flap_get_flap (flap) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "flap", NULL, NULL);
  g_assert_null (adap_flap_get_flap (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_separator (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::separator", G_CALLBACK (increment), &notified);

  g_object_get (flap, "separator", &widget, NULL);
  g_assert_null (widget);

  adap_flap_set_separator (flap, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_flap_set_separator (flap, widget);
  g_assert_true (adap_flap_get_separator (flap) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "separator", NULL, NULL);
  g_assert_null (adap_flap_get_separator (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_flap_position (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  GtkPackType position;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::flap-position", G_CALLBACK (increment), &notified);

  g_object_get (flap, "flap-position", &position, NULL);
  g_assert_cmpint (position, ==, GTK_PACK_START);

  adap_flap_set_flap_position (flap, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_flap_position (flap, GTK_PACK_END);
  g_assert_cmpint (adap_flap_get_flap_position (flap), ==, GTK_PACK_END);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "flap-position", GTK_PACK_START, NULL);
  g_assert_cmpint (adap_flap_get_flap_position (flap), ==, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_reveal_flap (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean reveal;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());

  g_signal_connect_swapped (flap, "notify::reveal-flap", G_CALLBACK (increment), &notified);

  g_object_get (flap, "reveal-flap", &reveal, NULL);
  g_assert_true (reveal);

  adap_flap_set_reveal_flap (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_reveal_flap (flap, FALSE);
  g_assert_false (adap_flap_get_reveal_flap (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "reveal-flap", TRUE, NULL);
  g_assert_true (adap_flap_get_reveal_flap (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_reveal_progress (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  double progress;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::reveal-progress", G_CALLBACK (increment), &notified);

  g_object_get (flap, "reveal-progress", &progress, NULL);
  g_assert_cmpint (progress, ==, 1.0);

  adap_flap_set_reveal_flap (flap, FALSE);
  g_assert_cmpint (adap_flap_get_reveal_progress (flap), ==, 0.0);
  g_assert_cmpint (notified, ==, 1);

  adap_flap_set_reveal_flap (flap, TRUE);
  g_assert_cmpint (adap_flap_get_reveal_progress (flap), ==, 1.0);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_fold_policy (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  AdapFlapFoldPolicy policy;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::fold-policy", G_CALLBACK (increment), &notified);

  g_object_get (flap, "fold-policy", &policy, NULL);
  g_assert_cmpint (policy, ==, ADAP_FLAP_FOLD_POLICY_AUTO);

  adap_flap_set_fold_policy (flap, ADAP_FLAP_FOLD_POLICY_AUTO);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_fold_policy (flap, ADAP_FLAP_FOLD_POLICY_NEVER);
  g_assert_cmpint (adap_flap_get_fold_policy (flap), ==, ADAP_FLAP_FOLD_POLICY_NEVER);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "fold-policy", ADAP_FLAP_FOLD_POLICY_ALWAYS, NULL);
  g_assert_cmpint (adap_flap_get_fold_policy (flap), ==, ADAP_FLAP_FOLD_POLICY_ALWAYS);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_fold_duration (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  guint duration;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::fold-duration", G_CALLBACK (increment), &notified);

  g_object_get (flap, "fold-duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 250);

  adap_flap_set_fold_duration (flap, 250);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_fold_duration (flap, 500);
  g_assert_cmpint (adap_flap_get_fold_duration (flap), ==, 500);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "fold-duration", 100, NULL);
  g_assert_cmpint (adap_flap_get_fold_duration (flap), ==, 100);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_folded (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean folded;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());
  adap_flap_set_fold_policy (flap, ADAP_FLAP_FOLD_POLICY_NEVER);

  g_signal_connect_swapped (flap, "notify::folded", G_CALLBACK (increment), &notified);

  g_object_get (flap, "folded", &folded, NULL);
  g_assert_false (folded);

  adap_flap_set_fold_policy (flap, ADAP_FLAP_FOLD_POLICY_ALWAYS);
  g_assert_true (adap_flap_get_folded (flap));
  g_assert_cmpint (notified, ==, 1);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_locked (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean locked;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());

  g_signal_connect_swapped (flap, "notify::locked", G_CALLBACK (increment), &notified);

  g_object_get (flap, "locked", &locked, NULL);
  g_assert_false (locked);

  adap_flap_set_locked (flap, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_locked (flap, TRUE);
  g_assert_true (adap_flap_get_locked (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "locked", FALSE, NULL);
  g_assert_false (adap_flap_get_locked (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_transition_type (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  AdapFlapTransitionType policy;
  int notified = 0;

  g_assert_nonnull (flap);

  g_signal_connect_swapped (flap, "notify::transition-type", G_CALLBACK (increment), &notified);

  g_object_get (flap, "transition-type", &policy, NULL);
  g_assert_cmpint (policy, ==, ADAP_FLAP_TRANSITION_TYPE_OVER);

  adap_flap_set_transition_type (flap, ADAP_FLAP_TRANSITION_TYPE_OVER);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_transition_type (flap, ADAP_FLAP_TRANSITION_TYPE_SLIDE);
  g_assert_cmpint (adap_flap_get_transition_type (flap), ==, ADAP_FLAP_TRANSITION_TYPE_SLIDE);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "transition-type", ADAP_FLAP_TRANSITION_TYPE_UNDER, NULL);
  g_assert_cmpint (adap_flap_get_transition_type (flap), ==, ADAP_FLAP_TRANSITION_TYPE_UNDER);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_modal (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean modal;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());

  g_signal_connect_swapped (flap, "notify::modal", G_CALLBACK (increment), &notified);

  g_object_get (flap, "modal", &modal, NULL);
  g_assert_true (modal);

  adap_flap_set_modal (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_modal (flap, FALSE);
  g_assert_false (adap_flap_get_modal (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "modal", TRUE, NULL);
  g_assert_true (adap_flap_get_modal (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_swipe_to_open (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean swipe_to_open;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());

  g_signal_connect_swapped (flap, "notify::swipe-to-open", G_CALLBACK (increment), &notified);

  g_object_get (flap, "swipe-to-open", &swipe_to_open, NULL);
  g_assert_true (swipe_to_open);

  adap_flap_set_swipe_to_open (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_swipe_to_open (flap, FALSE);
  g_assert_false (adap_flap_get_swipe_to_open (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "swipe-to-open", TRUE, NULL);
  g_assert_true (adap_flap_get_swipe_to_open (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

static void
test_adap_flap_swipe_to_close (void)
{
  AdapFlap *flap = g_object_ref_sink (ADAP_FLAP (adap_flap_new ()));
  gboolean swipe_to_close;
  int notified = 0;

  g_assert_nonnull (flap);

  adap_flap_set_flap (flap, gtk_button_new ());

  g_signal_connect_swapped (flap, "notify::swipe-to-close", G_CALLBACK (increment), &notified);

  g_object_get (flap, "swipe-to-close", &swipe_to_close, NULL);
  g_assert_true (swipe_to_close);

  adap_flap_set_swipe_to_close (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_flap_set_swipe_to_close (flap, FALSE);
  g_assert_false (adap_flap_get_swipe_to_close (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "swipe-to-close", TRUE, NULL);
  g_assert_true (adap_flap_get_swipe_to_close (flap));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (flap);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/Flap/flap", test_adap_flap_flap);
  g_test_add_func ("/Adapta/Flap/separator", test_adap_flap_separator);
  g_test_add_func ("/Adapta/Flap/flap_position", test_adap_flap_flap_position);
  g_test_add_func ("/Adapta/Flap/reveal_flap", test_adap_flap_reveal_flap);
  g_test_add_func ("/Adapta/Flap/reveal_progress", test_adap_flap_reveal_progress);
  g_test_add_func ("/Adapta/Flap/fold_policy", test_adap_flap_fold_policy);
  g_test_add_func ("/Adapta/Flap/fold_duration", test_adap_flap_fold_duration);
  g_test_add_func ("/Adapta/Flap/folded", test_adap_flap_folded);
  g_test_add_func ("/Adapta/Flap/locked", test_adap_flap_locked);
  g_test_add_func ("/Adapta/Flap/transition_type", test_adap_flap_transition_type);
  g_test_add_func ("/Adapta/Flap/modal", test_adap_flap_modal);
  g_test_add_func ("/Adapta/Flap/swipe_to_open", test_adap_flap_swipe_to_open);
  g_test_add_func ("/Adapta/Flap/swipe_to_close", test_adap_flap_swipe_to_close);

  return g_test_run ();
}
