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

/* Copied and modified from gtkbuilder.c. */

#include "adw-gtkbuilder-utils-private.h"

/*< private >
 * @builder: a `GtkBuilder`
 * @context: the `GtkBuildableParseContext`
 * @parent_name: the name of the expected parent element
 * @error: return location for an error
 *
 * Checks that the parent element of the currently handled
 * start tag is @parent_name and set @error if it isn't.
 *
 * This is intended to be called in start_element vfuncs to
 * ensure that element nesting is as intended.
 *
 * Returns: %TRUE if @parent_name is the parent element
 */
gboolean
_gtk_builder_check_parent (GtkBuilder                *builder,
                           GtkBuildableParseContext  *context,
                           const gchar               *parent_name,
                           GError                   **error)
{
  GPtrArray *stack;
  int line, col;
  const char *parent;
  const char *element;

  stack = gtk_buildable_parse_context_get_element_stack (context);

  element = g_ptr_array_index (stack, stack->len - 1);
  parent = stack->len > 1 ? g_ptr_array_index (stack, stack->len - 2) : "";

  if (g_str_equal (parent_name, parent) ||
      (g_str_equal (parent_name, "object") && g_str_equal (parent, "template")))
    return TRUE;

  gtk_buildable_parse_context_get_position (context, &line, &col);
  g_set_error (error,
               GTK_BUILDER_ERROR,
               GTK_BUILDER_ERROR_INVALID_TAG,
               "%d:%d Can't use <%s> here",
               line, col, element);

  return FALSE;
}

/*< private >
 * _gtk_builder_prefix_error:
 * @builder: a `GtkBuilder`
 * @context: the `GtkBuildableParseContext`
 * @error: an error
 *
 * Calls g_prefix_error() to prepend a filename:line:column marker
 * to the given error. The filename is taken from @builder, and
 * the line and column are obtained by calling
 * g_markup_parse_context_get_position().
 *
 * This is intended to be called on errors returned by
 * g_markup_collect_attributes() in a start_element vfunc.
 */
void
_gtk_builder_prefix_error (GtkBuilder                *builder,
                           GtkBuildableParseContext  *context,
                           GError                   **error)
{
  int line, col;

  gtk_buildable_parse_context_get_position (context, &line, &col);
  g_prefix_error (error, ":%d:%d ", line, col);
}

/*< private >
 * _gtk_builder_error_unhandled_tag:
 * @builder: a `GtkBuilder`
 * @context: the `GtkBuildableParseContext`
 * @object: name of the object that is being handled
 * @element_name: name of the element whose start tag is being handled
 * @error: return location for the error
 *
 * Sets @error to a suitable error indicating that an @element_name
 * tag is not expected in the custom markup for @object.
 *
 * This is intended to be called in a start_element vfunc.
 */
void
_gtk_builder_error_unhandled_tag (GtkBuilder                *builder,
                                  GtkBuildableParseContext  *context,
                                  const char                *object,
                                  const char                *element_name,
                                  GError                   **error)
{
  int line, col;

  gtk_buildable_parse_context_get_position (context, &line, &col);
  g_set_error (error,
               GTK_BUILDER_ERROR,
               GTK_BUILDER_ERROR_UNHANDLED_TAG,
               "%d:%d Unsupported tag for %s: <%s>",
               line, col,
               object, element_name);
}

const char *
_gtk_builder_parser_translate (const char *domain,
                               const char *context,
                               const char *text)
{
  const char *s;

  if (context)
    s = g_dpgettext2 (domain, context, text);
  else
    s = g_dgettext (domain, text);

  return s;
}
