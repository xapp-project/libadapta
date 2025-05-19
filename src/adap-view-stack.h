/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2021 Frederick Schenk
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Based on gtkstack.h
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/ba44668478b7184bec02609f292691b85a2c6cdd/gtk/gtkstack.h
 */

#pragma once

#if !defined  (_ADWAITA_INSIDE) && !defined  (ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_STACK_PAGE (adw_view_stack_page_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwViewStackPage, adw_view_stack_page, ADW, VIEW_STACK_PAGE, GObject)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_stack_page_get_child (AdwViewStackPage *self);

ADW_AVAILABLE_IN_ALL
const char *adw_view_stack_page_get_name (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_stack_page_set_name (AdwViewStackPage *self,
                                          const char       *name);

ADW_AVAILABLE_IN_ALL
const char *adw_view_stack_page_get_title (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_stack_page_set_title (AdwViewStackPage *self,
                                           const char       *title);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_stack_page_get_use_underline (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_stack_page_set_use_underline (AdwViewStackPage *self,
                                                gboolean          use_underline);

ADW_AVAILABLE_IN_ALL
const char *adw_view_stack_page_get_icon_name (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_stack_page_set_icon_name (AdwViewStackPage *self,
                                               const char       *icon_name);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_stack_page_get_needs_attention (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_stack_page_set_needs_attention (AdwViewStackPage *self,
                                                  gboolean          needs_attention);

ADW_AVAILABLE_IN_ALL
guint adw_view_stack_page_get_badge_number (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void  adw_view_stack_page_set_badge_number (AdwViewStackPage *self,
                                            guint             badge_number);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_stack_page_get_visible (AdwViewStackPage *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_stack_page_set_visible (AdwViewStackPage *self,
                                          gboolean          visible);

#define ADW_TYPE_VIEW_STACK (adw_view_stack_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwViewStack, adw_view_stack, ADW, VIEW_STACK, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_stack_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwViewStackPage *adw_view_stack_add       (AdwViewStack *self,
                                            GtkWidget    *child);
ADW_AVAILABLE_IN_ALL
AdwViewStackPage *adw_view_stack_add_named (AdwViewStack *self,
                                            GtkWidget    *child,
                                            const char   *name);
ADW_AVAILABLE_IN_ALL
AdwViewStackPage *adw_view_stack_add_titled (AdwViewStack *self,
                                             GtkWidget    *child,
                                             const char   *name,
                                             const char   *title);
ADW_AVAILABLE_IN_1_2
AdwViewStackPage *adw_view_stack_add_titled_with_icon (AdwViewStack *self,
                                                       GtkWidget    *child,
                                                       const char   *name,
                                                       const char   *title,
                                                       const char   *icon_name);

ADW_AVAILABLE_IN_ALL
void adw_view_stack_remove (AdwViewStack *self,
                            GtkWidget    *child);

ADW_AVAILABLE_IN_ALL
AdwViewStackPage *adw_view_stack_get_page (AdwViewStack *self,
                                           GtkWidget    *child);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_stack_get_child_by_name (AdwViewStack *self,
                                             const char   *name);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_stack_get_visible_child (AdwViewStack *self);
ADW_AVAILABLE_IN_ALL
void       adw_view_stack_set_visible_child (AdwViewStack *self,
                                             GtkWidget    *child);

ADW_AVAILABLE_IN_ALL
const char *adw_view_stack_get_visible_child_name (AdwViewStack *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_stack_set_visible_child_name (AdwViewStack *self,
                                                   const char   *name);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_stack_get_hhomogeneous (AdwViewStack *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_stack_set_hhomogeneous (AdwViewStack *self,
                                          gboolean      hhomogeneous);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_stack_get_vhomogeneous (AdwViewStack *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_stack_set_vhomogeneous (AdwViewStack *self,
                                          gboolean      vhomogeneous);

ADW_AVAILABLE_IN_ALL
GtkSelectionModel *adw_view_stack_get_pages (AdwViewStack *self);

#define ADW_TYPE_VIEW_STACK_PAGES (adw_view_stack_pages_get_type ())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwViewStackPages, adw_view_stack_pages, ADW, VIEW_STACK_PAGES, GObject)

ADW_AVAILABLE_IN_1_4
AdwViewStackPage *adw_view_stack_pages_get_selected_page (AdwViewStackPages *self);
ADW_AVAILABLE_IN_1_4
void              adw_view_stack_pages_set_selected_page (AdwViewStackPages *self,
                                                          AdwViewStackPage  *page);

G_END_DECLS
