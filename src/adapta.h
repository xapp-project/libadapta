/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(4, 13, 4)
# error "libadwaita requires gtk4 >= 4.13.4"
#endif

#if !GLIB_CHECK_VERSION(2, 76, 0)
# error "libadwaita requires glib-2.0 >= 2.76.0"
#endif

#define _ADWAITA_INSIDE

#include "adw-version.h"
#include "adw-about-dialog.h"
#include "adw-about-window.h"
#include "adw-action-row.h"
#include "adw-alert-dialog.h"
#include "adw-animation.h"
#include "adw-animation-target.h"
#include "adw-animation-util.h"
#include "adw-application.h"
#include "adw-application-window.h"
#include "adw-avatar.h"
#include "adw-banner.h"
#include "adw-bin.h"
#include "adw-breakpoint.h"
#include "adw-breakpoint-bin.h"
#include "adw-button-content.h"
#include "adw-carousel.h"
#include "adw-carousel-indicator-dots.h"
#include "adw-carousel-indicator-lines.h"
#include "adw-clamp.h"
#include "adw-clamp-layout.h"
#include "adw-clamp-scrollable.h"
#include "adw-combo-row.h"
#include "adw-dialog.h"
#include "adw-easing.h"
#include "adw-entry-row.h"
#include "adw-enum-list-model.h"
#include "adw-expander-row.h"
#include "adw-flap.h"
#include "adw-fold-threshold-policy.h"
#include "adw-header-bar.h"
#include "adw-leaflet.h"
#include "adw-length-unit.h"
#include "adw-main.h"
#include "adw-message-dialog.h"
#include "adw-navigation-direction.h"
#include "adw-navigation-split-view.h"
#include "adw-navigation-view.h"
#include "adw-overlay-split-view.h"
#include "adw-password-entry-row.h"
#include "adw-preferences-dialog.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-preferences-row.h"
#include "adw-preferences-window.h"
#include "adw-spin-row.h"
#include "adw-split-button.h"
#include "adw-spring-animation.h"
#include "adw-spring-params.h"
#include "adw-squeezer.h"
#include "adw-status-page.h"
#include "adw-style-manager.h"
#include "adw-swipe-tracker.h"
#include "adw-swipeable.h"
#include "adw-switch-row.h"
#include "adw-tab-bar.h"
#include "adw-tab-button.h"
#include "adw-tab-overview.h"
#include "adw-tab-view.h"
#include "adw-timed-animation.h"
#include "adw-toast-overlay.h"
#include "adw-toast.h"
#include "adw-toolbar-view.h"
#include "adw-view-stack.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-bar.h"
#include "adw-view-switcher-title.h"
#include "adw-window.h"
#include "adw-window-title.h"

#undef _ADWAITA_INSIDE

G_END_DECLS
