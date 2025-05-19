/*
 * Copyright (C) 2022 Purism SPC
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

#define ADW_TYPE_TAB_THUMBNAIL (adw_tab_thumbnail_get_type())

G_DECLARE_FINAL_TYPE (AdwTabThumbnail, adw_tab_thumbnail, ADW, TAB_THUMBNAIL, GtkWidget)

AdwTabThumbnail *adw_tab_thumbnail_new (AdwTabView *view,
                                        gboolean    pinned) G_GNUC_WARN_UNUSED_RESULT;

AdwTabPage *adw_tab_thumbnail_get_page (AdwTabThumbnail *self);
void        adw_tab_thumbnail_set_page (AdwTabThumbnail *self,
                                        AdwTabPage      *page);

gboolean adw_tab_thumbnail_get_inverted (AdwTabThumbnail *self);
void     adw_tab_thumbnail_set_inverted (AdwTabThumbnail *self,
                                         gboolean         inverted);

void adw_tab_thumbnail_setup_extra_drop_target (AdwTabThumbnail *self,
                                                GdkDragAction    actions,
                                                GType           *types,
                                                gsize            n_types);

void adw_tab_thumbnail_set_extra_drag_preload (AdwTabThumbnail *self,
                                               gboolean         preload);

GtkWidget *adw_tab_thumbnail_get_thumbnail (AdwTabThumbnail *self);

void adw_tab_thumbnail_fade_out (AdwTabThumbnail *self);
void adw_tab_thumbnail_fade_in  (AdwTabThumbnail *self);

G_END_DECLS
