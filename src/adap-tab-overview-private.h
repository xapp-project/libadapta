/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-tab-overview.h"

#include "adap-tab-grid-private.h"

G_BEGIN_DECLS

AdapTabGrid *adap_tab_overview_get_tab_grid        (AdapTabOverview *self);
AdapTabGrid *adap_tab_overview_get_pinned_tab_grid (AdapTabOverview *self);

G_END_DECLS
