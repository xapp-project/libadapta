/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-animation-target.h"

G_BEGIN_DECLS

void adw_animation_target_set_value (AdwAnimationTarget *self,
                                     double              value);

G_END_DECLS
