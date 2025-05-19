/*
 * Copyright 2022 Christopher Davis <christopherdavis@gnome.org>
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

#define ADAP_TYPE_SPIN_ROW (adap_spin_row_get_type ())

ADAP_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdapSpinRow, adap_spin_row, ADAP, SPIN_ROW, AdapActionRow)

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_spin_row_new            (GtkAdjustment *adjustment,
                                        double         climb_rate,
                                        guint          digits) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_spin_row_new_with_range (double         min,
                                        double         max,
                                        double         step) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_1_4
void       adap_spin_row_configure      (AdapSpinRow    *self,
                                        GtkAdjustment *adjustment,
                                        double         climb_rate,
                                        guint          digits);

ADAP_AVAILABLE_IN_1_4
GtkAdjustment *adap_spin_row_get_adjustment (AdapSpinRow    *self);
ADAP_AVAILABLE_IN_1_4
void           adap_spin_row_set_adjustment (AdapSpinRow    *self,
                                            GtkAdjustment *adjustment);

ADAP_AVAILABLE_IN_1_4
double adap_spin_row_get_climb_rate (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void   adap_spin_row_set_climb_rate (AdapSpinRow *self,
                                    double      climb_rate);

ADAP_AVAILABLE_IN_1_4
guint adap_spin_row_get_digits (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void  adap_spin_row_set_digits (AdapSpinRow *self,
                               guint       digits);

ADAP_AVAILABLE_IN_1_4
gboolean adap_spin_row_get_numeric (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void     adap_spin_row_set_numeric (AdapSpinRow *self,
                                   gboolean    numeric);

ADAP_AVAILABLE_IN_1_4
gboolean adap_spin_row_get_snap_to_ticks (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void     adap_spin_row_set_snap_to_ticks (AdapSpinRow *self,
                                         gboolean    snap_to_ticks);

ADAP_AVAILABLE_IN_1_4
GtkSpinButtonUpdatePolicy adap_spin_row_get_update_policy (AdapSpinRow                *self);
ADAP_AVAILABLE_IN_1_4
void                      adap_spin_row_set_update_policy (AdapSpinRow                *self,
                                                          GtkSpinButtonUpdatePolicy  policy);

ADAP_AVAILABLE_IN_1_4
double adap_spin_row_get_value (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void   adap_spin_row_set_value (AdapSpinRow *self,
                               double      value);

ADAP_AVAILABLE_IN_1_4
gboolean adap_spin_row_get_wrap (AdapSpinRow *self);
ADAP_AVAILABLE_IN_1_4
void     adap_spin_row_set_wrap (AdapSpinRow *self,
                                gboolean    wrap);

ADAP_AVAILABLE_IN_1_4
void adap_spin_row_update (AdapSpinRow *self);

ADAP_AVAILABLE_IN_1_4
void adap_spin_row_set_range (AdapSpinRow *self,
                             double      min,
                             double      max);

G_END_DECLS
