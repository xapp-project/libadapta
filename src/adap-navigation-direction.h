/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <glib-object.h>
#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_NAVIGATION_DIRECTION_BACK,
  ADAP_NAVIGATION_DIRECTION_FORWARD,
} AdapNavigationDirection;

G_END_DECLS
