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
test_adap_button_content_icon_name (void)
{
  AdapButtonContent *content = g_object_ref_sink (ADAP_BUTTON_CONTENT (adap_button_content_new ()));
  char *icon_name;
  int notified = 0;

  g_assert_nonnull (content);

  g_signal_connect_swapped (content, "notify::icon-name", G_CALLBACK (increment), &notified);

  g_object_get (content, "icon-name", &icon_name, NULL);
  g_assert_cmpstr (icon_name, ==, "");

  adap_button_content_set_icon_name (content, "");
  g_assert_cmpint (notified, ==, 0);

  adap_button_content_set_icon_name (content, "document-open-symbolic");
  g_assert_cmpstr (adap_button_content_get_icon_name (content), ==, "document-open-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (content, "icon-name", "", NULL);
  g_assert_cmpstr (adap_button_content_get_icon_name (content), ==, "");
  g_assert_cmpint (notified, ==, 2);

  g_free (icon_name);
  g_assert_finalize_object (content);
}

static void
test_adap_button_content_label (void)
{
  AdapButtonContent *content = g_object_ref_sink (ADAP_BUTTON_CONTENT (adap_button_content_new ()));
  char *label;
  int notified = 0;

  g_assert_nonnull (content);

  g_signal_connect_swapped (content, "notify::label", G_CALLBACK (increment), &notified);

  g_object_get (content, "label", &label, NULL);
  g_assert_cmpstr (label, ==, "");

  adap_button_content_set_label (content, "");
  g_assert_cmpint (notified, ==, 0);

  adap_button_content_set_label (content, "Open");
  g_assert_cmpstr (adap_button_content_get_label (content), ==, "Open");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (content, "label", "", NULL);
  g_assert_cmpstr (adap_button_content_get_label (content), ==, "");
  g_assert_cmpint (notified, ==, 2);

  g_free (label);
  g_assert_finalize_object (content);
}

static void
test_adap_button_content_use_underline (void)
{
  AdapButtonContent *content = g_object_ref_sink (ADAP_BUTTON_CONTENT (adap_button_content_new ()));
  gboolean use_underline;
  int notified = 0;

  g_assert_nonnull (content);

  g_signal_connect_swapped (content, "notify::use-underline", G_CALLBACK (increment), &notified);

  g_object_get (content, "use-underline", &use_underline, NULL);
  g_assert_false (use_underline);

  adap_button_content_set_use_underline (content, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_button_content_set_use_underline (content, TRUE);
  g_assert_true (adap_button_content_get_use_underline (content));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (content, "use-underline", FALSE, NULL);
  g_assert_false (adap_button_content_get_use_underline (content));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (content);
}

static void
test_adap_button_content_style_class_button (void)
{
  GtkWidget *window = gtk_window_new ();
  GtkWidget *button = gtk_button_new ();
  AdapButtonContent *content = ADAP_BUTTON_CONTENT (adap_button_content_new ());

  g_assert_nonnull (content);

  gtk_window_set_child (GTK_WINDOW (window), button);
  gtk_window_present (GTK_WINDOW (window));

  gtk_button_set_child (GTK_BUTTON (button), GTK_WIDGET (content));
  g_assert_true (gtk_widget_has_css_class (button, "image-text-button"));

  gtk_button_set_child (GTK_BUTTON (button), NULL);
  g_assert_false (gtk_widget_has_css_class (button, "image-text-button"));

  g_assert_finalize_object (window);
}

static void
test_adap_button_content_style_class_split_button (void)
{
  GtkWidget *window = gtk_window_new ();
  GtkWidget *button = adap_split_button_new ();
  AdapButtonContent *content = ADAP_BUTTON_CONTENT (adap_button_content_new ());

  g_assert_nonnull (content);

  gtk_window_set_child (GTK_WINDOW (window), button);
  gtk_window_present (GTK_WINDOW (window));

  adap_split_button_set_child (ADAP_SPLIT_BUTTON (button), GTK_WIDGET (content));
  g_assert_true (gtk_widget_has_css_class (button, "image-text-button"));

  adap_split_button_set_child (ADAP_SPLIT_BUTTON (button), NULL);
  g_assert_false (gtk_widget_has_css_class (button, "image-text-button"));

  g_assert_finalize_object (window);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/ButtonContent/icon_name", test_adap_button_content_icon_name);
  g_test_add_func ("/Adapta/ButtonContent/label", test_adap_button_content_label);
  g_test_add_func ("/Adapta/ButtonContent/use_underline", test_adap_button_content_use_underline);
  g_test_add_func ("/Adapta/ButtonContent/style_class_button", test_adap_button_content_style_class_button);
  g_test_add_func ("/Adapta/ButtonContent/style_class_split_button", test_adap_button_content_style_class_split_button);

  return g_test_run ();
}
