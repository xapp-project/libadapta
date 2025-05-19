/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include "adap-preferences-row.h"

G_BEGIN_DECLS

#define ADAP_TYPE_ENTRY_ROW (adap_entry_row_get_type())

ADAP_AVAILABLE_IN_1_2
G_DECLARE_DERIVABLE_TYPE (AdapEntryRow, adap_entry_row, ADAP, ENTRY_ROW, AdapPreferencesRow)

/**
 * AdapEntryRowClass
 * @parent_class: The parent class
 */
struct _AdapEntryRowClass
{
  AdapPreferencesRowClass parent_class;
};

ADAP_AVAILABLE_IN_1_2
GtkWidget *adap_entry_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_2
void adap_entry_row_add_prefix (AdapEntryRow *self,
                               GtkWidget   *widget);
ADAP_AVAILABLE_IN_1_2
void adap_entry_row_add_suffix (AdapEntryRow *self,
                               GtkWidget   *widget);
ADAP_AVAILABLE_IN_1_2
void adap_entry_row_remove     (AdapEntryRow *self,
                               GtkWidget   *widget);

ADAP_AVAILABLE_IN_1_2
gboolean adap_entry_row_get_show_apply_button (AdapEntryRow *self);
ADAP_AVAILABLE_IN_1_2
void     adap_entry_row_set_show_apply_button (AdapEntryRow *self,
                                              gboolean     show_apply_button);

ADAP_AVAILABLE_IN_1_2
GtkInputHints adap_entry_row_get_input_hints (AdapEntryRow  *self);
ADAP_AVAILABLE_IN_1_2
void          adap_entry_row_set_input_hints (AdapEntryRow  *self,
                                             GtkInputHints hints);

ADAP_AVAILABLE_IN_1_2
GtkInputPurpose adap_entry_row_get_input_purpose (AdapEntryRow    *self);
ADAP_AVAILABLE_IN_1_2
void            adap_entry_row_set_input_purpose (AdapEntryRow    *self,
                                                 GtkInputPurpose purpose);

ADAP_AVAILABLE_IN_1_2
gboolean adap_entry_row_get_enable_emoji_completion (AdapEntryRow *self);
ADAP_AVAILABLE_IN_1_2
void     adap_entry_row_set_enable_emoji_completion (AdapEntryRow *self,
                                                    gboolean     enable_emoji_completion);

ADAP_AVAILABLE_IN_1_2
PangoAttrList *adap_entry_row_get_attributes (AdapEntryRow   *self);
ADAP_AVAILABLE_IN_1_2
void           adap_entry_row_set_attributes (AdapEntryRow   *self,
                                             PangoAttrList *attributes);

ADAP_AVAILABLE_IN_1_2
gboolean adap_entry_row_get_activates_default (AdapEntryRow *self);
ADAP_AVAILABLE_IN_1_2
void     adap_entry_row_set_activates_default (AdapEntryRow *self,
                                              gboolean     activates);

ADAP_AVAILABLE_IN_1_5
guint adap_entry_row_get_text_length (AdapEntryRow *self);

ADAP_AVAILABLE_IN_1_3
gboolean adap_entry_row_grab_focus_without_selecting (AdapEntryRow *self);

G_END_DECLS
