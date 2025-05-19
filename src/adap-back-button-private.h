/*
 * Copyright (C) 2023 Purism SPC
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

#include "adap-bin.h"

G_BEGIN_DECLS

#define ADAP_TYPE_BACK_BUTTON (adap_back_button_get_type())

G_DECLARE_FINAL_TYPE (AdapBackButton, adap_back_button, ADAP, BACK_BUTTON, AdapBin)

GtkWidget *adap_back_button_new (void);

G_END_DECLS
