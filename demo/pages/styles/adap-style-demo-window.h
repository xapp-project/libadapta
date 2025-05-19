#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_STYLE_DEMO_WINDOW (adap_style_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapStyleDemoWindow, adap_style_demo_window, ADAP, STYLE_DEMO_WINDOW, AdapDialog)

AdapStyleDemoWindow *adap_style_demo_window_new (void);

G_END_DECLS
