/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_STATUS_PAGE (adap_status_page_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapStatusPage, adap_status_page, ADAP, STATUS_PAGE, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_status_page_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_status_page_get_icon_name (AdapStatusPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_status_page_set_icon_name (AdapStatusPage *self,
                                           const char    *icon_name);

ADAP_AVAILABLE_IN_ALL
GdkPaintable *adap_status_page_get_paintable (AdapStatusPage *self);
ADAP_AVAILABLE_IN_ALL
void          adap_status_page_set_paintable (AdapStatusPage *self,
                                             GdkPaintable  *paintable);

ADAP_AVAILABLE_IN_ALL
const char *adap_status_page_get_title (AdapStatusPage *self);
ADAP_AVAILABLE_IN_ALL
void        adap_status_page_set_title (AdapStatusPage *self,
                                       const char    *title);

ADAP_AVAILABLE_IN_ALL
const char      *adap_status_page_get_description (AdapStatusPage *self);
ADAP_AVAILABLE_IN_ALL
void             adap_status_page_set_description (AdapStatusPage *self,
                                                  const char    *description);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_status_page_get_child (AdapStatusPage *self);
ADAP_AVAILABLE_IN_ALL
void       adap_status_page_set_child (AdapStatusPage *self,
                                      GtkWidget     *child);

G_END_DECLS
