#include "adap-demo-page-lists.h"

#include <glib/gi18n.h>

struct _AdapDemoPageLists
{
  AdapBin parent_instance;
};

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdapDemoPageLists, adap_demo_page_lists, ADAP_TYPE_BIN)

static void
entry_apply_cb (AdapDemoPageLists *self)
{
  AdapToast *toast = adap_toast_new ("Changes applied");

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
adap_demo_page_lists_class_init (AdapDemoPageListsClass *klass)
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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/lists/adap-demo-page-lists.ui");

  gtk_widget_class_bind_template_callback (widget_class, entry_apply_cb);
}

static void
adap_demo_page_lists_init (AdapDemoPageLists *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
