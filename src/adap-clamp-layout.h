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

#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_CLAMP_LAYOUT (adw_clamp_layout_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwClampLayout, adw_clamp_layout, ADW, CLAMP_LAYOUT, GtkLayoutManager)

ADW_AVAILABLE_IN_ALL
GtkLayoutManager *adw_clamp_layout_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
int  adw_clamp_layout_get_maximum_size (AdwClampLayout *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_layout_set_maximum_size (AdwClampLayout *self,
                                        int             maximum_size);

ADW_AVAILABLE_IN_ALL
int  adw_clamp_layout_get_tightening_threshold (AdwClampLayout *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_layout_set_tightening_threshold (AdwClampLayout *self,
                                                int             tightening_threshold);

ADW_AVAILABLE_IN_1_4
AdwLengthUnit adw_clamp_layout_get_unit (AdwClampLayout *self);
ADW_AVAILABLE_IN_1_4
void          adw_clamp_layout_set_unit (AdwClampLayout *self,
                                         AdwLengthUnit   unit);

G_END_DECLS
