/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-breakpoint.h"

G_BEGIN_DECLS

void adap_breakpoint_transition (AdapBreakpoint *from,
                                AdapBreakpoint *to);

gboolean adap_breakpoint_check_condition (AdapBreakpoint *self,
                                         GtkSettings   *settings,
                                         int            width,
                                         int            height);

G_END_DECLS
