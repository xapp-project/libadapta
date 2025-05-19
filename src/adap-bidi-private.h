/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <glib.h>
#include <pango/pango.h>

G_BEGIN_DECLS

PangoDirection adap_find_base_dir (const char *text,
                                  int         length);

G_END_DECLS
