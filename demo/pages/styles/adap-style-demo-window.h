#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_STYLE_DEMO_WINDOW (adw_style_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwStyleDemoWindow, adw_style_demo_window, ADW, STYLE_DEMO_WINDOW, AdwDialog)

AdwStyleDemoWindow *adw_style_demo_window_new (void);

G_END_DECLS
