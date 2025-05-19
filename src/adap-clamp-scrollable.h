/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_CLAMP_SCROLLABLE (adw_clamp_scrollable_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwClampScrollable, adw_clamp_scrollable, ADW, CLAMP_SCROLLABLE, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_scrollable_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_scrollable_get_child (AdwClampScrollable *self);
ADW_AVAILABLE_IN_ALL
void       adw_clamp_scrollable_set_child (AdwClampScrollable *self,
                                           GtkWidget          *child);

ADW_AVAILABLE_IN_ALL
int  adw_clamp_scrollable_get_maximum_size (AdwClampScrollable *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_scrollable_set_maximum_size (AdwClampScrollable *self,
                                            int                 maximum_size);

ADW_AVAILABLE_IN_ALL
int  adw_clamp_scrollable_get_tightening_threshold (AdwClampScrollable *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_scrollable_set_tightening_threshold (AdwClampScrollable *self,
                                                    int                 tightening_threshold);

ADW_AVAILABLE_IN_1_4
AdwLengthUnit adw_clamp_scrollable_get_unit (AdwClampScrollable *self);
ADW_AVAILABLE_IN_1_4
void          adw_clamp_scrollable_set_unit (AdwClampScrollable *self,
                                             AdwLengthUnit       unit);

G_END_DECLS
