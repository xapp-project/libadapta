/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "adap-preferences-group.h"

G_BEGIN_DECLS

GListModel *adap_preferences_group_get_rows (AdapPreferencesGroup *self) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
