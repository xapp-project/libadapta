#include "adw-demo-page-welcome.h"

#include <glib/gi18n.h>

struct _AdwDemoPageWelcome
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageWelcome, adw_demo_page_welcome, ADW_TYPE_BIN)

static void
adw_demo_page_welcome_class_init (AdwDemoPageWelcomeClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/welcome/adw-demo-page-welcome.ui");
}

static void
adw_demo_page_welcome_init (AdwDemoPageWelcome *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
