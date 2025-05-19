/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_SHEET_CONTROLS (adw_sheet_controls_get_type ())

G_DECLARE_FINAL_TYPE (AdwSheetControls, adw_sheet_controls, ADW, SHEET_CONTROLS, GtkWidget)

GtkWidget *adw_sheet_controls_new                    (GtkPackType side);

GtkPackType adw_sheet_controls_get_side (AdwSheetControls *self);
void        adw_sheet_controls_set_side (AdwSheetControls *self,
                                         GtkPackType       side);

const char *adw_sheet_controls_get_decoration_layout (AdwSheetControls *self);
void        adw_sheet_controls_set_decoration_layout (AdwSheetControls *self,
                                                      const char       *layout);

gboolean     adw_sheet_controls_get_empty (AdwSheetControls *self);

G_END_DECLS
