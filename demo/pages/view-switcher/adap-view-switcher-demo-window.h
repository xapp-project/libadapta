#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_VIEW_SWITCHER_DEMO_WINDOW (adap_view_switcher_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapViewSwitcherDemoWindow, adap_view_switcher_demo_window, ADAP, VIEW_SWITCHER_DEMO_WINDOW, AdapDialog)

AdapViewSwitcherDemoWindow *adap_view_switcher_demo_window_new (void);

G_END_DECLS
