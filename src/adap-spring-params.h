/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include "adap-version.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define ADAP_TYPE_SPRING_PARAMS (adap_spring_params_get_type())

typedef struct _AdapSpringParams AdapSpringParams;

ADAP_AVAILABLE_IN_ALL
GType adap_spring_params_get_type (void) G_GNUC_CONST;

ADAP_AVAILABLE_IN_ALL
AdapSpringParams *adap_spring_params_new         (double damping_ratio,
                                                double mass,
                                                double stiffness) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_ALL
AdapSpringParams *adap_spring_params_new_full    (double damping,
                                                double mass,
                                                double stiffness) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapSpringParams *adap_spring_params_ref   (AdapSpringParams *self);
ADAP_AVAILABLE_IN_ALL
void             adap_spring_params_unref (AdapSpringParams *self);

ADAP_AVAILABLE_IN_ALL
double adap_spring_params_get_damping       (AdapSpringParams *self);
ADAP_AVAILABLE_IN_ALL
double adap_spring_params_get_damping_ratio (AdapSpringParams *self);
ADAP_AVAILABLE_IN_ALL
double adap_spring_params_get_mass          (AdapSpringParams *self);
ADAP_AVAILABLE_IN_ALL
double adap_spring_params_get_stiffness     (AdapSpringParams *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdapSpringParams, adap_spring_params_unref)

G_END_DECLS
