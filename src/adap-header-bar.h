/*
 * Copyright (c) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_HEADER_BAR (adw_header_bar_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwHeaderBar, adw_header_bar, ADW, HEADER_BAR, GtkWidget)

typedef enum {
  ADW_CENTERING_POLICY_LOOSE,
  ADW_CENTERING_POLICY_STRICT,
} AdwCenteringPolicy;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_header_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void adw_header_bar_pack_start (AdwHeaderBar *self,
                                GtkWidget    *child);
ADW_AVAILABLE_IN_ALL
void adw_header_bar_pack_end   (AdwHeaderBar *self,
                                GtkWidget    *child);
ADW_AVAILABLE_IN_ALL
void adw_header_bar_remove     (AdwHeaderBar *self,
                                GtkWidget    *child);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_header_bar_get_title_widget (AdwHeaderBar *self);
ADW_AVAILABLE_IN_ALL
void       adw_header_bar_set_title_widget (AdwHeaderBar *self,
                                            GtkWidget    *title_widget);

ADW_AVAILABLE_IN_ALL
gboolean adw_header_bar_get_show_start_title_buttons (AdwHeaderBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_header_bar_set_show_start_title_buttons (AdwHeaderBar *self,
                                                      gboolean      setting);

ADW_AVAILABLE_IN_ALL
gboolean adw_header_bar_get_show_end_title_buttons (AdwHeaderBar *self);
ADW_AVAILABLE_IN_ALL
void     adw_header_bar_set_show_end_title_buttons (AdwHeaderBar *self,
                                                    gboolean      setting);

ADW_AVAILABLE_IN_1_4
gboolean adw_header_bar_get_show_back_button (AdwHeaderBar *self);
ADW_AVAILABLE_IN_1_4
void     adw_header_bar_set_show_back_button (AdwHeaderBar *self,
                                              gboolean      show_back_button);

ADW_AVAILABLE_IN_ALL
const char *adw_header_bar_get_decoration_layout (AdwHeaderBar *self);
ADW_AVAILABLE_IN_ALL
void        adw_header_bar_set_decoration_layout (AdwHeaderBar *self,
                                                  const char   *layout);

ADW_AVAILABLE_IN_ALL
AdwCenteringPolicy adw_header_bar_get_centering_policy (AdwHeaderBar       *self);
ADW_AVAILABLE_IN_ALL
void               adw_header_bar_set_centering_policy (AdwHeaderBar       *self,
                                                        AdwCenteringPolicy  centering_policy);

ADW_AVAILABLE_IN_1_4
gboolean adw_header_bar_get_show_title (AdwHeaderBar *self);
ADW_AVAILABLE_IN_1_4
void     adw_header_bar_set_show_title (AdwHeaderBar *self,
                                        gboolean      show_title);

G_END_DECLS
