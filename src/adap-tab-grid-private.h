/*
 * Copyright (C) 2020-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "adw-tab-thumbnail-private.h"
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB_GRID (adw_tab_grid_get_type())

G_DECLARE_FINAL_TYPE (AdwTabGrid, adw_tab_grid, ADW, TAB_GRID, GtkWidget)

void adw_tab_grid_set_view (AdwTabGrid *self,
                            AdwTabView *view);

void adw_tab_grid_attach_page (AdwTabGrid *self,
                               AdwTabPage *page,
                               int         position);
void adw_tab_grid_detach_page (AdwTabGrid *self,
                               AdwTabPage *page);
void adw_tab_grid_select_page (AdwTabGrid *self,
                               AdwTabPage *page);

void adw_tab_grid_try_focus_selected_tab (AdwTabGrid *self,
                                          gboolean    animate);
gboolean adw_tab_grid_is_page_focused    (AdwTabGrid *self,
                                          AdwTabPage *page);

void adw_tab_grid_setup_extra_drop_target (AdwTabGrid    *self,
                                           GdkDragAction  actions,
                                           GType         *types,
                                           gsize          n_types);

gboolean adw_tab_grid_get_extra_drag_preload (AdwTabGrid *self);
void     adw_tab_grid_set_extra_drag_preload (AdwTabGrid *self,
                                              gboolean    preload);

gboolean adw_tab_grid_get_inverted (AdwTabGrid *self);
void     adw_tab_grid_set_inverted (AdwTabGrid *self,
                                    gboolean    inverted);

AdwTabThumbnail *adw_tab_grid_get_transition_thumbnail (AdwTabGrid *self);

void adw_tab_grid_set_visible_range (AdwTabGrid *self,
                                     double      lower,
                                     double      upper,
                                     double      page_size,
                                     double      lower_inset,
                                     double      upper_inset);

void adw_tab_grid_adjustment_shifted (AdwTabGrid *self,
                                      double      delta);

double adw_tab_grid_get_scrolled_tab_y (AdwTabGrid *self);

void adw_tab_grid_reset_scrolled_tab (AdwTabGrid *self);

void adw_tab_grid_scroll_to_page (AdwTabGrid *self,
                                  AdwTabPage *page,
                                  gboolean    animate);

void adw_tab_grid_set_hovering (AdwTabGrid *self,
                                gboolean    hovering);

void adw_tab_grid_set_search_terms (AdwTabGrid *self,
                                    const char *terms);

gboolean adw_tab_grid_get_empty (AdwTabGrid *self);

gboolean adw_tab_grid_focus_first_row (AdwTabGrid *self,
                                       int         column);
gboolean adw_tab_grid_focus_last_row  (AdwTabGrid *self,
                                       int         column);

void adw_tab_grid_focus_page (AdwTabGrid *self,
                              AdwTabPage *page);

int adw_tab_grid_measure_height_final (AdwTabGrid *self,
                                       int         for_width);

G_END_DECLS
