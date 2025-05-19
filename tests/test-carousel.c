/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
allocate_carousel (AdapCarousel *carousel)
{
  int width, height;

  gtk_widget_measure (GTK_WIDGET (carousel), GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &width, NULL, NULL);
  gtk_widget_measure (GTK_WIDGET (carousel), GTK_ORIENTATION_VERTICAL, width,
                      NULL, &height, NULL, NULL);
  gtk_widget_allocate (GTK_WIDGET (carousel), width, height, 0, NULL);
}

static void
test_adap_carousel_add_remove (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  GtkWidget *child1, *child2, *child3;
  int notified = 0;

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");

  g_signal_connect_swapped (carousel, "notify::n-pages", G_CALLBACK (increment), &notified);

  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 0);

  adap_carousel_append (carousel, child1);
  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adap_carousel_prepend (carousel, child2);
  allocate_carousel (carousel);
  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 2);
  g_assert_true (adap_carousel_get_nth_page (carousel, 0) == child2);
  g_assert_true (adap_carousel_get_nth_page (carousel, 1) == child1);
  g_assert_true (G_APPROX_VALUE (adap_carousel_get_position (carousel), 1, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  adap_carousel_insert (carousel, child3, 1);
  allocate_carousel (carousel);
  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 3);
  g_assert_true (adap_carousel_get_nth_page (carousel, 0) == child2);
  g_assert_true (adap_carousel_get_nth_page (carousel, 1) == child3);
  g_assert_true (adap_carousel_get_nth_page (carousel, 2) == child1);
  g_assert_true (G_APPROX_VALUE (adap_carousel_get_position (carousel), 2, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 3);

  adap_carousel_scroll_to (carousel, child3, FALSE);
  adap_carousel_remove (carousel, child2);
  allocate_carousel (carousel);
  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 2);
  g_assert_true (G_APPROX_VALUE (adap_carousel_get_position (carousel), 0, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 4);

  adap_carousel_remove (carousel, child1);
  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 1);
  g_assert_cmpint (notified, ==, 5);

  g_assert_finalize_object (carousel);
}

static void
assert_carousel_positions (AdapCarousel *carousel,
                           GtkWidget   *child1,
                           GtkWidget   *child2,
                           GtkWidget   *child3,
                           GtkWidget   *child4,
                           double       position)
{
  allocate_carousel (carousel);
  g_assert_true (adap_carousel_get_nth_page (carousel, 0) == child1);
  g_assert_true (adap_carousel_get_nth_page (carousel, 1) == child2);
  g_assert_true (adap_carousel_get_nth_page (carousel, 2) == child3);
  g_assert_true (adap_carousel_get_nth_page (carousel, 3) == child4);
  g_assert_true (G_APPROX_VALUE (adap_carousel_get_position (carousel), position, DBL_EPSILON));
}

static void
test_adap_carousel_reorder (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  GtkWidget *child1, *child2, *child3, *child4;

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");
  child4 = gtk_label_new ("");

  adap_carousel_append (carousel, child1);
  adap_carousel_append (carousel, child2);
  adap_carousel_append (carousel, child3);
  adap_carousel_append (carousel, child4);
  allocate_carousel (carousel);

  g_assert_cmpuint (adap_carousel_get_n_pages (carousel), ==, 4);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);

  /* No-op */
  adap_carousel_reorder (carousel, child1, 0);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);
  adap_carousel_reorder (carousel, child2, 1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);
  adap_carousel_reorder (carousel, child3, 2);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);
  adap_carousel_reorder (carousel, child4, 3);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);

  adap_carousel_reorder (carousel, child4, 4);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);
  adap_carousel_reorder (carousel, child4, -1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);

  adap_carousel_scroll_to (carousel, child1, FALSE);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);
  adap_carousel_reorder (carousel, child2, 2);
  assert_carousel_positions (carousel, child1, child3, child2, child4, 0);
  adap_carousel_reorder (carousel, child2, 1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 0);

  adap_carousel_scroll_to (carousel, child2, FALSE);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 1);
  adap_carousel_reorder (carousel, child2, 2);
  assert_carousel_positions (carousel, child1, child3, child2, child4, 2);
  adap_carousel_reorder (carousel, child2, 1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 1);

  adap_carousel_scroll_to (carousel, child3, FALSE);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 2);
  adap_carousel_reorder (carousel, child2, 2);
  assert_carousel_positions (carousel, child1, child3, child2, child4, 1);
  adap_carousel_reorder (carousel, child2, 1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 2);

  adap_carousel_scroll_to (carousel, child4, FALSE);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 3);
  adap_carousel_reorder (carousel, child2, 2);
  assert_carousel_positions (carousel, child1, child3, child2, child4, 3);
  adap_carousel_reorder (carousel, child2, 1);
  assert_carousel_positions (carousel, child1, child2, child3, child4, 3);

  g_assert_finalize_object (carousel);
}

static void
test_adap_carousel_interactive (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  gboolean interactive;
  int notified = 0;

  g_signal_connect_swapped (carousel, "notify::interactive", G_CALLBACK (increment), &notified);

  /* Accessors */
  g_assert_true (adap_carousel_get_interactive (carousel));
  adap_carousel_set_interactive (carousel, FALSE);
  g_assert_false (adap_carousel_get_interactive (carousel));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "interactive", TRUE, NULL);
  g_object_get (carousel, "interactive", &interactive, NULL);
  g_assert_true (interactive);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  adap_carousel_set_interactive (carousel, TRUE);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (carousel);
}

static void
test_adap_carousel_spacing (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  guint spacing;
  int notified = 0;

  g_signal_connect_swapped (carousel, "notify::spacing", G_CALLBACK (increment), &notified);

  /* Accessors */
  g_assert_cmpuint (adap_carousel_get_spacing (carousel), ==, 0);
  adap_carousel_set_spacing (carousel, 12);
  g_assert_cmpuint (adap_carousel_get_spacing (carousel), ==, 12);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "spacing", 6, NULL);
  g_object_get (carousel, "spacing", &spacing, NULL);
  g_assert_cmpuint (spacing, ==, 6);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  adap_carousel_set_spacing (carousel, 6);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (carousel);
}

static void
test_adap_carousel_allow_mouse_drag (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  gboolean allow_mouse_drag;
  int notified = 0;

  g_signal_connect_swapped (carousel, "notify::allow-mouse-drag", G_CALLBACK (increment), &notified);

  /* Accessors */
  g_assert_true (adap_carousel_get_allow_mouse_drag (carousel));
  adap_carousel_set_allow_mouse_drag (carousel, FALSE);
  g_assert_false (adap_carousel_get_allow_mouse_drag (carousel));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "allow-mouse-drag", TRUE, NULL);
  g_object_get (carousel, "allow-mouse-drag", &allow_mouse_drag, NULL);
  g_assert_true (allow_mouse_drag);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  adap_carousel_set_allow_mouse_drag (carousel, TRUE);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (carousel);
}

static void
test_adap_carousel_allow_long_swipes (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  gboolean allow_long_swipes;
  int notified = 0;

  g_signal_connect_swapped (carousel, "notify::allow-long-swipes", G_CALLBACK (increment), &notified);

  /* Accessors */
  g_assert_false (adap_carousel_get_allow_long_swipes (carousel));
  adap_carousel_set_allow_long_swipes (carousel, TRUE);
  g_assert_true (adap_carousel_get_allow_long_swipes (carousel));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "allow-long-swipes", FALSE, NULL);
  g_object_get (carousel, "allow-long-swipes", &allow_long_swipes, NULL);
  g_assert_false (allow_long_swipes);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  adap_carousel_set_allow_long_swipes (carousel, FALSE);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (carousel);
}

static void
test_adap_carousel_reveal_duration (void)
{
  AdapCarousel *carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  guint duration;
  int notified = 0;

  g_signal_connect_swapped (carousel, "notify::reveal-duration", G_CALLBACK (increment), &notified);

  /* Accessors */
  g_assert_cmpuint (adap_carousel_get_reveal_duration (carousel), ==, 0);
  adap_carousel_set_reveal_duration (carousel, 200);
  g_assert_cmpuint (adap_carousel_get_reveal_duration (carousel), ==, 200);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "reveal-duration", 500, NULL);
  g_object_get (carousel, "reveal-duration", &duration, NULL);
  g_assert_cmpuint (duration, ==, 500);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  adap_carousel_set_reveal_duration (carousel, 500);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (carousel);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/Carousel/add_remove", test_adap_carousel_add_remove);
  g_test_add_func("/Adapta/Carousel/reorder", test_adap_carousel_reorder);
  g_test_add_func("/Adapta/Carousel/interactive", test_adap_carousel_interactive);
  g_test_add_func("/Adapta/Carousel/spacing", test_adap_carousel_spacing);
  g_test_add_func("/Adapta/Carousel/allow_mouse_drag", test_adap_carousel_allow_mouse_drag);
  g_test_add_func("/Adapta/Carousel/allow_long_swipes", test_adap_carousel_allow_long_swipes);
  g_test_add_func("/Adapta/Carousel/reveal_duration", test_adap_carousel_reveal_duration);
  return g_test_run();
}
