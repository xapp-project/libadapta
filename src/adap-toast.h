/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_TOAST_PRIORITY_NORMAL,
  ADW_TOAST_PRIORITY_HIGH,
} AdwToastPriority;

#define ADW_TYPE_TOAST (adw_toast_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwToast, adw_toast, ADW, TOAST, GObject)

ADW_AVAILABLE_IN_ALL
AdwToast *adw_toast_new        (const char *title) G_GNUC_WARN_UNUSED_RESULT;
ADW_AVAILABLE_IN_1_2
AdwToast *adw_toast_new_format (const char *format,
                                ...) G_GNUC_PRINTF (1, 2) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
const char *adw_toast_get_title (AdwToast   *self);
ADW_AVAILABLE_IN_ALL
void        adw_toast_set_title (AdwToast   *self,
                                 const char *title);

ADW_AVAILABLE_IN_ALL
const char *adw_toast_get_button_label (AdwToast   *self);
ADW_AVAILABLE_IN_ALL
void        adw_toast_set_button_label (AdwToast   *self,
                                        const char *button_label);

ADW_AVAILABLE_IN_ALL
const char *adw_toast_get_action_name (AdwToast   *self);
ADW_AVAILABLE_IN_ALL
void        adw_toast_set_action_name (AdwToast   *self,
                                       const char *action_name);

ADW_AVAILABLE_IN_ALL
GVariant *adw_toast_get_action_target_value  (AdwToast   *self);
ADW_AVAILABLE_IN_ALL
void      adw_toast_set_action_target_value  (AdwToast   *self,
                                              GVariant   *action_target);
ADW_AVAILABLE_IN_ALL
void      adw_toast_set_action_target        (AdwToast   *self,
                                              const char *format_string,
                                              ...);
ADW_AVAILABLE_IN_ALL
void      adw_toast_set_detailed_action_name (AdwToast   *self,
                                              const char *detailed_action_name);

ADW_AVAILABLE_IN_ALL
AdwToastPriority adw_toast_get_priority (AdwToast         *self);
ADW_AVAILABLE_IN_ALL
void             adw_toast_set_priority (AdwToast         *self,
                                         AdwToastPriority  priority);

ADW_AVAILABLE_IN_ALL
guint adw_toast_get_timeout (AdwToast *self);
ADW_AVAILABLE_IN_ALL
void  adw_toast_set_timeout (AdwToast *self,
                             guint     timeout);

ADW_AVAILABLE_IN_1_2
GtkWidget *adw_toast_get_custom_title (AdwToast *self);
ADW_AVAILABLE_IN_1_2
void       adw_toast_set_custom_title (AdwToast  *self,
                                       GtkWidget *widget);

ADW_AVAILABLE_IN_1_4
gboolean adw_toast_get_use_markup (AdwToast *self);
ADW_AVAILABLE_IN_1_4
void     adw_toast_set_use_markup (AdwToast *self,
                                   gboolean  use_markup);

ADW_AVAILABLE_IN_ALL
void adw_toast_dismiss (AdwToast *self);

G_END_DECLS
