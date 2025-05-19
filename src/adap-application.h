/*
 * Copyright (C) 2021 Nahuel Gomez Castro
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-style-manager.h"

G_BEGIN_DECLS

#define ADW_TYPE_APPLICATION (adw_application_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwApplication, adw_application, ADW, APPLICATION, GtkApplication)

/**
 * AdwApplicationClass:
 * @parent_class: The parent class
 */
struct _AdwApplicationClass
{
  GtkApplicationClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
AdwApplication *adw_application_new (const char        *application_id,
                                     GApplicationFlags  flags) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwStyleManager *adw_application_get_style_manager (AdwApplication *self);

G_END_DECLS
