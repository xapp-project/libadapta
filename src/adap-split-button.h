/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_SPLIT_BUTTON (adw_split_button_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwSplitButton, adw_split_button, ADW, SPLIT_BUTTON, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_split_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_split_button_get_label (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void        adw_split_button_set_label (AdwSplitButton *self,
                                        const char     *label);

ADW_AVAILABLE_IN_ALL
gboolean adw_split_button_get_use_underline (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void     adw_split_button_set_use_underline (AdwSplitButton *self,
                                             gboolean        use_underline);

ADW_AVAILABLE_IN_ALL
const char *adw_split_button_get_icon_name (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void        adw_split_button_set_icon_name (AdwSplitButton *self,
                                            const char     *icon_name);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_split_button_get_child (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void       adw_split_button_set_child (AdwSplitButton *self,
                                       GtkWidget      *child);

ADW_AVAILABLE_IN_1_4
gboolean adw_split_button_get_can_shrink (AdwSplitButton *self);
ADW_AVAILABLE_IN_1_4
void     adw_split_button_set_can_shrink (AdwSplitButton *self,
                                          gboolean        can_shrink);

ADW_AVAILABLE_IN_ALL
GMenuModel *adw_split_button_get_menu_model (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void        adw_split_button_set_menu_model (AdwSplitButton *self,
                                             GMenuModel     *menu_model);

ADW_AVAILABLE_IN_ALL
GtkPopover *adw_split_button_get_popover (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void        adw_split_button_set_popover (AdwSplitButton *self,
                                          GtkPopover     *popover);

ADW_AVAILABLE_IN_ALL
GtkArrowType adw_split_button_get_direction (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void         adw_split_button_set_direction (AdwSplitButton *self,
                                             GtkArrowType    direction);

ADW_AVAILABLE_IN_1_2
const char *adw_split_button_get_dropdown_tooltip (AdwSplitButton *self);
ADW_AVAILABLE_IN_1_2
void        adw_split_button_set_dropdown_tooltip (AdwSplitButton *self,
                                                   const char     *tooltip);

ADW_AVAILABLE_IN_ALL
void adw_split_button_popup (AdwSplitButton *self);
ADW_AVAILABLE_IN_ALL
void adw_split_button_popdown (AdwSplitButton *self);

G_END_DECLS
