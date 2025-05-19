/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-entry-row.h"

G_BEGIN_DECLS

#define ADAP_TYPE_PASSWORD_ENTRY_ROW (adap_password_entry_row_get_type())

ADAP_AVAILABLE_IN_1_2
G_DECLARE_FINAL_TYPE (AdapPasswordEntryRow, adap_password_entry_row, ADAP, PASSWORD_ENTRY_ROW, AdapEntryRow)

ADAP_AVAILABLE_IN_1_2
GtkWidget *adap_password_entry_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
