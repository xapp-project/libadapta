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

#define ADAP_TYPE_INSPECTOR_PAGE (adap_inspector_page_get_type())

G_DECLARE_FINAL_TYPE (AdapInspectorPage, adap_inspector_page, ADAP, INSPECTOR_PAGE, GtkWidget)

G_END_DECLS
