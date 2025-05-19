/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_FADING_LABEL (adw_fading_label_get_type())

G_DECLARE_FINAL_TYPE (AdwFadingLabel, adw_fading_label, ADW, FADING_LABEL, GtkWidget)

const char *adw_fading_label_get_label (AdwFadingLabel *self);
void        adw_fading_label_set_label (AdwFadingLabel *self,
                                        const char     *label);

float adw_fading_label_get_align (AdwFadingLabel *self);
void  adw_fading_label_set_align (AdwFadingLabel *self,
                                  float           align);

G_END_DECLS
