/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-bin.h"

G_BEGIN_DECLS

#define ADW_TYPE_BACK_BUTTON (adw_back_button_get_type())

G_DECLARE_FINAL_TYPE (AdwBackButton, adw_back_button, ADW, BACK_BUTTON, AdwBin)

GtkWidget *adw_back_button_new (void);

G_END_DECLS
