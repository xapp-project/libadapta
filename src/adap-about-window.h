/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-window.h"

G_BEGIN_DECLS

#define ADAP_TYPE_ABOUT_WINDOW (adap_about_window_get_type())

ADAP_AVAILABLE_IN_1_2
G_DECLARE_FINAL_TYPE (AdapAboutWindow, adap_about_window, ADAP, ABOUT_WINDOW, AdapWindow)

ADAP_AVAILABLE_IN_1_2
GtkWidget *adap_about_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_about_window_new_from_appdata (const char *resource_path,
                                              const char *release_notes_version) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_application_name (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_application_name (AdapAboutWindow *self,
                                                   const char     *application_name);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_application_icon (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_application_icon (AdapAboutWindow *self,
                                                   const char     *application_icon);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_developer_name (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_developer_name (AdapAboutWindow *self,
                                                 const char     *developer_name);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_version (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_version (AdapAboutWindow *self,
                                          const char     *version);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_release_notes_version (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_release_notes_version (AdapAboutWindow *self,
                                                        const char     *version);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_release_notes (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_release_notes (AdapAboutWindow *self,
                                                const char     *release_notes);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_comments (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_comments (AdapAboutWindow *self,
                                           const char     *comments);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_website (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_website (AdapAboutWindow *self,
                                          const char     *website);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_support_url (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_support_url (AdapAboutWindow *self,
                                              const char     *support_url);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_issue_url (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_issue_url (AdapAboutWindow *self,
                                            const char     *issue_url);

ADAP_AVAILABLE_IN_1_2
void adap_about_window_add_link (AdapAboutWindow *self,
                                const char     *title,
                                const char     *url);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_debug_info (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_debug_info (AdapAboutWindow *self,
                                             const char     *debug_info);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_debug_info_filename (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_debug_info_filename (AdapAboutWindow *self,
                                                      const char     *filename);

ADAP_AVAILABLE_IN_1_2
const char * const *adap_about_window_get_developers (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void                adap_about_window_set_developers (AdapAboutWindow  *self,
                                                     const char     **developers);

ADAP_AVAILABLE_IN_1_2
const char * const *adap_about_window_get_designers (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void                adap_about_window_set_designers (AdapAboutWindow  *self,
                                                    const char     **designers);

ADAP_AVAILABLE_IN_1_2
const char * const *adap_about_window_get_artists (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void                adap_about_window_set_artists (AdapAboutWindow  *self,
                                                  const char     **artists);

ADAP_AVAILABLE_IN_1_2
const char * const *adap_about_window_get_documenters (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void                adap_about_window_set_documenters (AdapAboutWindow  *self,
                                                      const char     **documenters);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_translator_credits (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_translator_credits (AdapAboutWindow *self,
                                                     const char     *translator_credits);

ADAP_AVAILABLE_IN_1_2
void adap_about_window_add_credit_section (AdapAboutWindow  *self,
                                          const char      *name,
                                          const char     **people);

ADAP_AVAILABLE_IN_1_2
void adap_about_window_add_acknowledgement_section (AdapAboutWindow  *self,
                                                   const char      *name,
                                                   const char     **people);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_copyright (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_copyright (AdapAboutWindow *self,
                                            const char     *copyright);

ADAP_AVAILABLE_IN_1_2
GtkLicense adap_about_window_get_license_type (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void       adap_about_window_set_license_type (AdapAboutWindow *self,
                                              GtkLicense      license_type);

ADAP_AVAILABLE_IN_1_2
const char *adap_about_window_get_license (AdapAboutWindow *self);
ADAP_AVAILABLE_IN_1_2
void        adap_about_window_set_license (AdapAboutWindow *self,
                                          const char     *license);

ADAP_AVAILABLE_IN_1_2
void adap_about_window_add_legal_section (AdapAboutWindow *self,
                                         const char     *title,
                                         const char     *copyright,
                                         GtkLicense      license_type,
                                         const char     *license);

ADAP_AVAILABLE_IN_1_2
void adap_show_about_window (GtkWindow  *parent,
                            const char *first_property_name,
                            ...) G_GNUC_NULL_TERMINATED;

ADAP_AVAILABLE_IN_1_4
void adap_show_about_window_from_appdata (GtkWindow  *parent,
                                         const char *resource_path,
                                         const char *release_notes_version,
                                         const char *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
