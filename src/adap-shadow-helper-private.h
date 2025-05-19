/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_SHADOW_HELPER (adap_shadow_helper_get_type())

G_DECLARE_FINAL_TYPE (AdapShadowHelper, adap_shadow_helper, ADAP, SHADOW_HELPER, GObject)

AdapShadowHelper *adap_shadow_helper_new (GtkWidget *widget) G_GNUC_WARN_UNUSED_RESULT;

void adap_shadow_helper_size_allocate (AdapShadowHelper *self,
                                      int              width,
                                      int              height,
                                      int              baseline,
                                      int              x,
                                      int              y,
                                      double           progress,
                                      GtkPanDirection  direction);

void adap_shadow_helper_snapshot (AdapShadowHelper *self,
                                 GtkSnapshot     *snapshot);

G_END_DECLS
