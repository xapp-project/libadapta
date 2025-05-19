/*
 * Copyright (C) 2020 Purism SPC
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

#define ADW_TYPE_AVATAR (adw_avatar_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwAvatar, adw_avatar, ADW, AVATAR, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_avatar_new (int         size,
                           const char *text,
                           gboolean    show_initials) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_avatar_get_icon_name (AdwAvatar  *self);
ADW_AVAILABLE_IN_ALL
void        adw_avatar_set_icon_name (AdwAvatar  *self,
                                      const char *icon_name);

ADW_AVAILABLE_IN_ALL
const char *adw_avatar_get_text (AdwAvatar  *self);
ADW_AVAILABLE_IN_ALL
void        adw_avatar_set_text (AdwAvatar  *self,
                                 const char *text);

ADW_AVAILABLE_IN_ALL
gboolean adw_avatar_get_show_initials (AdwAvatar *self);
ADW_AVAILABLE_IN_ALL
void     adw_avatar_set_show_initials (AdwAvatar *self,
                                       gboolean   show_initials);

ADW_AVAILABLE_IN_ALL
GdkPaintable *adw_avatar_get_custom_image (AdwAvatar    *self);
ADW_AVAILABLE_IN_ALL
void          adw_avatar_set_custom_image (AdwAvatar    *self,
                                           GdkPaintable *custom_image);

ADW_AVAILABLE_IN_ALL
int  adw_avatar_get_size (AdwAvatar *self);
ADW_AVAILABLE_IN_ALL
void adw_avatar_set_size (AdwAvatar *self,
                          int        size);

ADW_AVAILABLE_IN_ALL
GdkTexture *adw_avatar_draw_to_texture (AdwAvatar *self,
                                        int        scale_factor) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
