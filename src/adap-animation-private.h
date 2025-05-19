/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-animation.h"

G_BEGIN_DECLS

struct _AdapAnimationClass
{
  GObjectClass parent_class;

  guint (*estimate_duration) (AdapAnimation *self);

  double (*calculate_value) (AdapAnimation *self,
                             guint         t);
};

G_END_DECLS
