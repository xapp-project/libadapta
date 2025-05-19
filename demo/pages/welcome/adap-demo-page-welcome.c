#include "adap-demo-page-welcome.h"

#include <glib/gi18n.h>

struct _AdapDemoPageWelcome
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageWelcome, adap_demo_page_welcome, ADAP_TYPE_BIN)

static void
adap_demo_page_welcome_class_init (AdapDemoPageWelcomeClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/welcome/adap-demo-page-welcome.ui");
}

static void
adap_demo_page_welcome_init (AdapDemoPageWelcome *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
