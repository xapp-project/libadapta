/*
 * Copyright (C) 2020 Purism SPC
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
#include "adw-tab-view.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAB (adw_tab_get_type())

G_DECLARE_FINAL_TYPE (AdwTab, adw_tab, ADW, TAB, GtkWidget)

AdwTab *adw_tab_new (AdwTabView *view,
                     gboolean    pinned) G_GNUC_WARN_UNUSED_RESULT;

AdwTabPage *adw_tab_get_page (AdwTab     *self);
void        adw_tab_set_page (AdwTab     *self,
                              AdwTabPage *page);

gboolean adw_tab_get_dragging (AdwTab   *self);
void     adw_tab_set_dragging (AdwTab   *self,
                               gboolean  dragging);

gboolean adw_tab_get_inverted (AdwTab   *self);
void     adw_tab_set_inverted (AdwTab   *self,
                               gboolean  inverted);

void adw_tab_set_fully_visible (AdwTab   *self,
                                gboolean  fully_visible);

void adw_tab_setup_extra_drop_target (AdwTab        *self,
                                      GdkDragAction  actions,
                                      GType         *types,
                                      gsize          n_types);

void adw_tab_set_extra_drag_preload (AdwTab   *self,
                                     gboolean  preload);

gboolean adw_tab_can_click_at (AdwTab *self,
                               float   x,
                               float   y);

G_END_DECLS
