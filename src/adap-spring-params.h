/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include "adw-version.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_TYPE_SPRING_PARAMS (adw_spring_params_get_type())

typedef struct _AdwSpringParams AdwSpringParams;

ADW_AVAILABLE_IN_ALL
GType adw_spring_params_get_type (void) G_GNUC_CONST;

ADW_AVAILABLE_IN_ALL
AdwSpringParams *adw_spring_params_new         (double damping_ratio,
                                                double mass,
                                                double stiffness) G_GNUC_WARN_UNUSED_RESULT;
ADW_AVAILABLE_IN_ALL
AdwSpringParams *adw_spring_params_new_full    (double damping,
                                                double mass,
                                                double stiffness) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwSpringParams *adw_spring_params_ref   (AdwSpringParams *self);
ADW_AVAILABLE_IN_ALL
void             adw_spring_params_unref (AdwSpringParams *self);

ADW_AVAILABLE_IN_ALL
double adw_spring_params_get_damping       (AdwSpringParams *self);
ADW_AVAILABLE_IN_ALL
double adw_spring_params_get_damping_ratio (AdwSpringParams *self);
ADW_AVAILABLE_IN_ALL
double adw_spring_params_get_mass          (AdwSpringParams *self);
ADW_AVAILABLE_IN_ALL
double adw_spring_params_get_stiffness     (AdwSpringParams *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdwSpringParams, adw_spring_params_unref)

G_END_DECLS
