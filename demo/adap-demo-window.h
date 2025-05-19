#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_DEMO_WINDOW (adap_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdapDemoWindow, adap_demo_window, ADAP, DEMO_WINDOW, AdapApplicationWindow)

AdapDemoWindow *adap_demo_window_new (GtkApplication *application);

G_END_DECLS
