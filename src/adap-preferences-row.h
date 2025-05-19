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

G_BEGIN_DECLS

#define ADAP_TYPE_PREFERENCES_ROW (adap_preferences_row_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapPreferencesRow, adap_preferences_row, ADAP, PREFERENCES_ROW, GtkListBoxRow)

/**
 * AdapPreferencesRowClass
 * @parent_class: The parent class
 */
struct _AdapPreferencesRowClass
{
  GtkListBoxRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_preferences_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_row_get_title (AdapPreferencesRow *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_row_set_title (AdapPreferencesRow *self,
                                           const char        *title);

ADAP_AVAILABLE_IN_ALL
gboolean adap_preferences_row_get_use_underline (AdapPreferencesRow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_preferences_row_set_use_underline (AdapPreferencesRow *self,
                                                gboolean           use_underline);

ADAP_AVAILABLE_IN_1_1
gboolean adap_preferences_row_get_title_selectable (AdapPreferencesRow *self);
ADAP_AVAILABLE_IN_1_1
void     adap_preferences_row_set_title_selectable (AdapPreferencesRow *self,
                                                   gboolean           title_selectable);


ADAP_AVAILABLE_IN_1_2
gboolean adap_preferences_row_get_use_markup (AdapPreferencesRow *self);
ADAP_AVAILABLE_IN_1_2
void     adap_preferences_row_set_use_markup (AdapPreferencesRow *self,
                                             gboolean           use_markup);

G_END_DECLS
