/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib.h>

G_BEGIN_DECLS

ADW_AVAILABLE_IN_ALL
void adw_init (void);

ADW_AVAILABLE_IN_ALL
gboolean adw_is_initialized (void);

G_END_DECLS
