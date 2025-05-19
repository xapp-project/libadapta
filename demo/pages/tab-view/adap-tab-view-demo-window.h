#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_TAB_VIEW_DEMO_WINDOW (adw_tab_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwTabViewDemoWindow, adw_tab_view_demo_window, ADW, TAB_VIEW_DEMO_WINDOW, AdwWindow)

AdwTabViewDemoWindow *adw_tab_view_demo_window_new (void);

void adw_tab_view_demo_window_prepopulate (AdwTabViewDemoWindow *self);

G_END_DECLS
