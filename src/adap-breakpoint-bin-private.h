/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-breakpoint-bin.h"

G_BEGIN_DECLS

void adw_breakpoint_bin_set_warnings (AdwBreakpointBin *self,
                                      gboolean          min_size_warnings,
                                      gboolean          overflow_warnings);
void adw_breakpoint_bin_set_warning_widget (AdwBreakpointBin *self,
                                            GtkWidget        *warning_widget);

gboolean adw_breakpoint_bin_has_breakpoints (AdwBreakpointBin *self);

void adw_breakpoint_bin_set_pass_through (AdwBreakpointBin *self,
                                          gboolean          pass_through);

void adw_breakpoint_bin_set_natural_size (AdwBreakpointBin *self,
                                          int               width,
                                          int               height);

G_END_DECLS
