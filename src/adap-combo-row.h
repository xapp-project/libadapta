/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-action-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_COMBO_ROW (adw_combo_row_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwComboRow, adw_combo_row, ADW, COMBO_ROW, AdwActionRow)

/**
 * AdwComboRowClass
 * @parent_class: The parent class
 */
struct _AdwComboRowClass
{
  AdwActionRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_combo_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GListModel *adw_combo_row_get_model (AdwComboRow *self);
ADW_AVAILABLE_IN_ALL
void        adw_combo_row_set_model (AdwComboRow *self,
                                     GListModel  *model);

ADW_AVAILABLE_IN_ALL
guint adw_combo_row_get_selected (AdwComboRow *self);
ADW_AVAILABLE_IN_ALL
void  adw_combo_row_set_selected (AdwComboRow *self,
                                  guint        position);

ADW_AVAILABLE_IN_ALL
gpointer adw_combo_row_get_selected_item (AdwComboRow *self);

ADW_AVAILABLE_IN_ALL
GtkListItemFactory *adw_combo_row_get_factory (AdwComboRow        *self);
ADW_AVAILABLE_IN_ALL
void                adw_combo_row_set_factory (AdwComboRow        *self,
                                               GtkListItemFactory *factory);

ADW_AVAILABLE_IN_ALL
GtkListItemFactory *adw_combo_row_get_list_factory (AdwComboRow        *self);
ADW_AVAILABLE_IN_ALL
void                adw_combo_row_set_list_factory (AdwComboRow        *self,
                                                    GtkListItemFactory *factory);

ADW_AVAILABLE_IN_ALL
GtkExpression *adw_combo_row_get_expression (AdwComboRow   *self);
ADW_AVAILABLE_IN_ALL
void           adw_combo_row_set_expression (AdwComboRow   *self,
                                             GtkExpression *expression);

ADW_AVAILABLE_IN_ALL
gboolean adw_combo_row_get_use_subtitle (AdwComboRow *self);
ADW_AVAILABLE_IN_ALL
void     adw_combo_row_set_use_subtitle (AdwComboRow *self,
                                         gboolean     use_subtitle);

ADW_AVAILABLE_IN_1_4
gboolean adw_combo_row_get_enable_search (AdwComboRow *self);
ADW_AVAILABLE_IN_1_4
void     adw_combo_row_set_enable_search (AdwComboRow *self,
                                          gboolean     enable_search);

G_END_DECLS
