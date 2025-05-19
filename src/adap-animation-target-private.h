/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-animation-target.h"

G_BEGIN_DECLS

void adap_animation_target_set_value (AdapAnimationTarget *self,
                                     double              value);

G_END_DECLS
