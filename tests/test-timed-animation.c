/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

static void
value_cb (double   value,
          gpointer user_data)
{
}

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_animation_value_from (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  double value;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::value-from", G_CALLBACK (increment), &notified);

  g_object_get (animation, "value-from", &value, NULL);
  g_assert_true (G_APPROX_VALUE (value, 10, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_value_from (animation, 20);
  g_object_get (animation, "value-from", &value, NULL);
  g_assert_true (G_APPROX_VALUE (value, 20, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "value-from", 30.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_timed_animation_get_value_from (animation), 30, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_value_to (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  double value;
  int notified = 0;

  g_assert_nonnull (animation);

  adap_animation_skip (ADAP_ANIMATION (animation));

  g_signal_connect_swapped (animation, "notify::value-to", G_CALLBACK (increment), &notified);

  g_object_get (animation, "value-to", &value, NULL);
  g_assert_true (G_APPROX_VALUE (value, 20, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_value_to (animation, 10);
  g_object_get (animation, "value-to", &value, NULL);
  g_assert_true (G_APPROX_VALUE (value, 10, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "value-to", 30.0, NULL);
  g_assert_true (G_APPROX_VALUE (adap_timed_animation_get_value_to (animation), 30, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_duration (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  guint duration;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::duration", G_CALLBACK (increment), &notified);

  g_object_get (animation, "duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 100);
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_duration (animation, 200);
  g_object_get (animation, "duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 200);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "duration", 300u, NULL);
  g_assert_cmpint (adap_timed_animation_get_duration (animation), ==, 300);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_easing (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  AdapEasing easing;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::easing", G_CALLBACK (increment), &notified);

  g_object_get (animation, "easing", &easing, NULL);
  g_assert_cmpint (easing, ==, ADAP_EASE_OUT_CUBIC);
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_easing (animation, ADAP_EASE_IN_CUBIC);
  g_object_get (animation, "easing", &easing, NULL);
  g_assert_cmpint (easing, ==, ADAP_EASE_IN_CUBIC);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "easing", ADAP_EASE_IN_OUT_CUBIC, NULL);
  g_assert_cmpint (adap_timed_animation_get_easing (animation), ==, ADAP_EASE_IN_OUT_CUBIC);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_repeat_count (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  guint repeat_count;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::repeat-count", G_CALLBACK (increment), &notified);

  g_object_get (animation, "repeat-count", &repeat_count, NULL);
  g_assert_cmpint (repeat_count, ==, 1);
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_repeat_count (animation, 2);
  g_object_get (animation, "repeat-count", &repeat_count, NULL);
  g_assert_cmpint (repeat_count, ==, 2);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "repeat-count", 3u, NULL);
  g_assert_cmpint (adap_timed_animation_get_repeat_count (animation), ==, 3);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_reverse (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  gboolean reverse;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::reverse", G_CALLBACK (increment), &notified);

  g_object_get (animation, "reverse", &reverse, NULL);
  g_assert_false (reverse);
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_reverse (animation, TRUE);
  g_object_get (animation, "reverse", &reverse, NULL);
  g_assert_true (reverse);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "reverse", FALSE, NULL);
  g_assert_false (adap_timed_animation_get_reverse (animation));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adap_animation_alternate (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdapAnimationTarget *target =
    adap_callback_animation_target_new (value_cb, NULL, NULL);
  AdapTimedAnimation *animation =
    ADAP_TIMED_ANIMATION (adap_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  gboolean alternate;
  int notified = 0;

  g_assert_nonnull (animation);

  g_signal_connect_swapped (animation, "notify::alternate", G_CALLBACK (increment), &notified);

  g_object_get (animation, "alternate", &alternate, NULL);
  g_assert_false (alternate);
  g_assert_cmpint (notified, ==, 0);

  adap_timed_animation_set_alternate (animation, TRUE);
  g_object_get (animation, "alternate", &alternate, NULL);
  g_assert_true (alternate);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "alternate", FALSE, NULL);
  g_assert_false (adap_timed_animation_get_alternate (animation));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/TimedAnimation/value_from", test_adap_animation_value_from);
  g_test_add_func("/Adapta/TimedAnimation/value_to", test_adap_animation_value_to);
  g_test_add_func("/Adapta/TimedAnimation/duration", test_adap_animation_duration);
  g_test_add_func("/Adapta/TimedAnimation/easing", test_adap_animation_easing);
  g_test_add_func("/Adapta/TimedAnimation/repeat_count", test_adap_animation_repeat_count);
  g_test_add_func("/Adapta/TimedAnimation/reverse", test_adap_animation_reverse);
  g_test_add_func("/Adapta/TimedAnimation/alternate", test_adap_animation_alternate);

  return g_test_run();
}
