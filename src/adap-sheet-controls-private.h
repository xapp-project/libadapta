/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_SHEET_CONTROLS (adap_sheet_controls_get_type ())

G_DECLARE_FINAL_TYPE (AdapSheetControls, adap_sheet_controls, ADAP, SHEET_CONTROLS, GtkWidget)

GtkWidget *adap_sheet_controls_new                    (GtkPackType side);

GtkPackType adap_sheet_controls_get_side (AdapSheetControls *self);
void        adap_sheet_controls_set_side (AdapSheetControls *self,
                                         GtkPackType       side);

const char *adap_sheet_controls_get_decoration_layout (AdapSheetControls *self);
void        adap_sheet_controls_set_decoration_layout (AdapSheetControls *self,
                                                      const char       *layout);

gboolean     adap_sheet_controls_get_empty (AdapSheetControls *self);

G_END_DECLS
