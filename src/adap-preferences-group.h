/*
 * Copyright (C) 2019 Purism SPC
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

#define ADAP_TYPE_PREFERENCES_GROUP (adap_preferences_group_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdapPreferencesGroup, adap_preferences_group, ADAP, PREFERENCES_GROUP, GtkWidget)

/**
 * AdapPreferencesGroupClass
 * @parent_class: The parent class
 */
struct _AdapPreferencesGroupClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
GtkWidget *adap_preferences_group_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
void adap_preferences_group_add    (AdapPreferencesGroup *self,
                                   GtkWidget           *child);
ADAP_AVAILABLE_IN_ALL
void adap_preferences_group_remove (AdapPreferencesGroup *self,
                                   GtkWidget           *child);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_group_get_title (AdapPreferencesGroup *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_group_set_title (AdapPreferencesGroup *self,
                                             const char          *title);

ADAP_AVAILABLE_IN_ALL
const char *adap_preferences_group_get_description (AdapPreferencesGroup *self);
ADAP_AVAILABLE_IN_ALL
void        adap_preferences_group_set_description (AdapPreferencesGroup *self,
                                                   const char          *description);

ADAP_AVAILABLE_IN_1_1
GtkWidget *adap_preferences_group_get_header_suffix (AdapPreferencesGroup *self);
ADAP_AVAILABLE_IN_1_1
void       adap_preferences_group_set_header_suffix (AdapPreferencesGroup *self,
                                                    GtkWidget           *suffix);

G_END_DECLS
