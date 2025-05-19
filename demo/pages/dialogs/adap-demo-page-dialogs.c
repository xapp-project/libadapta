#include "adap-demo-page-dialogs.h"

#include <glib/gi18n.h>

struct _AdapDemoPageDialogs
{
  AdapBin parent_instance;

  AdapToast *last_toast;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageDialogs, adap_demo_page_dialogs, ADAP_TYPE_BIN)

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
toast_dismissed_cb (AdapToast           *toast,
                    AdapDemoPageDialogs *self)
{
  if (toast == self->last_toast)
    self->last_toast = NULL;
}

static void
alert_cb (AdapAlertDialog     *dialog,
          GAsyncResult       *result,
          AdapDemoPageDialogs *self)
{
  const char *response = adap_alert_dialog_choose_finish (dialog, result);
  AdapToast *toast = adap_toast_new_format (_("Dialog response: %s"), response);
  g_signal_connect_object (toast, "dismissed", G_CALLBACK (toast_dismissed_cb), self, 0);

  if (self->last_toast)
    adap_toast_dismiss (self->last_toast);
  self->last_toast = toast;

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
demo_alert_dialog_cb (AdapDemoPageDialogs *self)
{
  AdapDialog *dialog;

  dialog = adap_alert_dialog_new (_("Save Changes?"),
                                 _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "discard", _("_Discard"),
                                  "save",    _("_Save"),
                                  NULL);

  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "discard",
                                            ADAP_RESPONSE_DESTRUCTIVE);
  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "save",
                                            ADAP_RESPONSE_SUGGESTED);

  adap_alert_dialog_set_default_response (ADAP_ALERT_DIALOG (dialog), "save");
  adap_alert_dialog_set_close_response (ADAP_ALERT_DIALOG (dialog), "cancel");

  adap_alert_dialog_choose (ADAP_ALERT_DIALOG (dialog), GTK_WIDGET (self), NULL,
                           (GAsyncReadyCallback) alert_cb, self);
}

static void
adap_demo_page_dialogs_class_init (AdapDemoPageDialogsClass *klass)
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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/dialogs/adap-demo-page-dialogs.ui");

  gtk_widget_class_install_action (widget_class, "demo.alert-dialog", NULL, (GtkWidgetActionActivateFunc) demo_alert_dialog_cb);
}

static void
adap_demo_page_dialogs_init (AdapDemoPageDialogs *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
