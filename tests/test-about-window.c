/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

#include "adapta-test-resources.h"

static void
test_adap_about_window_from_appdata (void)
{
  AdapAboutWindow *window = ADAP_ABOUT_WINDOW (adap_about_window_new_from_appdata ("/org/gnome/Adapta1/Test/org.gnome.Adapta1.Test.metainfo.xml", "1.0"));

  g_assert_nonnull (window);

  g_assert_cmpstr (adap_about_window_get_release_notes (window), ==, "<p>Testing Build</p>\n");
  g_assert_cmpstr (adap_about_window_get_release_notes_version (window), ==, "1.0");
  g_assert_cmpstr (adap_about_window_get_version (window), ==, "1.0");
  g_assert_cmpstr (adap_about_window_get_application_icon (window), ==, "org.gnome.Adapta1.Test");
  g_assert_cmpstr (adap_about_window_get_application_name (window), ==, "Adapta Test");
  g_assert_cmpstr (adap_about_window_get_developer_name (window), ==, "The GNOME Project");
  g_assert_cmpstr (adap_about_window_get_issue_url (window), ==, "https://gitlab.gnome.org/GNOME/libadapta/issues");
  g_assert_cmpstr (adap_about_window_get_support_url (window), ==, "http://www.gnome.org/friends/");
  g_assert_cmpstr (adap_about_window_get_website (window), ==, "https://gitlab.gnome.org/GNOME/libadapta");
  g_assert_cmpuint (adap_about_window_get_license_type (window), ==, GTK_LICENSE_LGPL_2_1);

  g_assert_finalize_object (window);

  window = ADAP_ABOUT_WINDOW (adap_about_window_new_from_appdata ("/org/gnome/Adapta1/Test/org.gnome.Adapta1.Test.metainfo.xml", "0.1"));

  g_assert_nonnull (window);

  g_assert_cmpstr (adap_about_window_get_release_notes (window), ==, "<p>Testing Build Older</p>\n");
  g_assert_cmpstr (adap_about_window_get_release_notes_version (window), ==, "0.1");
  g_assert_cmpstr (adap_about_window_get_version (window), ==, "1.0");

  g_assert_finalize_object (window);

  window = ADAP_ABOUT_WINDOW (adap_about_window_new_from_appdata ("/org/gnome/Adapta1/Test/org.gnome.Adapta1.Test.metainfo.xml", NULL));

  g_assert_nonnull (window);

  g_assert_cmpstr (adap_about_window_get_release_notes (window), ==, "");
  g_assert_cmpstr (adap_about_window_get_release_notes_version (window), ==, "");
  g_assert_cmpstr (adap_about_window_get_version (window), ==, "1.0");

  g_assert_finalize_object (window);
}

static void
test_adap_about_window_create (void)
{
  AdapAboutWindow *window = ADAP_ABOUT_WINDOW (adap_about_window_new ());

  const char *developers[] = {
    "Angela Avery",
    NULL
  };

  const char *designers[] = {
    "GNOME Design Team",
    NULL
  };

  const char *artists[] = {
    "GNOME Design Team",
    NULL
  };

  const char *documenters[] = {
    "Angela Avery",
    NULL
  };

  const char *credits[] = {
    "Angela Avery",
    NULL
  };

  const char *acknowledgements[] = {
    "Angela Avery",
    NULL
  };

  g_assert_nonnull (window);

  g_object_set (G_OBJECT (window),
                "application-name", "Example",
                "application-icon", "org.gnome.Example",
                "developer-name", "Angela Avery",
                "version", "1.2.3",
                "release-notes-version", "1.2.0",
                "release-notes", "<p>Example</p>",
                "comments", "Comments",
                "website", "https://example.org",
                "issue-url", "https://example.org",
                "support-url", "https://example.org",
                "debug-info", "Debug",
                "debug-info-filename", "debug.txt",
                "developers", developers,
                "designers", designers,
                "artists", artists,
                "documenters", documenters,
                "translator-credits", "translator-credits",
                "copyright", "© 2022 Angela Avery",
                "license-type", GTK_LICENSE_GPL_3_0,
                NULL);

  g_assert_cmpstr (adap_about_window_get_application_name (window), ==, "Example");
  g_assert_cmpstr (adap_about_window_get_application_icon (window), ==, "org.gnome.Example");
  g_assert_cmpstr (adap_about_window_get_developer_name (window), ==, "Angela Avery");
  g_assert_cmpstr (adap_about_window_get_version (window), ==, "1.2.3");
  g_assert_cmpstr (adap_about_window_get_release_notes_version (window), ==, "1.2.0");
  g_assert_cmpstr (adap_about_window_get_release_notes (window), ==, "<p>Example</p>");
  g_assert_cmpstr (adap_about_window_get_comments (window), ==, "Comments");
  g_assert_cmpstr (adap_about_window_get_website (window), ==, "https://example.org");
  g_assert_cmpstr (adap_about_window_get_issue_url (window), ==, "https://example.org");
  g_assert_cmpstr (adap_about_window_get_support_url (window), ==, "https://example.org");
  g_assert_cmpstr (adap_about_window_get_debug_info (window), ==, "Debug");
  g_assert_cmpstr (adap_about_window_get_debug_info_filename (window), ==, "debug.txt");
  g_assert_cmpstrv (adap_about_window_get_developers (window), developers);
  g_assert_cmpstrv (adap_about_window_get_designers (window), designers);
  g_assert_cmpstrv (adap_about_window_get_artists (window), artists);
  g_assert_cmpstrv (adap_about_window_get_documenters (window), documenters);
  g_assert_cmpstr (adap_about_window_get_translator_credits (window), ==, "translator-credits");
  g_assert_cmpstr (adap_about_window_get_copyright (window), ==, "© 2022 Angela Avery");
  g_assert_cmpuint (adap_about_window_get_license_type (window), ==, GTK_LICENSE_GPL_3_0);

  adap_about_window_add_link (window, "Example", "https://example.org");
  adap_about_window_add_credit_section (window, "Example", credits);
  adap_about_window_add_acknowledgement_section (window, "Example", acknowledgements);
  adap_about_window_add_legal_section (window, "Example", "© 2022 Example", GTK_LICENSE_GPL_3_0, NULL);
  adap_about_window_add_legal_section (window, "Example", "© 2022 Example", GTK_LICENSE_CUSTOM, "License");

  g_assert_finalize_object (window);
}

int
main (int   argc,
      char *argv[])
{
  GResource *test_resources;

  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  test_resources = test_get_resource ();
  g_resources_register (test_resources);

  g_test_add_func ("/Adapta/AboutWindow/create", test_adap_about_window_create);
  g_test_add_func ("/Adapta/AboutWindow/from_appdata", test_adap_about_window_from_appdata);

  return g_test_run ();
}
