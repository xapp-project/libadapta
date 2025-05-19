/*
 * Copyright (C) 2021 Purism SPC
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
test_adap_toast_title (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  char *title;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");

  adap_toast_set_title (toast, "Another title");
  g_assert_cmpstr (adap_toast_get_title (toast), ==, "Another title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "title", "Title", NULL);
  g_assert_cmpstr (adap_toast_get_title (toast), ==, "Title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (toast);
}

static void
test_adap_toast_title_format (void)
{
  AdapToast *toast;
  const int n_value = 42;
  char *title;

  toast = adap_toast_new_format ("Title %d", n_value);

  g_assert_nonnull (toast);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title 42");

  g_free (title);
  g_assert_finalize_object (toast);
}

static void
test_adap_toast_button_label (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  char *button_label;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::button-label", G_CALLBACK (increment), &notified);

  g_object_get (toast, "button-label", &button_label, NULL);
  g_assert_null (button_label);

  adap_toast_set_button_label (toast, "Button");
  g_assert_cmpstr (adap_toast_get_button_label (toast), ==, "Button");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "button-label", "Button 2", NULL);
  g_assert_cmpstr (adap_toast_get_button_label (toast), ==, "Button 2");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adap_toast_action_name (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  char *action_name;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::action-name", G_CALLBACK (increment), &notified);

  g_object_get (toast, "action-name", &action_name, NULL);
  g_assert_null (action_name);

  adap_toast_set_action_name (toast, "win.something");
  g_assert_cmpstr (adap_toast_get_action_name (toast), ==, "win.something");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "action-name", "win.something-else", NULL);
  g_assert_cmpstr (adap_toast_get_action_name (toast), ==, "win.something-else");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adap_toast_action_target (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  GVariant *action_target, *variant;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::action-target", G_CALLBACK (increment), &notified);

  g_object_get (toast, "action-target", &action_target, NULL);
  g_assert_null (action_target);

  variant = g_variant_ref_sink (g_variant_new_int32 (1));
  adap_toast_set_action_target_value (toast, g_variant_new_int32 (1));
  g_assert_cmpvariant (adap_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 1);
  g_variant_unref (variant);

  variant = g_variant_ref_sink (g_variant_new_int32 (2));
  g_object_set (toast, "action-target", g_variant_new_int32 (2), NULL);
  g_assert_cmpvariant (adap_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 2);
  g_variant_unref (variant);

  variant = g_variant_ref_sink (g_variant_new_int32 (3));
  adap_toast_set_action_target (toast, "i", 3);
  g_assert_cmpvariant (adap_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 3);
  g_variant_unref (variant);

  g_assert_finalize_object (toast);
}

static void
test_adap_toast_detailed_action_name (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  GVariant *variant = g_variant_ref_sink (g_variant_new_int32 (2));

  g_assert_nonnull (toast);

  g_assert_null (adap_toast_get_action_name (toast));
  g_assert_null (adap_toast_get_action_target_value (toast));

  adap_toast_set_detailed_action_name (toast, "win.something");
  g_assert_cmpstr (adap_toast_get_action_name (toast), ==, "win.something");
  g_assert_null (adap_toast_get_action_target_value (toast));

  adap_toast_set_detailed_action_name (toast, "win.something(2)");
  g_assert_cmpstr (adap_toast_get_action_name (toast), ==, "win.something");
  g_assert_cmpvariant (adap_toast_get_action_target_value (toast), variant);

  g_variant_unref (variant);
  g_assert_finalize_object (toast);
}

static void
test_adap_toast_priority (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  AdapToastPriority priority;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::priority", G_CALLBACK (increment), &notified);

  g_object_get (toast, "priority", &priority, NULL);
  g_assert_cmpint (priority, ==, ADAP_TOAST_PRIORITY_NORMAL);

  adap_toast_set_priority (toast, ADAP_TOAST_PRIORITY_HIGH);
  g_assert_cmpint (adap_toast_get_priority (toast), ==, ADAP_TOAST_PRIORITY_HIGH);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "priority", ADAP_TOAST_PRIORITY_NORMAL, NULL);
  g_assert_cmpint (adap_toast_get_priority (toast), ==, ADAP_TOAST_PRIORITY_NORMAL);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adap_toast_timeout (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  guint timeout;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::timeout", G_CALLBACK (increment), &notified);

  g_object_get (toast, "timeout", &timeout, NULL);
  g_assert_cmpint (timeout, ==, 5);

  adap_toast_set_timeout (toast, 10);
  g_assert_cmpint (adap_toast_get_timeout (toast), ==, 10);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "timeout", 5, NULL);
  g_assert_cmpint (adap_toast_get_timeout (toast), ==, 5);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adap_toast_dismiss (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  AdapToastOverlay *overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new ()));

  g_assert_nonnull (overlay);
  g_assert_nonnull (toast);

  adap_toast_overlay_add_toast (overlay, g_object_ref (toast));
  adap_toast_dismiss (toast);

  /* Repeat dismiss() calls should no-op */
  adap_toast_overlay_add_toast (overlay, g_object_ref (toast));
  adap_toast_dismiss (toast);
  adap_toast_dismiss (toast);
  adap_toast_dismiss (toast);

  g_assert_finalize_object (overlay);
  g_assert_finalize_object (toast);
}

static void
test_adap_toast_custom_title (void)
{
  AdapToast *toast = adap_toast_new ("Title");
  GtkWidget *widget = NULL;
  char *title;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::custom-title", G_CALLBACK (increment), &notified);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");
  g_object_get (toast, "custom-title", &widget, NULL);
  g_assert_null (widget);

  adap_toast_set_title (toast, "Another title");
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (gtk_label_new ("Custom title"));
  adap_toast_set_custom_title (toast, widget);
  g_assert_true (adap_toast_get_custom_title (toast) == widget);
  g_assert_null (adap_toast_get_title (toast));
  g_assert_cmpint (notified, ==, 1);

  adap_toast_set_title (toast, "Final title");
  g_assert_null (adap_toast_get_custom_title (toast));
  g_assert_cmpstr (adap_toast_get_title (toast), ==, "Final title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (toast);
  g_assert_finalize_object (widget);
}

static void
test_adap_toast_custom_title_overlay (void)
{
  AdapToastOverlay *first_overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new ()));
  AdapToastOverlay *second_overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new ()));
  AdapToast *toast = adap_toast_new ("");
  GtkWidget *widget = gtk_label_new ("Custom title");

  g_assert_nonnull (first_overlay);
  g_assert_nonnull (second_overlay);
  g_assert_nonnull (toast);

  adap_toast_set_custom_title (toast, g_object_ref (widget));

  adap_toast_overlay_add_toast (first_overlay, g_object_ref (toast));
  adap_toast_dismiss (toast);
  adap_toast_overlay_add_toast (second_overlay, g_object_ref (toast));

  g_assert_finalize_object (first_overlay);
  g_assert_finalize_object (second_overlay);
  g_assert_finalize_object (toast);
  g_assert_finalize_object (widget);
}

static void
test_adap_toast_use_markup (void)
{
  AdapToastOverlay *toast_overlay = g_object_ref_sink (ADAP_TOAST_OVERLAY (adap_toast_overlay_new()));
  AdapToast *toast = adap_toast_new ("");

  g_assert_nonnull (toast_overlay);
  g_assert_nonnull (toast);

  adap_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));
  adap_toast_set_use_markup (toast, FALSE);
  adap_toast_set_title (toast, "<span false>bad markup</sp>");

  g_assert_cmpstr (adap_toast_get_title (toast), ==, "<span false>bad markup</sp>");

  g_assert_finalize_object (toast_overlay);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/Toast/title", test_adap_toast_title);
  g_test_add_func ("/Adapta/Toast/title_format", test_adap_toast_title_format);
  g_test_add_func ("/Adapta/Toast/button_label", test_adap_toast_button_label);
  g_test_add_func ("/Adapta/Toast/action_name", test_adap_toast_action_name);
  g_test_add_func ("/Adapta/Toast/action_target", test_adap_toast_action_target);
  g_test_add_func ("/Adapta/Toast/detailed_action_name", test_adap_toast_detailed_action_name);
  g_test_add_func ("/Adapta/Toast/priority", test_adap_toast_priority);
  g_test_add_func ("/Adapta/Toast/timeout", test_adap_toast_timeout);
  g_test_add_func ("/Adapta/Toast/dismiss", test_adap_toast_dismiss);
  g_test_add_func ("/Adapta/Toast/custom_title", test_adap_toast_custom_title);
  g_test_add_func ("/Adapta/Toast/custom_title_overlay", test_adap_toast_custom_title_overlay);
  g_test_add_func ("/Adapta/Toast/use_markup", test_adap_toast_use_markup);

  return g_test_run ();
}
