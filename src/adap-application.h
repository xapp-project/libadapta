/*
 * Copyright (C) 2021 Nahuel Gomez Castro
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-style-manager.h"

G_BEGIN_DECLS

#define ADAP_TYPE_APPLICATION (adap_application_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapApplication, adap_application, ADAP, APPLICATION, GtkApplication)

/**
 * AdapApplicationClass:
 * @parent_class: The parent class
 */
struct _AdapApplicationClass
{
  GtkApplicationClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
AdapApplication *adap_application_new (const char        *application_id,
                                     GApplicationFlags  flags) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapStyleManager *adap_application_get_style_manager (AdapApplication *self);

G_END_DECLS
