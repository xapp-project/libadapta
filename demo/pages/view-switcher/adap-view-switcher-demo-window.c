#include "adap-view-switcher-demo-window.h"

#include <glib/gi18n.h>

struct _AdapViewSwitcherDemoWindow
{
  AdapDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapViewSwitcherDemoWindow, adap_view_switcher_demo_window, ADAP_TYPE_DIALOG)

static void
adap_view_switcher_demo_window_class_init (AdapViewSwitcherDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/view-switcher/adap-view-switcher-demo-window.ui");
}

static void
adap_view_switcher_demo_window_init (AdapViewSwitcherDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdapViewSwitcherDemoWindow *
adap_view_switcher_demo_window_new (void)
{
  return g_object_new (ADAP_TYPE_VIEW_SWITCHER_DEMO_WINDOW, NULL);
}
