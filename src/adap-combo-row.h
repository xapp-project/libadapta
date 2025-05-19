/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-action-row.h"

G_BEGIN_DECLS

#define ADAP_TYPE_COMBO_ROW (adap_combo_row_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapComboRow, adap_combo_row, ADAP, COMBO_ROW, AdapActionRow)

/**
 * AdapComboRowClass
 * @parent_class: The parent class
 */
struct _AdapComboRowClass
{
  AdapActionRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_combo_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GListModel *adap_combo_row_get_model (AdapComboRow *self);
ADAP_AVAILABLE_IN_ALL
void        adap_combo_row_set_model (AdapComboRow *self,
                                     GListModel  *model);

ADAP_AVAILABLE_IN_ALL
guint adap_combo_row_get_selected (AdapComboRow *self);
ADAP_AVAILABLE_IN_ALL
void  adap_combo_row_set_selected (AdapComboRow *self,
                                  guint        position);

ADAP_AVAILABLE_IN_ALL
gpointer adap_combo_row_get_selected_item (AdapComboRow *self);

ADAP_AVAILABLE_IN_ALL
GtkListItemFactory *adap_combo_row_get_factory (AdapComboRow        *self);
ADAP_AVAILABLE_IN_ALL
void                adap_combo_row_set_factory (AdapComboRow        *self,
                                               GtkListItemFactory *factory);

ADAP_AVAILABLE_IN_ALL
GtkListItemFactory *adap_combo_row_get_list_factory (AdapComboRow        *self);
ADAP_AVAILABLE_IN_ALL
void                adap_combo_row_set_list_factory (AdapComboRow        *self,
                                                    GtkListItemFactory *factory);

ADAP_AVAILABLE_IN_ALL
GtkExpression *adap_combo_row_get_expression (AdapComboRow   *self);
ADAP_AVAILABLE_IN_ALL
void           adap_combo_row_set_expression (AdapComboRow   *self,
                                             GtkExpression *expression);

ADAP_AVAILABLE_IN_ALL
gboolean adap_combo_row_get_use_subtitle (AdapComboRow *self);
ADAP_AVAILABLE_IN_ALL
void     adap_combo_row_set_use_subtitle (AdapComboRow *self,
                                         gboolean     use_subtitle);

ADAP_AVAILABLE_IN_1_4
gboolean adap_combo_row_get_enable_search (AdapComboRow *self);
ADAP_AVAILABLE_IN_1_4
void     adap_combo_row_set_enable_search (AdapComboRow *self,
                                          gboolean     enable_search);

G_END_DECLS
