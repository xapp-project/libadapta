/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2023 GNOME Foundation Inc
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
#include "adw-navigation-view.h"
#include "adw-preferences-page.h"
#include "adw-toast.h"

G_BEGIN_DECLS

#define ADW_TYPE_PREFERENCES_DIALOG (adw_preferences_dialog_get_type())

ADW_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdwPreferencesDialog, adw_preferences_dialog, ADW, PREFERENCES_DIALOG, AdwDialog)

/**
 * AdwPreferencesDialogClass
 * @parent_class: The parent class
 */
struct _AdwPreferencesDialogClass
{
  AdwDialogClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_preferences_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_5
void adw_preferences_dialog_add    (AdwPreferencesDialog *self,
                                    AdwPreferencesPage   *page);
ADW_AVAILABLE_IN_1_5
void adw_preferences_dialog_remove (AdwPreferencesDialog *self,
                                    AdwPreferencesPage   *page);

ADW_AVAILABLE_IN_1_5
AdwPreferencesPage *adw_preferences_dialog_get_visible_page (AdwPreferencesDialog *self);
ADW_AVAILABLE_IN_1_5
void                adw_preferences_dialog_set_visible_page (AdwPreferencesDialog *self,
                                                             AdwPreferencesPage   *page);

ADW_AVAILABLE_IN_1_5
const char *adw_preferences_dialog_get_visible_page_name (AdwPreferencesDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_preferences_dialog_set_visible_page_name (AdwPreferencesDialog *self,
                                                          const char           *name);

ADW_AVAILABLE_IN_1_5
gboolean adw_preferences_dialog_get_search_enabled (AdwPreferencesDialog *self);
ADW_AVAILABLE_IN_1_5
void     adw_preferences_dialog_set_search_enabled (AdwPreferencesDialog *self,
                                                    gboolean              search_enabled);

ADW_AVAILABLE_IN_1_5
void     adw_preferences_dialog_push_subpage (AdwPreferencesDialog *self,
                                              AdwNavigationPage    *page);
ADW_AVAILABLE_IN_1_5
gboolean adw_preferences_dialog_pop_subpage  (AdwPreferencesDialog *self);

ADW_AVAILABLE_IN_1_5
void adw_preferences_dialog_add_toast (AdwPreferencesDialog *self,
                                       AdwToast             *toast);

G_END_DECLS
