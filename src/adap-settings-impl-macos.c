/*
 * Copyright (C) 2022 Christian Hergert <christian@hergert.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-settings-impl-private.h"

#include <AppKit/AppKit.h>

struct _AdapSettingsImplMacOS
{
  AdapSettingsImpl parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapSettingsImplMacOS, adap_settings_impl_macos, ADAP_TYPE_SETTINGS_IMPL)

@interface ThemeChangedObserver : NSObject
{
  AdapSettingsImpl *impl;
}
@end

@implementation ThemeChangedObserver

-(instancetype)initWithSettings:(AdapSettingsImpl *)_impl
{
  [self init];
  g_set_weak_pointer (&self->impl, _impl);
  return self;
}

-(void)dealloc
{
  g_clear_weak_pointer (&self->impl);
  [super dealloc];
}

static AdapSystemColorScheme
get_ns_color_scheme (void)
{
  if (@available(*, macOS 10.14)) {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSString *style = [userDefaults stringForKey:@"AppleInterfaceStyle"];
    BOOL isDark = [style isEqualToString:@"Dark"];
#if 0
    BOOL isAuto = [userDefaults boolForKey:@"AppleInterfaceStyleSwitchesAutomatically"];
    BOOL isHighContrast = NO;

    /* We can get HighContrast using [NSAppearance currentAppearance] and
     * checking for the variants with HighContrast in their name, however
     * those do not update when the notifications come in (or ever it
     * seems unless a NSView changes them while drawing. If we can monitor
     * a NSView, we could watch for effectiveAppearance changes.
     */
#endif

    return isDark ?
      ADAP_SYSTEM_COLOR_SCHEME_PREFER_DARK :
      ADAP_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return ADAP_SYSTEM_COLOR_SCHEME_DEFAULT;
}

-(void)appDidChangeTheme:(NSNotification *)notification
{
  if (self->impl != NULL)
    adap_settings_impl_set_color_scheme (self->impl, get_ns_color_scheme ());
}
@end

static void
adap_settings_impl_macos_class_init (AdapSettingsImplMacOSClass *klass)
{
}

static void
adap_settings_impl_macos_init (AdapSettingsImplMacOS *self)
{
}

AdapSettingsImpl *
adap_settings_impl_macos_new (gboolean enable_color_scheme,
                             gboolean enable_high_contrast)
{
  AdapSettingsImplMacOS *self = g_object_new (ADAP_TYPE_SETTINGS_IMPL_MACOS, NULL);

  if (!enable_color_scheme)
    return ADAP_SETTINGS_IMPL (self);

  if (@available(*, macOS 10.14)) {
    static ThemeChangedObserver *observer;

    observer = [[ThemeChangedObserver alloc] initWithSettings:(AdapSettingsImpl *)self];

    [[NSDistributedNotificationCenter defaultCenter]
      addObserver:observer
        selector:@selector(appDidChangeTheme:)
            name:@"AppleInterfaceThemeChangedNotification"
          object:nil];

    [observer appDidChangeTheme:nil];

    adap_settings_impl_set_features (ADAP_SETTINGS_IMPL (self),
                                    /* has_color_scheme */ TRUE,
                                    /* has_high_contrast */ FALSE);
  }

  return ADAP_SETTINGS_IMPL (self);
}
