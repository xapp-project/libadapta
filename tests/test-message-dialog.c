/*
 * Copyright (C) 2022 Purism SPC
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
test_adap_message_dialog_heading (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  char *heading;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::heading", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "heading", &heading, NULL);
  g_assert_cmpstr (heading, ==, "");

  adap_message_dialog_set_heading (dialog, "Heading");
  g_assert_cmpstr (adap_message_dialog_get_heading (dialog), ==, "Heading");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "heading", "Heading 2", NULL);
  g_assert_cmpstr (adap_message_dialog_get_heading (dialog), ==, "Heading 2");
  g_assert_cmpint (notified, ==, 2);

  g_free (heading);
  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_heading_use_markup (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  gboolean use_markup;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::heading-use-markup", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "heading-use-markup", &use_markup, NULL);
  g_assert_false (use_markup);

  adap_message_dialog_set_heading_use_markup (dialog, TRUE);
  g_assert_true (adap_message_dialog_get_heading_use_markup (dialog));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "heading-use-markup", FALSE, NULL);
  g_assert_false (adap_message_dialog_get_heading_use_markup (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_body (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  char *body;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::body", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "body", &body, NULL);
  g_assert_cmpstr (body, ==, "");

  adap_message_dialog_set_body (dialog, "Body");
  g_assert_cmpstr (adap_message_dialog_get_body (dialog), ==, "Body");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "body", "Body 2", NULL);
  g_assert_cmpstr (adap_message_dialog_get_body (dialog), ==, "Body 2");
  g_assert_cmpint (notified, ==, 2);

  g_free (body);
  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_body_use_markup (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  gboolean use_markup;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::body-use-markup", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "body-use-markup", &use_markup, NULL);
  g_assert_false (use_markup);

  adap_message_dialog_set_body_use_markup (dialog, TRUE);
  g_assert_true (adap_message_dialog_get_body_use_markup (dialog));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "body-use-markup", FALSE, NULL);
  g_assert_false (adap_message_dialog_get_body_use_markup (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_format (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_format_heading_markup (dialog, "Heading <b>%d</b>", 42);
  g_assert_cmpstr (adap_message_dialog_get_heading (dialog), ==, "Heading <b>42</b>");
  g_assert_true (adap_message_dialog_get_heading_use_markup (dialog));

  adap_message_dialog_format_heading (dialog, "Heading %d", 42);
  g_assert_cmpstr (adap_message_dialog_get_heading (dialog), ==, "Heading 42");
  g_assert_false (adap_message_dialog_get_heading_use_markup (dialog));

  adap_message_dialog_format_body_markup (dialog, "Body <b>%d</b>", 42);
  g_assert_cmpstr (adap_message_dialog_get_body (dialog), ==, "Body <b>42</b>");
  g_assert_true (adap_message_dialog_get_body_use_markup (dialog));

  adap_message_dialog_format_body (dialog, "Body %d", 42);
  g_assert_cmpstr (adap_message_dialog_get_body (dialog), ==, "Body 42");
  g_assert_false (adap_message_dialog_get_body_use_markup (dialog));

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_extra_child (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::extra-child", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "extra-child", &widget, NULL);
  g_assert_null (widget);

  adap_message_dialog_set_extra_child (dialog, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adap_message_dialog_set_extra_child (dialog, widget);
  g_assert_true (adap_message_dialog_get_extra_child (dialog) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "extra-child", NULL, NULL);
  g_assert_null (adap_message_dialog_get_extra_child (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_add_response (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_response (dialog, "response1", "Response 1");
  adap_message_dialog_add_response (dialog, "response2", "Response 2");

  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response1"), ==, "Response 1");
  g_assert_true (adap_message_dialog_get_response_enabled (dialog, "response1"));
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response1"), ==, ADAP_RESPONSE_DEFAULT);

  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response2"), ==, "Response 2");
  g_assert_true (adap_message_dialog_get_response_enabled (dialog, "response2"));
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response2"), ==, ADAP_RESPONSE_DEFAULT);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_add_responses (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_responses (dialog,
                                    "response1", "Response 1",
                                    "response2", "Response 2",
                                    NULL);

  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response1"), ==, "Response 1");
  g_assert_true (adap_message_dialog_get_response_enabled (dialog, "response1"));
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response1"), ==, ADAP_RESPONSE_DEFAULT);

  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response2"), ==, "Response 2");
  g_assert_true (adap_message_dialog_get_response_enabled (dialog, "response2"));
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response2"), ==, ADAP_RESPONSE_DEFAULT);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_remove_response (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_response (dialog, "response1", "Response 1");
  adap_message_dialog_add_response (dialog, "response2", "Response 2");
  adap_message_dialog_remove_response (dialog, "response1");

  g_assert_false (adap_message_dialog_has_response (dialog, "response1"));
  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response2"), ==, "Response 2");

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_response_label (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_response (dialog, "response", "Response");
  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response"), ==, "Response");

  adap_message_dialog_set_response_label (dialog, "response", "Label");
  g_assert_cmpstr (adap_message_dialog_get_response_label (dialog, "response"), ==, "Label");

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_response_enabled (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_response (dialog, "response", "Response");
  g_assert_true (adap_message_dialog_get_response_enabled (dialog, "response"));

  adap_message_dialog_set_response_enabled (dialog, "response", FALSE);
  g_assert_false (adap_message_dialog_get_response_enabled (dialog, "response"));

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_response_appearance (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));

  g_assert_nonnull (dialog);

  adap_message_dialog_add_response (dialog, "response", "Response");
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response"), ==, ADAP_RESPONSE_DEFAULT);

  adap_message_dialog_set_response_appearance (dialog, "response", ADAP_RESPONSE_DESTRUCTIVE);
  g_assert_cmpint (adap_message_dialog_get_response_appearance (dialog, "response"), ==, ADAP_RESPONSE_DESTRUCTIVE);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_response_signal (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  int responses = 0, responses_cancel = 0, responses_save = 0;

  g_signal_connect_swapped (dialog, "response", G_CALLBACK (increment), &responses);
  g_signal_connect_swapped (dialog, "response::cancel", G_CALLBACK (increment), &responses_cancel);
  g_signal_connect_swapped (dialog, "response::save", G_CALLBACK (increment), &responses_save);

  adap_message_dialog_add_response (dialog, "cancel", "Cancel");
  adap_message_dialog_add_response (dialog, "save", "Save");

  adap_message_dialog_response (dialog, "cancel");
  g_assert_cmpint (responses, ==, 1);
  g_assert_cmpint (responses_cancel, ==, 1);
  g_assert_cmpint (responses_save, ==, 0);

  adap_message_dialog_response (dialog, "save");
  g_assert_cmpint (responses, ==, 2);
  g_assert_cmpint (responses_cancel, ==, 1);
  g_assert_cmpint (responses_save, ==, 1);

  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_default_response (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  char *response;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::default-response", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "default-response", &response, NULL);
  g_assert_null (response);

  adap_message_dialog_set_default_response (dialog, "save");
  g_assert_cmpstr (adap_message_dialog_get_default_response (dialog), ==, "save");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "default-response", "load", NULL);
  g_assert_cmpstr (adap_message_dialog_get_default_response (dialog), ==, "load");
  g_assert_cmpint (notified, ==, 2);

  g_free (response);
  g_assert_finalize_object (dialog);
}

static void
test_adap_message_dialog_close_response (void)
{
  AdapMessageDialog *dialog = ADAP_MESSAGE_DIALOG (adap_message_dialog_new (NULL, NULL, NULL));
  char *response;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::close-response", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "close-response", &response, NULL);
  g_assert_cmpstr (response, ==, "close");

  adap_message_dialog_set_close_response (dialog, "save");
  g_assert_cmpstr (adap_message_dialog_get_close_response (dialog), ==, "save");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "close-response", "cancel", NULL);
  g_assert_cmpstr (adap_message_dialog_get_close_response (dialog), ==, "cancel");
  g_assert_cmpint (notified, ==, 2);

  g_free (response);
  g_assert_finalize_object (dialog);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/MessageDialog/heading", test_adap_message_dialog_heading);
  g_test_add_func ("/Adapta/MessageDialog/heading_use_markup", test_adap_message_dialog_heading_use_markup);
  g_test_add_func ("/Adapta/MessageDialog/body", test_adap_message_dialog_body);
  g_test_add_func ("/Adapta/MessageDialog/body_use_markup", test_adap_message_dialog_body_use_markup);
  g_test_add_func ("/Adapta/MessageDialog/format", test_adap_message_dialog_format);
  g_test_add_func ("/Adapta/MessageDialog/extra_child", test_adap_message_dialog_extra_child);
  g_test_add_func ("/Adapta/MessageDialog/add_response", test_adap_message_dialog_add_response);
  g_test_add_func ("/Adapta/MessageDialog/add_responses", test_adap_message_dialog_add_responses);
  g_test_add_func ("/Adapta/MessageDialog/remove_response", test_adap_message_dialog_remove_response);
  g_test_add_func ("/Adapta/MessageDialog/response_label", test_adap_message_dialog_response_label);
  g_test_add_func ("/Adapta/MessageDialog/response_enabled", test_adap_message_dialog_response_enabled);
  g_test_add_func ("/Adapta/MessageDialog/response_appearance", test_adap_message_dialog_response_appearance);
  g_test_add_func ("/Adapta/MessageDialog/response_signal", test_adap_message_dialog_response_signal);
  g_test_add_func ("/Adapta/MessageDialog/default_response", test_adap_message_dialog_default_response);
  g_test_add_func ("/Adapta/MessageDialog/close_response", test_adap_message_dialog_close_response);

  return g_test_run ();
}
