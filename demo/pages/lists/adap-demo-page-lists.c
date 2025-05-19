#include "adw-demo-page-lists.h"

#include <glib/gi18n.h>

struct _AdwDemoPageLists
{
  AdwBin parent_instance;
};

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdwDemoPageLists, adw_demo_page_lists, ADW_TYPE_BIN)

static void
entry_apply_cb (AdwDemoPageLists *self)
{
  AdwToast *toast = adw_toast_new ("Changes applied");

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
adw_demo_page_lists_class_init (AdwDemoPageListsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  signals[SIGNAL_ADD_TOAST] =
    g_signal_new ("add-toast",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  ADW_TYPE_TOAST);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/lists/adw-demo-page-lists.ui");

  gtk_widget_class_bind_template_callback (widget_class, entry_apply_cb);
}

static void
adw_demo_page_lists_init (AdwDemoPageLists *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
