/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-carousel.h"

G_BEGIN_DECLS

#define ADAP_TYPE_CAROUSEL_INDICATOR_DOTS (adap_carousel_indicator_dots_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapCarouselIndicatorDots, adap_carousel_indicator_dots, ADAP, CAROUSEL_INDICATOR_DOTS, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_carousel_indicator_dots_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapCarousel *adap_carousel_indicator_dots_get_carousel (AdapCarouselIndicatorDots *self);
ADAP_AVAILABLE_IN_ALL
void         adap_carousel_indicator_dots_set_carousel (AdapCarouselIndicatorDots *self,
                                                       AdapCarousel              *carousel);

G_END_DECLS
