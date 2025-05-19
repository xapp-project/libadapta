/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include "adap-version.h"

#include <gtk/gtk.h>
#include "adap-swipeable.h"

G_BEGIN_DECLS

#define ADAP_TYPE_SWIPE_TRACKER (adap_swipe_tracker_get_type())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdapSwipeTracker, adap_swipe_tracker, ADAP, SWIPE_TRACKER, GObject)

ADAP_AVAILABLE_IN_ALL
AdapSwipeTracker *adap_swipe_tracker_new (AdapSwipeable *swipeable) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapSwipeable *adap_swipe_tracker_get_swipeable (AdapSwipeTracker *self);

ADAP_AVAILABLE_IN_ALL
gboolean adap_swipe_tracker_get_enabled (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_ALL
void     adap_swipe_tracker_set_enabled (AdapSwipeTracker *self,
                                        gboolean         enabled);

ADAP_AVAILABLE_IN_ALL
gboolean adap_swipe_tracker_get_reversed (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_ALL
void     adap_swipe_tracker_set_reversed (AdapSwipeTracker *self,
                                         gboolean         reversed);

ADAP_AVAILABLE_IN_ALL
gboolean         adap_swipe_tracker_get_allow_mouse_drag (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_ALL
void             adap_swipe_tracker_set_allow_mouse_drag (AdapSwipeTracker *self,
                                                         gboolean         allow_mouse_drag);

ADAP_AVAILABLE_IN_ALL
gboolean adap_swipe_tracker_get_allow_long_swipes (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_ALL
void     adap_swipe_tracker_set_allow_long_swipes (AdapSwipeTracker *self,
                                                  gboolean         allow_long_swipes);

ADAP_AVAILABLE_IN_1_4
gboolean adap_swipe_tracker_get_lower_overshoot (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_1_4
void     adap_swipe_tracker_set_lower_overshoot (AdapSwipeTracker *self,
                                                gboolean         overshoot);

ADAP_AVAILABLE_IN_1_4
gboolean adap_swipe_tracker_get_upper_overshoot (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_1_4
void     adap_swipe_tracker_set_upper_overshoot (AdapSwipeTracker *self,
                                                gboolean         overshoot);

ADAP_AVAILABLE_IN_1_5
gboolean adap_swipe_tracker_get_allow_window_handle (AdapSwipeTracker *self);
ADAP_AVAILABLE_IN_1_5
void     adap_swipe_tracker_set_allow_window_handle (AdapSwipeTracker *self,
                                                    gboolean         allow_window_handle);

ADAP_AVAILABLE_IN_ALL
void adap_swipe_tracker_shift_position (AdapSwipeTracker *self,
                                       double           delta);

G_END_DECLS
