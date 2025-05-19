/*
 * Copyright (C) 2022 Jamie Murphy <hello@itsjamie.dev>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-enums.h"

G_BEGIN_DECLS

#define ADAP_TYPE_BANNER (adap_banner_get_type())

ADAP_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdapBanner, adap_banner, ADAP, BANNER, GtkWidget)

ADAP_AVAILABLE_IN_1_3
GtkWidget *adap_banner_new (const char *title) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_3
const char  *adap_banner_get_title (AdapBanner *self);
ADAP_AVAILABLE_IN_1_3
void         adap_banner_set_title (AdapBanner  *self,
                                   const char *title);

ADAP_AVAILABLE_IN_1_3
const char  *adap_banner_get_button_label (AdapBanner *self);
ADAP_AVAILABLE_IN_1_3
void         adap_banner_set_button_label (AdapBanner  *self,
                                          const char *label);

ADAP_AVAILABLE_IN_1_3
gboolean adap_banner_get_revealed (AdapBanner *self);
ADAP_AVAILABLE_IN_1_3
void     adap_banner_set_revealed (AdapBanner *self,
                                  gboolean   revealed);

ADAP_AVAILABLE_IN_1_3
gboolean adap_banner_get_use_markup (AdapBanner *self);
ADAP_AVAILABLE_IN_1_3
void     adap_banner_set_use_markup (AdapBanner *self,
                                    gboolean   use_markup);

G_END_DECLS
