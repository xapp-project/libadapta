/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_BREAKPOINT_CONDITION (adw_breakpoint_condition_get_type ())

typedef enum {
  ADW_BREAKPOINT_CONDITION_MIN_WIDTH,
  ADW_BREAKPOINT_CONDITION_MAX_WIDTH,
  ADW_BREAKPOINT_CONDITION_MIN_HEIGHT,
  ADW_BREAKPOINT_CONDITION_MAX_HEIGHT,
} AdwBreakpointConditionLengthType;

typedef enum {
  ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
  ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO,
} AdwBreakpointConditionRatioType;

typedef struct _AdwBreakpointCondition AdwBreakpointCondition;

ADW_AVAILABLE_IN_1_4
GType adw_breakpoint_condition_get_type (void) G_GNUC_CONST;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_new_length (AdwBreakpointConditionLengthType type,
                                                             double                           value,
                                                             AdwLengthUnit                    unit) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_new_ratio (AdwBreakpointConditionRatioType type,
                                                            int                             width,
                                                            int                             height) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_new_and (AdwBreakpointCondition *condition_1,
                                                          AdwBreakpointCondition *condition_2) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_new_or (AdwBreakpointCondition *condition_1,
                                                         AdwBreakpointCondition *condition_2) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_copy (AdwBreakpointCondition *self);
ADW_AVAILABLE_IN_1_4
void                    adw_breakpoint_condition_free (AdwBreakpointCondition *self);

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_condition_parse (const char *str);

ADW_AVAILABLE_IN_1_4
char *adw_breakpoint_condition_to_string (AdwBreakpointCondition *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdwBreakpointCondition, adw_breakpoint_condition_free)

#define ADW_TYPE_BREAKPOINT (adw_breakpoint_get_type())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwBreakpoint, adw_breakpoint, ADW, BREAKPOINT, GObject)

ADW_AVAILABLE_IN_1_4
AdwBreakpoint *adw_breakpoint_new (AdwBreakpointCondition *condition) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwBreakpointCondition *adw_breakpoint_get_condition (AdwBreakpoint          *self);
ADW_AVAILABLE_IN_1_4
void                    adw_breakpoint_set_condition (AdwBreakpoint          *self,
                                                      AdwBreakpointCondition *condition);

ADW_AVAILABLE_IN_1_4
void adw_breakpoint_add_setter (AdwBreakpoint *self,
                                GObject       *object,
                                const char    *property,
                                const GValue  *value);

ADW_AVAILABLE_IN_1_4
void adw_breakpoint_add_setters        (AdwBreakpoint *self,
                                        GObject       *first_object,
                                        const char    *first_property,
                                        ...) G_GNUC_NULL_TERMINATED;
ADW_AVAILABLE_IN_1_4
void adw_breakpoint_add_settersv       (AdwBreakpoint *self,
                                        int            n_setters,
                                        GObject       *objects[],
                                        const char    *names[],
                                        const GValue  *values[]);
ADW_AVAILABLE_IN_1_4
void adw_breakpoint_add_setters_valist (AdwBreakpoint *self,
                                        GObject       *first_object,
                                        const char    *first_property,
                                        va_list        args);

G_END_DECLS
