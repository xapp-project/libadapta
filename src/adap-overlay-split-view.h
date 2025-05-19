/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_OVERLAY_SPLIT_VIEW (adw_overlay_split_view_get_type ())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwOverlaySplitView, adw_overlay_split_view, ADW, OVERLAY_SPLIT_VIEW, GtkWidget)

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_overlay_split_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_overlay_split_view_get_sidebar (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void       adw_overlay_split_view_set_sidebar (AdwOverlaySplitView *self,
                                               GtkWidget           *sidebar);

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_overlay_split_view_get_content (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void       adw_overlay_split_view_set_content (AdwOverlaySplitView *self,
                                               GtkWidget           *content);

ADW_AVAILABLE_IN_1_4
gboolean adw_overlay_split_view_get_collapsed (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_overlay_split_view_set_collapsed (AdwOverlaySplitView *self,
                                               gboolean             collapsed);

ADW_AVAILABLE_IN_1_4
GtkPackType adw_overlay_split_view_get_sidebar_position (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void        adw_overlay_split_view_set_sidebar_position (AdwOverlaySplitView *self,
                                                         GtkPackType          position);

ADW_AVAILABLE_IN_1_4
gboolean adw_overlay_split_view_get_show_sidebar (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_overlay_split_view_set_show_sidebar (AdwOverlaySplitView *self,
                                                  gboolean             show_sidebar);

ADW_AVAILABLE_IN_1_4
gboolean adw_overlay_split_view_get_pin_sidebar (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_overlay_split_view_set_pin_sidebar (AdwOverlaySplitView *self,
                                                 gboolean             pin_sidebar);

ADW_AVAILABLE_IN_1_4
gboolean adw_overlay_split_view_get_enable_show_gesture (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_overlay_split_view_set_enable_show_gesture (AdwOverlaySplitView *self,
                                                         gboolean             enable_show_gesture);

ADW_AVAILABLE_IN_1_4
gboolean adw_overlay_split_view_get_enable_hide_gesture (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_overlay_split_view_set_enable_hide_gesture (AdwOverlaySplitView *self,
                                                         gboolean             enable_hide_gesture);

ADW_AVAILABLE_IN_1_4
double adw_overlay_split_view_get_min_sidebar_width (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_overlay_split_view_set_min_sidebar_width (AdwOverlaySplitView *self,
                                                     double               width);

ADW_AVAILABLE_IN_1_4
double adw_overlay_split_view_get_max_sidebar_width (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_overlay_split_view_set_max_sidebar_width (AdwOverlaySplitView *self,
                                                     double               width);

ADW_AVAILABLE_IN_1_4
double adw_overlay_split_view_get_sidebar_width_fraction (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_overlay_split_view_set_sidebar_width_fraction (AdwOverlaySplitView *self,
                                                          double               fraction);

ADW_AVAILABLE_IN_1_4
AdwLengthUnit adw_overlay_split_view_get_sidebar_width_unit (AdwOverlaySplitView *self);
ADW_AVAILABLE_IN_1_4
void          adw_overlay_split_view_set_sidebar_width_unit (AdwOverlaySplitView *self,
                                                             AdwLengthUnit        unit);

G_END_DECLS
