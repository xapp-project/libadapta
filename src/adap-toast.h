/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_TOAST_PRIORITY_NORMAL,
  ADAP_TOAST_PRIORITY_HIGH,
} AdapToastPriority;

#define ADAP_TYPE_TOAST (adap_toast_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapToast, adap_toast, ADAP, TOAST, GObject)

ADAP_AVAILABLE_IN_ALL
AdapToast *adap_toast_new        (const char *title) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_1_2
AdapToast *adap_toast_new_format (const char *format,
                                ...) G_GNUC_PRINTF (1, 2) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
const char *adap_toast_get_title (AdapToast   *self);
ADAP_AVAILABLE_IN_ALL
void        adap_toast_set_title (AdapToast   *self,
                                 const char *title);

ADAP_AVAILABLE_IN_ALL
const char *adap_toast_get_button_label (AdapToast   *self);
ADAP_AVAILABLE_IN_ALL
void        adap_toast_set_button_label (AdapToast   *self,
                                        const char *button_label);

ADAP_AVAILABLE_IN_ALL
const char *adap_toast_get_action_name (AdapToast   *self);
ADAP_AVAILABLE_IN_ALL
void        adap_toast_set_action_name (AdapToast   *self,
                                       const char *action_name);

ADAP_AVAILABLE_IN_ALL
GVariant *adap_toast_get_action_target_value  (AdapToast   *self);
ADAP_AVAILABLE_IN_ALL
void      adap_toast_set_action_target_value  (AdapToast   *self,
                                              GVariant   *action_target);
ADAP_AVAILABLE_IN_ALL
void      adap_toast_set_action_target        (AdapToast   *self,
                                              const char *format_string,
                                              ...);
ADAP_AVAILABLE_IN_ALL
void      adap_toast_set_detailed_action_name (AdapToast   *self,
                                              const char *detailed_action_name);

ADAP_AVAILABLE_IN_ALL
AdapToastPriority adap_toast_get_priority (AdapToast         *self);
ADAP_AVAILABLE_IN_ALL
void             adap_toast_set_priority (AdapToast         *self,
                                         AdapToastPriority  priority);

ADAP_AVAILABLE_IN_ALL
guint adap_toast_get_timeout (AdapToast *self);
ADAP_AVAILABLE_IN_ALL
void  adap_toast_set_timeout (AdapToast *self,
                             guint     timeout);

ADAP_AVAILABLE_IN_1_2
GtkWidget *adap_toast_get_custom_title (AdapToast *self);
ADAP_AVAILABLE_IN_1_2
void       adap_toast_set_custom_title (AdapToast  *self,
                                       GtkWidget *widget);

ADAP_AVAILABLE_IN_1_4
gboolean adap_toast_get_use_markup (AdapToast *self);
ADAP_AVAILABLE_IN_1_4
void     adap_toast_set_use_markup (AdapToast *self,
                                   gboolean  use_markup);

ADAP_AVAILABLE_IN_ALL
void adap_toast_dismiss (AdapToast *self);

G_END_DECLS
