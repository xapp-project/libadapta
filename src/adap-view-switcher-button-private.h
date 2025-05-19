/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_BUTTON (adw_view_switcher_button_get_type())

G_DECLARE_FINAL_TYPE (AdwViewSwitcherButton, adw_view_switcher_button, ADW, VIEW_SWITCHER_BUTTON, GtkToggleButton)

GtkWidget  *adw_view_switcher_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

const char *adw_view_switcher_button_get_icon_name (AdwViewSwitcherButton *self);
void        adw_view_switcher_button_set_icon_name (AdwViewSwitcherButton *self,
                                                    const char            *icon_name);

GtkIconSize adw_view_switcher_button_get_icon_size (AdwViewSwitcherButton *self);
void        adw_view_switcher_button_set_icon_size (AdwViewSwitcherButton *self,
                                                    GtkIconSize            icon_size);

gboolean adw_view_switcher_button_get_needs_attention (AdwViewSwitcherButton *self);
void     adw_view_switcher_button_set_needs_attention (AdwViewSwitcherButton *self,
                                                       gboolean               needs_attention);

guint adw_view_switcher_button_get_badge_number (AdwViewSwitcherButton *self);
void  adw_view_switcher_button_set_badge_number (AdwViewSwitcherButton *self,
                                                 guint                  badge_number);

const char *adw_view_switcher_button_get_label (AdwViewSwitcherButton *self);
void        adw_view_switcher_button_set_label (AdwViewSwitcherButton *self,
                                                const char            *label);

G_END_DECLS
