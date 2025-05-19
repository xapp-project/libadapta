#include <adapta.h>
#include <glib/gi18n.h>

#define STYLE "" \
".camera {" \
"  background: #444444;" \
"  color: white;" \
"}" \
".camera headerbar {" \
"  background: none;" \
"  box-shadow: none; " \
"  color: inherit;" \
"}"

static void
simple_cb (void)
{
  GtkWidget *window, *box, *bin;
  AdapBreakpoint *breakpoint;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top (box, 12);
  gtk_widget_set_margin_bottom (box, 12);
  gtk_widget_set_margin_start (box, 12);
  gtk_widget_set_margin_end (box, 12);

  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 1"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 2"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 3"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 4"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 5"));

  bin = adap_breakpoint_bin_new ();
  gtk_widget_set_size_request (bin, 200, 300);
  adap_breakpoint_bin_set_child (ADAP_BREAKPOINT_BIN (bin), box);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 420pt"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              NULL);
  adap_breakpoint_bin_add_breakpoint (ADAP_BREAKPOINT_BIN (bin), breakpoint);

  window = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Simple");
  gtk_window_set_child (GTK_WINDOW (window), bin);
  gtk_window_set_default_size (GTK_WINDOW (window), 700, 300);

  gtk_window_present (GTK_WINDOW (window));
}

static void
browser_cb (void)
{
  GtkWidget *window, *view, *top_bar, *bottom_bar;
  GtkWidget *clamp, *box, *entry;
  GtkWidget *back, *forward, *refresh, *star;
  AdapBreakpoint *breakpoint;

  back = gtk_button_new_from_icon_name ("go-previous-symbolic");
  forward = gtk_button_new_from_icon_name ("go-next-symbolic");
  refresh = gtk_button_new_from_icon_name ("view-refresh-symbolic");
  star = gtk_button_new_from_icon_name ("starred-symbolic");

  entry = gtk_entry_new ();
  gtk_editable_set_max_width_chars (GTK_EDITABLE (entry), 200);

  clamp = adap_clamp_new ();
  adap_clamp_set_maximum_size (ADAP_CLAMP (clamp), 600);
  adap_clamp_set_tightening_threshold (ADAP_CLAMP (clamp), 400);
  adap_clamp_set_child (ADAP_CLAMP (clamp), entry);

  top_bar = adap_header_bar_new ();

  adap_header_bar_pack_start (ADAP_HEADER_BAR (top_bar), back);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (top_bar), forward);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (top_bar), refresh);
  adap_header_bar_set_title_widget (ADAP_HEADER_BAR (top_bar), clamp);
  adap_header_bar_pack_end (ADAP_HEADER_BAR (top_bar),
                           gtk_button_new_from_icon_name ("open-menu-symbolic"));
  adap_header_bar_pack_end (ADAP_HEADER_BAR (top_bar), star);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (box), TRUE);
  gtk_widget_add_css_class (box, "toolbar");

  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("go-previous-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("go-next-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("view-refresh-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("starred-symbolic"));

  bottom_bar = adap_clamp_new ();
  adap_clamp_set_maximum_size (ADAP_CLAMP (bottom_bar), 400);
  adap_clamp_set_child (ADAP_CLAMP (bottom_bar), box);
  gtk_widget_set_visible (bottom_bar, FALSE);

  view = adap_toolbar_view_new ();
  adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (view), top_bar);
  adap_toolbar_view_set_top_bar_style (ADAP_TOOLBAR_VIEW (view), ADAP_TOOLBAR_RAISED);
  adap_toolbar_view_add_bottom_bar (ADAP_TOOLBAR_VIEW (view), bottom_bar);
  adap_toolbar_view_set_bottom_bar_style (ADAP_TOOLBAR_VIEW (view), ADAP_TOOLBAR_RAISED);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Browser");
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  adap_window_set_content (ADAP_WINDOW (window), view);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 500px"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (back), "visible", FALSE,
                              G_OBJECT (forward), "visible", FALSE,
                              G_OBJECT (refresh), "visible", FALSE,
                              G_OBJECT (star), "visible", FALSE,
                              G_OBJECT (bottom_bar), "visible", TRUE,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
camera_cb (void)
{
  GtkWidget *window, *overlay, *box, *bar1, *bar2, *headerbar;
  AdapBreakpoint *breakpoint;

  /* Single vertical by default */

  bar1 = gtk_window_handle_new ();
  gtk_widget_set_size_request (bar1, 60, 60);
  gtk_widget_add_css_class (bar1, "osd");
  gtk_widget_set_halign (bar1, GTK_ALIGN_END);
  gtk_widget_set_hexpand (bar1, TRUE);
  gtk_widget_set_vexpand (bar1, TRUE);

  bar2 = gtk_window_handle_new ();
  gtk_widget_set_size_request (bar2, 60, 60);
  gtk_widget_add_css_class (bar2, "osd");
  gtk_widget_set_visible (bar2, FALSE);
  gtk_widget_set_halign (bar2, GTK_ALIGN_START);
  gtk_widget_set_hexpand (bar2, TRUE);
  gtk_widget_set_vexpand (bar2, TRUE);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_append (GTK_BOX (box), bar2);
  gtk_box_append (GTK_BOX (box), bar1);

  headerbar = adap_header_bar_new ();
  gtk_widget_set_valign (headerbar, GTK_ALIGN_START);
  adap_header_bar_set_show_title (ADAP_HEADER_BAR (headerbar), FALSE);

  overlay = gtk_overlay_new ();
  gtk_overlay_set_child (GTK_OVERLAY (overlay), box);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), headerbar);

  window = adap_window_new ();
  gtk_widget_set_size_request (window, 300, 300);
  gtk_widget_add_css_class (window, "camera");
  adap_window_set_content (ADAP_WINDOW (window), overlay);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 580);

  /* Single horizontal */
  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-aspect-ratio: 4/3"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              G_OBJECT (bar1), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar1), "valign", GTK_ALIGN_END,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  /* Dual vertical */
  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-height: 400px"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (bar2), "visible", TRUE,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  /* Dual horizontal */
  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-aspect-ratio: 4/3 and max-width: 450px"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              G_OBJECT (bar1), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar1), "valign", GTK_ALIGN_END,
                              G_OBJECT (bar2), "visible", TRUE,
                              G_OBJECT (bar2), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar2), "valign", GTK_ALIGN_START,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static GtkWidget *
create_content (GtkWindow *parent)
{
  GtkWidget *box, *button;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
  gtk_widget_set_margin_top (box, 48);
  gtk_widget_set_margin_bottom (box, 48);
  gtk_widget_set_margin_start (box, 48);
  gtk_widget_set_margin_end (box, 48);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label ("Simple");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (simple_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Browser");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (browser_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Camera");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (camera_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

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
  GtkCssProvider *provider;
  gboolean done = FALSE;

  adap_init ();

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (provider, STYLE);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Breakpoints");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  g_object_unref (provider);

  return 0;
}
