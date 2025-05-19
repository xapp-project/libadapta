#include "adap-demo-page-toasts.h"

#include <glib/gi18n.h>

struct _AdapDemoPageToasts
{
  AdapBin parent_instance;

  AdapToast *undo_toast;
  int toast_undo_items;
};

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdapDemoPageToasts, adap_demo_page_toasts, ADAP_TYPE_BIN)

static void
add_toast (AdapDemoPageToasts *self,
           AdapToast          *toast)
{
  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
dismissed_cb (AdapDemoPageToasts *self)
{
  self->undo_toast = NULL;
  self->toast_undo_items = 0;

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);
}

static void
toast_add_cb (AdapDemoPageToasts *self)
{
  add_toast (self, adap_toast_new (_("Simple Toast")));
}

static void
toast_add_with_button_cb (AdapDemoPageToasts *self)
{
  self->toast_undo_items++;

  if (self->undo_toast) {
    char *title =
      g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
                                 "<span font_features='tnum=1'>%d</span> items deleted",
                                 self->toast_undo_items), self->toast_undo_items);

    adap_toast_set_title (self->undo_toast, title);

    /* Bump the toast timeout */
    add_toast (self, g_object_ref (self->undo_toast));

    g_free (title);
  } else {
    self->undo_toast = adap_toast_new_format (_("‘%s’ deleted"), "Lorem Ipsum");

    adap_toast_set_priority (self->undo_toast, ADAP_TOAST_PRIORITY_HIGH);
    adap_toast_set_button_label (self->undo_toast, _("_Undo"));
    adap_toast_set_action_name (self->undo_toast, "toast.undo");

    g_signal_connect_swapped (self->undo_toast, "dismissed", G_CALLBACK (dismissed_cb), self);

    add_toast (self, self->undo_toast);

    gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", TRUE);
  }
}

static void
toast_add_with_long_title_cb (AdapDemoPageToasts *self)
{
  add_toast (self, adap_toast_new (_("Lorem ipsum dolor sit amet, "
                                    "consectetur adipiscing elit, "
                                    "sed do eiusmod tempor incididunt "
                                    "ut labore et dolore magnam aliquam "
                                    "quaerat voluptatem.")));
}

static void
toast_dismiss_cb (AdapDemoPageToasts *self)
{
  if (self->undo_toast)
    adap_toast_dismiss (self->undo_toast);
}

static void
adap_demo_page_toasts_class_init (AdapDemoPageToastsClass *klass)
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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/toasts/adap-demo-page-toasts.ui");

  gtk_widget_class_install_action (widget_class, "toast.add", NULL, (GtkWidgetActionActivateFunc) toast_add_cb);
  gtk_widget_class_install_action (widget_class, "toast.add-with-button", NULL, (GtkWidgetActionActivateFunc) toast_add_with_button_cb);
  gtk_widget_class_install_action (widget_class, "toast.add-with-long-title", NULL, (GtkWidgetActionActivateFunc) toast_add_with_long_title_cb);
  gtk_widget_class_install_action (widget_class, "toast.dismiss", NULL, (GtkWidgetActionActivateFunc) toast_dismiss_cb);
}

static void
adap_demo_page_toasts_init (AdapDemoPageToasts *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);
}

void
adap_demo_page_toasts_undo (AdapDemoPageToasts *self)
{
  char *title =
    g_strdup_printf (ngettext ("Undoing deleting <span font_features='tnum=1'>%d</span> item…",
                               "Undoing deleting <span font_features='tnum=1'>%d</span> items…",
                               self->toast_undo_items), self->toast_undo_items);
  AdapToast *toast = adap_toast_new (title);

  adap_toast_set_priority (toast, ADAP_TOAST_PRIORITY_HIGH);

  add_toast (self, toast);

  g_free (title);
}
