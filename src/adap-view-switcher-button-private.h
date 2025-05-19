/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_VIEW_SWITCHER_BUTTON (adap_view_switcher_button_get_type())

G_DECLARE_FINAL_TYPE (AdapViewSwitcherButton, adap_view_switcher_button, ADAP, VIEW_SWITCHER_BUTTON, GtkToggleButton)

GtkWidget  *adap_view_switcher_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

const char *adap_view_switcher_button_get_icon_name (AdapViewSwitcherButton *self);
void        adap_view_switcher_button_set_icon_name (AdapViewSwitcherButton *self,
                                                    const char            *icon_name);

GtkIconSize adap_view_switcher_button_get_icon_size (AdapViewSwitcherButton *self);
void        adap_view_switcher_button_set_icon_size (AdapViewSwitcherButton *self,
                                                    GtkIconSize            icon_size);

gboolean adap_view_switcher_button_get_needs_attention (AdapViewSwitcherButton *self);
void     adap_view_switcher_button_set_needs_attention (AdapViewSwitcherButton *self,
                                                       gboolean               needs_attention);

guint adap_view_switcher_button_get_badge_number (AdapViewSwitcherButton *self);
void  adap_view_switcher_button_set_badge_number (AdapViewSwitcherButton *self,
                                                 guint                  badge_number);

const char *adap_view_switcher_button_get_label (AdapViewSwitcherButton *self);
void        adap_view_switcher_button_set_label (AdapViewSwitcherButton *self,
                                                const char            *label);

G_END_DECLS
