/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-window.h"

G_BEGIN_DECLS

#define ADW_TYPE_ABOUT_WINDOW (adw_about_window_get_type())

ADW_AVAILABLE_IN_1_2
G_DECLARE_FINAL_TYPE (AdwAboutWindow, adw_about_window, ADW, ABOUT_WINDOW, AdwWindow)

ADW_AVAILABLE_IN_1_2
GtkWidget *adw_about_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_about_window_new_from_appdata (const char *resource_path,
                                              const char *release_notes_version) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_application_name (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_application_name (AdwAboutWindow *self,
                                                   const char     *application_name);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_application_icon (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_application_icon (AdwAboutWindow *self,
                                                   const char     *application_icon);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_developer_name (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_developer_name (AdwAboutWindow *self,
                                                 const char     *developer_name);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_version (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_version (AdwAboutWindow *self,
                                          const char     *version);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_release_notes_version (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_release_notes_version (AdwAboutWindow *self,
                                                        const char     *version);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_release_notes (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_release_notes (AdwAboutWindow *self,
                                                const char     *release_notes);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_comments (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_comments (AdwAboutWindow *self,
                                           const char     *comments);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_website (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_website (AdwAboutWindow *self,
                                          const char     *website);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_support_url (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_support_url (AdwAboutWindow *self,
                                              const char     *support_url);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_issue_url (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_issue_url (AdwAboutWindow *self,
                                            const char     *issue_url);

ADW_AVAILABLE_IN_1_2
void adw_about_window_add_link (AdwAboutWindow *self,
                                const char     *title,
                                const char     *url);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_debug_info (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_debug_info (AdwAboutWindow *self,
                                             const char     *debug_info);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_debug_info_filename (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_debug_info_filename (AdwAboutWindow *self,
                                                      const char     *filename);

ADW_AVAILABLE_IN_1_2
const char * const *adw_about_window_get_developers (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void                adw_about_window_set_developers (AdwAboutWindow  *self,
                                                     const char     **developers);

ADW_AVAILABLE_IN_1_2
const char * const *adw_about_window_get_designers (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void                adw_about_window_set_designers (AdwAboutWindow  *self,
                                                    const char     **designers);

ADW_AVAILABLE_IN_1_2
const char * const *adw_about_window_get_artists (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void                adw_about_window_set_artists (AdwAboutWindow  *self,
                                                  const char     **artists);

ADW_AVAILABLE_IN_1_2
const char * const *adw_about_window_get_documenters (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void                adw_about_window_set_documenters (AdwAboutWindow  *self,
                                                      const char     **documenters);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_translator_credits (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_translator_credits (AdwAboutWindow *self,
                                                     const char     *translator_credits);

ADW_AVAILABLE_IN_1_2
void adw_about_window_add_credit_section (AdwAboutWindow  *self,
                                          const char      *name,
                                          const char     **people);

ADW_AVAILABLE_IN_1_2
void adw_about_window_add_acknowledgement_section (AdwAboutWindow  *self,
                                                   const char      *name,
                                                   const char     **people);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_copyright (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_copyright (AdwAboutWindow *self,
                                            const char     *copyright);

ADW_AVAILABLE_IN_1_2
GtkLicense adw_about_window_get_license_type (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void       adw_about_window_set_license_type (AdwAboutWindow *self,
                                              GtkLicense      license_type);

ADW_AVAILABLE_IN_1_2
const char *adw_about_window_get_license (AdwAboutWindow *self);
ADW_AVAILABLE_IN_1_2
void        adw_about_window_set_license (AdwAboutWindow *self,
                                          const char     *license);

ADW_AVAILABLE_IN_1_2
void adw_about_window_add_legal_section (AdwAboutWindow *self,
                                         const char     *title,
                                         const char     *copyright,
                                         GtkLicense      license_type,
                                         const char     *license);

ADW_AVAILABLE_IN_1_2
void adw_show_about_window (GtkWindow  *parent,
                            const char *first_property_name,
                            ...) G_GNUC_NULL_TERMINATED;

ADW_AVAILABLE_IN_1_4
void adw_show_about_window_from_appdata (GtkWindow  *parent,
                                         const char *resource_path,
                                         const char *release_notes_version,
                                         const char *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
