/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
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
  ADAP_FOLD_THRESHOLD_POLICY_MINIMUM,
  ADAP_FOLD_THRESHOLD_POLICY_NATURAL,
} AdapFoldThresholdPolicy;

G_END_DECLS
