/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include "adw-preferences-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_ENTRY_ROW (adw_entry_row_get_type())

ADW_AVAILABLE_IN_1_2
G_DECLARE_DERIVABLE_TYPE (AdwEntryRow, adw_entry_row, ADW, ENTRY_ROW, AdwPreferencesRow)

/**
 * AdwEntryRowClass
 * @parent_class: The parent class
 */
struct _AdwEntryRowClass
{
  AdwPreferencesRowClass parent_class;
};

ADW_AVAILABLE_IN_1_2
GtkWidget *adw_entry_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_2
void adw_entry_row_add_prefix (AdwEntryRow *self,
                               GtkWidget   *widget);
ADW_AVAILABLE_IN_1_2
void adw_entry_row_add_suffix (AdwEntryRow *self,
                               GtkWidget   *widget);
ADW_AVAILABLE_IN_1_2
void adw_entry_row_remove     (AdwEntryRow *self,
                               GtkWidget   *widget);

ADW_AVAILABLE_IN_1_2
gboolean adw_entry_row_get_show_apply_button (AdwEntryRow *self);
ADW_AVAILABLE_IN_1_2
void     adw_entry_row_set_show_apply_button (AdwEntryRow *self,
                                              gboolean     show_apply_button);

ADW_AVAILABLE_IN_1_2
GtkInputHints adw_entry_row_get_input_hints (AdwEntryRow  *self);
ADW_AVAILABLE_IN_1_2
void          adw_entry_row_set_input_hints (AdwEntryRow  *self,
                                             GtkInputHints hints);

ADW_AVAILABLE_IN_1_2
GtkInputPurpose adw_entry_row_get_input_purpose (AdwEntryRow    *self);
ADW_AVAILABLE_IN_1_2
void            adw_entry_row_set_input_purpose (AdwEntryRow    *self,
                                                 GtkInputPurpose purpose);

ADW_AVAILABLE_IN_1_2
gboolean adw_entry_row_get_enable_emoji_completion (AdwEntryRow *self);
ADW_AVAILABLE_IN_1_2
void     adw_entry_row_set_enable_emoji_completion (AdwEntryRow *self,
                                                    gboolean     enable_emoji_completion);

ADW_AVAILABLE_IN_1_2
PangoAttrList *adw_entry_row_get_attributes (AdwEntryRow   *self);
ADW_AVAILABLE_IN_1_2
void           adw_entry_row_set_attributes (AdwEntryRow   *self,
                                             PangoAttrList *attributes);

ADW_AVAILABLE_IN_1_2
gboolean adw_entry_row_get_activates_default (AdwEntryRow *self);
ADW_AVAILABLE_IN_1_2
void     adw_entry_row_set_activates_default (AdwEntryRow *self,
                                              gboolean     activates);

ADW_AVAILABLE_IN_1_5
guint adw_entry_row_get_text_length (AdwEntryRow *self);

ADW_AVAILABLE_IN_1_3
gboolean adw_entry_row_grab_focus_without_selecting (AdwEntryRow *self);

G_END_DECLS
