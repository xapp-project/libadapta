#include "adw-demo-page-dialogs.h"

#include <glib/gi18n.h>

struct _AdwDemoPageDialogs
{
  AdwBin parent_instance;

  AdwToast *last_toast;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageDialogs, adw_demo_page_dialogs, ADW_TYPE_BIN)

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
toast_dismissed_cb (AdwToast           *toast,
                    AdwDemoPageDialogs *self)
{
  if (toast == self->last_toast)
    self->last_toast = NULL;
}

static void
alert_cb (AdwAlertDialog     *dialog,
          GAsyncResult       *result,
          AdwDemoPageDialogs *self)
{
  const char *response = adw_alert_dialog_choose_finish (dialog, result);
  AdwToast *toast = adw_toast_new_format (_("Dialog response: %s"), response);
  g_signal_connect_object (toast, "dismissed", G_CALLBACK (toast_dismissed_cb), self, 0);

  if (self->last_toast)
    adw_toast_dismiss (self->last_toast);
  self->last_toast = toast;

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
demo_alert_dialog_cb (AdwDemoPageDialogs *self)
{
  AdwDialog *dialog;

  dialog = adw_alert_dialog_new (_("Save Changes?"),
                                 _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "discard", _("_Discard"),
                                  "save",    _("_Save"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "discard",
                                            ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "save",
                                            ADW_RESPONSE_SUGGESTED);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "save");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  adw_alert_dialog_choose (ADW_ALERT_DIALOG (dialog), GTK_WIDGET (self), NULL,
                           (GAsyncReadyCallback) alert_cb, self);
}

static void
adw_demo_page_dialogs_class_init (AdwDemoPageDialogsClass *klass)
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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/dialogs/adw-demo-page-dialogs.ui");

  gtk_widget_class_install_action (widget_class, "demo.alert-dialog", NULL, (GtkWidgetActionActivateFunc) demo_alert_dialog_cb);
}

static void
adw_demo_page_dialogs_init (AdwDemoPageDialogs *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
