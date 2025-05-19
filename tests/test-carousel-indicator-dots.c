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
test_adap_carousel_indicator_dots_carousel (void)
{
  AdapCarouselIndicatorDots *dots = g_object_ref_sink (ADAP_CAROUSEL_INDICATOR_DOTS (adap_carousel_indicator_dots_new ()));
  AdapCarousel *carousel;
  int notified = 0;

  g_assert_nonnull (dots);

  g_signal_connect_swapped (dots, "notify::carousel", G_CALLBACK (increment), &notified);

  carousel = g_object_ref_sink (ADAP_CAROUSEL (adap_carousel_new ()));
  g_assert_nonnull (carousel);

  g_assert_null (adap_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 0);

  adap_carousel_indicator_dots_set_carousel (dots, carousel);
  g_assert (adap_carousel_indicator_dots_get_carousel (dots) == carousel);
  g_assert_cmpint (notified, ==, 1);

  adap_carousel_indicator_dots_set_carousel (dots, NULL);
  g_assert_null (adap_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dots);
  g_assert_finalize_object (carousel);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/CarouselIndicatorDots/carousel", test_adap_carousel_indicator_dots_carousel);
  return g_test_run();
}
