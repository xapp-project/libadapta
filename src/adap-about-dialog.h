/*
 * Copyright (C) 2021-2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-dialog.h"

G_BEGIN_DECLS

#define ADAP_TYPE_ABOUT_DIALOG (adap_about_dialog_get_type())

ADAP_AVAILABLE_IN_1_5
G_DECLARE_FINAL_TYPE (AdapAboutDialog, adap_about_dialog, ADAP, ABOUT_DIALOG, AdapDialog)

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_about_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_about_dialog_new_from_appdata (const char *resource_path,
                                              const char *release_notes_version) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_application_name (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_application_name (AdapAboutDialog *self,
                                                   const char     *application_name);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_application_icon (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_application_icon (AdapAboutDialog *self,
                                                   const char     *application_icon);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_developer_name (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_developer_name (AdapAboutDialog *self,
                                                 const char     *developer_name);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_version (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_version (AdapAboutDialog *self,
                                          const char     *version);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_release_notes_version (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_release_notes_version (AdapAboutDialog *self,
                                                        const char     *version);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_release_notes (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_release_notes (AdapAboutDialog *self,
                                                const char     *release_notes);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_comments (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_comments (AdapAboutDialog *self,
                                           const char     *comments);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_website (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_website (AdapAboutDialog *self,
                                          const char     *website);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_support_url (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_support_url (AdapAboutDialog *self,
                                              const char     *support_url);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_issue_url (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_issue_url (AdapAboutDialog *self,
                                            const char     *issue_url);

ADAP_AVAILABLE_IN_1_5
void adap_about_dialog_add_link (AdapAboutDialog *self,
                                const char     *title,
                                const char     *url);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_debug_info (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_debug_info (AdapAboutDialog *self,
                                             const char     *debug_info);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_debug_info_filename (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_debug_info_filename (AdapAboutDialog *self,
                                                      const char     *filename);

ADAP_AVAILABLE_IN_1_5
const char * const *adap_about_dialog_get_developers (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void                adap_about_dialog_set_developers (AdapAboutDialog  *self,
                                                     const char     **developers);

ADAP_AVAILABLE_IN_1_5
const char * const *adap_about_dialog_get_designers (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void                adap_about_dialog_set_designers (AdapAboutDialog  *self,
                                                    const char     **designers);

ADAP_AVAILABLE_IN_1_5
const char * const *adap_about_dialog_get_artists (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void                adap_about_dialog_set_artists (AdapAboutDialog  *self,
                                                  const char     **artists);

ADAP_AVAILABLE_IN_1_5
const char * const *adap_about_dialog_get_documenters (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void                adap_about_dialog_set_documenters (AdapAboutDialog  *self,
                                                      const char     **documenters);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_translator_credits (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_translator_credits (AdapAboutDialog *self,
                                                     const char     *translator_credits);

ADAP_AVAILABLE_IN_1_5
void adap_about_dialog_add_credit_section (AdapAboutDialog  *self,
                                          const char      *name,
                                          const char     **people);

ADAP_AVAILABLE_IN_1_5
void adap_about_dialog_add_acknowledgement_section (AdapAboutDialog  *self,
                                                   const char      *name,
                                                   const char     **people);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_copyright (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_copyright (AdapAboutDialog *self,
                                            const char     *copyright);

ADAP_AVAILABLE_IN_1_5
GtkLicense adap_about_dialog_get_license_type (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void       adap_about_dialog_set_license_type (AdapAboutDialog *self,
                                              GtkLicense      license_type);

ADAP_AVAILABLE_IN_1_5
const char *adap_about_dialog_get_license (AdapAboutDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_about_dialog_set_license (AdapAboutDialog *self,
                                          const char     *license);

ADAP_AVAILABLE_IN_1_5
void adap_about_dialog_add_legal_section (AdapAboutDialog *self,
                                         const char     *title,
                                         const char     *copyright,
                                         GtkLicense      license_type,
                                         const char     *license);

ADAP_AVAILABLE_IN_1_5
void adap_show_about_dialog (GtkWidget  *parent,
                            const char *first_property_name,
                            ...) G_GNUC_NULL_TERMINATED;

ADAP_AVAILABLE_IN_1_5
void adap_show_about_dialog_from_appdata (GtkWidget  *parent,
                                         const char *resource_path,
                                         const char *release_notes_version,
                                         const char *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
