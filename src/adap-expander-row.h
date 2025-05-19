/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-preferences-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_EXPANDER_ROW (adw_expander_row_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwExpanderRow, adw_expander_row, ADW, EXPANDER_ROW, AdwPreferencesRow)

/**
 * AdwExpanderRowClass
 * @parent_class: The parent class
 */
struct _AdwExpanderRowClass
{
  AdwPreferencesRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_expander_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_4_FOR (adw_expander_row_add_suffix)
void adw_expander_row_add_action (AdwExpanderRow *self,
                                  GtkWidget      *widget);
ADW_AVAILABLE_IN_ALL
void adw_expander_row_add_prefix (AdwExpanderRow *self,
                                  GtkWidget      *widget);
ADW_AVAILABLE_IN_1_4
void adw_expander_row_add_suffix (AdwExpanderRow *self,
                                  GtkWidget      *widget);

ADW_AVAILABLE_IN_ALL
void adw_expander_row_add_row    (AdwExpanderRow *self,
                                  GtkWidget      *child);
ADW_AVAILABLE_IN_ALL
void adw_expander_row_remove (AdwExpanderRow *self,
                              GtkWidget      *child);

ADW_AVAILABLE_IN_ALL
const char *adw_expander_row_get_subtitle (AdwExpanderRow *self);
ADW_AVAILABLE_IN_ALL
void        adw_expander_row_set_subtitle (AdwExpanderRow *self,
                                           const char     *subtitle);

ADW_DEPRECATED_IN_1_3_FOR (adw_expander_row_add_prefix)
const char *adw_expander_row_get_icon_name (AdwExpanderRow *self);
ADW_DEPRECATED_IN_1_3_FOR (adw_expander_row_add_prefix)
void        adw_expander_row_set_icon_name (AdwExpanderRow *self,
                                            const char     *icon_name);

ADW_AVAILABLE_IN_ALL
gboolean adw_expander_row_get_expanded (AdwExpanderRow *self);
ADW_AVAILABLE_IN_ALL
void     adw_expander_row_set_expanded (AdwExpanderRow *self,
                                        gboolean        expanded);

ADW_AVAILABLE_IN_ALL
gboolean adw_expander_row_get_enable_expansion (AdwExpanderRow *self);
ADW_AVAILABLE_IN_ALL
void     adw_expander_row_set_enable_expansion (AdwExpanderRow *self,
                                                gboolean        enable_expansion);

ADW_AVAILABLE_IN_ALL
gboolean adw_expander_row_get_show_enable_switch (AdwExpanderRow *self);
ADW_AVAILABLE_IN_ALL
void     adw_expander_row_set_show_enable_switch (AdwExpanderRow *self,
                                                  gboolean        show_enable_switch);

ADW_AVAILABLE_IN_1_3
int  adw_expander_row_get_title_lines (AdwExpanderRow *self);
ADW_AVAILABLE_IN_1_3
void adw_expander_row_set_title_lines (AdwExpanderRow *self,
                                       int             title_lines);

ADW_AVAILABLE_IN_1_3
int  adw_expander_row_get_subtitle_lines (AdwExpanderRow *self);
ADW_AVAILABLE_IN_1_3
void adw_expander_row_set_subtitle_lines (AdwExpanderRow *self,
                                          int             subtitle_lines);

G_END_DECLS
