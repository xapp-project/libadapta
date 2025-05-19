/*
 * Copyright (C) 2018-2020 Purism SPC
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

#define ADAP_TYPE_CLAMP_LAYOUT (adap_clamp_layout_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapClampLayout, adap_clamp_layout, ADAP, CLAMP_LAYOUT, GtkLayoutManager)

ADAP_AVAILABLE_IN_ALL
GtkLayoutManager *adap_clamp_layout_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_layout_get_maximum_size (AdapClampLayout *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_layout_set_maximum_size (AdapClampLayout *self,
                                        int             maximum_size);

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_layout_get_tightening_threshold (AdapClampLayout *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_layout_set_tightening_threshold (AdapClampLayout *self,
                                                int             tightening_threshold);

ADAP_AVAILABLE_IN_1_4
AdapLengthUnit adap_clamp_layout_get_unit (AdapClampLayout *self);
ADAP_AVAILABLE_IN_1_4
void          adap_clamp_layout_set_unit (AdapClampLayout *self,
                                         AdapLengthUnit   unit);

G_END_DECLS
