/*
 * Copyright (C) 2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-dialog.h"
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_RESPONSE_DEFAULT,
  ADW_RESPONSE_SUGGESTED,
  ADW_RESPONSE_DESTRUCTIVE,
} AdwResponseAppearance;

#define ADW_TYPE_ALERT_DIALOG (adw_alert_dialog_get_type())

ADW_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdwAlertDialog, adw_alert_dialog, ADW, ALERT_DIALOG, AdwDialog)

struct _AdwAlertDialogClass
{
  AdwDialogClass parent_class;

  void (* response) (AdwAlertDialog *self,
                     const char     *response);

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_alert_dialog_new (const char *heading,
                                 const char *body);

ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_get_heading (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_set_heading (AdwAlertDialog *self,
                                          const char     *heading);

ADW_AVAILABLE_IN_1_5
gboolean adw_alert_dialog_get_heading_use_markup (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void     adw_alert_dialog_set_heading_use_markup (AdwAlertDialog *self,
                                                  gboolean        use_markup);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_format_heading (AdwAlertDialog *self,
                                      const char     *format,
                                      ...) G_GNUC_PRINTF (2, 3);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_format_heading_markup (AdwAlertDialog *self,
                                             const char     *format,
                                             ...) G_GNUC_PRINTF (2, 3);

ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_get_body (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_set_body (AdwAlertDialog *self,
                                       const char     *body);

ADW_AVAILABLE_IN_1_5
gboolean adw_alert_dialog_get_body_use_markup (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void     adw_alert_dialog_set_body_use_markup (AdwAlertDialog *self,
                                               gboolean        use_markup);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_format_body (AdwAlertDialog *self,
                                   const char     *format,
                                   ...) G_GNUC_PRINTF (2, 3);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_format_body_markup (AdwAlertDialog *self,
                                          const char     *format,
                                          ...) G_GNUC_PRINTF (2, 3);

ADW_AVAILABLE_IN_1_5
GtkWidget *adw_alert_dialog_get_extra_child (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void       adw_alert_dialog_set_extra_child (AdwAlertDialog *self,
                                             GtkWidget      *child);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_add_response (AdwAlertDialog *self,
                                    const char     *id,
                                    const char     *label);

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_add_responses (AdwAlertDialog *self,
                                     const char     *first_id,
                                     ...) G_GNUC_NULL_TERMINATED;

ADW_AVAILABLE_IN_1_5
void adw_alert_dialog_remove_response (AdwAlertDialog *self,
                                       const char     *id);

ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_get_response_label (AdwAlertDialog *self,
                                                 const char     *response);
ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_set_response_label (AdwAlertDialog *self,
                                                 const char     *response,
                                                 const char     *label);

ADW_AVAILABLE_IN_1_5
AdwResponseAppearance adw_alert_dialog_get_response_appearance (AdwAlertDialog        *self,
                                                                const char            *response);
ADW_AVAILABLE_IN_1_5
void                  adw_alert_dialog_set_response_appearance (AdwAlertDialog        *self,
                                                                const char            *response,
                                                                AdwResponseAppearance  appearance);

ADW_AVAILABLE_IN_1_5
gboolean adw_alert_dialog_get_response_enabled (AdwAlertDialog *self,
                                                const char     *response);
ADW_AVAILABLE_IN_1_5
void     adw_alert_dialog_set_response_enabled (AdwAlertDialog *self,
                                                const char     *response,
                                                gboolean        enabled);

ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_get_default_response (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_set_default_response (AdwAlertDialog *self,
                                                   const char     *response);

ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_get_close_response (AdwAlertDialog *self);
ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_set_close_response (AdwAlertDialog *self,
                                                 const char     *response);

ADW_AVAILABLE_IN_1_5
gboolean adw_alert_dialog_has_response (AdwAlertDialog *self,
                                        const char     *response);

ADW_AVAILABLE_IN_1_5
void        adw_alert_dialog_choose        (AdwAlertDialog      *self,
                                            GtkWidget           *parent,
                                            GCancellable        *cancellable,
                                            GAsyncReadyCallback  callback,
                                            gpointer             user_data);
ADW_AVAILABLE_IN_1_5
const char *adw_alert_dialog_choose_finish (AdwAlertDialog   *self,
                                            GAsyncResult     *result);

G_END_DECLS
