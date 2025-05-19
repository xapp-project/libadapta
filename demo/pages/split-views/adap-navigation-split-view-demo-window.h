#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_NAVIGATION_SPLIT_VIEW_DEMO_WINDOW (adap_navigation_split_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapNavigationSplitViewDemoWindow, adap_navigation_split_view_demo_window, ADAP, NAVIGATION_SPLIT_VIEW_DEMO_WINDOW, AdapDialog)

AdapNavigationSplitViewDemoWindow *adap_navigation_split_view_demo_window_new (void);

G_END_DECLS
