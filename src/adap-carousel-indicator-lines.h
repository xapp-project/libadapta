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

#define ADW_TYPE_CAROUSEL_INDICATOR_LINES (adw_carousel_indicator_lines_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwCarouselIndicatorLines, adw_carousel_indicator_lines, ADW, CAROUSEL_INDICATOR_LINES, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_carousel_indicator_lines_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwCarousel *adw_carousel_indicator_lines_get_carousel (AdwCarouselIndicatorLines *self);
ADW_AVAILABLE_IN_ALL
void         adw_carousel_indicator_lines_set_carousel (AdwCarouselIndicatorLines *self,
                                                        AdwCarousel               *carousel);

G_END_DECLS
