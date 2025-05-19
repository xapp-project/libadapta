/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-spring-params.h"

G_BEGIN_DECLS

#define ADAP_TYPE_CAROUSEL (adap_carousel_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapCarousel, adap_carousel, ADAP, CAROUSEL, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_carousel_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void adap_carousel_prepend (AdapCarousel *self,
                           GtkWidget   *child);
ADAP_AVAILABLE_IN_ALL
void adap_carousel_append  (AdapCarousel *self,
                           GtkWidget   *child);
ADAP_AVAILABLE_IN_ALL
void adap_carousel_insert  (AdapCarousel *self,
                           GtkWidget   *child,
                           int          position);

ADAP_AVAILABLE_IN_ALL
void adap_carousel_reorder (AdapCarousel *self,
                           GtkWidget   *child,
                           int          position);

ADAP_AVAILABLE_IN_ALL
void adap_carousel_remove (AdapCarousel *self,
                          GtkWidget   *child);

ADAP_AVAILABLE_IN_ALL
void adap_carousel_scroll_to (AdapCarousel *self,
                             GtkWidget   *widget,
                             gboolean     animate);

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_carousel_get_nth_page (AdapCarousel *self,
                                      guint        n);
ADAP_AVAILABLE_IN_ALL
guint      adap_carousel_get_n_pages  (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
double     adap_carousel_get_position (AdapCarousel *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_carousel_get_interactive (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void     adap_carousel_set_interactive (AdapCarousel *self,
                                       gboolean     interactive);

ADAP_AVAILABLE_IN_ALL
guint adap_carousel_get_spacing (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void  adap_carousel_set_spacing (AdapCarousel *self,
                                guint        spacing);

ADAP_AVAILABLE_IN_ALL
AdapSpringParams *adap_carousel_get_scroll_params (AdapCarousel     *self);
ADAP_AVAILABLE_IN_ALL
void             adap_carousel_set_scroll_params (AdapCarousel     *self,
                                                 AdapSpringParams *params);

ADAP_AVAILABLE_IN_ALL
gboolean adap_carousel_get_allow_mouse_drag (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void     adap_carousel_set_allow_mouse_drag (AdapCarousel *self,
                                            gboolean     allow_mouse_drag);

ADAP_AVAILABLE_IN_ALL
gboolean adap_carousel_get_allow_scroll_wheel (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void     adap_carousel_set_allow_scroll_wheel (AdapCarousel *self,
                                              gboolean     allow_scroll_wheel);

ADAP_AVAILABLE_IN_ALL
gboolean adap_carousel_get_allow_long_swipes (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void     adap_carousel_set_allow_long_swipes (AdapCarousel *self,
                                             gboolean     allow_long_swipes);

ADAP_AVAILABLE_IN_ALL
guint adap_carousel_get_reveal_duration (AdapCarousel *self);
ADAP_AVAILABLE_IN_ALL
void  adap_carousel_set_reveal_duration (AdapCarousel *self,
                                        guint        reveal_duration);
G_END_DECLS
