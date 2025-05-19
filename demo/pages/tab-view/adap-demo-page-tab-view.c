#include "adw-demo-page-tab-view.h"

#include <glib/gi18n.h>

#include "adw-tab-view-demo-window.h"

struct _AdwDemoPageTabView
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageTabView, adw_demo_page_tab_view, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageTabView *self)
{
  AdwTabViewDemoWindow *window = adw_tab_view_demo_window_new ();
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  adw_tab_view_demo_window_prepopulate (window);

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (root));
  gtk_window_present (GTK_WINDOW (window));
}

static void
adw_demo_page_tab_view_class_init (AdwDemoPageTabViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/tab-view/adw-demo-page-tab-view.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_tab_view_init (AdwDemoPageTabView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
