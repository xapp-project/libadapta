#include "adap-demo-page-banners.h"

#include <glib/gi18n.h>

struct _AdapDemoPageBanners
{
  AdapBin parent_instance;

  AdapBanner *banner;
  AdapEntryRow *button_label_row;
};

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_TYPE (AdapDemoPageBanners, adap_demo_page_banners, ADAP_TYPE_BIN)

static void
toggle_button_cb (AdapDemoPageBanners *self)
{
  if (g_strcmp0 (adap_banner_get_button_label (self->banner), "") == 0)
    adap_banner_set_button_label (self->banner, gtk_editable_get_text (GTK_EDITABLE (self->button_label_row)));
  else
    adap_banner_set_button_label (self->banner, NULL);
}

static void
banner_activate_cb (AdapDemoPageBanners *self)
{
  AdapToast *toast = adap_toast_new (_("Banner action triggered"));

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
adap_demo_page_banners_class_init (AdapDemoPageBannersClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  signals[SIGNAL_ADD_TOAST] =
    g_signal_new ("add-toast",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  ADAP_TYPE_TOAST);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta1/Demo/ui/pages/banners/adap-demo-page-banners.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageBanners, banner);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageBanners, button_label_row);

  gtk_widget_class_install_action (widget_class, "demo.toggle-button", NULL, (GtkWidgetActionActivateFunc) toggle_button_cb);
  gtk_widget_class_install_action (widget_class, "demo.activate", NULL, (GtkWidgetActionActivateFunc) banner_activate_cb);
}

static void
adap_demo_page_banners_init (AdapDemoPageBanners *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
