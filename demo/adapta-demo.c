#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adapta.h>

#include "adap-demo-debug-info.h"
#include "adap-demo-preferences-window.h"
#include "adap-demo-window.h"

static void
show_inspector (GSimpleAction *action,
                GVariant      *state,
                gpointer       user_data)
{
  gtk_window_set_interactive_debugging (TRUE);
}

static void
show_preferences (GSimpleAction *action,
                  GVariant      *state,
                  gpointer       user_data)
{
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  AdapDemoPreferencesWindow *preferences = adap_demo_preferences_window_new ();

  adap_dialog_present (ADAP_DIALOG (preferences), GTK_WIDGET (window));
}

static void
show_about (GSimpleAction *action,
            GVariant      *state,
            gpointer       user_data)
{
  const char *developers[] = {
    "Adrien Plazas",
    "Alice Mikhaylenko",
    "Andrei Lișiță",
    "Guido Günther",
    "Jamie Murphy",
    "Julian Sparber",
    "Manuel Genovés",
    "Zander Brown",
    NULL
  };

  const char *designers[] = {
    "GNOME Design Team",
    NULL
 };

  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  char *debug_info;
  AdapDialog *about;

  debug_info = adap_demo_generate_debug_info ();

  about = adap_about_dialog_new_from_appdata ("/org/gnome/Adapta1/Demo/org.gnome.Adapta1.Demo.metainfo.xml", NULL);
  adap_about_dialog_set_version (ADAP_ABOUT_DIALOG (about), ADAP_VERSION_S);
  adap_about_dialog_set_debug_info (ADAP_ABOUT_DIALOG (about), debug_info);
  adap_about_dialog_set_debug_info_filename (ADAP_ABOUT_DIALOG (about), "adapta-1-demo-debug-info.txt");
  adap_about_dialog_set_copyright (ADAP_ABOUT_DIALOG (about), "© 2017–2022 Purism SPC");
  adap_about_dialog_set_developers (ADAP_ABOUT_DIALOG (about), developers);
  adap_about_dialog_set_designers (ADAP_ABOUT_DIALOG (about), designers);
  adap_about_dialog_set_artists (ADAP_ABOUT_DIALOG (about), designers);
  adap_about_dialog_set_translator_credits (ADAP_ABOUT_DIALOG (about), _("translator-credits"));

  adap_about_dialog_add_link (ADAP_ABOUT_DIALOG (about),
                             _("_Documentation"),
                             "https://gnome.pages.gitlab.gnome.org/libadapta/doc/main/");
  adap_about_dialog_add_link (ADAP_ABOUT_DIALOG (about),
                             _("_Chat"),
                             "https://matrix.to/#/#libadapta:gnome.org");

  adap_dialog_present (about, GTK_WIDGET (window));

  g_free (debug_info);
}

static void
show_window (GtkApplication *app)
{
  AdapDemoWindow *window;

  window = adap_demo_window_new (app);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  AdapApplication *app;
  int status;
  static GActionEntry app_entries[] = {
    { "inspector", show_inspector, NULL, NULL, NULL },
    { "preferences", show_preferences, NULL, NULL, NULL },
    { "about", show_about, NULL, NULL, NULL },
  };

  app = adap_application_new ("org.gnome.Adapta1.Demo", G_APPLICATION_NON_UNIQUE);
  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);
  g_signal_connect (app, "activate", G_CALLBACK (show_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
