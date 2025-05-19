#include <adapta.h>
#include <glib/gi18n.h>

static void
response_cb (AdapAlertDialog *dialog,
             const char     *response)
{
  g_message ("Response: %s", response);
}

static void
response_text_cb (AdapAlertDialog *dialog,
                  const char     *response)
{
  GtkWidget *entry = adap_alert_dialog_get_extra_child (dialog);
  const char *text;

  g_assert (GTK_IS_EDITABLE (entry));

  text = gtk_editable_get_text (GTK_EDITABLE (entry));

  g_message ("Response: %s, text: %s", response, text);
}

static void
dialog_cb (AdapAlertDialog *dialog,
           GAsyncResult   *result,
           gpointer        user_data)
{
  const char *response = adap_alert_dialog_choose_finish (dialog, result);

  g_message ("Response: %s", response);
}

/* This dialog will always have horizontal buttons */
static void
simple_cb (GtkWidget *parent)
{
  AdapDialog *dialog =
    adap_alert_dialog_new (_("Replace File?"),
                          _("A file named “example.png” already exists. Do you want to replace it?"));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "replace", _("_Replace"),
                                  NULL);

  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "replace",
                                            ADAP_RESPONSE_DESTRUCTIVE);

  adap_alert_dialog_set_default_response (ADAP_ALERT_DIALOG (dialog), "cancel");
  adap_alert_dialog_set_close_response (ADAP_ALERT_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adap_dialog_present (dialog, parent);
}

/* This dialog will have horizontal or vertical buttons, depending on the available room */
static void
adaptive_cb (GtkWidget *parent)
{
  AdapDialog *dialog =
    adap_alert_dialog_new (_("Save Changes?"),
                          _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "discard", _("_Discard Changes"),
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

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adap_dialog_present (dialog, parent);
}

/* This dialog will always have vertical buttons */
static void
wide_cb (GtkWidget *parent)
{
  AdapDialog *dialog =
    adap_alert_dialog_new (_("Do you want to empty the wastebasket before you unmount?"),
                          _("In order to regain the free space on the volume the wastebasket must be emptied. All deleted items on the volume will be permanently lost."));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "ignore", _("Do _not Empty Wastebasket"),
                                  "cancel", _("_Cancel"),
                                  "empty",  _("_Empty Wastebasket"),
                                  NULL);

  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "empty",
                                            ADAP_RESPONSE_DESTRUCTIVE);

  adap_alert_dialog_set_default_response (ADAP_ALERT_DIALOG (dialog), "cancel");
  adap_alert_dialog_set_close_response (ADAP_ALERT_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adap_dialog_present (dialog, parent);
}

static void
entry_changed_cb (GtkEditable    *editable,
                  AdapAlertDialog *dialog)
{
  const char *text = gtk_editable_get_text (editable);

  if (text && *text) {
    adap_alert_dialog_set_response_enabled (dialog, "add", TRUE);
    gtk_widget_remove_css_class (GTK_WIDGET (editable), "error");
  } else {
    adap_alert_dialog_set_response_enabled (dialog, "add", FALSE);
    gtk_widget_add_css_class (GTK_WIDGET (editable), "error");
  }
}

static void
child_cb (GtkWidget *parent)
{
  AdapDialog *dialog;
  GtkWidget *entry;

  dialog =
    adap_alert_dialog_new (_("Add New Profile"),
                          _("Enter name of the new profile"));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "add", _("_Add"),
                                  NULL);

  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "add",
                                            ADAP_RESPONSE_SUGGESTED);

  adap_alert_dialog_set_default_response (ADAP_ALERT_DIALOG (dialog), "add");
  adap_alert_dialog_set_close_response (ADAP_ALERT_DIALOG (dialog), "cancel");

  adap_alert_dialog_set_response_enabled (ADAP_ALERT_DIALOG (dialog), "add", FALSE);

  entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Name"));
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  g_signal_connect (entry, "changed", G_CALLBACK (entry_changed_cb), dialog);
  adap_alert_dialog_set_extra_child (ADAP_ALERT_DIALOG (dialog), entry);

  g_signal_connect (dialog, "response::add", G_CALLBACK (response_text_cb), NULL);
  g_signal_connect (dialog, "response::cancel", G_CALLBACK (response_cb), NULL);

  adap_dialog_present (dialog, parent);
}

static void
async_cb (GtkWidget *parent)
{
  AdapDialog *dialog =
    adap_alert_dialog_new (_("Replace File?"),
                          _("A file named “example.png” already exists. Do you want to replace it?"));

  adap_alert_dialog_add_responses (ADAP_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "replace", _("_Replace"),
                                  NULL);

  adap_alert_dialog_set_response_appearance (ADAP_ALERT_DIALOG (dialog),
                                            "replace",
                                            ADAP_RESPONSE_DESTRUCTIVE);

  adap_alert_dialog_set_default_response (ADAP_ALERT_DIALOG (dialog), "cancel");
  adap_alert_dialog_set_close_response (ADAP_ALERT_DIALOG (dialog), "cancel");

  adap_alert_dialog_choose (ADAP_ALERT_DIALOG (dialog), GTK_WIDGET (parent),
                            NULL, (GAsyncReadyCallback) dialog_cb, NULL);
}

static GtkWidget *
create_content (GtkWidget *parent)
{
  GtkWidget *view, *box, *button;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
  gtk_widget_set_margin_top (box, 48);
  gtk_widget_set_margin_bottom (box, 48);
  gtk_widget_set_margin_start (box, 48);
  gtk_widget_set_margin_end (box, 48);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label ("Simple Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (simple_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Adaptive Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (adaptive_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Wide Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (wide_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Extra Child");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (child_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Async Call");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (async_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  view = adap_toolbar_view_new ();
  adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (view), adap_header_bar_new ());
  adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (view), box);

  return view;
}

static void
close_cb (gboolean *done)
{
  *done = TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *window;
  gboolean done = FALSE;

  adap_init ();

  window = adap_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Alert Dialogs");
  adap_window_set_content (ADAP_WINDOW (window), create_content (window));
  gtk_widget_set_size_request (window, 360, -1);
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
