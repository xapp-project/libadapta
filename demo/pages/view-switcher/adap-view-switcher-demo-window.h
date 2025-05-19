#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_DEMO_WINDOW (adw_view_switcher_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwViewSwitcherDemoWindow, adw_view_switcher_demo_window, ADW, VIEW_SWITCHER_DEMO_WINDOW, AdwDialog)

AdwViewSwitcherDemoWindow *adw_view_switcher_demo_window_new (void);

G_END_DECLS
