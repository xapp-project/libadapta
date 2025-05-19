/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-toast.h"
#include "adap-toast-overlay.h"

G_BEGIN_DECLS

AdapToastOverlay *adap_toast_get_overlay (AdapToast        *self);
void             adap_toast_set_overlay (AdapToast        *self,
                                        AdapToastOverlay *overlay);

G_END_DECLS
