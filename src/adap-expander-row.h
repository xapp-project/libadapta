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

#include <gtk/gtk.h>
#include "adap-preferences-row.h"

G_BEGIN_DECLS

#define ADAP_TYPE_EXPANDER_ROW (adap_expander_row_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapExpanderRow, adap_expander_row, ADAP, EXPANDER_ROW, AdapPreferencesRow)

/**
 * AdapExpanderRowClass
 * @parent_class: The parent class
 */
struct _AdapExpanderRowClass
{
  AdapPreferencesRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_expander_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_DEPRECATED_IN_1_4_FOR (adap_expander_row_add_suffix)
void adap_expander_row_add_action (AdapExpanderRow *self,
                                  GtkWidget      *widget);
ADAP_AVAILABLE_IN_ALL
void adap_expander_row_add_prefix (AdapExpanderRow *self,
                                  GtkWidget      *widget);
ADAP_AVAILABLE_IN_1_4
void adap_expander_row_add_suffix (AdapExpanderRow *self,
                                  GtkWidget      *widget);

ADAP_AVAILABLE_IN_ALL
void adap_expander_row_add_row    (AdapExpanderRow *self,
                                  GtkWidget      *child);
ADAP_AVAILABLE_IN_ALL
void adap_expander_row_remove (AdapExpanderRow *self,
                              GtkWidget      *child);

ADAP_AVAILABLE_IN_ALL
const char *adap_expander_row_get_subtitle (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_ALL
void        adap_expander_row_set_subtitle (AdapExpanderRow *self,
                                           const char     *subtitle);

ADAP_DEPRECATED_IN_1_3_FOR (adap_expander_row_add_prefix)
const char *adap_expander_row_get_icon_name (AdapExpanderRow *self);
ADAP_DEPRECATED_IN_1_3_FOR (adap_expander_row_add_prefix)
void        adap_expander_row_set_icon_name (AdapExpanderRow *self,
                                            const char     *icon_name);

ADAP_AVAILABLE_IN_ALL
gboolean adap_expander_row_get_expanded (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_expander_row_set_expanded (AdapExpanderRow *self,
                                        gboolean        expanded);

ADAP_AVAILABLE_IN_ALL
gboolean adap_expander_row_get_enable_expansion (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_expander_row_set_enable_expansion (AdapExpanderRow *self,
                                                gboolean        enable_expansion);

ADAP_AVAILABLE_IN_ALL
gboolean adap_expander_row_get_show_enable_switch (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_expander_row_set_show_enable_switch (AdapExpanderRow *self,
                                                  gboolean        show_enable_switch);

ADAP_AVAILABLE_IN_1_3
int  adap_expander_row_get_title_lines (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_1_3
void adap_expander_row_set_title_lines (AdapExpanderRow *self,
                                       int             title_lines);

ADAP_AVAILABLE_IN_1_3
int  adap_expander_row_get_subtitle_lines (AdapExpanderRow *self);
ADAP_AVAILABLE_IN_1_3
void adap_expander_row_set_subtitle_lines (AdapExpanderRow *self,
                                          int             subtitle_lines);

G_END_DECLS
