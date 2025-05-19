/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-breakpoint-bin.h"

G_BEGIN_DECLS

void adap_breakpoint_bin_set_warnings (AdapBreakpointBin *self,
                                      gboolean          min_size_warnings,
                                      gboolean          overflow_warnings);
void adap_breakpoint_bin_set_warning_widget (AdapBreakpointBin *self,
                                            GtkWidget        *warning_widget);

gboolean adap_breakpoint_bin_has_breakpoints (AdapBreakpointBin *self);

void adap_breakpoint_bin_set_pass_through (AdapBreakpointBin *self,
                                          gboolean          pass_through);

void adap_breakpoint_bin_set_natural_size (AdapBreakpointBin *self,
                                          int               width,
                                          int               height);

G_END_DECLS
