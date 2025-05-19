#include "adap-demo-page-styles.h"

#include <glib/gi18n.h>

#include "adap-style-demo-window.h"

struct _AdapDemoPageStyles
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageStyles, adap_demo_page_styles, ADAP_TYPE_BIN)

static void
demo_run_cb (AdapDemoPageStyles *self)
{
  AdapStyleDemoWindow *window = adap_style_demo_window_new ();

  adap_dialog_present (ADAP_DIALOG (window), GTK_WIDGET (self));
}

static void
adap_demo_page_styles_class_init (AdapDemoPageStylesClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/styles/adap-demo-page-styles.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adap_demo_page_styles_init (AdapDemoPageStyles *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
