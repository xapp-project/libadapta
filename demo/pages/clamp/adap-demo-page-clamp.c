#include "adap-demo-page-clamp.h"

#include <glib/gi18n.h>

struct _AdapDemoPageClamp
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageClamp, adap_demo_page_clamp, ADAP_TYPE_BIN)

static void
adap_demo_page_clamp_class_init (AdapDemoPageClampClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/clamp/adap-demo-page-clamp.ui");
}

static void
adap_demo_page_clamp_init (AdapDemoPageClamp *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
