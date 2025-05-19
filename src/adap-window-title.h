/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_WINDOW_TITLE (adap_window_title_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapWindowTitle, adap_window_title, ADAP, WINDOW_TITLE, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_window_title_new (const char *title,
                                 const char *subtitle) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_window_title_get_title (AdapWindowTitle *self);
ADAP_AVAILABLE_IN_ALL
void        adap_window_title_set_title (AdapWindowTitle *self,
                                        const char     *title);

ADAP_AVAILABLE_IN_ALL
const char *adap_window_title_get_subtitle (AdapWindowTitle *self);
ADAP_AVAILABLE_IN_ALL
void        adap_window_title_set_subtitle (AdapWindowTitle *self,
                                           const char     *subtitle);

G_END_DECLS
