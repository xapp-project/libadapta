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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_INDICATOR_BIN (adap_indicator_bin_get_type())

G_DECLARE_FINAL_TYPE (AdapIndicatorBin, adap_indicator_bin, ADAP, INDICATOR_BIN, GtkWidget)

GtkWidget *adap_indicator_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adap_indicator_bin_get_child (AdapIndicatorBin *self);
void       adap_indicator_bin_set_child (AdapIndicatorBin *self,
                                        GtkWidget       *child);

gboolean adap_indicator_bin_get_needs_attention (AdapIndicatorBin *self);
void     adap_indicator_bin_set_needs_attention (AdapIndicatorBin *self,
                                                gboolean         needs_attention);

const char *adap_indicator_bin_get_badge (AdapIndicatorBin *self);
void        adap_indicator_bin_set_badge (AdapIndicatorBin *self,
                                         const char      *badge);

G_END_DECLS
