#include <adapta.h>

static const char *names[] = {
  "Aaron",
  "Andy",
  "Amelia",
  "Alice",
  "Adam",
  "Audrey",
  "Ashleigh",
  "Allan",
  "Abigail",
  "Arthur",
  "Alena",
  "Alex",
  "Anthony",
  "Anna",
};

static GtkWidget *
create_content (void)
{
  GtkWidget *box;
  int i;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top (box, 12);
  gtk_widget_set_margin_bottom (box, 12);
  gtk_widget_set_margin_start (box, 12);
  gtk_widget_set_margin_end (box, 12);

  for (i = 0; i < G_N_ELEMENTS (names); i++) {
    GtkWidget *avatar = adap_avatar_new (64, names[i], TRUE);

    gtk_box_append (GTK_BOX (box), avatar);
  }

  return box;
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

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Avatar Colors");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
