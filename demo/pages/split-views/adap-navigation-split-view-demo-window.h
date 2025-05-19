#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_NAVIGATION_SPLIT_VIEW_DEMO_WINDOW (adw_navigation_split_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwNavigationSplitViewDemoWindow, adw_navigation_split_view_demo_window, ADW, NAVIGATION_SPLIT_VIEW_DEMO_WINDOW, AdwDialog)

AdwNavigationSplitViewDemoWindow *adw_navigation_split_view_demo_window_new (void);

G_END_DECLS
