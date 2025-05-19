/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-preferences-group.h"

G_BEGIN_DECLS

#define ADW_TYPE_PREFERENCES_PAGE (adw_preferences_page_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwPreferencesPage, adw_preferences_page, ADW, PREFERENCES_PAGE, GtkWidget)

/**
 * AdwPreferencesPageClass
 * @parent_class: The parent class
 */
struct _AdwPreferencesPageClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_preferences_page_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void adw_preferences_page_add    (AdwPreferencesPage  *self,
                                  AdwPreferencesGroup *group);
ADW_AVAILABLE_IN_ALL
void adw_preferences_page_remove (AdwPreferencesPage  *self,
                                  AdwPreferencesGroup *group);

ADW_AVAILABLE_IN_ALL
const char *adw_preferences_page_get_icon_name (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_preferences_page_set_icon_name (AdwPreferencesPage *self,
                                                const char         *icon_name);

ADW_AVAILABLE_IN_ALL
const char *adw_preferences_page_get_title (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_preferences_page_set_title (AdwPreferencesPage *self,
                                            const char         *title);

ADW_AVAILABLE_IN_1_4
const char *adw_preferences_page_get_description (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_1_4
void        adw_preferences_page_set_description (AdwPreferencesPage *self,
                                                  const char         *description);

ADW_AVAILABLE_IN_ALL
const char *adw_preferences_page_get_name (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_preferences_page_set_name (AdwPreferencesPage *self,
                                           const char         *name);

ADW_AVAILABLE_IN_ALL
gboolean adw_preferences_page_get_use_underline (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void     adw_preferences_page_set_use_underline (AdwPreferencesPage *self,
                                                 gboolean            use_underline);

ADW_AVAILABLE_IN_1_3
void adw_preferences_page_scroll_to_top (AdwPreferencesPage *self);

G_END_DECLS
