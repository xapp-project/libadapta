/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-length-unit.h"

G_BEGIN_DECLS

#define ADAP_TYPE_BREAKPOINT_CONDITION (adap_breakpoint_condition_get_type ())

typedef enum {
  ADAP_BREAKPOINT_CONDITION_MIN_WIDTH,
  ADAP_BREAKPOINT_CONDITION_MAX_WIDTH,
  ADAP_BREAKPOINT_CONDITION_MIN_HEIGHT,
  ADAP_BREAKPOINT_CONDITION_MAX_HEIGHT,
} AdapBreakpointConditionLengthType;

typedef enum {
  ADAP_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
  ADAP_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO,
} AdapBreakpointConditionRatioType;

typedef struct _AdapBreakpointCondition AdapBreakpointCondition;

ADAP_AVAILABLE_IN_1_4
GType adap_breakpoint_condition_get_type (void) G_GNUC_CONST;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_new_length (AdapBreakpointConditionLengthType type,
                                                             double                           value,
                                                             AdapLengthUnit                    unit) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_new_ratio (AdapBreakpointConditionRatioType type,
                                                            int                             width,
                                                            int                             height) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_new_and (AdapBreakpointCondition *condition_1,
                                                          AdapBreakpointCondition *condition_2) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_new_or (AdapBreakpointCondition *condition_1,
                                                         AdapBreakpointCondition *condition_2) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_copy (AdapBreakpointCondition *self);
ADAP_AVAILABLE_IN_1_4
void                    adap_breakpoint_condition_free (AdapBreakpointCondition *self);

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_condition_parse (const char *str);

ADAP_AVAILABLE_IN_1_4
char *adap_breakpoint_condition_to_string (AdapBreakpointCondition *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdapBreakpointCondition, adap_breakpoint_condition_free)

#define ADAP_TYPE_BREAKPOINT (adap_breakpoint_get_type())

ADAP_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdapBreakpoint, adap_breakpoint, ADAP, BREAKPOINT, GObject)

ADAP_AVAILABLE_IN_1_4
AdapBreakpoint *adap_breakpoint_new (AdapBreakpointCondition *condition) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapBreakpointCondition *adap_breakpoint_get_condition (AdapBreakpoint          *self);
ADAP_AVAILABLE_IN_1_4
void                    adap_breakpoint_set_condition (AdapBreakpoint          *self,
                                                      AdapBreakpointCondition *condition);

ADAP_AVAILABLE_IN_1_4
void adap_breakpoint_add_setter (AdapBreakpoint *self,
                                GObject       *object,
                                const char    *property,
                                const GValue  *value);

ADAP_AVAILABLE_IN_1_4
void adap_breakpoint_add_setters        (AdapBreakpoint *self,
                                        GObject       *first_object,
                                        const char    *first_property,
                                        ...) G_GNUC_NULL_TERMINATED;
ADAP_AVAILABLE_IN_1_4
void adap_breakpoint_add_settersv       (AdapBreakpoint *self,
                                        int            n_setters,
                                        GObject       *objects[],
                                        const char    *names[],
                                        const GValue  *values[]);
ADAP_AVAILABLE_IN_1_4
void adap_breakpoint_add_setters_valist (AdapBreakpoint *self,
                                        GObject       *first_object,
                                        const char    *first_property,
                                        va_list        args);

G_END_DECLS
