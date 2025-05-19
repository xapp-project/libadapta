/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-tab-overview.h"

#include "adw-tab-grid-private.h"

G_BEGIN_DECLS

AdwTabGrid *adw_tab_overview_get_tab_grid        (AdwTabOverview *self);
AdwTabGrid *adw_tab_overview_get_pinned_tab_grid (AdwTabOverview *self);

G_END_DECLS
