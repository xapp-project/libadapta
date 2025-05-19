/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-main.h"

G_BEGIN_DECLS

/* Initializes the public GObject types, which is needed to ensure they are
 * discoverable, for example so they can easily be used with GtkBuilder.
 *
 * The function is implemented in adw-public-types.c which is generated at
 * compile time by gen-public-types.sh
 */
void adw_init_public_types (void);

gboolean adw_is_granite_present (void);

G_END_DECLS
