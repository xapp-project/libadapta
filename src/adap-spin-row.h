/*
 * Copyright 2022 Christopher Davis <christopherdavis@gnome.org>
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

#define ADW_TYPE_SPIN_ROW (adw_spin_row_get_type ())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwSpinRow, adw_spin_row, ADW, SPIN_ROW, AdwActionRow)

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_spin_row_new            (GtkAdjustment *adjustment,
                                        double         climb_rate,
                                        guint          digits) G_GNUC_WARN_UNUSED_RESULT;
ADW_AVAILABLE_IN_1_4
GtkWidget *adw_spin_row_new_with_range (double         min,
                                        double         max,
                                        double         step) G_GNUC_WARN_UNUSED_RESULT;
ADW_AVAILABLE_IN_1_4
void       adw_spin_row_configure      (AdwSpinRow    *self,
                                        GtkAdjustment *adjustment,
                                        double         climb_rate,
                                        guint          digits);

ADW_AVAILABLE_IN_1_4
GtkAdjustment *adw_spin_row_get_adjustment (AdwSpinRow    *self);
ADW_AVAILABLE_IN_1_4
void           adw_spin_row_set_adjustment (AdwSpinRow    *self,
                                            GtkAdjustment *adjustment);

ADW_AVAILABLE_IN_1_4
double adw_spin_row_get_climb_rate (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void   adw_spin_row_set_climb_rate (AdwSpinRow *self,
                                    double      climb_rate);

ADW_AVAILABLE_IN_1_4
guint adw_spin_row_get_digits (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void  adw_spin_row_set_digits (AdwSpinRow *self,
                               guint       digits);

ADW_AVAILABLE_IN_1_4
gboolean adw_spin_row_get_numeric (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void     adw_spin_row_set_numeric (AdwSpinRow *self,
                                   gboolean    numeric);

ADW_AVAILABLE_IN_1_4
gboolean adw_spin_row_get_snap_to_ticks (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void     adw_spin_row_set_snap_to_ticks (AdwSpinRow *self,
                                         gboolean    snap_to_ticks);

ADW_AVAILABLE_IN_1_4
GtkSpinButtonUpdatePolicy adw_spin_row_get_update_policy (AdwSpinRow                *self);
ADW_AVAILABLE_IN_1_4
void                      adw_spin_row_set_update_policy (AdwSpinRow                *self,
                                                          GtkSpinButtonUpdatePolicy  policy);

ADW_AVAILABLE_IN_1_4
double adw_spin_row_get_value (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void   adw_spin_row_set_value (AdwSpinRow *self,
                               double      value);

ADW_AVAILABLE_IN_1_4
gboolean adw_spin_row_get_wrap (AdwSpinRow *self);
ADW_AVAILABLE_IN_1_4
void     adw_spin_row_set_wrap (AdwSpinRow *self,
                                gboolean    wrap);

ADW_AVAILABLE_IN_1_4
void adw_spin_row_update (AdwSpinRow *self);

ADW_AVAILABLE_IN_1_4
void adw_spin_row_set_range (AdwSpinRow *self,
                             double      min,
                             double      max);

G_END_DECLS
