/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib.h>

#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_LINEAR,
  ADW_EASE_IN_QUAD,
  ADW_EASE_OUT_QUAD,
  ADW_EASE_IN_OUT_QUAD,
  ADW_EASE_IN_CUBIC,
  ADW_EASE_OUT_CUBIC,
  ADW_EASE_IN_OUT_CUBIC,
  ADW_EASE_IN_QUART,
  ADW_EASE_OUT_QUART,
  ADW_EASE_IN_OUT_QUART,
  ADW_EASE_IN_QUINT,
  ADW_EASE_OUT_QUINT,
  ADW_EASE_IN_OUT_QUINT,
  ADW_EASE_IN_SINE,
  ADW_EASE_OUT_SINE,
  ADW_EASE_IN_OUT_SINE,
  ADW_EASE_IN_EXPO,
  ADW_EASE_OUT_EXPO,
  ADW_EASE_IN_OUT_EXPO,
  ADW_EASE_IN_CIRC,
  ADW_EASE_OUT_CIRC,
  ADW_EASE_IN_OUT_CIRC,
  ADW_EASE_IN_ELASTIC,
  ADW_EASE_OUT_ELASTIC,
  ADW_EASE_IN_OUT_ELASTIC,
  ADW_EASE_IN_BACK,
  ADW_EASE_OUT_BACK,
  ADW_EASE_IN_OUT_BACK,
  ADW_EASE_IN_BOUNCE,
  ADW_EASE_OUT_BOUNCE,
  ADW_EASE_IN_OUT_BOUNCE,
} AdwEasing;

ADW_AVAILABLE_IN_ALL
double adw_easing_ease (AdwEasing self,
                        double    value);

G_END_DECLS
