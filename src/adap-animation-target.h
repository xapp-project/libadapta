/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
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

#define ADAP_TYPE_ANIMATION_TARGET (adap_animation_target_get_type())

ADAP_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdapAnimationTarget, adap_animation_target, ADAP, ANIMATION_TARGET, GObject)


/**
 * AdapAnimationTargetFunc:
 * @value: The animation value
 * @user_data: (nullable): The user data provided when creating the target
 *
 * Prototype for animation targets based on user callbacks.
 */
typedef void (*AdapAnimationTargetFunc) (double   value,
                                        gpointer user_data);

#define ADAP_TYPE_CALLBACK_ANIMATION_TARGET (adap_callback_animation_target_get_type())

ADAP_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (AdapCallbackAnimationTarget, adap_callback_animation_target, ADAP, CALLBACK_ANIMATION_TARGET, AdapAnimationTarget)

ADAP_AVAILABLE_IN_ALL
AdapAnimationTarget *adap_callback_animation_target_new (AdapAnimationTargetFunc callback,
                                                       gpointer               user_data,
                                                       GDestroyNotify         destroy) G_GNUC_WARN_UNUSED_RESULT;

#define ADAP_TYPE_PROPERTY_ANIMATION_TARGET (adap_property_animation_target_get_type())

ADAP_AVAILABLE_IN_1_2
GDK_DECLARE_INTERNAL_TYPE (AdapPropertyAnimationTarget, adap_property_animation_target, ADAP, PROPERTY_ANIMATION_TARGET, AdapAnimationTarget)

ADAP_AVAILABLE_IN_1_2
AdapAnimationTarget *adap_property_animation_target_new           (GObject    *object,
                                                                 const char *property_name) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_1_2
AdapAnimationTarget *adap_property_animation_target_new_for_pspec (GObject    *object,
                                                                 GParamSpec *pspec) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_1_2
GObject    *adap_property_animation_target_get_object (AdapPropertyAnimationTarget *self);
ADAP_AVAILABLE_IN_1_2
GParamSpec *adap_property_animation_target_get_pspec  (AdapPropertyAnimationTarget *self);

G_END_DECLS
