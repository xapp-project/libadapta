/*
 * Copyright (C) 2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-dialog.h"
#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_RESPONSE_DEFAULT,
  ADAP_RESPONSE_SUGGESTED,
  ADAP_RESPONSE_DESTRUCTIVE,
} AdapResponseAppearance;

#define ADAP_TYPE_ALERT_DIALOG (adap_alert_dialog_get_type())

ADAP_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdapAlertDialog, adap_alert_dialog, ADAP, ALERT_DIALOG, AdapDialog)

struct _AdapAlertDialogClass
{
  AdapDialogClass parent_class;

  void (* response) (AdapAlertDialog *self,
                     const char     *response);

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_alert_dialog_new (const char *heading,
                                 const char *body);

ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_get_heading (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_set_heading (AdapAlertDialog *self,
                                          const char     *heading);

ADAP_AVAILABLE_IN_1_5
gboolean adap_alert_dialog_get_heading_use_markup (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void     adap_alert_dialog_set_heading_use_markup (AdapAlertDialog *self,
                                                  gboolean        use_markup);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_format_heading (AdapAlertDialog *self,
                                      const char     *format,
                                      ...) G_GNUC_PRINTF (2, 3);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_format_heading_markup (AdapAlertDialog *self,
                                             const char     *format,
                                             ...) G_GNUC_PRINTF (2, 3);

ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_get_body (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_set_body (AdapAlertDialog *self,
                                       const char     *body);

ADAP_AVAILABLE_IN_1_5
gboolean adap_alert_dialog_get_body_use_markup (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void     adap_alert_dialog_set_body_use_markup (AdapAlertDialog *self,
                                               gboolean        use_markup);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_format_body (AdapAlertDialog *self,
                                   const char     *format,
                                   ...) G_GNUC_PRINTF (2, 3);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_format_body_markup (AdapAlertDialog *self,
                                          const char     *format,
                                          ...) G_GNUC_PRINTF (2, 3);

ADAP_AVAILABLE_IN_1_5
GtkWidget *adap_alert_dialog_get_extra_child (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void       adap_alert_dialog_set_extra_child (AdapAlertDialog *self,
                                             GtkWidget      *child);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_add_response (AdapAlertDialog *self,
                                    const char     *id,
                                    const char     *label);

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_add_responses (AdapAlertDialog *self,
                                     const char     *first_id,
                                     ...) G_GNUC_NULL_TERMINATED;

ADAP_AVAILABLE_IN_1_5
void adap_alert_dialog_remove_response (AdapAlertDialog *self,
                                       const char     *id);

ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_get_response_label (AdapAlertDialog *self,
                                                 const char     *response);
ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_set_response_label (AdapAlertDialog *self,
                                                 const char     *response,
                                                 const char     *label);

ADAP_AVAILABLE_IN_1_5
AdapResponseAppearance adap_alert_dialog_get_response_appearance (AdapAlertDialog        *self,
                                                                const char            *response);
ADAP_AVAILABLE_IN_1_5
void                  adap_alert_dialog_set_response_appearance (AdapAlertDialog        *self,
                                                                const char            *response,
                                                                AdapResponseAppearance  appearance);

ADAP_AVAILABLE_IN_1_5
gboolean adap_alert_dialog_get_response_enabled (AdapAlertDialog *self,
                                                const char     *response);
ADAP_AVAILABLE_IN_1_5
void     adap_alert_dialog_set_response_enabled (AdapAlertDialog *self,
                                                const char     *response,
                                                gboolean        enabled);

ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_get_default_response (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_set_default_response (AdapAlertDialog *self,
                                                   const char     *response);

ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_get_close_response (AdapAlertDialog *self);
ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_set_close_response (AdapAlertDialog *self,
                                                 const char     *response);

ADAP_AVAILABLE_IN_1_5
gboolean adap_alert_dialog_has_response (AdapAlertDialog *self,
                                        const char     *response);

ADAP_AVAILABLE_IN_1_5
void        adap_alert_dialog_choose        (AdapAlertDialog      *self,
                                            GtkWidget           *parent,
                                            GCancellable        *cancellable,
                                            GAsyncReadyCallback  callback,
                                            gpointer             user_data);
ADAP_AVAILABLE_IN_1_5
const char *adap_alert_dialog_choose_finish (AdapAlertDialog   *self,
                                            GAsyncResult     *result);

G_END_DECLS
