#include <adapta.h>

static void
activate_cb (GtkApplication *app)
{
  GtkWidget *window = gtk_application_window_new (app);
  GtkWidget *label = gtk_label_new ("Hello World");

  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_window_set_child (GTK_WINDOW (window), label);
  gtk_window_present (GTK_WINDOW (window));
}

int
main (int   argc,
      char *argv[])
{
  g_autoptr (AdapApplication) app = NULL;

  app = adap_application_new ("org.example.Hello", 0);

  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
