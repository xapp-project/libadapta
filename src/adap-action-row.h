/*
 * Copyright (C) 2018 Purism SPC
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

#define ADAP_TYPE_ACTION_ROW (adap_action_row_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapActionRow, adap_action_row, ADAP, ACTION_ROW, AdapPreferencesRow)

/**
 * AdapActionRowClass
 * @parent_class: The parent class
 * @activate: Activates the row to trigger its main action.
 */
struct _AdapActionRowClass
{
  AdapPreferencesRowClass parent_class;

  void (*activate) (AdapActionRow *self);

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_action_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void adap_action_row_add_prefix (AdapActionRow *self,
                                GtkWidget    *widget);
ADAP_AVAILABLE_IN_ALL
void adap_action_row_add_suffix (AdapActionRow *self,
                                GtkWidget    *widget);
ADAP_AVAILABLE_IN_ALL
void adap_action_row_remove     (AdapActionRow *self,
                                GtkWidget    *widget);

ADAP_AVAILABLE_IN_ALL
const char  *adap_action_row_get_subtitle (AdapActionRow *self);
ADAP_AVAILABLE_IN_ALL
void         adap_action_row_set_subtitle (AdapActionRow *self,
                                          const char   *subtitle);

ADAP_DEPRECATED_IN_1_3_FOR (adap_action_row_add_prefix)
const char  *adap_action_row_get_icon_name (AdapActionRow *self);
ADAP_DEPRECATED_IN_1_3_FOR (adap_action_row_add_prefix)
void         adap_action_row_set_icon_name (AdapActionRow *self,
                                           const char   *icon_name);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_action_row_get_activatable_widget (AdapActionRow *self);
ADAP_AVAILABLE_IN_ALL
void       adap_action_row_set_activatable_widget (AdapActionRow *self,
                                                  GtkWidget    *widget);

ADAP_AVAILABLE_IN_ALL
int  adap_action_row_get_title_lines (AdapActionRow *self);
ADAP_AVAILABLE_IN_ALL
void adap_action_row_set_title_lines (AdapActionRow *self,
                                     int           title_lines);

ADAP_AVAILABLE_IN_ALL
int  adap_action_row_get_subtitle_lines (AdapActionRow *self);
ADAP_AVAILABLE_IN_ALL
void adap_action_row_set_subtitle_lines (AdapActionRow *self,
                                        int           subtitle_lines);
ADAP_AVAILABLE_IN_1_3
gboolean
adap_action_row_get_subtitle_selectable (AdapActionRow *self);
ADAP_AVAILABLE_IN_1_3
void
adap_action_row_set_subtitle_selectable (AdapActionRow *self,
                                        gboolean      subtitle_selectable);

ADAP_AVAILABLE_IN_ALL
void adap_action_row_activate (AdapActionRow *self);

G_END_DECLS
