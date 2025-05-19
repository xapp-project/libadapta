/*
 * Copyright (C) 2022 Jamie Murphy <hello@itsjamie.dev>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
test_adap_banner_revealed (void)
{
  AdapBanner *banner = g_object_ref_sink (ADAP_BANNER (adap_banner_new ("")));

  g_assert_nonnull (banner);

  g_assert_false (adap_banner_get_revealed (banner));

  adap_banner_set_revealed (banner, TRUE);
  g_assert_true (adap_banner_get_revealed (banner));

  adap_banner_set_revealed (banner, FALSE);
  g_assert_false (adap_banner_get_revealed (banner));

  g_assert_finalize_object (banner);
}

static void
test_adap_banner_title (void)
{
  AdapBanner *banner = g_object_ref_sink (ADAP_BANNER (adap_banner_new ("")));

  g_assert_nonnull (banner);

  g_assert_cmpstr (adap_banner_get_title (banner), ==, "");

  adap_banner_set_title (banner, "Dummy title");
  g_assert_cmpstr (adap_banner_get_title (banner), ==, "Dummy title");

  adap_banner_set_use_markup (banner, FALSE);
  adap_banner_set_title (banner, "Invalid <b>markup");
  g_assert_cmpstr (adap_banner_get_title (banner), ==, "Invalid <b>markup");

  g_assert_finalize_object (banner);
}

static void
test_adap_banner_button_label (void)
{
  AdapBanner *banner = g_object_ref_sink (ADAP_BANNER (adap_banner_new ("")));
  char *button_label;

  g_assert_nonnull (banner);

  g_object_get (banner, "button-label", &button_label, NULL);
  g_assert_null (button_label);

  adap_banner_set_button_label (banner, "Dummy label");
  g_assert_cmpstr (adap_banner_get_button_label (banner), ==, "Dummy label");

  adap_banner_set_button_label (banner, NULL);
  g_assert_cmpstr (adap_banner_get_button_label (banner), ==, "");

  g_object_set (banner, "button-label", "Button 2", NULL);
  g_assert_cmpstr (adap_banner_get_button_label (banner), ==, "Button 2");

  g_assert_finalize_object (banner);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/Banner/revealed", test_adap_banner_revealed);
  g_test_add_func ("/Adapta/Banner/title", test_adap_banner_title);
  g_test_add_func ("/Adapta/Banner/button_label", test_adap_banner_button_label);

  return g_test_run ();
}
