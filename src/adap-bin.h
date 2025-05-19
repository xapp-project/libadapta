/*
 * Copyright (C) 2021 Purism SPC
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

G_BEGIN_DECLS

#define ADAP_TYPE_BIN (adap_bin_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapBin, adap_bin, ADAP, BIN, GtkWidget)

struct _AdapBinClass
{
  GtkWidgetClass parent_class;
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_bin_get_child (AdapBin    *self);
ADAP_AVAILABLE_IN_ALL
void       adap_bin_set_child (AdapBin    *self,
                              GtkWidget *child);

G_END_DECLS
