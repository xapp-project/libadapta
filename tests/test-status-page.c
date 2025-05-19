/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
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
test_adap_status_page_icon_name (void)
{
  AdapStatusPage *status_page = ADAP_STATUS_PAGE (g_object_ref_sink (adap_status_page_new ()));
  const char *icon_name = NULL;
  int notified = 0;

  g_assert_nonnull (status_page);

  g_signal_connect_swapped (status_page, "notify::icon-name", G_CALLBACK (increment), &notified);

  g_object_get (status_page, "icon-name", &icon_name, NULL);
  g_assert_cmpstr (icon_name, ==, NULL);

  adap_status_page_set_icon_name (status_page, NULL);
  g_assert_cmpint (notified, ==, 0);

  adap_status_page_set_icon_name (status_page, "some-icon-symbolic");
  g_assert_cmpstr (adap_status_page_get_icon_name (status_page), ==, "some-icon-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "icon-name", "other-icon-symbolic", NULL);
  g_assert_cmpstr (adap_status_page_get_icon_name (status_page), ==, "other-icon-symbolic");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (status_page);
}

static void
test_adap_status_page_title (void)
{
  AdapStatusPage *status_page = ADAP_STATUS_PAGE (g_object_ref_sink (adap_status_page_new ()));
  char *title;
  int notified = 0;

  g_assert_nonnull (status_page);

  g_signal_connect_swapped (status_page, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (status_page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");

  adap_status_page_set_title (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  adap_status_page_set_title (status_page, "Some Title");
  g_assert_cmpstr (adap_status_page_get_title (status_page), ==, "Some Title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "title", "Other Title", NULL);
  g_assert_cmpstr (adap_status_page_get_title (status_page), ==, "Other Title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (status_page);
}

static void
test_adap_status_page_description (void)
{
  AdapStatusPage *status_page = ADAP_STATUS_PAGE (g_object_ref_sink (adap_status_page_new ()));
  char *description;
  int notified = 0;

  g_assert_nonnull (status_page);

  g_signal_connect_swapped (status_page, "notify::description", G_CALLBACK (increment), &notified);

  g_object_get (status_page, "description", &description, NULL);
  g_assert_cmpstr (description, ==, "");

  adap_status_page_set_description (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  adap_status_page_set_description (status_page, "Some description");
  g_assert_cmpstr (adap_status_page_get_description (status_page), ==, "Some description");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "description", "Other description", NULL);
  g_assert_cmpstr (adap_status_page_get_description (status_page), ==, "Other description");
  g_assert_cmpint (notified, ==, 2);

  g_free (description);
  g_assert_finalize_object (status_page);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/StatusPage/icon_name", test_adap_status_page_icon_name);
  g_test_add_func ("/Adapta/StatusPage/title", test_adap_status_page_title);
  g_test_add_func ("/Adapta/StatusPage/description", test_adap_status_page_description);

  return g_test_run ();
}
