/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <glib-object.h>
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_NAVIGATION_DIRECTION_BACK,
  ADW_NAVIGATION_DIRECTION_FORWARD,
} AdwNavigationDirection;

G_END_DECLS
