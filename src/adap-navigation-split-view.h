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
#include "adw-navigation-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_get_type())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwNavigationSplitView, adw_navigation_split_view, ADW, NAVIGATION_SPLIT_VIEW, GtkWidget)

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_navigation_split_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
AdwNavigationPage *adw_navigation_split_view_get_sidebar (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void               adw_navigation_split_view_set_sidebar (AdwNavigationSplitView *self,
                                                          AdwNavigationPage      *sidebar);

ADW_AVAILABLE_IN_1_4
AdwNavigationPage *adw_navigation_split_view_get_content (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void               adw_navigation_split_view_set_content (AdwNavigationSplitView *self,
                                                          AdwNavigationPage      *content);

ADW_AVAILABLE_IN_1_4
gboolean adw_navigation_split_view_get_collapsed (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_navigation_split_view_set_collapsed (AdwNavigationSplitView *self,
                                                  gboolean                collapsed);

ADW_AVAILABLE_IN_1_4
gboolean adw_navigation_split_view_get_show_content (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void     adw_navigation_split_view_set_show_content (AdwNavigationSplitView *self,
                                                     gboolean                show_content);

ADW_AVAILABLE_IN_1_4
double adw_navigation_split_view_get_min_sidebar_width (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_navigation_split_view_set_min_sidebar_width (AdwNavigationSplitView *self,
                                                        double                  width);

ADW_AVAILABLE_IN_1_4
double adw_navigation_split_view_get_max_sidebar_width (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_navigation_split_view_set_max_sidebar_width (AdwNavigationSplitView *self,
                                                        double                  width);

ADW_AVAILABLE_IN_1_4
double adw_navigation_split_view_get_sidebar_width_fraction (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void   adw_navigation_split_view_set_sidebar_width_fraction (AdwNavigationSplitView *self,
                                                             double                  fraction);

ADW_AVAILABLE_IN_1_4
AdwLengthUnit adw_navigation_split_view_get_sidebar_width_unit (AdwNavigationSplitView *self);
ADW_AVAILABLE_IN_1_4
void          adw_navigation_split_view_set_sidebar_width_unit (AdwNavigationSplitView *self,
                                                                AdwLengthUnit           unit);

G_END_DECLS
