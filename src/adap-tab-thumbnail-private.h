/*
 * Copyright (C) 2022 Purism SPC
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

#include "adap-tab-view.h"

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_THUMBNAIL (adap_tab_thumbnail_get_type())

G_DECLARE_FINAL_TYPE (AdapTabThumbnail, adap_tab_thumbnail, ADAP, TAB_THUMBNAIL, GtkWidget)

AdapTabThumbnail *adap_tab_thumbnail_new (AdapTabView *view,
                                        gboolean    pinned) G_GNUC_WARN_UNUSED_RESULT;

AdapTabPage *adap_tab_thumbnail_get_page (AdapTabThumbnail *self);
void        adap_tab_thumbnail_set_page (AdapTabThumbnail *self,
                                        AdapTabPage      *page);

gboolean adap_tab_thumbnail_get_inverted (AdapTabThumbnail *self);
void     adap_tab_thumbnail_set_inverted (AdapTabThumbnail *self,
                                         gboolean         inverted);

void adap_tab_thumbnail_setup_extra_drop_target (AdapTabThumbnail *self,
                                                GdkDragAction    actions,
                                                GType           *types,
                                                gsize            n_types);

void adap_tab_thumbnail_set_extra_drag_preload (AdapTabThumbnail *self,
                                               gboolean         preload);

GtkWidget *adap_tab_thumbnail_get_thumbnail (AdapTabThumbnail *self);

void adap_tab_thumbnail_fade_out (AdapTabThumbnail *self);
void adap_tab_thumbnail_fade_in  (AdapTabThumbnail *self);

G_END_DECLS
