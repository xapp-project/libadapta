#include "adap-demo-page-view-switcher.h"

#include <glib/gi18n.h>

#include "adap-view-switcher-demo-window.h"

struct _AdapDemoPageViewSwitcher
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageViewSwitcher, adap_demo_page_view_switcher, ADAP_TYPE_BIN)

static void
demo_run_cb (AdapDemoPageViewSwitcher *self)
{
  AdapViewSwitcherDemoWindow *window = adap_view_switcher_demo_window_new ();

  adap_dialog_present (ADAP_DIALOG (window), GTK_WIDGET (self));
}

static void
adap_demo_page_view_switcher_class_init (AdapDemoPageViewSwitcherClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/view-switcher/adap-demo-page-view-switcher.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adap_demo_page_view_switcher_init (AdapDemoPageViewSwitcher *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
