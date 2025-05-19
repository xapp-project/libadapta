/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-navigation-view.h"
#include "adap-preferences-page.h"
#include "adap-toast.h"
#include "adap-window.h"

G_BEGIN_DECLS

#define ADAP_TYPE_PREFERENCES_WINDOW (adap_preferences_window_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapPreferencesWindow, adap_preferences_window, ADAP, PREFERENCES_WINDOW, AdapWindow)

/**
 * AdapPreferencesWindowClass
 * @parent_class: The parent class
 */
struct _AdapPreferencesWindowClass
{
  AdapWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_preferences_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void adap_preferences_window_add    (AdapPreferencesWindow *self,
                                    AdapPreferencesPage   *page);
ADAP_AVAILABLE_IN_ALL
void adap_preferences_window_remove (AdapPreferencesWindow *self,
                                    AdapPreferencesPage   *page);

ADAP_AVAILABLE_IN_ALL
AdapPreferencesPage *adap_preferences_window_get_visible_page (AdapPreferencesWindow *self);
ADAP_AVAILABLE_IN_ALL
void                adap_preferences_window_set_visible_page (AdapPreferencesWindow *self,
                                                             AdapPreferencesPage   *page);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_window_get_visible_page_name (AdapPreferencesWindow *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_window_set_visible_page_name (AdapPreferencesWindow *self,
                                                          const char           *name);

ADAP_AVAILABLE_IN_ALL
gboolean adap_preferences_window_get_search_enabled (AdapPreferencesWindow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_preferences_window_set_search_enabled (AdapPreferencesWindow *self,
                                                    gboolean              search_enabled);

ADAP_DEPRECATED_IN_1_4_FOR (adap_navigation_page_get_can_pop)
gboolean adap_preferences_window_get_can_navigate_back (AdapPreferencesWindow *self);
ADAP_DEPRECATED_IN_1_4_FOR (adap_navigation_page_set_can_pop)
void     adap_preferences_window_set_can_navigate_back (AdapPreferencesWindow *self,
                                                       gboolean              can_navigate_back);

ADAP_DEPRECATED_IN_1_4_FOR (adap_preferences_window_push_subpage)
void adap_preferences_window_present_subpage (AdapPreferencesWindow *self,
                                             GtkWidget            *subpage);
ADAP_DEPRECATED_IN_1_4_FOR (adap_preferences_window_pop_subpage)
void adap_preferences_window_close_subpage   (AdapPreferencesWindow *self);

ADAP_AVAILABLE_IN_1_4
void     adap_preferences_window_push_subpage (AdapPreferencesWindow *self,
                                              AdapNavigationPage    *page);
ADAP_AVAILABLE_IN_1_4
gboolean adap_preferences_window_pop_subpage  (AdapPreferencesWindow *self);

ADAP_AVAILABLE_IN_ALL
void adap_preferences_window_add_toast (AdapPreferencesWindow *self,
                                       AdapToast             *toast);

G_END_DECLS
