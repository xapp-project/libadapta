#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_PREFERENCES_WINDOW (adw_demo_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoPreferencesWindow, adw_demo_preferences_window, ADW, DEMO_PREFERENCES_WINDOW, AdwPreferencesDialog)

AdwDemoPreferencesWindow *adw_demo_preferences_window_new (void);

G_END_DECLS
