#include "adw-view-switcher-demo-window.h"

#include <glib/gi18n.h>

struct _AdwViewSwitcherDemoWindow
{
  AdwDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwViewSwitcherDemoWindow, adw_view_switcher_demo_window, ADW_TYPE_DIALOG)

static void
adw_view_switcher_demo_window_class_init (AdwViewSwitcherDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/view-switcher/adw-view-switcher-demo-window.ui");
}

static void
adw_view_switcher_demo_window_init (AdwViewSwitcherDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwViewSwitcherDemoWindow *
adw_view_switcher_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_DEMO_WINDOW, NULL);
}
