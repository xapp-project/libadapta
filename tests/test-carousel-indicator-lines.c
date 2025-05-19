/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
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
test_adap_carousel_indicator_lines_carousel (void)
{
  AdapCarouselIndicatorLines *lines = g_object_ref_sink (ADAP_CAROUSEL_INDICATOR_LINES (adap_carousel_indicator_lines_new ()));
  AdapCarousel *carousel;
  int notified = 0;

  g_assert_nonnull (lines);

  g_signal_connect_swapped (lines, "notify::carousel", G_CALLBACK (increment), &notified);

  carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  g_assert_nonnull (carousel);

  g_assert_null (adap_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 0);

  adap_carousel_indicator_lines_set_carousel (lines, carousel);
  g_assert (adap_carousel_indicator_lines_get_carousel (lines) == carousel);
  g_assert_cmpint (notified, ==, 1);

  adap_carousel_indicator_lines_set_carousel (lines, NULL);
  g_assert_null (adap_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (lines);
  g_assert_finalize_object (carousel);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/CarouselInidicatorLines/carousel", test_adap_carousel_indicator_lines_carousel);
  return g_test_run();
}
