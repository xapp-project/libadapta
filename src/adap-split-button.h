/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_SPLIT_BUTTON (adap_split_button_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapSplitButton, adap_split_button, ADAP, SPLIT_BUTTON, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_split_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_split_button_get_label (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void        adap_split_button_set_label (AdapSplitButton *self,
                                        const char     *label);

ADAP_AVAILABLE_IN_ALL
gboolean adap_split_button_get_use_underline (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void     adap_split_button_set_use_underline (AdapSplitButton *self,
                                             gboolean        use_underline);

ADAP_AVAILABLE_IN_ALL
const char *adap_split_button_get_icon_name (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void        adap_split_button_set_icon_name (AdapSplitButton *self,
                                            const char     *icon_name);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_split_button_get_child (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void       adap_split_button_set_child (AdapSplitButton *self,
                                       GtkWidget      *child);

ADAP_AVAILABLE_IN_1_4
gboolean adap_split_button_get_can_shrink (AdapSplitButton *self);
ADAP_AVAILABLE_IN_1_4
void     adap_split_button_set_can_shrink (AdapSplitButton *self,
                                          gboolean        can_shrink);

ADAP_AVAILABLE_IN_ALL
GMenuModel *adap_split_button_get_menu_model (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void        adap_split_button_set_menu_model (AdapSplitButton *self,
                                             GMenuModel     *menu_model);

ADAP_AVAILABLE_IN_ALL
GtkPopover *adap_split_button_get_popover (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void        adap_split_button_set_popover (AdapSplitButton *self,
                                          GtkPopover     *popover);

ADAP_AVAILABLE_IN_ALL
GtkArrowType adap_split_button_get_direction (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void         adap_split_button_set_direction (AdapSplitButton *self,
                                             GtkArrowType    direction);

ADAP_AVAILABLE_IN_1_2
const char *adap_split_button_get_dropdown_tooltip (AdapSplitButton *self);
ADAP_AVAILABLE_IN_1_2
void        adap_split_button_set_dropdown_tooltip (AdapSplitButton *self,
                                                   const char     *tooltip);

ADAP_AVAILABLE_IN_ALL
void adap_split_button_popup (AdapSplitButton *self);
ADAP_AVAILABLE_IN_ALL
void adap_split_button_popdown (AdapSplitButton *self);

G_END_DECLS
