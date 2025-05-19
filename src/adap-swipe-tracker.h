/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-swipeable.h"

G_BEGIN_DECLS

#define ADW_TYPE_SWIPE_TRACKER (adw_swipe_tracker_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwSwipeTracker, adw_swipe_tracker, ADW, SWIPE_TRACKER, GObject)

ADW_AVAILABLE_IN_ALL
AdwSwipeTracker *adw_swipe_tracker_new (AdwSwipeable *swipeable) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
AdwSwipeable *adw_swipe_tracker_get_swipeable (AdwSwipeTracker *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_swipe_tracker_get_enabled (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_ALL
void     adw_swipe_tracker_set_enabled (AdwSwipeTracker *self,
                                        gboolean         enabled);

ADW_AVAILABLE_IN_ALL
gboolean adw_swipe_tracker_get_reversed (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_ALL
void     adw_swipe_tracker_set_reversed (AdwSwipeTracker *self,
                                         gboolean         reversed);

ADW_AVAILABLE_IN_ALL
gboolean         adw_swipe_tracker_get_allow_mouse_drag (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_ALL
void             adw_swipe_tracker_set_allow_mouse_drag (AdwSwipeTracker *self,
                                                         gboolean         allow_mouse_drag);

ADW_AVAILABLE_IN_ALL
gboolean adw_swipe_tracker_get_allow_long_swipes (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_ALL
void     adw_swipe_tracker_set_allow_long_swipes (AdwSwipeTracker *self,
                                                  gboolean         allow_long_swipes);

ADW_AVAILABLE_IN_1_4
gboolean adw_swipe_tracker_get_lower_overshoot (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_1_4
void     adw_swipe_tracker_set_lower_overshoot (AdwSwipeTracker *self,
                                                gboolean         overshoot);

ADW_AVAILABLE_IN_1_4
gboolean adw_swipe_tracker_get_upper_overshoot (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_1_4
void     adw_swipe_tracker_set_upper_overshoot (AdwSwipeTracker *self,
                                                gboolean         overshoot);

ADW_AVAILABLE_IN_1_5
gboolean adw_swipe_tracker_get_allow_window_handle (AdwSwipeTracker *self);
ADW_AVAILABLE_IN_1_5
void     adw_swipe_tracker_set_allow_window_handle (AdwSwipeTracker *self,
                                                    gboolean         allow_window_handle);

ADW_AVAILABLE_IN_ALL
void adw_swipe_tracker_shift_position (AdwSwipeTracker *self,
                                       double           delta);

G_END_DECLS
