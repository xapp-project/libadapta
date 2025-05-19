#include "adap-demo-page-navigation-view.h"

#include <glib/gi18n.h>

#include "adap-navigation-view-demo-window.h"

struct _AdapDemoPageNavigationView
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageNavigationView, adap_demo_page_navigation_view, ADAP_TYPE_BIN)

static void
demo_run_cb (AdapDemoPageNavigationView *self)
{
  AdapNavigationViewDemoWindow *window = adap_navigation_view_demo_window_new ();

  adap_dialog_present (ADAP_DIALOG (window), GTK_WIDGET (self));
}

static void
adap_demo_page_navigation_view_class_init (AdapDemoPageNavigationViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/navigation-view/adap-demo-page-navigation-view.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adap_demo_page_navigation_view_init (AdapDemoPageNavigationView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
