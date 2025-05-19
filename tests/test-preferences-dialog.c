/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
test_adap_preferences_dialog_add_remove (void)
{
  AdapPreferencesDialog *dialog = g_object_ref_sink (ADAP_PREFERENCES_DIALOG (adap_preferences_dialog_new ()));
  AdapPreferencesPage *page;

  g_assert_nonnull (dialog);

  page = ADAP_PREFERENCES_PAGE (adap_preferences_page_new ());
  g_assert_nonnull (page);
  adap_preferences_dialog_add (dialog, page);

  adap_preferences_dialog_remove (dialog, page);

  g_assert_finalize_object (dialog);
}

static void
test_adap_preferences_dialog_add_toast (void)
{
  AdapPreferencesDialog *dialog = g_object_ref_sink (ADAP_PREFERENCES_DIALOG (adap_preferences_dialog_new ()));
  AdapToast *toast = adap_toast_new ("Test Notification");

  g_assert_nonnull (dialog);
  g_assert_nonnull (toast);

  adap_preferences_dialog_add_toast (dialog, g_object_ref (toast));

  g_assert_finalize_object (dialog);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/PreferencesDialog/add_remove", test_adap_preferences_dialog_add_remove);
  g_test_add_func("/Adapta/PreferencesDialog/add_toast", test_adap_preferences_dialog_add_toast);

  return g_test_run();
}
