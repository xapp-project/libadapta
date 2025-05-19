/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-breakpoint.h"

G_BEGIN_DECLS

void adw_breakpoint_transition (AdwBreakpoint *from,
                                AdwBreakpoint *to);

gboolean adw_breakpoint_check_condition (AdwBreakpoint *self,
                                         GtkSettings   *settings,
                                         int            width,
                                         int            height);

G_END_DECLS
