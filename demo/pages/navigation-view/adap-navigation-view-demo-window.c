#include "adw-navigation-view-demo-window.h"

#include <glib/gi18n.h>

struct _AdwNavigationViewDemoWindow
{
  AdwDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwNavigationViewDemoWindow, adw_navigation_view_demo_window, ADW_TYPE_DIALOG)

static void
adw_navigation_view_demo_window_class_init (AdwNavigationViewDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/navigation-view/adw-navigation-view-demo-window.ui");
}

static void
adw_navigation_view_demo_window_init (AdwNavigationViewDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwNavigationViewDemoWindow *
adw_navigation_view_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_NAVIGATION_VIEW_DEMO_WINDOW, NULL);
}
