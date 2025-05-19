/*
 * Copyright (C) 2022 Christian Hergert <christian@hergert.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <AppKit/AppKit.h>

struct _AdwSettingsImplMacOS
{
  AdwSettingsImpl parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplMacOS, adw_settings_impl_macos, ADW_TYPE_SETTINGS_IMPL)

@interface ThemeChangedObserver : NSObject
{
  AdwSettingsImpl *impl;
}
@end

@implementation ThemeChangedObserver

-(instancetype)initWithSettings:(AdwSettingsImpl *)_impl
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

static AdwSystemColorScheme
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
      ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK :
      ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

-(void)appDidChangeTheme:(NSNotification *)notification
{
  if (self->impl != NULL)
    adw_settings_impl_set_color_scheme (self->impl, get_ns_color_scheme ());
}
@end

static void
adw_settings_impl_macos_class_init (AdwSettingsImplMacOSClass *klass)
{
}

static void
adw_settings_impl_macos_init (AdwSettingsImplMacOS *self)
{
}

AdwSettingsImpl *
adw_settings_impl_macos_new (gboolean enable_color_scheme,
                             gboolean enable_high_contrast)
{
  AdwSettingsImplMacOS *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_MACOS, NULL);

  if (!enable_color_scheme)
    return ADW_SETTINGS_IMPL (self);

  if (@available(*, macOS 10.14)) {
    static ThemeChangedObserver *observer;

    observer = [[ThemeChangedObserver alloc] initWithSettings:(AdwSettingsImpl *)self];

    [[NSDistributedNotificationCenter defaultCenter]
      addObserver:observer
        selector:@selector(appDidChangeTheme:)
            name:@"AppleInterfaceThemeChangedNotification"
          object:nil];

    [observer appDidChangeTheme:nil];

    adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                    /* has_color_scheme */ TRUE,
                                    /* has_high_contrast */ FALSE);
  }

  return ADW_SETTINGS_IMPL (self);
}
