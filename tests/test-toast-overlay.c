/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
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
test_adap_toast_overlay_child (void)
{
  AdapToastOverlay *toast_overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (toast_overlay);

  g_signal_connect_swapped (toast_overlay, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (toast_overlay, "child", &widget, NULL);
  g_assert_null (widget);

  adap_toast_overlay_set_child (toast_overlay, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_toast_overlay_set_child (toast_overlay, widget);
  g_assert_true (adap_toast_overlay_get_child (toast_overlay) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast_overlay, "child", NULL, NULL);
  g_assert_null (adap_toast_overlay_get_child (toast_overlay));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast_overlay);
}

static void
test_adap_toast_overlay_add_toast (void)
{
  AdapToastOverlay *toast_overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new ()));
  AdapToast *toast = adap_toast_new ("Test Notification");

  g_assert_nonnull (toast_overlay);
  g_assert_nonnull (toast);

  adap_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));
  adap_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));
  adap_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));

  g_assert_finalize_object (toast_overlay);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/ToastOverlay/child", test_adap_toast_overlay_child);
  g_test_add_func ("/Adapta/ToastOverlay/add_toast", test_adap_toast_overlay_add_toast);

  return g_test_run ();
}
