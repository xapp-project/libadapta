/*
 * Copyright (C) 2018 Purism SPC
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

#define ADW_TYPE_CLAMP (adw_clamp_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwClamp, adw_clamp, ADW, CLAMP, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_get_child (AdwClamp  *self);
ADW_AVAILABLE_IN_ALL
void       adw_clamp_set_child (AdwClamp  *self,
                                GtkWidget *child);

ADW_AVAILABLE_IN_ALL
int  adw_clamp_get_maximum_size (AdwClamp *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_set_maximum_size (AdwClamp *self,
                                 int       maximum_size);

ADW_AVAILABLE_IN_ALL
int  adw_clamp_get_tightening_threshold (AdwClamp *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_set_tightening_threshold (AdwClamp *self,
                                         int       tightening_threshold);

ADW_AVAILABLE_IN_1_4
AdwLengthUnit adw_clamp_get_unit (AdwClamp      *self);
ADW_AVAILABLE_IN_1_4
void          adw_clamp_set_unit (AdwClamp      *self,
                                  AdwLengthUnit  unit);

G_END_DECLS
