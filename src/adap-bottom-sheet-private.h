/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_BOTTOM_SHEET (adap_bottom_sheet_get_type())

G_DECLARE_FINAL_TYPE (AdapBottomSheet, adap_bottom_sheet, ADAP, BOTTOM_SHEET, GtkWidget)

GtkWidget *adap_bottom_sheet_new (void);

GtkWidget *adap_bottom_sheet_get_child (AdapBottomSheet *self);
void       adap_bottom_sheet_set_child (AdapBottomSheet *self,
                                       GtkWidget      *child);

GtkWidget *adap_bottom_sheet_get_sheet (AdapBottomSheet *self);
void       adap_bottom_sheet_set_sheet (AdapBottomSheet *self,
                                       GtkWidget      *sheet);

gboolean adap_bottom_sheet_get_open (AdapBottomSheet *self);
void     adap_bottom_sheet_set_open (AdapBottomSheet *self,
                                    gboolean        open);

float adap_bottom_sheet_get_align (AdapBottomSheet *self);
void  adap_bottom_sheet_set_align (AdapBottomSheet *self,
                                  float           align);

gboolean adap_bottom_sheet_get_show_drag_handle (AdapBottomSheet *self);
void     adap_bottom_sheet_set_show_drag_handle (AdapBottomSheet *self,
                                                gboolean        show_drag_handle);

gboolean adap_bottom_sheet_get_modal (AdapBottomSheet *self);
void     adap_bottom_sheet_set_modal (AdapBottomSheet *self,
                                     gboolean        modal);

gboolean adap_bottom_sheet_get_can_close (AdapBottomSheet *self);
void     adap_bottom_sheet_set_can_close (AdapBottomSheet *self,
                                         gboolean        can_close);

void adap_bottom_sheet_set_min_natural_width (AdapBottomSheet *self,
                                             int             min_natural_width);

GtkWidget *adap_bottom_sheet_get_sheet_bin (AdapBottomSheet *self);

G_END_DECLS
