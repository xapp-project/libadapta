#include "adw-demo-page-buttons.h"

#include <glib/gi18n.h>

struct _AdwDemoPageButtons
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageButtons, adw_demo_page_buttons, ADW_TYPE_BIN)

static void
adw_demo_page_buttons_class_init (AdwDemoPageButtonsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/buttons/adw-demo-page-buttons.ui");
}

static void
adw_demo_page_buttons_init (AdwDemoPageButtons *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
