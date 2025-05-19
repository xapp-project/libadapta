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

G_BEGIN_DECLS

#define ADW_TYPE_PREFERENCES_ROW (adw_preferences_row_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwPreferencesRow, adw_preferences_row, ADW, PREFERENCES_ROW, GtkListBoxRow)

/**
 * AdwPreferencesRowClass
 * @parent_class: The parent class
 */
struct _AdwPreferencesRowClass
{
  GtkListBoxRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_preferences_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_preferences_row_get_title (AdwPreferencesRow *self);
ADW_AVAILABLE_IN_ALL
void        adw_preferences_row_set_title (AdwPreferencesRow *self,
                                           const char        *title);

ADW_AVAILABLE_IN_ALL
gboolean adw_preferences_row_get_use_underline (AdwPreferencesRow *self);
ADW_AVAILABLE_IN_ALL
void     adw_preferences_row_set_use_underline (AdwPreferencesRow *self,
                                                gboolean           use_underline);

ADW_AVAILABLE_IN_1_1
gboolean adw_preferences_row_get_title_selectable (AdwPreferencesRow *self);
ADW_AVAILABLE_IN_1_1
void     adw_preferences_row_set_title_selectable (AdwPreferencesRow *self,
                                                   gboolean           title_selectable);


ADW_AVAILABLE_IN_1_2
gboolean adw_preferences_row_get_use_markup (AdwPreferencesRow *self);
ADW_AVAILABLE_IN_1_2
void     adw_preferences_row_set_use_markup (AdwPreferencesRow *self,
                                             gboolean           use_markup);

G_END_DECLS
