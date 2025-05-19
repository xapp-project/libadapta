/*
 * Copyright (C) 2020 Purism SPC
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

#define ADAP_TYPE_AVATAR (adap_avatar_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapAvatar, adap_avatar, ADAP, AVATAR, GtkWidget)

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_avatar_new (int         size,
                           const char *text,
                           gboolean    show_initials) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_avatar_get_icon_name (AdapAvatar  *self);
ADAP_AVAILABLE_IN_ALL
void        adap_avatar_set_icon_name (AdapAvatar  *self,
                                      const char *icon_name);

ADAP_AVAILABLE_IN_ALL
const char *adap_avatar_get_text (AdapAvatar  *self);
ADAP_AVAILABLE_IN_ALL
void        adap_avatar_set_text (AdapAvatar  *self,
                                 const char *text);

ADAP_AVAILABLE_IN_ALL
gboolean adap_avatar_get_show_initials (AdapAvatar *self);
ADAP_AVAILABLE_IN_ALL
void     adap_avatar_set_show_initials (AdapAvatar *self,
                                       gboolean   show_initials);

ADAP_AVAILABLE_IN_ALL
GdkPaintable *adap_avatar_get_custom_image (AdapAvatar    *self);
ADAP_AVAILABLE_IN_ALL
void          adap_avatar_set_custom_image (AdapAvatar    *self,
                                           GdkPaintable *custom_image);

ADAP_AVAILABLE_IN_ALL
int  adap_avatar_get_size (AdapAvatar *self);
ADAP_AVAILABLE_IN_ALL
void adap_avatar_set_size (AdapAvatar *self,
                          int        size);

ADAP_AVAILABLE_IN_ALL
GdkTexture *adap_avatar_draw_to_texture (AdapAvatar *self,
                                        int        scale_factor) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
