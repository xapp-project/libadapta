/*
 * Copyright (C) 2023 Purism SPC
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
#include "adw-enums.h"

G_BEGIN_DECLS

#define ADW_TYPE_TOOLBAR_VIEW (adw_toolbar_view_get_type())

typedef enum {
  ADW_TOOLBAR_FLAT,
  ADW_TOOLBAR_RAISED,
  ADW_TOOLBAR_RAISED_BORDER,
} AdwToolbarStyle;

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwToolbarView, adw_toolbar_view, ADW, TOOLBAR_VIEW, GtkWidget)

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_toolbar_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_toolbar_view_get_content (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
void       adw_toolbar_view_set_content (AdwToolbarView *self,
                                         GtkWidget      *content);

ADW_AVAILABLE_IN_1_4
void adw_toolbar_view_add_top_bar (AdwToolbarView *self,
                                   GtkWidget      *widget);

ADW_AVAILABLE_IN_1_4
void adw_toolbar_view_add_bottom_bar (AdwToolbarView *self,
                                      GtkWidget      *widget);

ADW_AVAILABLE_IN_1_4
void adw_toolbar_view_remove (AdwToolbarView *self,
                              GtkWidget      *widget);

ADW_AVAILABLE_IN_1_4
AdwToolbarStyle adw_toolbar_view_get_top_bar_style (AdwToolbarView  *self);
ADW_AVAILABLE_IN_1_4
void            adw_toolbar_view_set_top_bar_style (AdwToolbarView  *self,
                                                    AdwToolbarStyle  style);

ADW_AVAILABLE_IN_1_4
AdwToolbarStyle adw_toolbar_view_get_bottom_bar_style (AdwToolbarView  *self);
ADW_AVAILABLE_IN_1_4
void            adw_toolbar_view_set_bottom_bar_style (AdwToolbarView  *self,
                                                       AdwToolbarStyle  style);

ADW_AVAILABLE_IN_1_4
gboolean adw_toolbar_view_get_reveal_top_bars (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
void     adw_toolbar_view_set_reveal_top_bars (AdwToolbarView *self,
                                               gboolean        reveal);

ADW_AVAILABLE_IN_1_4
gboolean adw_toolbar_view_get_reveal_bottom_bars (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
void     adw_toolbar_view_set_reveal_bottom_bars (AdwToolbarView *self,
                                                  gboolean        reveal);

ADW_AVAILABLE_IN_1_4
gboolean adw_toolbar_view_get_extend_content_to_top_edge (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
void     adw_toolbar_view_set_extend_content_to_top_edge (AdwToolbarView *self,
                                                          gboolean        extend);

ADW_AVAILABLE_IN_1_4
gboolean adw_toolbar_view_get_extend_content_to_bottom_edge (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
void     adw_toolbar_view_set_extend_content_to_bottom_edge (AdwToolbarView *self,
                                                             gboolean        extend);

ADW_AVAILABLE_IN_1_4
int adw_toolbar_view_get_top_bar_height    (AdwToolbarView *self);
ADW_AVAILABLE_IN_1_4
int adw_toolbar_view_get_bottom_bar_height (AdwToolbarView *self);

G_END_DECLS
