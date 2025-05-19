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

G_BEGIN_DECLS

typedef enum {
  ADAP_LENGTH_UNIT_PX,
  ADAP_LENGTH_UNIT_PT,
  ADAP_LENGTH_UNIT_SP,
} AdapLengthUnit;

ADAP_AVAILABLE_IN_1_4
double adap_length_unit_to_px (AdapLengthUnit  unit,
                              double         value,
                              GtkSettings   *settings);

ADAP_AVAILABLE_IN_1_4
double adap_length_unit_from_px (AdapLengthUnit  unit,
                                double         value,
                                GtkSettings   *settings);
G_END_DECLS
