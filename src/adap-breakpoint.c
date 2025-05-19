/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-breakpoint-private.h"

#include "adw-gtkbuilder-utils-private.h"
#include "adw-length-unit.h"
#include "adw-marshalers.h"

#include <gobject/gvaluecollector.h>

/**
 * AdwBreakpoint:
 *
 * Describes a breakpoint for [class@Window] or [class@Dialog].
 *
 * Breakpoints are used to create adaptive UI, allowing to change the layout
 * depending on available size.
 *
 * Breakpoint is a size threshold, specified by its condition, as well as one or
 * more setters.
 *
 * Each setter has a target object, a property and a value. When a breakpoint
 * is applied, each setter sets the target property on their target object to
 * the specified value, and reset it back to the original value when it's
 * unapplied.
 *
 * For more complicated scenarios, [signal@Breakpoint::apply] and
 * [signal@Breakpoint::unapply] can be used instead.
 *
 * Breakpoints can be used within [class@Window], [class@ApplicationWindow],
 * [class@Dialog] or [class@BreakpointBin].
 *
 * ## `AdwBreakpoint` as `GtkBuildable`:
 *
 * `AdwBreakpoint` supports specifying its condition via the `<condition>`
 * element. The contents of the element must be a string in a format accepted by
 * [func@BreakpointCondition.parse].
 *
 * It also supports adding setters via the `<setter>` element. Each `<setter>`
 * element must have the `object` attribute specifying the target object, and
 * the `property` attribute specifying the property name. The contents of the
 * element are used as the setter value.
 *
 * For `G_TYPE_OBJECT` and `G_TYPE_BOXED` derived properties, empty contents are
 * treated as `NULL`.
 *
 * Setter values can be translated with the usual `translatable`, `context` and
 * `comments` attributes.
 *
 * Example of an `AdwBreakpoint` UI definition:
 *
 * ```xml
 * <object class="AdwBreakpoint">
 *   <condition>max-width: 400px</condition>
 *   <setter object="button" property="visible">True</setter>
 *   <setter object="box" property="orientation">vertical</setter>
 *   <setter object="page" property="title" translatable="yes">Example</setter>
 * </object>
 * ```
 *
 * Since: 1.4
 */

/**
 * AdwBreakpointCondition:
 *
 * Describes condition for an [class@Breakpoint].
 *
 * Since: 1.4
 */

/**
 * AdwBreakpointConditionLengthType:
 * @ADW_BREAKPOINT_CONDITION_MIN_WIDTH: true if the width is greater than or
 *   equal to the condition value
 * @ADW_BREAKPOINT_CONDITION_MAX_WIDTH: true if the width is less than or
 *   equal to the condition value
 * @ADW_BREAKPOINT_CONDITION_MIN_HEIGHT: true if the height is greater than or
 *   equal to the condition value
 * @ADW_BREAKPOINT_CONDITION_MAX_HEIGHT: true if the height is less than or
 *   equal to the condition value
 *
 * Describes length types for [struct@BreakpointCondition].
 *
 * See [ctor@BreakpointCondition.new_length].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.4
 */

/**
 * AdwBreakpointConditionRatioType:
 * @ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO: true if the aspect ratio is
 *   greater than or equal to the condition value
 * @ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO: true if the aspect ratio is
 *   less than or equal to the condition value
 *
 * Describes ratio types for [struct@BreakpointCondition].
 *
 * See [ctor@BreakpointCondition.new_ratio].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.4
 */

G_DEFINE_BOXED_TYPE (AdwBreakpointCondition, adw_breakpoint_condition,
                     adw_breakpoint_condition_copy, adw_breakpoint_condition_free)

typedef enum {
  CONDITION_LENGTH,
  CONDITION_RATIO,
  CONDITION_MULTI
} ConditionType;

typedef enum {
  MULTI_CONDITION_ALL,
  MULTI_CONDITION_ANY,
} MultiConditionType;

typedef enum {
  CONDITION_PARSER_ERROR_INVALID_VALUE = 1,
  CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER,
  CONDITION_PARSER_ERROR_UNKNOWN_OPERATOR,
  CONDITION_PARSER_ERROR_UNKNOWN_TYPE,
  CONDITION_PARSER_ERROR_UNKNOWN_UNIT,
  CONDITION_PARSER_ERROR_VALUE_OUT_OF_RANGE,
} ConditionParserError;

struct _AdwBreakpointCondition
{
  ConditionType type;

  union {
    struct {
      AdwBreakpointConditionLengthType type;
      double value;
      AdwLengthUnit unit;
    } length;

    struct {
      AdwBreakpointConditionRatioType type;
      int width;
      int height;
    } ratio;

    struct {
      MultiConditionType type;
      AdwBreakpointCondition *condition_1;
      AdwBreakpointCondition *condition_2;
    } multi;
  } data;
};

static gboolean
check_condition (AdwBreakpointCondition *self,
                 GtkSettings            *settings,
                 int                     width,
                 int                     height)
{
  g_assert (self != NULL);

  if (self->type == CONDITION_MULTI) {
    gboolean check_1 = check_condition (self->data.multi.condition_1,
                                        settings, width, height);
    gboolean check_2 = check_condition (self->data.multi.condition_2,
                                        settings, width, height);

    if (self->data.multi.type == MULTI_CONDITION_ALL)
      return check_1 && check_2;
    else
      return check_1 || check_2;
  }

  if (self->type == CONDITION_LENGTH) {
    double value_px = adw_length_unit_to_px (self->data.length.unit,
                                             self->data.length.value,
                                             settings);

    switch (self->data.length.type) {
    case ADW_BREAKPOINT_CONDITION_MIN_WIDTH:
      return width >= value_px;
    case ADW_BREAKPOINT_CONDITION_MAX_WIDTH:
      return width <= value_px;
    case ADW_BREAKPOINT_CONDITION_MIN_HEIGHT:
      return height >= value_px;
    case ADW_BREAKPOINT_CONDITION_MAX_HEIGHT:
      return height <= value_px;
    default:
      g_assert_not_reached ();
    }
  }

  if (self->type == CONDITION_RATIO) {
    double ratio = (double) self->data.ratio.width / self->data.ratio.height;

    switch (self->data.ratio.type) {
    case ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO:
      return (double) width / height >= ratio;
    case ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO:
      return (double) width / height <= ratio;
    default:
      g_assert_not_reached ();
    }
  }

  g_assert_not_reached ();
}

/**
 * adw_breakpoint_condition_new_length:
 * @type: the length type
 * @value: the length value
 * @unit: the length unit
 *
 * Creates a condition that triggers on length changes.
 *
 * Returns: the newly created condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_new_length (AdwBreakpointConditionLengthType type,
                                     double                           value,
                                     AdwLengthUnit                    unit)
{
  AdwBreakpointCondition *self;

  g_return_val_if_fail (type >= ADW_BREAKPOINT_CONDITION_MIN_WIDTH, NULL);
  g_return_val_if_fail (type <= ADW_BREAKPOINT_CONDITION_MAX_HEIGHT, NULL);
  g_return_val_if_fail (unit >= ADW_LENGTH_UNIT_PX, NULL);
  g_return_val_if_fail (unit <= ADW_LENGTH_UNIT_SP, NULL);

  self = g_new0 (AdwBreakpointCondition, 1);
  self->type = CONDITION_LENGTH;
  self->data.length.type = type;
  self->data.length.value = value;
  self->data.length.unit = unit;

  return self;
}

/**
 * adw_breakpoint_condition_new_ratio:
 * @type: the ratio type
 * @width: ratio width
 * @height: ratio height
 *
 * Creates a condition that triggers on ratio changes.
 *
 * The ratio is represented as @width divided by @height.
 *
 * Returns: the newly created condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_new_ratio (AdwBreakpointConditionRatioType type,
                                    int                             width,
                                    int                             height)
{
  AdwBreakpointCondition *self;

  g_return_val_if_fail (type >= ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO, NULL);
  g_return_val_if_fail (type <= ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO, NULL);
  g_return_val_if_fail (width >= 0, NULL);
  g_return_val_if_fail (height >= 1, NULL);

  self = g_new0 (AdwBreakpointCondition, 1);
  self->type = CONDITION_RATIO;
  self->data.ratio.type = type;
  self->data.ratio.width = width;
  self->data.ratio.height = height;

  return self;
}

/**
 * adw_breakpoint_condition_new_and: (constructor)
 * @condition_1: (transfer full): first condition
 * @condition_2: (transfer full): second condition
 *
 * Creates a condition that triggers when @condition_1 and @condition_2 are both
 * true.
 *
 * Returns: the newly created condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_new_and (AdwBreakpointCondition *condition_1,
                                  AdwBreakpointCondition *condition_2)
{
  AdwBreakpointCondition *self;

  g_return_val_if_fail (condition_1 != NULL, NULL);
  g_return_val_if_fail (condition_2 != NULL, NULL);

  self = g_new0 (AdwBreakpointCondition, 1);
  self->type = CONDITION_MULTI;
  self->data.multi.type = MULTI_CONDITION_ALL;
  self->data.multi.condition_1 = condition_1;
  self->data.multi.condition_2 = condition_2;

  return self;
}

/**
 * adw_breakpoint_condition_new_or: (constructor)
 * @condition_1: (transfer full): first condition
 * @condition_2: (transfer full): second condition
 *
 * Creates a condition that triggers when either @condition_1 or @condition_2 is
 * true.
 *
 * Returns: the newly created condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_new_or (AdwBreakpointCondition *condition_1,
                                 AdwBreakpointCondition *condition_2)
{
  AdwBreakpointCondition *self;

  g_return_val_if_fail (condition_1 != NULL, NULL);
  g_return_val_if_fail (condition_2 != NULL, NULL);

  self = g_new0 (AdwBreakpointCondition, 1);
  self->type = CONDITION_MULTI;
  self->data.multi.type = MULTI_CONDITION_ANY;
  self->data.multi.condition_1 = condition_1;
  self->data.multi.condition_2 = condition_2;

  return self;
}

/**
 * adw_breakpoint_condition_copy:
 * @self: a breakpoint condition
 *
 * Copies @self.
 *
 * Returns: (transfer full): a copy of @self
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_copy (AdwBreakpointCondition *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  if (self->type == CONDITION_LENGTH) {
    return adw_breakpoint_condition_new_length (self->data.length.type,
                                                self->data.length.value,
                                                self->data.length.unit);
  }

  if (self->type == CONDITION_RATIO) {
    return adw_breakpoint_condition_new_ratio (self->data.ratio.type,
                                               self->data.ratio.width,
                                               self->data.ratio.height);
  }

  if (self->type == CONDITION_MULTI) {
    switch (self->data.multi.type) {
    case MULTI_CONDITION_ALL:
      return adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (self->data.multi.condition_1),
                                               adw_breakpoint_condition_copy (self->data.multi.condition_2));
    case MULTI_CONDITION_ANY:
      return adw_breakpoint_condition_new_or (adw_breakpoint_condition_copy (self->data.multi.condition_1),
                                              adw_breakpoint_condition_copy (self->data.multi.condition_2));
    default:
      g_assert_not_reached ();
    }
  }

  g_assert_not_reached ();
}

/**
 * adw_breakpoint_condition_free:
 * @self: a breakpoint condition
 *
 * Frees @self.
 *
 * Since: 1.4
 */
void
adw_breakpoint_condition_free (AdwBreakpointCondition *self)
{
  g_return_if_fail (self != NULL);

  if (self->type == CONDITION_MULTI) {
    adw_breakpoint_condition_free (self->data.multi.condition_1);
    adw_breakpoint_condition_free (self->data.multi.condition_2);
  }

  g_free (self);
}

#define SKIP_WHITESPACES(s) while (*(s) == ' ') (s)++;

static gboolean
parse_int (const char            *str,
           char                 **endp,
           int                   *number,
           ConditionParserError  *error)
{
  long int ret = strtol (str, endp, 10);

  if (errno == ERANGE || *endp == str) {
    *error = CONDITION_PARSER_ERROR_INVALID_VALUE;
    return FALSE;
  }

  *number = CLAMP (ret, G_MININT, G_MAXINT);

  return TRUE;
}

static gboolean
parse_double (const char            *str,
              char                 **endp,
              double                *number,
              ConditionParserError  *error)
{
  double ret = g_ascii_strtod (str, endp);

  if (errno == ERANGE || *endp == str || isinf (ret) || isnan (ret)) {
    *error = CONDITION_PARSER_ERROR_INVALID_VALUE;
    return FALSE;
  }

  *number = ret;

  return TRUE;
}

/* Parse a single length or ratio condition */
static AdwBreakpointCondition *
parse_single (const char            *str,
              char                 **endp,
              ConditionParserError  *error)
{
  ConditionType type = -1;
  AdwBreakpointConditionLengthType length_type = -1;
  AdwBreakpointConditionRatioType ratio_type = -1;

  if (!strncmp (str, "min-width", 9)) {
    type = CONDITION_LENGTH;
    length_type = ADW_BREAKPOINT_CONDITION_MIN_WIDTH;
    str += 9;
  } else if (!strncmp (str, "max-width", 9)) {
    type = CONDITION_LENGTH;
    length_type = ADW_BREAKPOINT_CONDITION_MAX_WIDTH;
    str += 9;
  } else if (!strncmp (str, "min-height", 10)) {
    type = CONDITION_LENGTH;
    length_type = ADW_BREAKPOINT_CONDITION_MIN_HEIGHT;
    str += 10;
  } else if (!strncmp (str, "max-height", 10)) {
    type = CONDITION_LENGTH;
    length_type = ADW_BREAKPOINT_CONDITION_MAX_HEIGHT;
    str += 10;
  } else if (!strncmp (str, "min-aspect-ratio", 16)) {
    type = CONDITION_RATIO;
    ratio_type = ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO;
    str += 16;
  } else if (!strncmp (str, "max-aspect-ratio", 16)) {
    type = CONDITION_RATIO;
    ratio_type = ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO;
    str += 16;
  } else {
    *endp = (char *) str;
    *error = CONDITION_PARSER_ERROR_UNKNOWN_TYPE;
    return NULL;
  }

  SKIP_WHITESPACES (str);

  if (*str == ':') {
    str++;
  } else {
    *endp = (char *) str;
    *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
    return NULL;
  }

  SKIP_WHITESPACES (str);
  *endp = (char *) str;

  if (type == CONDITION_LENGTH) {
    AdwLengthUnit unit;
    double value;

    if (!parse_double (str, endp, &value, error))
      return NULL;

    if (value < 0) {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_VALUE_OUT_OF_RANGE;
      return NULL;
    }

    str = *endp;

    SKIP_WHITESPACES (str);

    if (!strncmp (str, "px", 2)) {
      unit = ADW_LENGTH_UNIT_PX;
      str += 2;
    } else if (!strncmp (str, "pt", 2)) {
      unit = ADW_LENGTH_UNIT_PT;
      str += 2;
    } else if (!strncmp (str, "sp", 2)) {
      unit = ADW_LENGTH_UNIT_SP;
      str += 2;
    } else if (*str == ' ' || *str == ')' || *str == '\0') {
      unit = ADW_LENGTH_UNIT_PX;
    } else {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_UNKNOWN_UNIT;
      return NULL;
    }

    if (*str != ' ' && *str != ')' && *str != '\0') {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
      return NULL;
    }

    *endp = (char *) str;
    return adw_breakpoint_condition_new_length (length_type, value, unit);
  }

  if (type == CONDITION_RATIO) {
    int width, height;

    if (!parse_int (str, endp, &width, error))
      return NULL;

    if (width < 0) {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_VALUE_OUT_OF_RANGE;
      return NULL;
    }

    str = *endp;
    SKIP_WHITESPACES (str);

    if (*str == '/') {
      str++;

      SKIP_WHITESPACES (str);
      *endp = (char *) str;

      if (!parse_int (str, endp, &height, error))
        return NULL;

      if (height < 1) {
        *endp = (char *) str;
        *error = CONDITION_PARSER_ERROR_VALUE_OUT_OF_RANGE;
        return NULL;
      }

      str = *endp;
    } else {
      height = 1;
    }

    if (*str != ' ' && *str != ')' && *str != '\0') {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
      return NULL;
    }

    *endp = (char *) str;
    return adw_breakpoint_condition_new_ratio (ratio_type, width, height);
  }

  g_assert_not_reached ();
}

static AdwBreakpointCondition *
parse_multi (const char            *str,
             char                 **endp,
             ConditionParserError  *error)
{
  AdwBreakpointCondition *cond_1, *cond_2;
  MultiConditionType multi_type = -1;

  SKIP_WHITESPACES (str);

  if (*str == '(') {
    str++;

    cond_1 = parse_multi (str, endp, error);
    str = *endp;

    if (!cond_1) {
      *endp = (char *) str;
      return NULL;
    }

    if (*str == ')') {
      str++;
    } else {
      g_clear_pointer (&cond_1, adw_breakpoint_condition_free);
      *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
    }
  } else {
    cond_1 = parse_single (str, endp, error);
    str = *endp;
  }

  if (!cond_1) {
    *endp = (char *) str;
    return NULL;
  }

  while (*str == ' ') {
    SKIP_WHITESPACES (str);
    *endp = (char *) str;

    if (!strncmp (str, "and", 3)) {
      multi_type = MULTI_CONDITION_ALL;
      str += 3;
    } else if (!strncmp (str, "or", 2)) {
      multi_type = MULTI_CONDITION_ANY;
      str += 2;
    } else if (*str == ' ' || *str == ')' || *str == '\0') {
      return cond_1;
    } else {
      *error = CONDITION_PARSER_ERROR_UNKNOWN_OPERATOR;
      g_clear_pointer (&cond_1, adw_breakpoint_condition_free);
      return NULL;
    }

    if (*str == ' ') {
      str++;
    } else {
      *endp = (char *) str;
      *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
      g_clear_pointer (&cond_1, adw_breakpoint_condition_free);
      return NULL;
    }

    SKIP_WHITESPACES (str);

    if (*str == '(') {
      str++;

      cond_2 = parse_multi (str, endp, error);
      str = *endp;

      if (!cond_2) {
        *endp = (char *) str;
        g_clear_pointer (&cond_1, adw_breakpoint_condition_free);
        return NULL;
      }

      if (*str == ')') {
        str++;
      } else {
        g_clear_pointer (&cond_2, adw_breakpoint_condition_free);
        *error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
      }
    } else {
      cond_2 = parse_single (str, endp, error);
      str = *endp;
    }

    if (!cond_2) {
      *endp = (char *) str;
      g_clear_pointer (&cond_1, adw_breakpoint_condition_free);
      return NULL;
    }

    switch (multi_type) {
    case MULTI_CONDITION_ALL:
      cond_1 = adw_breakpoint_condition_new_and (cond_1, cond_2);
      break;
    case MULTI_CONDITION_ANY:
      cond_1 = adw_breakpoint_condition_new_or (cond_1, cond_2);
      break;
    default:
      g_assert_not_reached ();
    }

    cond_2 = NULL;
  }

  SKIP_WHITESPACES (str);
  *endp = (char *) str;

  return cond_1;
}

/**
 * adw_breakpoint_condition_parse:
 * @str: the string specifying the condition
 *
 * Parses a condition from a string.
 *
 * Length conditions are specified as `<type>: <value>[<unit>]`, where:
 *
 * - `<type>` can be `min-width`, `max-width`, `min-height` or `max-height`
 * - `<value>` is a fractional number
 * - `<unit>` can be `px`, `pt` or `sp`
 *
 * If the unit is omitted, `px` is assumed.
 *
 * See [ctor@BreakpointCondition.new_length].
 *
 * Examples:
 *
 * - `min-width: 500px`
 * - `min-height: 400pt`
 * - `max-width: 100sp`
 * - `max-height: 500`
 *
 * Ratio conditions are specified as `<type>: <width>[/<height>]`, where:
 *
 * - `<type>` can be `min-aspect-ratio` or `max-aspect-ratio`
 * - `<width>` and `<height>` are integer numbers
 *
 * See [ctor@BreakpointCondition.new_ratio].
 *
 * The ratio is represented as `<width>` divided by `<height>`.
 *
 * If `<height>` is omitted, it's assumed to be 1.
 *
 * Examples:
 *
 * - `min-aspect-ratio: 4/3`
 * - `max-aspect-ratio: 1`
 *
 * The logical operators `and`, `or` can be used to compose a complex condition
 * as follows:
 *
 * - `<condition> and <condition>`: the condition is true when both
 *   `<condition>`s are true, same as when using
 *   [ctor@BreakpointCondition.new_and]
 * - `<condition> or <condition>`: the condition is true when either of the
 *   `<condition>`s is true, same as when using
 *   [ctor@BreakpointCondition.new_or]
 *
 * Examples:
 *
 * - `min-width: 400px and max-aspect-ratio: 4/3`
 * - `max-width: 360sp or max-width: 360px`
 *
 * Conditions can be further nested using parentheses, for example:
 *
 * - `min-width: 400px and (max-aspect-ratio: 4/3 or max-height: 400px)`
 *
 * If parentheses are omitted, the first operator takes priority.
 *
 * Returns: the parsed condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_condition_parse (const char *str)
{
  AdwBreakpointCondition *ret;
  char *endp;
  ConditionParserError error = 0;

  g_return_val_if_fail (str != NULL, NULL);

  SKIP_WHITESPACES (str);

  ret = parse_multi (str, &endp, &error);

  if (*endp != '\0') {
    g_clear_pointer (&ret, adw_breakpoint_condition_free);

    if (!error)
      error = CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER;
  }

  if (!ret) {
    GString *line = g_string_new (NULL);
    char *line_str;
    int i;
    const char *message;

    switch (error) {
    case CONDITION_PARSER_ERROR_INVALID_VALUE:
      message = "unable to parse value";
      break;
    case CONDITION_PARSER_ERROR_UNEXPECTED_CHARACTER:
      message = "unexpected character";
      break;
    case CONDITION_PARSER_ERROR_UNKNOWN_OPERATOR:
      message = "unknown operator";
      break;
    case CONDITION_PARSER_ERROR_UNKNOWN_TYPE:
      message = "unknown type";
      break;
    case CONDITION_PARSER_ERROR_UNKNOWN_UNIT:
      message = "unknown unit";
      break;
    case CONDITION_PARSER_ERROR_VALUE_OUT_OF_RANGE:
      message = "value is out of range";
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    for (i = 0; i < endp - str; i++)
      g_string_append_c (line, '-');

    g_string_append_c (line, '^');
    line_str = g_string_free_and_steal (line);

    g_critical ("Unable to parse condition: %s\n%s\n%s", message, str, line_str);

    g_free (line_str);
  }

  return ret;
}

/**
 * adw_breakpoint_condition_to_string:
 * @self: a breakpoint condition
 *
 * Returns a textual representation of @self.
 *
 * The returned string can be parsed by [func@BreakpointCondition.parse].
 *
 * Returns: A newly allocated text string
 *
 * Since: 1.4
 */
char *
adw_breakpoint_condition_to_string (AdwBreakpointCondition *self)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];

  g_return_val_if_fail (self != NULL, NULL);

  /* Example: "max-width: 400px" */
  if (self->type == CONDITION_LENGTH) {
    const char *type, *unit;

    switch (self->data.length.type) {
    case ADW_BREAKPOINT_CONDITION_MIN_WIDTH:
      type = "min-width";
      break;
    case ADW_BREAKPOINT_CONDITION_MAX_WIDTH:
      type = "max-width";
      break;
    case ADW_BREAKPOINT_CONDITION_MIN_HEIGHT:
      type = "min-height";
      break;
    case ADW_BREAKPOINT_CONDITION_MAX_HEIGHT:
      type = "max-height";
      break;
    default:
      g_assert_not_reached ();
    }

    switch (self->data.length.unit) {
    case ADW_LENGTH_UNIT_PX:
      unit = "px";
      break;
    case ADW_LENGTH_UNIT_PT:
      unit = "pt";
      break;
    case ADW_LENGTH_UNIT_SP:
      unit = "sp";
      break;
    default:
      g_assert_not_reached ();
    }

    g_ascii_dtostr (buf, sizeof (buf), self->data.length.value);

    return g_strdup_printf ("%s: %s%s", type, buf, unit);
  }

  /* Example: "max-aspect-ratio: 4/3" */
  if (self->type == CONDITION_RATIO) {
    const char *type;

    switch (self->data.ratio.type) {
    case ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO:
      type = "min-aspect-ratio";
      break;
    case ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO:
      type = "max-aspect-ratio";
      break;
    default:
      g_assert_not_reached ();
    }

    /* Simplify x/1 and 0/y into x and y respectively */
    if (self->data.ratio.width == 0 ||
        self->data.ratio.height == 1)
      return g_strdup_printf ("%s: %d", type,
                              self->data.ratio.width);

    return g_strdup_printf ("%s: %d/%d", type,
                            self->data.ratio.width,
                            self->data.ratio.height);
  }

  /* Example: "max-width: 400px and max-height: 300px" */
  if (self->type == CONDITION_MULTI) {
    const char *operator;
    char *ret;
    char *str_1, *str_2;
    gboolean parentheses_1, parentheses_2;

    switch (self->data.multi.type) {
    case MULTI_CONDITION_ALL:
      operator = "and";
      break;
    case MULTI_CONDITION_ANY:
      operator = "or";
      break;
    default:
      g_assert_not_reached ();
    }

    str_1 = adw_breakpoint_condition_to_string (self->data.multi.condition_1);
    str_2 = adw_breakpoint_condition_to_string (self->data.multi.condition_2);

    /* Omit parentheses for nested multi conditions of the same type,
     * so that we get "X and Y and Z" and not "X and (Y and Z)" */
    parentheses_1 = self->data.multi.condition_1->type == CONDITION_MULTI &&
                    self->data.multi.condition_1->data.multi.type != self->data.multi.type;
    parentheses_2 = self->data.multi.condition_2->type == CONDITION_MULTI &&
                    self->data.multi.condition_2->data.multi.type != self->data.multi.type;

    if (parentheses_1 && parentheses_2) {
      ret = g_strdup_printf ("(%s) %s (%s)", str_1, operator, str_2);
    } else if (parentheses_1) {
      ret = g_strdup_printf ("(%s) %s %s", str_1, operator, str_2);
    } else if (parentheses_2) {
      ret = g_strdup_printf ("%s %s (%s)", str_1, operator, str_2);
    } else {
      ret = g_strdup_printf ("%s %s %s", str_1, operator, str_2);
    }

    g_free (str_1);
    g_free (str_2);

    return ret;
  }

  g_assert_not_reached ();
}

typedef struct {
  AdwBreakpoint *breakpoint;
  GObject *object;
  GParamSpec *pspec;
  GValue value;
  GValue original_value;
} SetterData;

struct _AdwBreakpoint
{
  GObject parent_instance;

  AdwBreakpointCondition *condition;
  GHashTable *setters;
  gboolean active;
};

static void adw_breakpoint_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwBreakpoint, adw_breakpoint, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_breakpoint_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONDITION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_APPLY,
  SIGNAL_UNAPPLY,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static inline GParamSpec *
find_pspec (GObject    *object,
            const char *name)
{
  GParamSpec *pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), name);

  if (!pspec) {
    g_critical ("Type '%s' does not have a property named '%s'",
                G_OBJECT_TYPE_NAME (object), name);
    return NULL;
  }

  return pspec;
}

static void free_setter_data (SetterData *data);

static void
setter_weak_notify (SetterData *setter,
                    GObject    *where_the_object_was)
{
  g_assert (setter);

  g_hash_table_steal (setter->breakpoint->setters, setter);

  setter->object = NULL;
  free_setter_data (setter);
}

static void
free_setter_data (SetterData *setter)
{
  if (setter->object)
    g_object_weak_unref (setter->object,
                         (GWeakNotify) setter_weak_notify,
                         setter);

  g_param_spec_unref (setter->pspec);
  g_value_unset (&setter->value);
  g_value_unset (&setter->original_value);

  g_free (setter);
}

static guint
setter_hash (const SetterData *setter)
{
  return g_direct_hash (setter->object) + g_direct_hash (setter->pspec);
}

static gboolean
setter_equal (const SetterData *a,
              const SetterData *b)
{
  return a->object == b->object && a->pspec == b->pspec;
}

static void
adw_breakpoint_dispose (GObject *object)
{
  AdwBreakpoint *self = ADW_BREAKPOINT (object);

  g_clear_pointer (&self->condition, adw_breakpoint_condition_free);
  g_clear_pointer (&self->setters, g_hash_table_unref);

  G_OBJECT_CLASS (adw_breakpoint_parent_class)->dispose (object);
}

static void
adw_breakpoint_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwBreakpoint *self = ADW_BREAKPOINT (object);

  switch (prop_id) {
  case PROP_CONDITION:
    g_value_set_boxed (value, adw_breakpoint_get_condition (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_breakpoint_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwBreakpoint *self = ADW_BREAKPOINT (object);

  switch (prop_id) {
  case PROP_CONDITION:
    adw_breakpoint_set_condition (self, g_value_get_boxed (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_breakpoint_class_init (AdwBreakpointClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_breakpoint_dispose;
  object_class->get_property = adw_breakpoint_get_property;
  object_class->set_property = adw_breakpoint_set_property;

  /**
   * AdwBreakpoint:condition: (attributes org.gtk.Property.get=adw_breakpoint_get_condition org.gtk.Property.set=adw_breakpoint_set_condition)
   *
   * The breakpoint's condition.
   *
   * Since: 1.4
   */
  props[PROP_CONDITION] =
    g_param_spec_boxed ("condition", NULL, NULL,
                        ADW_TYPE_BREAKPOINT_CONDITION,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwBreakpoint::apply:
   *
   * Emitted when the breakpoint is applied.
   *
   * This signal is emitted after the setters have been applied.
   *
   * Since: 1.4
   */
  signals[SIGNAL_APPLY] =
    g_signal_new ("apply",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_APPLY],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwBreakpoint::unapply:
   *
   * Emitted when the breakpoint is unapplied.
   *
   * This signal is emitted before resetting the setter values.
   *
   * Since: 1.4
   */
  signals[SIGNAL_UNAPPLY] =
    g_signal_new ("unapply",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_UNAPPLY],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);
}

static void
adw_breakpoint_init (AdwBreakpoint *self)
{
  self->setters = g_hash_table_new_full ((GHashFunc) setter_hash,
                                         (GEqualFunc) setter_equal,
                                         NULL,
                                         (GDestroyNotify) free_setter_data);
}

typedef struct {
  GObject *object;
  GtkBuilder *builder;

  GString *condition;
} ConditionParserData;

static void
condition_data_free (gpointer data)
{
  ConditionParserData *setter = data;

  g_string_free (setter->condition, TRUE);
  g_free (setter);
}

static void
condition_start_element (GtkBuildableParseContext  *context,
                         const char                *element_name,
                         const char               **names,
                         const char               **values,
                         gpointer                   user_data,
                         GError                   **error)
{
  ConditionParserData *data = user_data;

  if (strcmp (element_name, "condition") == 0) {
    _gtk_builder_check_parent (data->builder, context, "object", error);

    return;
  }

  _gtk_builder_error_unhandled_tag (data->builder, context,
                                    "AdwBreakpoint", element_name,
                                    error);
}

static void
condition_text (GtkBuildableParseContext  *context,
                const char                *text,
                gsize                      text_len,
                gpointer                   user_data,
                GError                   **error)
{
  ConditionParserData *data = user_data;

  g_string_append_len (data->condition, text, text_len);
}

static const GtkBuildableParser condition_parser = {
  condition_start_element,
  NULL,
  condition_text,
  NULL
};

typedef struct {
  GObject *object;
  GtkBuilder *builder;

  char *object_id;
  char *property_name;
  GString *value;

  char *context;
  gboolean translatable;
} SetterParserData;

static void
setter_data_free (gpointer data)
{
  SetterParserData *setter = data;

  g_free (setter->object_id);
  g_free (setter->property_name);
  g_string_free (setter->value, TRUE);
  g_free (setter->context);
  g_free (setter);
}

static void
setter_start_element (GtkBuildableParseContext  *context,
                      const char                *element_name,
                      const char               **names,
                      const char               **values,
                      gpointer                   user_data,
                      GError                   **error)
{
  SetterParserData *data = user_data;
  const char *object_str = NULL;
  const char *property_str = NULL;
  const char *msg_context = NULL;
  gboolean translatable = FALSE;

  if (strcmp (element_name, "setter") != 0) {
    _gtk_builder_error_unhandled_tag (data->builder, context,
                                      "AdwBreakpoint", element_name,
                                      error);
    return;
  }

  if (!_gtk_builder_check_parent (data->builder, context, "object", error))
    return;

  if (!g_markup_collect_attributes (element_name, names, values, error,
                                    G_MARKUP_COLLECT_STRING, "object", &object_str,
                                    G_MARKUP_COLLECT_STRING, "property", &property_str,
                                    G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "translatable", &translatable,
                                    G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "comments", NULL,
                                    G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "context", &msg_context,
                                    G_MARKUP_COLLECT_INVALID)) {
    _gtk_builder_prefix_error (data->builder, context, error);
    return;
  }

  data->object_id = g_strdup (object_str);
  data->property_name = g_strdup (property_str);
  data->translatable = translatable;
  data->context = g_strdup (msg_context);
}

static void
setter_text (GtkBuildableParseContext  *context,
             const char                *text,
             gsize                      text_len,
             gpointer                   user_data,
             GError                   **error)
{
  SetterParserData *data = user_data;

  g_string_append_len (data->value, text, text_len);
}

static const GtkBuildableParser setter_parser = {
  setter_start_element,
  NULL,
  setter_text,
  NULL
};

static gboolean
adw_breakpoint_buildable_custom_tag_start (GtkBuildable       *buildable,
                                           GtkBuilder         *builder,
                                           GObject            *child,
                                           const char         *tagname,
                                           GtkBuildableParser *parser,
                                           gpointer           *parser_data)
{
  if (child)
    return FALSE;

  if (strcmp (tagname, "setter") == 0) {
    SetterParserData *data;

    data = g_new0 (SetterParserData, 1);
    data->object = G_OBJECT (buildable);
    data->builder = builder;
    data->value = g_string_new ("");

    *parser = setter_parser;
    *parser_data = data;

    return TRUE;
  }

  if (strcmp (tagname, "condition") == 0) {
    ConditionParserData *data;

    data = g_new0 (ConditionParserData, 1);
    data->object = G_OBJECT (buildable);
    data->builder = builder;
    data->condition = g_string_new ("");

    *parser = condition_parser;
    *parser_data = data;

    return TRUE;
  }

  return FALSE;
}

static void
adw_breakpoint_buildable_custom_finished (GtkBuildable *buildable,
                                          GtkBuilder   *builder,
                                          GObject      *child,
                                          const char   *tagname,
                                          gpointer      user_data)
{
  if (strcmp (tagname, "setter") == 0) {
    SetterParserData *data = user_data;
    GObject *object;
    GParamSpec *pspec;
    const char *value_str;
    GValue value = G_VALUE_INIT;
    GError *error = NULL;
    GType type;

    object = gtk_builder_get_object (data->builder, data->object_id);

    if (!object) {
      g_critical ("Unable to find object '%s' for setter", data->object_id);
      setter_data_free (data);
      return;
    }

    pspec = find_pspec (object, data->property_name);

    if (!pspec) {
      setter_data_free (data);
      return;
    }

    if (data->translatable && data->value->len)
      value_str = _gtk_builder_parser_translate (gtk_builder_get_translation_domain (builder),
                                                 data->context,
                                                 data->value->str);
    else
      value_str = data->value->str;

    type = G_TYPE_FUNDAMENTAL (G_PARAM_SPEC_VALUE_TYPE (pspec));

    /* Treat empty strings like NULL */
    if ((type == G_TYPE_OBJECT || type == G_TYPE_INTERFACE || type == G_TYPE_BOXED) && !g_strcmp0 (value_str, "")) {
      g_value_init (&value, type);

      g_value_set_object (&value, NULL);
    } else if (!gtk_builder_value_from_string (builder, pspec, value_str, &value, &error)) {
      g_warning ("Invalid value %s for property %s: %s",
                 value_str, data->property_name, error->message);
      g_error_free (error);
      setter_data_free (data);
      return;
    }

    adw_breakpoint_add_setter (ADW_BREAKPOINT (data->object),
                               object,
                               data->property_name,
                               &value);

    g_value_unset (&value);

    setter_data_free (data);
    return;
  }

  if (strcmp (tagname, "condition") == 0) {
    ConditionParserData *data = user_data;
    AdwBreakpointCondition *condition;

    condition = adw_breakpoint_condition_parse (data->condition->str);

    if (condition) {
      adw_breakpoint_set_condition (ADW_BREAKPOINT (data->object), condition);
      adw_breakpoint_condition_free (condition);
    }

    condition_data_free (data);
    return;
  }

  parent_buildable_iface->custom_finished (buildable, builder, child,
                                           tagname, user_data);
}

static void
adw_breakpoint_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->custom_tag_start = adw_breakpoint_buildable_custom_tag_start;
  iface->custom_finished = adw_breakpoint_buildable_custom_finished;
}

/**
 * adw_breakpoint_new:
 * @condition: (transfer full): the condition
 *
 * Creates a new `AdwBreakpoint` with @condition.
 *
 * Returns: the newly created `AdwBreakpoint`
 *
 * Since: 1.4
 */
AdwBreakpoint *
adw_breakpoint_new (AdwBreakpointCondition *condition)
{
  AdwBreakpoint *breakpoint;

  g_return_val_if_fail (condition != NULL, NULL);

  breakpoint = g_object_new (ADW_TYPE_BREAKPOINT,
                             "condition", condition,
                             NULL);

  adw_breakpoint_condition_free (condition);

  return breakpoint;
}

/**
 * adw_breakpoint_get_condition: (attributes org.gtk.Method.get_property=condition)
 * @self: a breakpoint
 *
 * Gets the condition for @self.
 *
 * Returns: (transfer none) (nullable): the condition
 *
 * Since: 1.4
 */
AdwBreakpointCondition *
adw_breakpoint_get_condition (AdwBreakpoint *self)
{
  g_return_val_if_fail (ADW_IS_BREAKPOINT (self), NULL);

  return self->condition;
}

/**
 * adw_breakpoint_set_condition: (attributes org.gtk.Method.set_property=condition)
 * @self: a breakpoint
 * @condition: (nullable): the new condition
 *
 * Sets the condition for @self.
 *
 * Since: 1.4
 */
void
adw_breakpoint_set_condition (AdwBreakpoint          *self,
                              AdwBreakpointCondition *condition)
{
  g_return_if_fail (ADW_IS_BREAKPOINT (self));

  if (self->condition == condition)
    return;

  g_clear_pointer (&self->condition, adw_breakpoint_condition_free);

  if (condition)
    self->condition = adw_breakpoint_condition_copy (condition);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONDITION]);
}

/**
 * adw_breakpoint_add_setter:
 * @self: a breakpoint
 * @object: the target object
 * @property: the target property
 * @value: (nullable): the value to set
 *
 * Adds a setter to @self.
 *
 * The setter will automatically set @property on @object to @value when
 * applying the breakpoint, and set it back to its original value upon
 * unapplying it.
 *
 * ::: note
 *     Setting properties to their original values does not work for properties
 *     that have irreversible side effects. For example, changing
 *     [property@Gtk.Button:label] while [property@Gtk.Button:icon-name] is set
 *     will reset the icon. However, resetting the label will not set
 *     `icon-name` to its original value.
 *
 * Use the [signal@Breakpoint::apply] and [signal@Breakpoint::unapply] signals
 * for those properties instead, as follows:
 *
 * ```c
 * static void
 * breakpoint_apply_cb (MyWidget *self)
 * {
 *   gtk_button_set_icon_name (self->button, "go-previous-symbolic");
 * }
 *
 * static void
 * breakpoint_apply_cb (MyWidget *self)
 * {
 *   gtk_button_set_label (self->button, _("_Back"));
 * }
 *
 * // ...
 *
 * g_signal_connect_swapped (breakpoint, "apply",
 *                           G_CALLBACK (breakpoint_apply_cb), self);
 * g_signal_connect_swapped (breakpoint, "unapply",
 *                           G_CALLBACK (breakpoint_unapply_cb), self);
 * ```
 *
 * Since: 1.4
 */
void
adw_breakpoint_add_setter (AdwBreakpoint *self,
                           GObject       *object,
                           const char    *property,
                           const GValue  *value)
{
  SetterData *setter;
  GParamSpec *pspec;
  GValue validated_value = G_VALUE_INIT;
  GValue original_value = G_VALUE_INIT;

  g_return_if_fail (ADW_IS_BREAKPOINT (self));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (property != NULL);
  g_return_if_fail (G_IS_VALUE (value));

  pspec = find_pspec (object, property);

  if (!pspec)
    return;

  g_value_init (&validated_value, pspec->value_type);

  if (!g_value_transform (value, &validated_value)) {
    g_error ("Unable to add setter for property '%s' of type '%s' from value of type '%s'",
             pspec->name,
             g_type_name (pspec->value_type),
             G_VALUE_TYPE_NAME (value));

    g_value_unset (&validated_value);

    return;
  } else if (g_param_value_validate (pspec, &validated_value) &&
             !(pspec->flags & G_PARAM_LAX_VALIDATION)) {
    char *contents = g_strdup_value_contents (value);

    g_warning ("Unable to add setter: value \"%s\" of type '%s' is invalid or"
               "out of range for property '%s' of type '%s'",
               contents,
               G_VALUE_TYPE_NAME (value),
               pspec->name,
               g_type_name (pspec->value_type));

    g_free (contents);
    g_value_unset (&validated_value);

    return;
  }

  g_object_get_property (object, property, &original_value);

  setter = g_new0 (SetterData, 1);

  setter->breakpoint = self;
  setter->object = object;
  setter->pspec = g_param_spec_ref (pspec);
  setter->value = validated_value;
  setter->original_value = original_value;

  g_object_weak_ref (object,
                     (GWeakNotify) setter_weak_notify,
                     setter);

  g_hash_table_insert (self->setters, setter, setter);

  if (self->active)
    g_object_set_property (setter->object,
                           setter->pspec->name,
                           &setter->value);
}

/**
 * adw_breakpoint_add_setters: (skip)
 * @self: a breakpoint
 * @first_object: the first target object
 * @first_property: the first target property
 * @...: the value of the first setter, followed by a list of object, property
 *   and value triplets, terminated by `NULL`
 *
 * Adds multiple setters to @self.
 *
 * See [method@Breakpoint.add_setter].
 *
 * Example:
 *
 * ```c
 * adw_breakpoint_add_setters (breakpoint,
 *                             G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
 *                             G_OBJECT (button), "halign", GTK_ALIGN_FILL,
 *                             G_OBJECT (button), "valign", GTK_ALIGN_END,
 *                             NULL);
 * ```
 *
 * Since: 1.4
 */
void
adw_breakpoint_add_setters (AdwBreakpoint *self,
                            GObject       *first_object,
                            const char    *first_property,
                            ...)
{
  va_list args;

  g_return_if_fail (ADW_IS_BREAKPOINT (self));
  g_return_if_fail (G_IS_OBJECT (first_object));
  g_return_if_fail (first_property != NULL);

  va_start (args, first_property);
  adw_breakpoint_add_setters_valist (self, first_object, first_property, args);
  va_end (args);
}

/**
 * adw_breakpoint_add_settersv: (rename-to adw_breakpoint_add_setters)
 * @self: a breakpoint
 * @n_setters: the number of setters to add
 * @objects: (array length=n_setters): setter target object
 * @names: (array length=n_setters): setter target properties
 * @values: (array length=n_setters): setter values
 *
 * Adds @n_setters setters to @self.
 *
 * This is a convenience function for adding multiple setters at once.
 *
 * See [method@Breakpoint.add_setter].
 *
 * This function is meant to be used by language bindings.
 *
 * Since: 1.4
 */
void
adw_breakpoint_add_settersv (AdwBreakpoint *self,
                             int            n_setters,
                             GObject       *objects[],
                             const char    *names[],
                             const GValue  *values[])
{
  int i;

  g_return_if_fail (ADW_IS_BREAKPOINT (self));

  for (i = 0; i < n_setters; i++)
    adw_breakpoint_add_setter (self, objects[i], names[i], values[i]);
}

/**
 * adw_breakpoint_add_setters_valist: (skip)
 * @self: a breakpoint
 * @first_object: the first target object
 * @first_property: the first target property
 * @args: the value of the first setter, followed by a list of object, property
 *   and value triplets, terminated by `NULL`
 *
 * Adds multiple setters to @self.
 *
 * See [method@Breakpoint.add_setters].
 *
 * Since: 1.4
 */
void
adw_breakpoint_add_setters_valist (AdwBreakpoint *self,
                                   GObject       *first_object,
                                   const char    *first_property,
                                   va_list        args)
{
  GObject *object = first_object;
  const char *property = first_property;

  g_return_if_fail (ADW_IS_BREAKPOINT (self));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (property != NULL);

  while (object && property) {
    GValue value = G_VALUE_INIT;
    GParamSpec *pspec;
    char *error;

    pspec = find_pspec (object, property);

    if (!pspec)
      break;

    G_VALUE_COLLECT_INIT (&value, pspec->value_type, args,
                          G_VALUE_NOCOPY_CONTENTS, &error);

    if (error) {
      g_warning ("%s: %s", G_STRLOC, error);
      g_free (error);
      break;
    }

    adw_breakpoint_add_setter (self, object, property, &value);

    g_value_unset (&value);

    object = va_arg (args, GObject *);

    if (object)
      property = va_arg (args, const char *);
  }
}

void
adw_breakpoint_transition (AdwBreakpoint *from,
                           AdwBreakpoint *to)
{
  GHashTableIter iter;
  SetterData *setter;

  g_assert (!from || ADW_IS_BREAKPOINT (from));
  g_assert (!from || from->active);
  g_assert (!to || ADW_IS_BREAKPOINT (to));
  g_assert (!to || !to->active);

  if (from) {
    g_signal_emit (from, signals[SIGNAL_UNAPPLY], 0);
    from->active = FALSE;

    g_hash_table_iter_init (&iter, from->setters);

    while (g_hash_table_iter_next (&iter, NULL, (gpointer) &setter)) {
      /* Don't unset the property if we'll immediately set it again afterwards */
      if (to && g_hash_table_contains (to->setters, setter))
        continue;

      g_object_set_property (setter->object,
                             setter->pspec->name,
                             &setter->original_value);
    }
  }

  if (to) {
    g_hash_table_iter_init (&iter, to->setters);

    while (g_hash_table_iter_next (&iter, NULL, (gpointer) &setter)) {
      g_object_set_property (setter->object,
                             setter->pspec->name,
                             &setter->value);
    }

    to->active = TRUE;
    g_signal_emit (to, signals[SIGNAL_APPLY], 0);
  }
}

gboolean
adw_breakpoint_check_condition (AdwBreakpoint *self,
                                GtkSettings   *settings,
                                int            width,
                                int            height)
{
  g_assert (ADW_IS_BREAKPOINT (self));

  if (!self->condition)
    return FALSE;

  return check_condition (self->condition, settings, width, height);
}
