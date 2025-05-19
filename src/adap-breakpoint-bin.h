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

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-breakpoint.h"

G_BEGIN_DECLS

#define ADAP_TYPE_BREAKPOINT_BIN (adap_breakpoint_bin_get_type())

ADAP_AVAILABLE_IN_1_4
G_DECLARE_DERIVABLE_TYPE (AdapBreakpointBin, adap_breakpoint_bin, ADAP, BREAKPOINT_BIN, GtkWidget)

struct _AdapBreakpointBinClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_breakpoint_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_breakpoint_bin_get_child (AdapBreakpointBin *self);
ADAP_AVAILABLE_IN_1_4
void       adap_breakpoint_bin_set_child (AdapBreakpointBin *self,
                                         GtkWidget        *child);

ADAP_AVAILABLE_IN_1_4
void adap_breakpoint_bin_add_breakpoint (AdapBreakpointBin *self,
                                        AdapBreakpoint    *breakpoint);

ADAP_AVAILABLE_IN_1_5
void adap_breakpoint_bin_remove_breakpoint (AdapBreakpointBin *self,
                                           AdapBreakpoint    *breakpoint);

ADAP_AVAILABLE_IN_1_4
AdapBreakpoint *adap_breakpoint_bin_get_current_breakpoint (AdapBreakpointBin *self);

G_END_DECLS
