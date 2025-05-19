/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <glib.h>

#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_LINEAR,
  ADAP_EASE_IN_QUAD,
  ADAP_EASE_OUT_QUAD,
  ADAP_EASE_IN_OUT_QUAD,
  ADAP_EASE_IN_CUBIC,
  ADAP_EASE_OUT_CUBIC,
  ADAP_EASE_IN_OUT_CUBIC,
  ADAP_EASE_IN_QUART,
  ADAP_EASE_OUT_QUART,
  ADAP_EASE_IN_OUT_QUART,
  ADAP_EASE_IN_QUINT,
  ADAP_EASE_OUT_QUINT,
  ADAP_EASE_IN_OUT_QUINT,
  ADAP_EASE_IN_SINE,
  ADAP_EASE_OUT_SINE,
  ADAP_EASE_IN_OUT_SINE,
  ADAP_EASE_IN_EXPO,
  ADAP_EASE_OUT_EXPO,
  ADAP_EASE_IN_OUT_EXPO,
  ADAP_EASE_IN_CIRC,
  ADAP_EASE_OUT_CIRC,
  ADAP_EASE_IN_OUT_CIRC,
  ADAP_EASE_IN_ELASTIC,
  ADAP_EASE_OUT_ELASTIC,
  ADAP_EASE_IN_OUT_ELASTIC,
  ADAP_EASE_IN_BACK,
  ADAP_EASE_OUT_BACK,
  ADAP_EASE_IN_OUT_BACK,
  ADAP_EASE_IN_BOUNCE,
  ADAP_EASE_OUT_BOUNCE,
  ADAP_EASE_IN_OUT_BOUNCE,
} AdapEasing;

ADAP_AVAILABLE_IN_ALL
double adap_easing_ease (AdapEasing self,
                        double    value);

G_END_DECLS
