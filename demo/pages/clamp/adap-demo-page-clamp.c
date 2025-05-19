#include "adw-demo-page-clamp.h"

#include <glib/gi18n.h>

struct _AdwDemoPageClamp
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageClamp, adw_demo_page_clamp, ADW_TYPE_BIN)

static void
adw_demo_page_clamp_class_init (AdwDemoPageClampClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/clamp/adw-demo-page-clamp.ui");
}

static void
adw_demo_page_clamp_init (AdwDemoPageClamp *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
