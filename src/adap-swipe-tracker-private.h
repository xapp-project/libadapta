/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-swipe-tracker.h"

G_BEGIN_DECLS

#define ADW_SWIPE_BORDER 32

void adw_swipe_tracker_reset (AdwSwipeTracker *self);

G_END_DECLS
