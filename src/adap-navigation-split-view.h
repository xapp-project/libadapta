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
#include "adap-navigation-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_NAVIGATION_SPLIT_VIEW (adap_navigation_split_view_get_type())

ADAP_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdapNavigationSplitView, adap_navigation_split_view, ADAP, NAVIGATION_SPLIT_VIEW, GtkWidget)

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_navigation_split_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
AdapNavigationPage *adap_navigation_split_view_get_sidebar (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void               adap_navigation_split_view_set_sidebar (AdapNavigationSplitView *self,
                                                          AdapNavigationPage      *sidebar);

ADAP_AVAILABLE_IN_1_4
AdapNavigationPage *adap_navigation_split_view_get_content (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void               adap_navigation_split_view_set_content (AdapNavigationSplitView *self,
                                                          AdapNavigationPage      *content);

ADAP_AVAILABLE_IN_1_4
gboolean adap_navigation_split_view_get_collapsed (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_navigation_split_view_set_collapsed (AdapNavigationSplitView *self,
                                                  gboolean                collapsed);

ADAP_AVAILABLE_IN_1_4
gboolean adap_navigation_split_view_get_show_content (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_navigation_split_view_set_show_content (AdapNavigationSplitView *self,
                                                     gboolean                show_content);

ADAP_AVAILABLE_IN_1_4
double adap_navigation_split_view_get_min_sidebar_width (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void   adap_navigation_split_view_set_min_sidebar_width (AdapNavigationSplitView *self,
                                                        double                  width);

ADAP_AVAILABLE_IN_1_4
double adap_navigation_split_view_get_max_sidebar_width (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void   adap_navigation_split_view_set_max_sidebar_width (AdapNavigationSplitView *self,
                                                        double                  width);

ADAP_AVAILABLE_IN_1_4
double adap_navigation_split_view_get_sidebar_width_fraction (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void   adap_navigation_split_view_set_sidebar_width_fraction (AdapNavigationSplitView *self,
                                                             double                  fraction);

ADAP_AVAILABLE_IN_1_4
AdapLengthUnit adap_navigation_split_view_get_sidebar_width_unit (AdapNavigationSplitView *self);
ADAP_AVAILABLE_IN_1_4
void          adap_navigation_split_view_set_sidebar_width_unit (AdapNavigationSplitView *self,
                                                                AdapLengthUnit           unit);

G_END_DECLS
