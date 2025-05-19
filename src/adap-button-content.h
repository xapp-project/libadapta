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

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_BUTTON_CONTENT (adap_button_content_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapButtonContent, adap_button_content, ADAP, BUTTON_CONTENT, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_button_content_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_button_content_get_label (AdapButtonContent  *self);
ADAP_AVAILABLE_IN_ALL
void        adap_button_content_set_label (AdapButtonContent *self,
                                          const char       *label);

ADAP_AVAILABLE_IN_ALL
const char *adap_button_content_get_icon_name (AdapButtonContent  *self);
ADAP_AVAILABLE_IN_ALL
void        adap_button_content_set_icon_name (AdapButtonContent *self,
                                              const char       *icon_name);

ADAP_AVAILABLE_IN_ALL
gboolean adap_button_content_get_use_underline (AdapButtonContent *self);
ADAP_AVAILABLE_IN_ALL
void     adap_button_content_set_use_underline (AdapButtonContent *self,
                                               gboolean          use_underline);

ADAP_AVAILABLE_IN_1_4
gboolean adap_button_content_get_can_shrink (AdapButtonContent *self);
ADAP_AVAILABLE_IN_1_4
void     adap_button_content_set_can_shrink (AdapButtonContent *self,
                                            gboolean          can_shrink);

G_END_DECLS
