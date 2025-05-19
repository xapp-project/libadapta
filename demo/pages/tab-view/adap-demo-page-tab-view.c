#include "adap-demo-page-tab-view.h"

#include <glib/gi18n.h>

#include "adap-tab-view-demo-window.h"

struct _AdapDemoPageTabView
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageTabView, adap_demo_page_tab_view, ADAP_TYPE_BIN)

static void
demo_run_cb (AdapDemoPageTabView *self)
{
  AdapTabViewDemoWindow *window = adap_tab_view_demo_window_new ();
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  adap_tab_view_demo_window_prepopulate (window);

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (root));
  gtk_window_present (GTK_WINDOW (window));
}

static void
adap_demo_page_tab_view_class_init (AdapDemoPageTabViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/tab-view/adap-demo-page-tab-view.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adap_demo_page_tab_view_init (AdapDemoPageTabView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
