/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(4, 13, 4)
# error "libadapta requires gtk4 >= 4.13.4"
#endif

#if !GLIB_CHECK_VERSION(2, 76, 0)
# error "libadapta requires glib-2.0 >= 2.76.0"
#endif

#define _ADAPTA_INSIDE

#include "adap-version.h"
#include "adap-about-dialog.h"
#include "adap-about-window.h"
#include "adap-action-row.h"
#include "adap-alert-dialog.h"
#include "adap-animation.h"
#include "adap-animation-target.h"
#include "adap-animation-util.h"
#include "adap-application.h"
#include "adap-application-window.h"
#include "adap-avatar.h"
#include "adap-banner.h"
#include "adap-bin.h"
#include "adap-breakpoint.h"
#include "adap-breakpoint-bin.h"
#include "adap-button-content.h"
#include "adap-carousel.h"
#include "adap-carousel-indicator-dots.h"
#include "adap-carousel-indicator-lines.h"
#include "adap-clamp.h"
#include "adap-clamp-layout.h"
#include "adap-clamp-scrollable.h"
#include "adap-combo-row.h"
#include "adap-dialog.h"
#include "adap-easing.h"
#include "adap-entry-row.h"
#include "adap-enum-list-model.h"
#include "adap-expander-row.h"
#include "adap-flap.h"
#include "adap-fold-threshold-policy.h"
#include "adap-header-bar.h"
#include "adap-leaflet.h"
#include "adap-length-unit.h"
#include "adap-main.h"
#include "adap-message-dialog.h"
#include "adap-navigation-direction.h"
#include "adap-navigation-split-view.h"
#include "adap-navigation-view.h"
#include "adap-overlay-split-view.h"
#include "adap-password-entry-row.h"
#include "adap-preferences-dialog.h"
#include "adap-preferences-group.h"
#include "adap-preferences-page.h"
#include "adap-preferences-row.h"
#include "adap-preferences-window.h"
#include "adap-spin-row.h"
#include "adap-split-button.h"
#include "adap-spring-animation.h"
#include "adap-spring-params.h"
#include "adap-squeezer.h"
#include "adap-status-page.h"
#include "adap-style-manager.h"
#include "adap-swipe-tracker.h"
#include "adap-swipeable.h"
#include "adap-switch-row.h"
#include "adap-tab-bar.h"
#include "adap-tab-button.h"
#include "adap-tab-overview.h"
#include "adap-tab-view.h"
#include "adap-timed-animation.h"
#include "adap-toast-overlay.h"
#include "adap-toast.h"
#include "adap-toolbar-view.h"
#include "adap-view-stack.h"
#include "adap-view-switcher.h"
#include "adap-view-switcher-bar.h"
#include "adap-view-switcher-title.h"
#include "adap-window.h"
#include "adap-window-title.h"

#undef _ADAPTA_INSIDE

G_END_DECLS
