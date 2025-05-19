#include <adapta.h>

static inline void
add_button (GtkGrid       *grid,
            int            column,
            int            row,
            GtkStateFlags  flags,
            const char    *style_class)
{
  GtkWidget *button;
  button = gtk_button_new_with_label ("button");
  gtk_widget_set_state_flags (button, flags, FALSE);
  if (style_class)
    gtk_widget_add_css_class (button, style_class);
  gtk_grid_attach (grid, button, column, row, 1, 1);
}

static void
create_column (GtkGrid       *grid,
               int            column,
               const char    *name,
               GtkStateFlags  flags)
{
  int row = 0;

  GtkWidget *label = gtk_label_new (name);
  gtk_grid_attach (grid, label, column, row++, 1, 1);

  add_button (grid, column, row++, flags, NULL);
  add_button (grid, column, row++, flags, "flat");
  add_button (grid, column, row++, flags, "suggested-action");
  add_button (grid, column, row++, flags, "destructive-action");
  add_button (grid, column, row++, flags, "osd");
  add_button (grid, column, row++, flags, "card");
}

static void
create_buttons (GtkGrid *grid)
{
  int column = 0;

  create_column (grid, column++, NULL,         GTK_STATE_FLAG_NORMAL);
  create_column (grid, column++, "hover",      GTK_STATE_FLAG_PRELIGHT);
  create_column (grid, column++, "h:active",   GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE);
  create_column (grid, column++, "focus",      GTK_STATE_FLAG_FOCUSED | GTK_STATE_FLAG_FOCUS_VISIBLE);
  create_column (grid, column++, "checked",    GTK_STATE_FLAG_CHECKED);
  create_column (grid, column++, "c:hover",    GTK_STATE_FLAG_CHECKED | GTK_STATE_FLAG_PRELIGHT);
  create_column (grid, column++, "c:h:active", GTK_STATE_FLAG_CHECKED | GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_ACTIVE);
  create_column (grid, column++, "c:focus",    GTK_STATE_FLAG_CHECKED | GTK_STATE_FLAG_FOCUSED | GTK_STATE_FLAG_FOCUS_VISIBLE);
  create_column (grid, column++, "disabled",   GTK_STATE_FLAG_INSENSITIVE);
  create_column (grid, column++, "d:checked",  GTK_STATE_FLAG_INSENSITIVE | GTK_STATE_FLAG_CHECKED);
  create_column (grid, column++, "drop",       GTK_STATE_FLAG_DROP_ACTIVE);
}

static GtkWidget *
create_content (void)
{
  GtkWidget *grid = gtk_grid_new ();

  gtk_widget_set_can_target (grid, FALSE);
  gtk_widget_set_can_focus (grid, FALSE);
  gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (grid, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top (grid, 6);
  gtk_widget_set_margin_bottom (grid, 6);
  gtk_widget_set_margin_start (grid, 6);
  gtk_widget_set_margin_end (grid, 6);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 6);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);

  create_buttons (GTK_GRID (grid));

  return grid;
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
  gtk_window_set_title (GTK_WINDOW (window), "Button States");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
