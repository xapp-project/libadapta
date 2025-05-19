/*
 * Copyright (C) 2021 Purism SPC
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

G_BEGIN_DECLS

#define ADW_TYPE_BIN (adw_bin_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwBin, adw_bin, ADW, BIN, GtkWidget)

struct _AdwBinClass
{
  GtkWidgetClass parent_class;
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_bin_get_child (AdwBin    *self);
ADW_AVAILABLE_IN_ALL
void       adw_bin_set_child (AdwBin    *self,
                              GtkWidget *child);

G_END_DECLS
