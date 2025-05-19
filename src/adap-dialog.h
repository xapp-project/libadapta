/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>

#include "adap-breakpoint.h"
#include "adap-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADAP_DIALOG_AUTO,
  ADAP_DIALOG_FLOATING,
  ADAP_DIALOG_BOTTOM_SHEET,
} AdapDialogPresentationMode;

#define ADAP_TYPE_DIALOG (adap_dialog_get_type())

ADAP_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdapDialog, adap_dialog, ADAP, DIALOG, GtkWidget)

struct _AdapDialogClass
{
  GtkWidgetClass parent_class;

  void (* close_attempt) (AdapDialog *dialog);
  void (* closed)        (AdapDialog *dialog);

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_1_5
AdapDialog *adap_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_5
GtkWidget *adap_dialog_get_child (AdapDialog *self);
ADAP_AVAILABLE_IN_1_5
void       adap_dialog_set_child (AdapDialog *self,
                                 GtkWidget *child);

ADAP_AVAILABLE_IN_1_5
const char *adap_dialog_get_title (AdapDialog  *self);
ADAP_AVAILABLE_IN_1_5
void        adap_dialog_set_title (AdapDialog  *self,
                                  const char *title);

ADAP_AVAILABLE_IN_1_5
gboolean adap_dialog_get_can_close (AdapDialog  *self);
ADAP_AVAILABLE_IN_1_5
void     adap_dialog_set_can_close (AdapDialog *self,
                                   gboolean   can_close);

ADAP_AVAILABLE_IN_1_5
int  adap_dialog_get_content_width (AdapDialog  *self);
ADAP_AVAILABLE_IN_1_5
void adap_dialog_set_content_width (AdapDialog *self,
                                   int        content_width);

ADAP_AVAILABLE_IN_1_5
int  adap_dialog_get_content_height (AdapDialog  *self);
ADAP_AVAILABLE_IN_1_5
void adap_dialog_set_content_height (AdapDialog *self,
                                    int        content_height);

ADAP_AVAILABLE_IN_1_5
gboolean adap_dialog_get_follows_content_size (AdapDialog *self);
ADAP_AVAILABLE_IN_1_5
void     adap_dialog_set_follows_content_size (AdapDialog *self,
                                              gboolean   follows_content_size);

ADAP_AVAILABLE_IN_1_5
AdapDialogPresentationMode adap_dialog_get_presentation_mode (AdapDialog                 *self);
ADAP_AVAILABLE_IN_1_5
void                      adap_dialog_set_presentation_mode (AdapDialog                 *self,
                                                            AdapDialogPresentationMode  presentation_mode);

ADAP_AVAILABLE_IN_1_5
GtkWidget *adap_dialog_get_focus (AdapDialog *self);
ADAP_AVAILABLE_IN_1_5
void       adap_dialog_set_focus (AdapDialog *self,
                                 GtkWidget *focus);

ADAP_AVAILABLE_IN_1_5
GtkWidget *adap_dialog_get_default_widget (AdapDialog *self);
ADAP_AVAILABLE_IN_1_5
void       adap_dialog_set_default_widget (AdapDialog *self,
                                          GtkWidget *default_widget);

ADAP_AVAILABLE_IN_1_5
gboolean adap_dialog_close (AdapDialog *self);

ADAP_AVAILABLE_IN_1_5
void adap_dialog_force_close (AdapDialog *self);

ADAP_AVAILABLE_IN_1_5
void adap_dialog_add_breakpoint (AdapDialog     *self,
                                AdapBreakpoint *breakpoint);

ADAP_AVAILABLE_IN_1_5
AdapBreakpoint *adap_dialog_get_current_breakpoint (AdapDialog *self);

ADAP_AVAILABLE_IN_1_5
void adap_dialog_present (AdapDialog *self,
                         GtkWidget *parent);

G_END_DECLS
