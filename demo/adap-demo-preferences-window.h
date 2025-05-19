#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_DEMO_PREFERENCES_WINDOW (adap_demo_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (AdapDemoPreferencesWindow, adap_demo_preferences_window, ADAP, DEMO_PREFERENCES_WINDOW, AdapPreferencesDialog)

AdapDemoPreferencesWindow *adap_demo_preferences_window_new (void);

G_END_DECLS
