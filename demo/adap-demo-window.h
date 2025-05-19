#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_WINDOW (adw_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoWindow, adw_demo_window, ADW, DEMO_WINDOW, AdwApplicationWindow)

AdwDemoWindow *adw_demo_window_new (GtkApplication *application);

G_END_DECLS
