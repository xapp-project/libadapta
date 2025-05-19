/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_BOTTOM_SHEET (adw_bottom_sheet_get_type())

G_DECLARE_FINAL_TYPE (AdwBottomSheet, adw_bottom_sheet, ADW, BOTTOM_SHEET, GtkWidget)

GtkWidget *adw_bottom_sheet_new (void);

GtkWidget *adw_bottom_sheet_get_child (AdwBottomSheet *self);
void       adw_bottom_sheet_set_child (AdwBottomSheet *self,
                                       GtkWidget      *child);

GtkWidget *adw_bottom_sheet_get_sheet (AdwBottomSheet *self);
void       adw_bottom_sheet_set_sheet (AdwBottomSheet *self,
                                       GtkWidget      *sheet);

gboolean adw_bottom_sheet_get_open (AdwBottomSheet *self);
void     adw_bottom_sheet_set_open (AdwBottomSheet *self,
                                    gboolean        open);

float adw_bottom_sheet_get_align (AdwBottomSheet *self);
void  adw_bottom_sheet_set_align (AdwBottomSheet *self,
                                  float           align);

gboolean adw_bottom_sheet_get_show_drag_handle (AdwBottomSheet *self);
void     adw_bottom_sheet_set_show_drag_handle (AdwBottomSheet *self,
                                                gboolean        show_drag_handle);

gboolean adw_bottom_sheet_get_modal (AdwBottomSheet *self);
void     adw_bottom_sheet_set_modal (AdwBottomSheet *self,
                                     gboolean        modal);

gboolean adw_bottom_sheet_get_can_close (AdwBottomSheet *self);
void     adw_bottom_sheet_set_can_close (AdwBottomSheet *self,
                                         gboolean        can_close);

void adw_bottom_sheet_set_min_natural_width (AdwBottomSheet *self,
                                             int             min_natural_width);

GtkWidget *adw_bottom_sheet_get_sheet_bin (AdwBottomSheet *self);

G_END_DECLS
