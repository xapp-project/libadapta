#include "adap-navigation-view-demo-window.h"

#include <glib/gi18n.h>

struct _AdapNavigationViewDemoWindow
{
  AdapDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapNavigationViewDemoWindow, adap_navigation_view_demo_window, ADAP_TYPE_DIALOG)

static void
adap_navigation_view_demo_window_class_init (AdapNavigationViewDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/navigation-view/adap-navigation-view-demo-window.ui");
}

static void
adap_navigation_view_demo_window_init (AdapNavigationViewDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdapNavigationViewDemoWindow *
adap_navigation_view_demo_window_new (void)
{
  return g_object_new (ADAP_TYPE_NAVIGATION_VIEW_DEMO_WINDOW, NULL);
}
