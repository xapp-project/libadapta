/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-spring-params.h"

G_BEGIN_DECLS

#define ADW_TYPE_CAROUSEL (adw_carousel_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwCarousel, adw_carousel, ADW, CAROUSEL, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_carousel_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void adw_carousel_prepend (AdwCarousel *self,
                           GtkWidget   *child);
ADW_AVAILABLE_IN_ALL
void adw_carousel_append  (AdwCarousel *self,
                           GtkWidget   *child);
ADW_AVAILABLE_IN_ALL
void adw_carousel_insert  (AdwCarousel *self,
                           GtkWidget   *child,
                           int          position);

ADW_AVAILABLE_IN_ALL
void adw_carousel_reorder (AdwCarousel *self,
                           GtkWidget   *child,
                           int          position);

ADW_AVAILABLE_IN_ALL
void adw_carousel_remove (AdwCarousel *self,
                          GtkWidget   *child);

ADW_AVAILABLE_IN_ALL
void adw_carousel_scroll_to (AdwCarousel *self,
                             GtkWidget   *widget,
                             gboolean     animate);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_carousel_get_nth_page (AdwCarousel *self,
                                      guint        n);
ADW_AVAILABLE_IN_ALL
guint      adw_carousel_get_n_pages  (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
double     adw_carousel_get_position (AdwCarousel *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_carousel_get_interactive (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void     adw_carousel_set_interactive (AdwCarousel *self,
                                       gboolean     interactive);

ADW_AVAILABLE_IN_ALL
guint adw_carousel_get_spacing (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void  adw_carousel_set_spacing (AdwCarousel *self,
                                guint        spacing);

ADW_AVAILABLE_IN_ALL
AdwSpringParams *adw_carousel_get_scroll_params (AdwCarousel     *self);
ADW_AVAILABLE_IN_ALL
void             adw_carousel_set_scroll_params (AdwCarousel     *self,
                                                 AdwSpringParams *params);

ADW_AVAILABLE_IN_ALL
gboolean adw_carousel_get_allow_mouse_drag (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void     adw_carousel_set_allow_mouse_drag (AdwCarousel *self,
                                            gboolean     allow_mouse_drag);

ADW_AVAILABLE_IN_ALL
gboolean adw_carousel_get_allow_scroll_wheel (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void     adw_carousel_set_allow_scroll_wheel (AdwCarousel *self,
                                              gboolean     allow_scroll_wheel);

ADW_AVAILABLE_IN_ALL
gboolean adw_carousel_get_allow_long_swipes (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void     adw_carousel_set_allow_long_swipes (AdwCarousel *self,
                                             gboolean     allow_long_swipes);

ADW_AVAILABLE_IN_ALL
guint adw_carousel_get_reveal_duration (AdwCarousel *self);
ADW_AVAILABLE_IN_ALL
void  adw_carousel_set_reveal_duration (AdwCarousel *self,
                                        guint        reveal_duration);
G_END_DECLS
