/* GTK - The GIMP Toolkit
 * Copyright (C) 1998-2002 James Henstridge <james@daa.com.au>
 * Copyright (C) 2006-2007 Async Open Source,
 *                         Johan Dahlin <jdahlin@async.com.br>,
 *                         Henrique Romano <henrique@async.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

gboolean _gtk_builder_check_parent (GtkBuilder                *builder,
                                    GtkBuildableParseContext  *context,
                                    const gchar               *parent_name,
                                    GError                   **error);

void _gtk_builder_prefix_error (GtkBuilder                *builder,
                                GtkBuildableParseContext  *context,
                                GError                   **error);

void _gtk_builder_error_unhandled_tag (GtkBuilder                *builder,
                                       GtkBuildableParseContext  *context,
                                       const char                *object,
                                       const char                *element_name,
                                       GError                   **error);

const char *_gtk_builder_parser_translate (const char *domain,
                                           const char *context,
                                           const char *text);

G_END_DECLS
