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

#include <glib-object.h>

G_BEGIN_DECLS

#define ADAP_TYPE_ENUM_LIST_ITEM (adap_enum_list_item_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapEnumListItem, adap_enum_list_item, ADAP, ENUM_LIST_ITEM, GObject)

ADAP_AVAILABLE_IN_ALL
int adap_enum_list_item_get_value (AdapEnumListItem *self);

ADAP_AVAILABLE_IN_ALL
const char *adap_enum_list_item_get_name (AdapEnumListItem *self);

ADAP_AVAILABLE_IN_ALL
const char *adap_enum_list_item_get_nick (AdapEnumListItem *self);

#define ADAP_TYPE_ENUM_LIST_MODEL (adap_enum_list_model_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapEnumListModel, adap_enum_list_model, ADAP, ENUM_LIST_MODEL, GObject)

ADAP_AVAILABLE_IN_ALL
AdapEnumListModel *adap_enum_list_model_new (GType enum_type) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
GType adap_enum_list_model_get_enum_type (AdapEnumListModel *self);

ADAP_AVAILABLE_IN_ALL
guint adap_enum_list_model_find_position (AdapEnumListModel *self,
                                         int               value);

G_END_DECLS
