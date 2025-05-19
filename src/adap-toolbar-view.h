/*
 * Copyright (C) 2023 Purism SPC
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
#include "adap-enums.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TOOLBAR_VIEW (adap_toolbar_view_get_type())

typedef enum {
  ADAP_TOOLBAR_FLAT,
  ADAP_TOOLBAR_RAISED,
  ADAP_TOOLBAR_RAISED_BORDER,
} AdapToolbarStyle;

ADAP_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdapToolbarView, adap_toolbar_view, ADAP, TOOLBAR_VIEW, GtkWidget)

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_toolbar_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_4
GtkWidget *adap_toolbar_view_get_content (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
void       adap_toolbar_view_set_content (AdapToolbarView *self,
                                         GtkWidget      *content);

ADAP_AVAILABLE_IN_1_4
void adap_toolbar_view_add_top_bar (AdapToolbarView *self,
                                   GtkWidget      *widget);

ADAP_AVAILABLE_IN_1_4
void adap_toolbar_view_add_bottom_bar (AdapToolbarView *self,
                                      GtkWidget      *widget);

ADAP_AVAILABLE_IN_1_4
void adap_toolbar_view_remove (AdapToolbarView *self,
                              GtkWidget      *widget);

ADAP_AVAILABLE_IN_1_4
AdapToolbarStyle adap_toolbar_view_get_top_bar_style (AdapToolbarView  *self);
ADAP_AVAILABLE_IN_1_4
void            adap_toolbar_view_set_top_bar_style (AdapToolbarView  *self,
                                                    AdapToolbarStyle  style);

ADAP_AVAILABLE_IN_1_4
AdapToolbarStyle adap_toolbar_view_get_bottom_bar_style (AdapToolbarView  *self);
ADAP_AVAILABLE_IN_1_4
void            adap_toolbar_view_set_bottom_bar_style (AdapToolbarView  *self,
                                                       AdapToolbarStyle  style);

ADAP_AVAILABLE_IN_1_4
gboolean adap_toolbar_view_get_reveal_top_bars (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_toolbar_view_set_reveal_top_bars (AdapToolbarView *self,
                                               gboolean        reveal);

ADAP_AVAILABLE_IN_1_4
gboolean adap_toolbar_view_get_reveal_bottom_bars (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_toolbar_view_set_reveal_bottom_bars (AdapToolbarView *self,
                                                  gboolean        reveal);

ADAP_AVAILABLE_IN_1_4
gboolean adap_toolbar_view_get_extend_content_to_top_edge (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_toolbar_view_set_extend_content_to_top_edge (AdapToolbarView *self,
                                                          gboolean        extend);

ADAP_AVAILABLE_IN_1_4
gboolean adap_toolbar_view_get_extend_content_to_bottom_edge (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
void     adap_toolbar_view_set_extend_content_to_bottom_edge (AdapToolbarView *self,
                                                             gboolean        extend);

ADAP_AVAILABLE_IN_1_4
int adap_toolbar_view_get_top_bar_height    (AdapToolbarView *self);
ADAP_AVAILABLE_IN_1_4
int adap_toolbar_view_get_bottom_bar_height (AdapToolbarView *self);

G_END_DECLS
