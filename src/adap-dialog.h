/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-breakpoint.h"
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_DIALOG_AUTO,
  ADW_DIALOG_FLOATING,
  ADW_DIALOG_BOTTOM_SHEET,
} AdwDialogPresentationMode;

#define ADW_TYPE_DIALOG (adw_dialog_get_type())

ADW_AVAILABLE_IN_1_5
G_DECLARE_DERIVABLE_TYPE (AdwDialog, adw_dialog, ADW, DIALOG, GtkWidget)

struct _AdwDialogClass
{
  GtkWidgetClass parent_class;

  void (* close_attempt) (AdwDialog *dialog);
  void (* closed)        (AdwDialog *dialog);

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_1_5
AdwDialog *adw_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_5
GtkWidget *adw_dialog_get_child (AdwDialog *self);
ADW_AVAILABLE_IN_1_5
void       adw_dialog_set_child (AdwDialog *self,
                                 GtkWidget *child);

ADW_AVAILABLE_IN_1_5
const char *adw_dialog_get_title (AdwDialog  *self);
ADW_AVAILABLE_IN_1_5
void        adw_dialog_set_title (AdwDialog  *self,
                                  const char *title);

ADW_AVAILABLE_IN_1_5
gboolean adw_dialog_get_can_close (AdwDialog  *self);
ADW_AVAILABLE_IN_1_5
void     adw_dialog_set_can_close (AdwDialog *self,
                                   gboolean   can_close);

ADW_AVAILABLE_IN_1_5
int  adw_dialog_get_content_width (AdwDialog  *self);
ADW_AVAILABLE_IN_1_5
void adw_dialog_set_content_width (AdwDialog *self,
                                   int        content_width);

ADW_AVAILABLE_IN_1_5
int  adw_dialog_get_content_height (AdwDialog  *self);
ADW_AVAILABLE_IN_1_5
void adw_dialog_set_content_height (AdwDialog *self,
                                    int        content_height);

ADW_AVAILABLE_IN_1_5
gboolean adw_dialog_get_follows_content_size (AdwDialog *self);
ADW_AVAILABLE_IN_1_5
void     adw_dialog_set_follows_content_size (AdwDialog *self,
                                              gboolean   follows_content_size);

ADW_AVAILABLE_IN_1_5
AdwDialogPresentationMode adw_dialog_get_presentation_mode (AdwDialog                 *self);
ADW_AVAILABLE_IN_1_5
void                      adw_dialog_set_presentation_mode (AdwDialog                 *self,
                                                            AdwDialogPresentationMode  presentation_mode);

ADW_AVAILABLE_IN_1_5
GtkWidget *adw_dialog_get_focus (AdwDialog *self);
ADW_AVAILABLE_IN_1_5
void       adw_dialog_set_focus (AdwDialog *self,
                                 GtkWidget *focus);

ADW_AVAILABLE_IN_1_5
GtkWidget *adw_dialog_get_default_widget (AdwDialog *self);
ADW_AVAILABLE_IN_1_5
void       adw_dialog_set_default_widget (AdwDialog *self,
                                          GtkWidget *default_widget);

ADW_AVAILABLE_IN_1_5
gboolean adw_dialog_close (AdwDialog *self);

ADW_AVAILABLE_IN_1_5
void adw_dialog_force_close (AdwDialog *self);

ADW_AVAILABLE_IN_1_5
void adw_dialog_add_breakpoint (AdwDialog     *self,
                                AdwBreakpoint *breakpoint);

ADW_AVAILABLE_IN_1_5
AdwBreakpoint *adw_dialog_get_current_breakpoint (AdwDialog *self);

ADW_AVAILABLE_IN_1_5
void adw_dialog_present (AdwDialog *self,
                         GtkWidget *parent);

G_END_DECLS
