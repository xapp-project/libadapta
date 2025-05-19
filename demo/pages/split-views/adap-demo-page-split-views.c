#include "adap-demo-page-split-views.h"

#include <glib/gi18n.h>

#include "adap-navigation-split-view-demo-window.h"
#include "adap-overlay-split-view-demo-window.h"

struct _AdapDemoPageSplitViews
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageSplitViews, adap_demo_page_split_views, ADAP_TYPE_BIN)

static void
demo_run_navigation_cb (AdapDemoPageSplitViews *self)
{
  AdapNavigationSplitViewDemoWindow *window = adap_navigation_split_view_demo_window_new ();

  adap_dialog_present (ADAP_DIALOG (window), GTK_WIDGET (self));
}

static void
demo_run_overlay_cb (AdapDemoPageSplitViews *self)
{
  AdapOverlaySplitViewDemoWindow *window = adap_overlay_split_view_demo_window_new ();

  adap_dialog_present (ADAP_DIALOG (window), GTK_WIDGET (self));
}

static void
adap_demo_page_split_views_class_init (AdapDemoPageSplitViewsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/split-views/adap-demo-page-split-views.ui");

  gtk_widget_class_install_action (widget_class, "demo.run-navigation", NULL, (GtkWidgetActionActivateFunc) demo_run_navigation_cb);
  gtk_widget_class_install_action (widget_class, "demo.run-overlay", NULL, (GtkWidgetActionActivateFunc) demo_run_overlay_cb);
}

static void
adap_demo_page_split_views_init (AdapDemoPageSplitViews *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
