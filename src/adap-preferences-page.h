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
#include "adap-preferences-group.h"

G_BEGIN_DECLS

#define ADAP_TYPE_PREFERENCES_PAGE (adap_preferences_page_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapPreferencesPage, adap_preferences_page, ADAP, PREFERENCES_PAGE, GtkWidget)

/**
 * AdapPreferencesPageClass
 * @parent_class: The parent class
 */
struct _AdapPreferencesPageClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_preferences_page_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void adap_preferences_page_add    (AdapPreferencesPage  *self,
                                  AdapPreferencesGroup *group);
ADAP_AVAILABLE_IN_ALL
void adap_preferences_page_remove (AdapPreferencesPage  *self,
                                  AdapPreferencesGroup *group);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_page_get_icon_name (AdapPreferencesPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_page_set_icon_name (AdapPreferencesPage *self,
                                                const char         *icon_name);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_page_get_title (AdapPreferencesPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_page_set_title (AdapPreferencesPage *self,
                                            const char         *title);

ADAP_AVAILABLE_IN_1_4
const char *adap_preferences_page_get_description (AdapPreferencesPage *self);
ADAP_AVAILABLE_IN_1_4
void        adap_preferences_page_set_description (AdapPreferencesPage *self,
                                                  const char         *description);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_page_get_name (AdapPreferencesPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_page_set_name (AdapPreferencesPage *self,
                                           const char         *name);

ADAP_AVAILABLE_IN_ALL
gboolean adap_preferences_page_get_use_underline (AdapPreferencesPage *self);
ADAP_AVAILABLE_IN_ALL
void     adap_preferences_page_set_use_underline (AdapPreferencesPage *self,
                                                 gboolean            use_underline);

ADAP_AVAILABLE_IN_1_3
void adap_preferences_page_scroll_to_top (AdapPreferencesPage *self);

G_END_DECLS
