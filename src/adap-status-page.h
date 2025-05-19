/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_STATUS_PAGE (adw_status_page_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwStatusPage, adw_status_page, ADW, STATUS_PAGE, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_status_page_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_status_page_get_icon_name (AdwStatusPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_status_page_set_icon_name (AdwStatusPage *self,
                                           const char    *icon_name);

ADW_AVAILABLE_IN_ALL
GdkPaintable *adw_status_page_get_paintable (AdwStatusPage *self);
ADW_AVAILABLE_IN_ALL
void          adw_status_page_set_paintable (AdwStatusPage *self,
                                             GdkPaintable  *paintable);

ADW_AVAILABLE_IN_ALL
const char *adw_status_page_get_title (AdwStatusPage *self);
ADW_AVAILABLE_IN_ALL
void        adw_status_page_set_title (AdwStatusPage *self,
                                       const char    *title);

ADW_AVAILABLE_IN_ALL
const char      *adw_status_page_get_description (AdwStatusPage *self);
ADW_AVAILABLE_IN_ALL
void             adw_status_page_set_description (AdwStatusPage *self,
                                                  const char    *description);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_status_page_get_child (AdwStatusPage *self);
ADW_AVAILABLE_IN_ALL
void       adw_status_page_set_child (AdwStatusPage *self,
                                      GtkWidget     *child);

G_END_DECLS
