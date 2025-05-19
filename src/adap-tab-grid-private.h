/*
 * Copyright (C) 2020-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "adap-tab-thumbnail-private.h"
#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_GRID (adap_tab_grid_get_type())

G_DECLARE_FINAL_TYPE (AdapTabGrid, adap_tab_grid, ADAP, TAB_GRID, GtkWidget)

void adap_tab_grid_set_view (AdapTabGrid *self,
                            AdapTabView *view);

void adap_tab_grid_attach_page (AdapTabGrid *self,
                               AdapTabPage *page,
                               int         position);
void adap_tab_grid_detach_page (AdapTabGrid *self,
                               AdapTabPage *page);
void adap_tab_grid_select_page (AdapTabGrid *self,
                               AdapTabPage *page);

void adap_tab_grid_try_focus_selected_tab (AdapTabGrid *self,
                                          gboolean    animate);
gboolean adap_tab_grid_is_page_focused    (AdapTabGrid *self,
                                          AdapTabPage *page);

void adap_tab_grid_setup_extra_drop_target (AdapTabGrid    *self,
                                           GdkDragAction  actions,
                                           GType         *types,
                                           gsize          n_types);

gboolean adap_tab_grid_get_extra_drag_preload (AdapTabGrid *self);
void     adap_tab_grid_set_extra_drag_preload (AdapTabGrid *self,
                                              gboolean    preload);

gboolean adap_tab_grid_get_inverted (AdapTabGrid *self);
void     adap_tab_grid_set_inverted (AdapTabGrid *self,
                                    gboolean    inverted);

AdapTabThumbnail *adap_tab_grid_get_transition_thumbnail (AdapTabGrid *self);

void adap_tab_grid_set_visible_range (AdapTabGrid *self,
                                     double      lower,
                                     double      upper,
                                     double      page_size,
                                     double      lower_inset,
                                     double      upper_inset);

void adap_tab_grid_adjustment_shifted (AdapTabGrid *self,
                                      double      delta);

double adap_tab_grid_get_scrolled_tab_y (AdapTabGrid *self);

void adap_tab_grid_reset_scrolled_tab (AdapTabGrid *self);

void adap_tab_grid_scroll_to_page (AdapTabGrid *self,
                                  AdapTabPage *page,
                                  gboolean    animate);

void adap_tab_grid_set_hovering (AdapTabGrid *self,
                                gboolean    hovering);

void adap_tab_grid_set_search_terms (AdapTabGrid *self,
                                    const char *terms);

gboolean adap_tab_grid_get_empty (AdapTabGrid *self);

gboolean adap_tab_grid_focus_first_row (AdapTabGrid *self,
                                       int         column);
gboolean adap_tab_grid_focus_last_row  (AdapTabGrid *self,
                                       int         column);

void adap_tab_grid_focus_page (AdapTabGrid *self,
                              AdapTabPage *page);

int adap_tab_grid_measure_height_final (AdapTabGrid *self,
                                       int         for_width);

G_END_DECLS
