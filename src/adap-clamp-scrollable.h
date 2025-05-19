/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-length-unit.h"

G_BEGIN_DECLS

#define ADAP_TYPE_CLAMP_SCROLLABLE (adap_clamp_scrollable_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapClampScrollable, adap_clamp_scrollable, ADAP, CLAMP_SCROLLABLE, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_clamp_scrollable_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_clamp_scrollable_get_child (AdapClampScrollable *self);
ADAP_AVAILABLE_IN_ALL
void       adap_clamp_scrollable_set_child (AdapClampScrollable *self,
                                           GtkWidget          *child);

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_scrollable_get_maximum_size (AdapClampScrollable *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_scrollable_set_maximum_size (AdapClampScrollable *self,
                                            int                 maximum_size);

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_scrollable_get_tightening_threshold (AdapClampScrollable *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_scrollable_set_tightening_threshold (AdapClampScrollable *self,
                                                    int                 tightening_threshold);

ADAP_AVAILABLE_IN_1_4
AdapLengthUnit adap_clamp_scrollable_get_unit (AdapClampScrollable *self);
ADAP_AVAILABLE_IN_1_4
void          adap_clamp_scrollable_set_unit (AdapClampScrollable *self,
                                             AdapLengthUnit       unit);

G_END_DECLS
