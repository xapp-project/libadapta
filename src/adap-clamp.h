/*
 * Copyright (C) 2018 Purism SPC
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

#define ADAP_TYPE_CLAMP (adap_clamp_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapClamp, adap_clamp, ADAP, CLAMP, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_clamp_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_clamp_get_child (AdapClamp  *self);
ADAP_AVAILABLE_IN_ALL
void       adap_clamp_set_child (AdapClamp  *self,
                                GtkWidget *child);

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_get_maximum_size (AdapClamp *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_set_maximum_size (AdapClamp *self,
                                 int       maximum_size);

ADAP_AVAILABLE_IN_ALL
int  adap_clamp_get_tightening_threshold (AdapClamp *self);
ADAP_AVAILABLE_IN_ALL
void adap_clamp_set_tightening_threshold (AdapClamp *self,
                                         int       tightening_threshold);

ADAP_AVAILABLE_IN_1_4
AdapLengthUnit adap_clamp_get_unit (AdapClamp      *self);
ADAP_AVAILABLE_IN_1_4
void          adap_clamp_set_unit (AdapClamp      *self,
                                  AdapLengthUnit  unit);

G_END_DECLS
