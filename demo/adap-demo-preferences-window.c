#include "adap-demo-preferences-window.h"

struct _AdapDemoPreferencesWindow
{
  AdapPreferencesDialog parent_instance;

  AdapNavigationPage *subpage1;
  AdapNavigationPage *subpage2;
};

G_DEFINE_FINAL_TYPE (AdapDemoPreferencesWindow, adap_demo_preferences_window, ADAP_TYPE_PREFERENCES_DIALOG)

AdapDemoPreferencesWindow *
adap_demo_preferences_window_new (void)
{
  return g_object_new (ADAP_TYPE_DEMO_PREFERENCES_WINDOW, NULL);
}

static void
subpage1_activated_cb (AdapDemoPreferencesWindow *self)
{
  adap_preferences_dialog_push_subpage (ADAP_PREFERENCES_DIALOG (self), self->subpage1);
}

static void
subpage2_activated_cb (AdapDemoPreferencesWindow *self)
{
  adap_preferences_dialog_push_subpage (ADAP_PREFERENCES_DIALOG (self), self->subpage2);
}

static void
toast_show_cb (AdapPreferencesDialog *window)
{
  adap_preferences_dialog_add_toast (window, adap_toast_new ("Example Toast"));
}

static void
adap_demo_preferences_window_class_init (AdapDemoPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/adap-demo-preferences-window.ui");

  gtk_widget_class_bind_template_child (widget_class, AdapDemoPreferencesWindow, subpage1);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPreferencesWindow, subpage2);

  gtk_widget_class_bind_template_callback (widget_class, subpage1_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage2_activated_cb);

  gtk_widget_class_install_action (widget_class, "toast.show", NULL, (GtkWidgetActionActivateFunc) toast_show_cb);
}

static void
adap_demo_preferences_window_init (AdapDemoPreferencesWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
