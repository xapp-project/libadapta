#include "adap-demo-page-buttons.h"

#include <glib/gi18n.h>

struct _AdapDemoPageButtons
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageButtons, adap_demo_page_buttons, ADAP_TYPE_BIN)

static void
adap_demo_page_buttons_class_init (AdapDemoPageButtonsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/buttons/adap-demo-page-buttons.ui");
}

static void
adap_demo_page_buttons_init (AdapDemoPageButtons *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
