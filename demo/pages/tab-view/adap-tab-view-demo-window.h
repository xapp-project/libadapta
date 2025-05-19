#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_VIEW_DEMO_WINDOW (adap_tab_view_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapTabViewDemoWindow, adap_tab_view_demo_window, ADAP, TAB_VIEW_DEMO_WINDOW, AdapWindow)

AdapTabViewDemoWindow *adap_tab_view_demo_window_new (void);

void adap_tab_view_demo_window_prepopulate (AdapTabViewDemoWindow *self);

G_END_DECLS
