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

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-breakpoint.h"

G_BEGIN_DECLS

#define ADW_TYPE_BREAKPOINT_BIN (adw_breakpoint_bin_get_type())

ADW_AVAILABLE_IN_1_4
G_DECLARE_DERIVABLE_TYPE (AdwBreakpointBin, adw_breakpoint_bin, ADW, BREAKPOINT_BIN, GtkWidget)

struct _AdwBreakpointBinClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_breakpoint_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_breakpoint_bin_get_child (AdwBreakpointBin *self);
ADW_AVAILABLE_IN_1_4
void       adw_breakpoint_bin_set_child (AdwBreakpointBin *self,
                                         GtkWidget        *child);

ADW_AVAILABLE_IN_1_4
void adw_breakpoint_bin_add_breakpoint (AdwBreakpointBin *self,
                                        AdwBreakpoint    *breakpoint);

ADW_AVAILABLE_IN_1_5
void adw_breakpoint_bin_remove_breakpoint (AdwBreakpointBin *self,
                                           AdwBreakpoint    *breakpoint);

ADW_AVAILABLE_IN_1_4
AdwBreakpoint *adw_breakpoint_bin_get_current_breakpoint (AdwBreakpointBin *self);

G_END_DECLS
