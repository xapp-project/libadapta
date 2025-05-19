/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <glib.h>

G_BEGIN_DECLS

ADAP_AVAILABLE_IN_ALL
void adap_init (void);

ADAP_AVAILABLE_IN_ALL
gboolean adap_is_initialized (void);

G_END_DECLS
