/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-animation.h"

G_BEGIN_DECLS

struct _AdwAnimationClass
{
  GObjectClass parent_class;

  guint (*estimate_duration) (AdwAnimation *self);

  double (*calculate_value) (AdwAnimation *self,
                             guint         t);
};

G_END_DECLS
