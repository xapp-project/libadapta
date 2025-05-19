/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-carousel.h"

G_BEGIN_DECLS

#define ADW_TYPE_CAROUSEL_INDICATOR_DOTS (adw_carousel_indicator_dots_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwCarouselIndicatorDots, adw_carousel_indicator_dots, ADW, CAROUSEL_INDICATOR_DOTS, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_carousel_indicator_dots_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwCarousel *adw_carousel_indicator_dots_get_carousel (AdwCarouselIndicatorDots *self);
ADW_AVAILABLE_IN_ALL
void         adw_carousel_indicator_dots_set_carousel (AdwCarouselIndicatorDots *self,
                                                       AdwCarousel              *carousel);

G_END_DECLS
