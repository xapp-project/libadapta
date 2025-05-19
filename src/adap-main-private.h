/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-main.h"

G_BEGIN_DECLS

/* Initializes the public GObject types, which is needed to ensure they are
 * discoverable, for example so they can easily be used with GtkBuilder.
 *
 * The function is implemented in adap-public-types.c which is generated at
 * compile time by gen-public-types.sh
 */
void adap_init_public_types (void);

gboolean adap_is_granite_present (void);

G_END_DECLS
