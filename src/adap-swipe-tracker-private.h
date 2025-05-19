/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-swipe-tracker.h"

G_BEGIN_DECLS

#define ADAP_SWIPE_BORDER 32

void adap_swipe_tracker_reset (AdapSwipeTracker *self);

G_END_DECLS
