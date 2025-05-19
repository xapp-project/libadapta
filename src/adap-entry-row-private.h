/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-entry-row.h"

G_BEGIN_DECLS

void adw_entry_row_set_indicator_icon_name (AdwEntryRow *self,
                                            const char  *icon_name);
void adw_entry_row_set_indicator_tooltip   (AdwEntryRow *self,
                                            const char  *tooltip);
void adw_entry_row_set_show_indicator      (AdwEntryRow *self,
                                            gboolean     show_indicator);

G_END_DECLS
