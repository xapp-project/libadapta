#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_OVERLAY_SPLIT_VIEW_DEMO_WINDOW (adw_overlay_split_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwOverlaySplitViewDemoWindow, adw_overlay_split_view_demo_window, ADW, OVERLAY_SPLIT_VIEW_DEMO_WINDOW, AdwDialog)

AdwOverlaySplitViewDemoWindow *adw_overlay_split_view_demo_window_new (void);

G_END_DECLS
