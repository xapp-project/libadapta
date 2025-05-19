#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_OVERLAY_SPLIT_VIEW_DEMO_WINDOW (adap_overlay_split_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapOverlaySplitViewDemoWindow, adap_overlay_split_view_demo_window, ADAP, OVERLAY_SPLIT_VIEW_DEMO_WINDOW, AdapDialog)

AdapOverlaySplitViewDemoWindow *adap_overlay_split_view_demo_window_new (void);

G_END_DECLS
