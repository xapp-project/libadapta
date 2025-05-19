/*
 * Copyright (C) 2020 Purism SPC
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

typedef enum /*< flags >*/ {
  ADAP_TAB_VIEW_SHORTCUT_NONE                    = 0,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_TAB             = 1 << 0,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB       = 1 << 1,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP         = 1 << 2,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN       = 1 << 3,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_HOME            = 1 << 4,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_END             = 1 << 5,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP   = 1 << 6,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN = 1 << 7,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME      = 1 << 8,
  ADAP_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END       = 1 << 9,
  ADAP_TAB_VIEW_SHORTCUT_ALT_DIGITS              = 1 << 10,
  ADAP_TAB_VIEW_SHORTCUT_ALT_ZERO                = 1 << 11,
  ADAP_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS           = 0xFFF
} AdapTabViewShortcuts;

#define ADAP_TYPE_TAB_PAGE (adap_tab_page_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapTabPage, adap_tab_page, ADAP, TAB_PAGE, GObject)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_tab_page_get_child (AdapTabPage *self);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_page_get_parent (AdapTabPage *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_page_get_selected (AdapTabPage *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_page_get_pinned (AdapTabPage *self);

ADAP_AVAILABLE_IN_ALL
const char *adap_tab_page_get_title (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_tab_page_set_title (AdapTabPage *self,
                                    const char *title);

ADAP_AVAILABLE_IN_ALL
const char *adap_tab_page_get_tooltip (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_tab_page_set_tooltip (AdapTabPage *self,
                                      const char *tooltip);

ADAP_AVAILABLE_IN_ALL
GIcon *adap_tab_page_get_icon (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void   adap_tab_page_set_icon (AdapTabPage *self,
                              GIcon      *icon);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_page_get_loading (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_page_set_loading (AdapTabPage *self,
                                   gboolean    loading);

ADAP_AVAILABLE_IN_ALL
GIcon *adap_tab_page_get_indicator_icon (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void   adap_tab_page_set_indicator_icon (AdapTabPage *self,
                                        GIcon      *indicator_icon);

ADAP_AVAILABLE_IN_1_2
const char *adap_tab_page_get_indicator_tooltip (AdapTabPage *self);
ADAP_AVAILABLE_IN_1_2
void        adap_tab_page_set_indicator_tooltip (AdapTabPage *self,
                                                const char *tooltip);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_page_get_indicator_activatable (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_page_set_indicator_activatable (AdapTabPage *self,
                                                 gboolean    activatable);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_page_get_needs_attention (AdapTabPage *self);
ADAP_AVAILABLE_IN_ALL
void     adap_tab_page_set_needs_attention (AdapTabPage *self,
                                           gboolean    needs_attention);

ADAP_AVAILABLE_IN_1_3
const char *adap_tab_page_get_keyword (AdapTabPage *self);
ADAP_AVAILABLE_IN_1_3
void        adap_tab_page_set_keyword (AdapTabPage *self,
                                      const char *keyword);

ADAP_AVAILABLE_IN_1_3
float adap_tab_page_get_thumbnail_xalign (AdapTabPage *self);
ADAP_AVAILABLE_IN_1_3
void  adap_tab_page_set_thumbnail_xalign (AdapTabPage *self,
                                         float       xalign);

ADAP_AVAILABLE_IN_1_3
float adap_tab_page_get_thumbnail_yalign (AdapTabPage *self);
ADAP_AVAILABLE_IN_1_3
void  adap_tab_page_set_thumbnail_yalign (AdapTabPage *self,
                                         float       yalign);

ADAP_AVAILABLE_IN_1_3
gboolean adap_tab_page_get_live_thumbnail (AdapTabPage *self);
ADAP_AVAILABLE_IN_1_3
void     adap_tab_page_set_live_thumbnail (AdapTabPage *self,
                                          gboolean    live_thumbnail);

ADAP_AVAILABLE_IN_1_3
void adap_tab_page_invalidate_thumbnail (AdapTabPage *self);

#define ADAP_TYPE_TAB_VIEW (adap_tab_view_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapTabView, adap_tab_view, ADAP, TAB_VIEW, GtkWidget)

ADAP_AVAILABLE_IN_ALL
AdapTabView *adap_tab_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
int adap_tab_view_get_n_pages        (AdapTabView *self);
ADAP_AVAILABLE_IN_ALL
int adap_tab_view_get_n_pinned_pages (AdapTabView *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_get_is_transferring_page (AdapTabView *self);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_get_selected_page (AdapTabView *self);
ADAP_AVAILABLE_IN_ALL
void        adap_tab_view_set_selected_page (AdapTabView *self,
                                            AdapTabPage *selected_page);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_select_previous_page (AdapTabView *self);
ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_select_next_page     (AdapTabView *self);

ADAP_AVAILABLE_IN_ALL
GIcon *adap_tab_view_get_default_icon (AdapTabView *self);
ADAP_AVAILABLE_IN_ALL
void   adap_tab_view_set_default_icon (AdapTabView *self,
                                      GIcon      *default_icon);

ADAP_AVAILABLE_IN_ALL
GMenuModel *adap_tab_view_get_menu_model (AdapTabView *self);
ADAP_AVAILABLE_IN_ALL
void        adap_tab_view_set_menu_model (AdapTabView *self,
                                         GMenuModel *menu_model);

ADAP_AVAILABLE_IN_1_2
AdapTabViewShortcuts adap_tab_view_get_shortcuts    (AdapTabView          *self);
ADAP_AVAILABLE_IN_1_2
void                adap_tab_view_set_shortcuts    (AdapTabView          *self,
                                                   AdapTabViewShortcuts  shortcuts);
ADAP_AVAILABLE_IN_1_2
void                adap_tab_view_add_shortcuts    (AdapTabView          *self,
                                                   AdapTabViewShortcuts  shortcuts);
ADAP_AVAILABLE_IN_1_2
void                adap_tab_view_remove_shortcuts (AdapTabView          *self,
                                                   AdapTabViewShortcuts  shortcuts);

ADAP_AVAILABLE_IN_ALL
void adap_tab_view_set_page_pinned (AdapTabView *self,
                                   AdapTabPage *page,
                                   gboolean    pinned);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_get_page (AdapTabView *self,
                                   GtkWidget  *child);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_get_nth_page (AdapTabView *self,
                                       int         position);

ADAP_AVAILABLE_IN_ALL
int adap_tab_view_get_page_position (AdapTabView *self,
                                    AdapTabPage *page);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_add_page (AdapTabView *self,
                                   GtkWidget  *child,
                                   AdapTabPage *parent);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_insert  (AdapTabView *self,
                                  GtkWidget  *child,
                                  int         position);
ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_prepend (AdapTabView *self,
                                  GtkWidget  *child);
ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_append  (AdapTabView *self,
                                  GtkWidget  *child);

ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_insert_pinned  (AdapTabView *self,
                                         GtkWidget  *child,
                                         int         position);
ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_prepend_pinned (AdapTabView *self,
                                         GtkWidget  *child);
ADAP_AVAILABLE_IN_ALL
AdapTabPage *adap_tab_view_append_pinned  (AdapTabView *self,
                                         GtkWidget  *child);

ADAP_AVAILABLE_IN_ALL
void adap_tab_view_close_page        (AdapTabView *self,
                                     AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
void adap_tab_view_close_page_finish (AdapTabView *self,
                                     AdapTabPage *page,
                                     gboolean    confirm);

ADAP_AVAILABLE_IN_ALL
void adap_tab_view_close_other_pages  (AdapTabView *self,
                                      AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
void adap_tab_view_close_pages_before (AdapTabView *self,
                                      AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
void adap_tab_view_close_pages_after  (AdapTabView *self,
                                      AdapTabPage *page);

ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_reorder_page     (AdapTabView *self,
                                        AdapTabPage *page,
                                        int         position);
ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_reorder_backward (AdapTabView *self,
                                        AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_reorder_forward  (AdapTabView *self,
                                        AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_reorder_first    (AdapTabView *self,
                                        AdapTabPage *page);
ADAP_AVAILABLE_IN_ALL
gboolean adap_tab_view_reorder_last     (AdapTabView *self,
                                        AdapTabPage *page);

ADAP_AVAILABLE_IN_ALL
void adap_tab_view_transfer_page (AdapTabView *self,
                                 AdapTabPage *page,
                                 AdapTabView *other_view,
                                 int         position);

ADAP_AVAILABLE_IN_ALL
GtkSelectionModel *adap_tab_view_get_pages (AdapTabView *self) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_3
void adap_tab_view_invalidate_thumbnails (AdapTabView *self);

G_END_DECLS
