/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-entry-row.h"

G_BEGIN_DECLS

void adap_entry_row_set_indicator_icon_name (AdapEntryRow *self,
                                            const char  *icon_name);
void adap_entry_row_set_indicator_tooltip   (AdapEntryRow *self,
                                            const char  *tooltip);
void adap_entry_row_set_show_indicator      (AdapEntryRow *self,
                                            gboolean     show_indicator);

G_END_DECLS
