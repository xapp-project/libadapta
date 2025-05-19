#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_NAVIGATION_VIEW_DEMO_WINDOW (adap_navigation_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapNavigationViewDemoWindow, adap_navigation_view_demo_window, ADAP, NAVIGATION_VIEW_DEMO_WINDOW, AdapDialog)

AdapNavigationViewDemoWindow *adap_navigation_view_demo_window_new (void);

G_END_DECLS
