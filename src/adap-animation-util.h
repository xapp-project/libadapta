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

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

ADAP_AVAILABLE_IN_ALL
double adap_lerp (double a,
                 double b,
                 double t);

ADAP_AVAILABLE_IN_ALL
gboolean adap_get_enable_animations (GtkWidget *widget);

G_END_DECLS
