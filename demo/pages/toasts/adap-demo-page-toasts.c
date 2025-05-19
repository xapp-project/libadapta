#include "adw-demo-page-toasts.h"

#include <glib/gi18n.h>

struct _AdwDemoPageToasts
{
  AdwBin parent_instance;

  AdwToast *undo_toast;
  int toast_undo_items;
};

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdwDemoPageToasts, adw_demo_page_toasts, ADW_TYPE_BIN)

static void
add_toast (AdwDemoPageToasts *self,
           AdwToast          *toast)
{
  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
dismissed_cb (AdwDemoPageToasts *self)
{
  self->undo_toast = NULL;
  self->toast_undo_items = 0;

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);
}

static void
toast_add_cb (AdwDemoPageToasts *self)
{
  add_toast (self, adw_toast_new (_("Simple Toast")));
}

static void
toast_add_with_button_cb (AdwDemoPageToasts *self)
{
  self->toast_undo_items++;

  if (self->undo_toast) {
    char *title =
      g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
                                 "<span font_features='tnum=1'>%d</span> items deleted",
                                 self->toast_undo_items), self->toast_undo_items);

    adw_toast_set_title (self->undo_toast, title);

    /* Bump the toast timeout */
    add_toast (self, g_object_ref (self->undo_toast));

    g_free (title);
  } else {
    self->undo_toast = adw_toast_new_format (_("‘%s’ deleted"), "Lorem Ipsum");

    adw_toast_set_priority (self->undo_toast, ADW_TOAST_PRIORITY_HIGH);
    adw_toast_set_button_label (self->undo_toast, _("_Undo"));
    adw_toast_set_action_name (self->undo_toast, "toast.undo");

    g_signal_connect_swapped (self->undo_toast, "dismissed", G_CALLBACK (dismissed_cb), self);

    add_toast (self, self->undo_toast);

    gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", TRUE);
  }
}

static void
toast_add_with_long_title_cb (AdwDemoPageToasts *self)
{
  add_toast (self, adw_toast_new (_("Lorem ipsum dolor sit amet, "
                                    "consectetur adipiscing elit, "
                                    "sed do eiusmod tempor incididunt "
                                    "ut labore et dolore magnam aliquam "
                                    "quaerat voluptatem.")));
}

static void
toast_dismiss_cb (AdwDemoPageToasts *self)
{
  if (self->undo_toast)
    adw_toast_dismiss (self->undo_toast);
}

static void
adw_demo_page_toasts_class_init (AdwDemoPageToastsClass *klass)
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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/toasts/adw-demo-page-toasts.ui");

  gtk_widget_class_install_action (widget_class, "toast.add", NULL, (GtkWidgetActionActivateFunc) toast_add_cb);
  gtk_widget_class_install_action (widget_class, "toast.add-with-button", NULL, (GtkWidgetActionActivateFunc) toast_add_with_button_cb);
  gtk_widget_class_install_action (widget_class, "toast.add-with-long-title", NULL, (GtkWidgetActionActivateFunc) toast_add_with_long_title_cb);
  gtk_widget_class_install_action (widget_class, "toast.dismiss", NULL, (GtkWidgetActionActivateFunc) toast_dismiss_cb);
}

static void
adw_demo_page_toasts_init (AdwDemoPageToasts *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);
}

void
adw_demo_page_toasts_undo (AdwDemoPageToasts *self)
{
  char *title =
    g_strdup_printf (ngettext ("Undoing deleting <span font_features='tnum=1'>%d</span> item…",
                               "Undoing deleting <span font_features='tnum=1'>%d</span> items…",
                               self->toast_undo_items), self->toast_undo_items);
  AdwToast *toast = adw_toast_new (title);

  adw_toast_set_priority (toast, ADW_TOAST_PRIORITY_HIGH);

  add_toast (self, toast);

  g_free (title);
}
