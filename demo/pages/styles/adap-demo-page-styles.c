#include "adw-demo-page-styles.h"

#include <glib/gi18n.h>

#include "adw-style-demo-window.h"

struct _AdwDemoPageStyles
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageStyles, adw_demo_page_styles, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageStyles *self)
{
  AdwStyleDemoWindow *window = adw_style_demo_window_new ();

  adw_dialog_present (ADW_DIALOG (window), GTK_WIDGET (self));
}

static void
adw_demo_page_styles_class_init (AdwDemoPageStylesClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/styles/adw-demo-page-styles.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_styles_init (AdwDemoPageStyles *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
