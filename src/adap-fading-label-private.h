/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_FADING_LABEL (adap_fading_label_get_type())

G_DECLARE_FINAL_TYPE (AdapFadingLabel, adap_fading_label, ADAP, FADING_LABEL, GtkWidget)

const char *adap_fading_label_get_label (AdapFadingLabel *self);
void        adap_fading_label_set_label (AdapFadingLabel *self,
                                        const char     *label);

float adap_fading_label_get_align (AdapFadingLabel *self);
void  adap_fading_label_set_align (AdapFadingLabel *self,
                                  float           align);

G_END_DECLS
