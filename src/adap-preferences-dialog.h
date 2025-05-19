/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2023 GNOME Foundation Inc
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
#include "adap-navigation-view.h"
#include "adap-preferences-page.h"
#include "adap-toast.h"

G_BEGIN_DECLS

#define ADAP_TYPE_PREFERENCES_DIALOG (adap_preferences_dialog_get_type())

ADAP_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdapPreferencesDialog, adap_preferences_dialog, ADAP, PREFERENCES_DIALOG, AdapDialog)

/**
 * AdapPreferencesDialogClass
 * @parent_class: The parent class
 */
struct _AdapPreferencesDialogClass
{
  AdapDialogClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_preferences_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_5
void adap_preferences_dialog_add    (AdapPreferencesDialog *self,
                                    AdapPreferencesPage   *page);
ADAP_AVAILABLE_IN_1_5
void adap_preferences_dialog_remove (AdapPreferencesDialog *self,
                                    AdapPreferencesPage   *page);

ADAP_AVAILABLE_IN_1_5
AdapPreferencesPage *adap_preferences_dialog_get_visible_page (AdapPreferencesDialog *self);
ADAP_AVAILABLE_IN_1_5
void                adap_preferences_dialog_set_visible_page (AdapPreferencesDialog *self,
                                                             AdapPreferencesPage   *page);

ADAP_AVAILABLE_IN_1_5
const char *adap_preferences_dialog_get_visible_page_name (AdapPreferencesDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_preferences_dialog_set_visible_page_name (AdapPreferencesDialog *self,
                                                          const char           *name);

ADAP_AVAILABLE_IN_1_5
gboolean adap_preferences_dialog_get_search_enabled (AdapPreferencesDialog *self);
ADAP_AVAILABLE_IN_1_5
void     adap_preferences_dialog_set_search_enabled (AdapPreferencesDialog *self,
                                                    gboolean              search_enabled);

ADAP_AVAILABLE_IN_1_5
void     adap_preferences_dialog_push_subpage (AdapPreferencesDialog *self,
                                              AdapNavigationPage    *page);
ADAP_AVAILABLE_IN_1_5
gboolean adap_preferences_dialog_pop_subpage  (AdapPreferencesDialog *self);

ADAP_AVAILABLE_IN_1_5
void adap_preferences_dialog_add_toast (AdapPreferencesDialog *self,
                                       AdapToast             *toast);

G_END_DECLS
