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
#include "adap-navigation-direction.h"

G_BEGIN_DECLS

#define ADAP_TYPE_SWIPEABLE (adap_swipeable_get_type ())

ADAP_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (AdapSwipeable, adap_swipeable, ADAP, SWIPEABLE, GtkWidget)

/**
 * AdapSwipeableInterface:
 * @parent: The parent interface.
 * @get_distance: Gets the swipe distance.
 * @get_snap_points: Gets the snap points.
 * @get_progress: Gets the current progress.
 * @get_cancel_progress: Gets the cancel progress.
 * @get_swipe_area: Gets the swipeable rectangle.
 *
 * An interface for swipeable widgets.
 **/
struct _AdapSwipeableInterface
{
  GTypeInterface parent;

  double  (*get_distance)        (AdapSwipeable *self);
  double *(*get_snap_points)     (AdapSwipeable *self,
                                  int          *n_snap_points);
  double  (*get_progress)        (AdapSwipeable *self);
  double  (*get_cancel_progress) (AdapSwipeable *self);
  void    (*get_swipe_area)      (AdapSwipeable           *self,
                                  AdapNavigationDirection  navigation_direction,
                                  gboolean                is_drag,
                                  GdkRectangle           *rect);

  /*< private >*/
  gpointer padding[4];
};

ADAP_AVAILABLE_IN_ALL
double adap_swipeable_get_distance (AdapSwipeable *self);

ADAP_AVAILABLE_IN_ALL
double *adap_swipeable_get_snap_points (AdapSwipeable *self,
                                       int          *n_snap_points) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
double adap_swipeable_get_progress (AdapSwipeable *self);

ADAP_AVAILABLE_IN_ALL
double adap_swipeable_get_cancel_progress (AdapSwipeable *self);

ADAP_AVAILABLE_IN_ALL
void adap_swipeable_get_swipe_area (AdapSwipeable           *self,
                                   AdapNavigationDirection  navigation_direction,
                                   gboolean                is_drag,
                                   GdkRectangle           *rect);

G_END_DECLS
