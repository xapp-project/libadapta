/*
 * Copyright (C) 2021-2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-dialog.h"

G_BEGIN_DECLS

#define ADW_TYPE_ABOUT_DIALOG (adw_about_dialog_get_type())

ADW_AVAILABLE_IN_1_5
G_DECLARE_FINAL_TYPE (AdwAboutDialog, adw_about_dialog, ADW, ABOUT_DIALOG, AdwDialog)

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_about_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_about_dialog_new_from_appdata (const char *resource_path,
                                              const char *release_notes_version) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_application_name (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_application_name (AdwAboutDialog *self,
                                                   const char     *application_name);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_application_icon (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_application_icon (AdwAboutDialog *self,
                                                   const char     *application_icon);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_developer_name (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_developer_name (AdwAboutDialog *self,
                                                 const char     *developer_name);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_version (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_version (AdwAboutDialog *self,
                                          const char     *version);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_release_notes_version (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_release_notes_version (AdwAboutDialog *self,
                                                        const char     *version);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_release_notes (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_release_notes (AdwAboutDialog *self,
                                                const char     *release_notes);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_comments (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_comments (AdwAboutDialog *self,
                                           const char     *comments);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_website (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_website (AdwAboutDialog *self,
                                          const char     *website);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_support_url (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_support_url (AdwAboutDialog *self,
                                              const char     *support_url);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_issue_url (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_issue_url (AdwAboutDialog *self,
                                            const char     *issue_url);

ADW_AVAILABLE_IN_1_5
void adw_about_dialog_add_link (AdwAboutDialog *self,
                                const char     *title,
                                const char     *url);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_debug_info (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_debug_info (AdwAboutDialog *self,
                                             const char     *debug_info);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_debug_info_filename (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_debug_info_filename (AdwAboutDialog *self,
                                                      const char     *filename);

ADW_AVAILABLE_IN_1_5
const char * const *adw_about_dialog_get_developers (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void                adw_about_dialog_set_developers (AdwAboutDialog  *self,
                                                     const char     **developers);

ADW_AVAILABLE_IN_1_5
const char * const *adw_about_dialog_get_designers (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void                adw_about_dialog_set_designers (AdwAboutDialog  *self,
                                                    const char     **designers);

ADW_AVAILABLE_IN_1_5
const char * const *adw_about_dialog_get_artists (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void                adw_about_dialog_set_artists (AdwAboutDialog  *self,
                                                  const char     **artists);

ADW_AVAILABLE_IN_1_5
const char * const *adw_about_dialog_get_documenters (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void                adw_about_dialog_set_documenters (AdwAboutDialog  *self,
                                                      const char     **documenters);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_translator_credits (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_translator_credits (AdwAboutDialog *self,
                                                     const char     *translator_credits);

ADW_AVAILABLE_IN_1_5
void adw_about_dialog_add_credit_section (AdwAboutDialog  *self,
                                          const char      *name,
                                          const char     **people);

ADW_AVAILABLE_IN_1_5
void adw_about_dialog_add_acknowledgement_section (AdwAboutDialog  *self,
                                                   const char      *name,
                                                   const char     **people);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_copyright (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_copyright (AdwAboutDialog *self,
                                            const char     *copyright);

ADW_AVAILABLE_IN_1_5
GtkLicense adw_about_dialog_get_license_type (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void       adw_about_dialog_set_license_type (AdwAboutDialog *self,
                                              GtkLicense      license_type);

ADW_AVAILABLE_IN_1_5
const char *adw_about_dialog_get_license (AdwAboutDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_about_dialog_set_license (AdwAboutDialog *self,
                                          const char     *license);

ADW_AVAILABLE_IN_1_5
void adw_about_dialog_add_legal_section (AdwAboutDialog *self,
                                         const char     *title,
                                         const char     *copyright,
                                         GtkLicense      license_type,
                                         const char     *license);

ADW_AVAILABLE_IN_1_5
void adw_show_about_dialog (GtkWidget  *parent,
                            const char *first_property_name,
                            ...) G_GNUC_NULL_TERMINATED;

ADW_AVAILABLE_IN_1_5
void adw_show_about_dialog_from_appdata (GtkWidget  *parent,
                                         const char *resource_path,
                                         const char *release_notes_version,
                                         const char *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
