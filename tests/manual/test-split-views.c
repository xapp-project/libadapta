#include <adapta.h>
#include <glib/gi18n.h>

static AdapNavigationPage *
create_page_with_child (const char *tag,
                        const char *title,
                        GtkWidget  *child)
{
  GtkWidget *header, *content, *page;

  header = adap_header_bar_new ();
  adap_header_bar_set_show_title (ADAP_HEADER_BAR (header), FALSE);

  content = adap_status_page_new ();
  adap_status_page_set_title (ADAP_STATUS_PAGE (content), title);
  adap_status_page_set_child (ADAP_STATUS_PAGE (content), child);

  page = adap_toolbar_view_new ();
  adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (page), header);
  adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (page), content);

  return adap_navigation_page_new_with_tag (page, title, tag);
}

static inline AdapNavigationPage *
create_page (const char *tag,
             const char *title)
{
  return create_page_with_child (tag, title, NULL);
}

static AdapNavigationPage *
create_page_with_button (const char  *tag,
                         const char  *title,
                         const char  *button_title,
                         const char  *button_action,
                         GtkWidget  **button)
{
  if (button_title && button_action) {
    GtkWidget *btn = gtk_button_new_with_label (button_title);
    gtk_button_set_can_shrink (GTK_BUTTON (btn), TRUE);
    gtk_widget_set_halign (btn, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class (btn, "pill");
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (btn), button_action);

    if (button)
      *button = btn;

    return create_page_with_child (tag, title, btn);
  }

  return create_page (tag, title);
}

static void
navigation_cb (void)
{
  GtkWidget *window, *view, *button;
  AdapNavigationPage *sidebar, *content;
  AdapBreakpoint *breakpoint;

  sidebar = create_page_with_button ("sidebar", "Sidebar", "Open Content", "navigation.push::content", &button);
  content = create_page ("content", "Content");

  gtk_widget_set_visible (button, FALSE);

  view = adap_navigation_split_view_new ();
  adap_navigation_split_view_set_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (view), sidebar);
  adap_navigation_split_view_set_content (ADAP_NAVIGATION_SPLIT_VIEW (view), content);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 400sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              G_OBJECT (button), "visible", TRUE,
                              NULL);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adap_window_set_content (ADAP_WINDOW (window), view);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));
}

static void
move_sidebar_cb (GObject             *button,
                 AdapOverlaySplitView *view)
{
  gboolean start = adap_overlay_split_view_get_sidebar_position (view) == GTK_PACK_START;

  adap_overlay_split_view_set_sidebar_position (view, start ? GTK_PACK_END : GTK_PACK_START);
}

static void
overlay_cb (void)
{
  GtkWidget *window, *view, *button, *toggle, *box;
  AdapNavigationPage *sidebar, *content;
  AdapBreakpoint *breakpoint;

  button = gtk_button_new_with_label ("Move Sidebar");
  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);
  gtk_widget_add_css_class (button, "pill");

  toggle = gtk_toggle_button_new_with_label ("Show Sidebar");
  gtk_button_set_can_shrink (GTK_BUTTON (toggle), TRUE);
  gtk_widget_add_css_class (toggle, "pill");

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), button);
  gtk_box_append (GTK_BOX (box), toggle);

  sidebar = create_page ("sidebar", "Sidebar");
  content = create_page_with_child ("content", "Content", box);

  view = adap_overlay_split_view_new ();
  adap_overlay_split_view_set_sidebar (ADAP_OVERLAY_SPLIT_VIEW (view), GTK_WIDGET (sidebar));
  adap_overlay_split_view_set_content (ADAP_OVERLAY_SPLIT_VIEW (view), GTK_WIDGET (content));

  g_object_bind_property (view, "show-sidebar", toggle, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_signal_connect (button, "clicked", G_CALLBACK (move_sidebar_cb), view);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 400sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              NULL);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Transient Sidebar");
  adap_window_set_content (ADAP_WINDOW (window), view);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));
}

static void
triple_pane_mail_cb (void)
{
  GtkWidget *outer_view, *inner_view, *window;
  AdapNavigationPage *folders, *inbox, *message, *inner_view_page;
  GtkWidget *inbox_button, *message_button;
  AdapBreakpoint *breakpoint;

  folders = create_page_with_button ("folders", "Folders", "Open Inbox", "navigation.push::inbox", &inbox_button);
  inbox = create_page_with_button ("inbox", "Inbox", "Open Message", "navigation.push::message", &message_button);
  message = create_page ("message", "Message");

  gtk_widget_set_visible (inbox_button, FALSE);
  gtk_widget_set_visible (message_button, FALSE);

  inner_view = adap_navigation_split_view_new ();
  adap_navigation_split_view_set_max_sidebar_width (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), 260);
  adap_navigation_split_view_set_sidebar_width_fraction (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), 0.38);
  adap_navigation_split_view_set_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), folders);
  adap_navigation_split_view_set_content (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), inbox);

  inner_view_page = adap_navigation_page_new (inner_view, "");

  outer_view = adap_navigation_split_view_new ();
  adap_navigation_split_view_set_min_sidebar_width (ADAP_NAVIGATION_SPLIT_VIEW (outer_view), 470);
  adap_navigation_split_view_set_max_sidebar_width (ADAP_NAVIGATION_SPLIT_VIEW (outer_view), 780);
  adap_navigation_split_view_set_sidebar_width_fraction (ADAP_NAVIGATION_SPLIT_VIEW (outer_view), 0.47);
  adap_navigation_split_view_set_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (outer_view), inner_view_page);
  adap_navigation_split_view_set_content (ADAP_NAVIGATION_SPLIT_VIEW (outer_view), message);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Mail");
  adap_window_set_content (ADAP_WINDOW (window), outer_view);
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 1200, 600);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 860sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (message_button), "visible", TRUE,
                              G_OBJECT (inner_view), "sidebar-width-fraction", 0.33f,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 500sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "sidebar-width-fraction", 0.33f,
                              G_OBJECT (inbox_button), "visible", TRUE,
                              G_OBJECT (message_button), "visible", TRUE,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
triple_pane_feeds_cb (void)
{
  GtkWidget *outer_view, *inner_view, *window, *button, *toggle, *box;
  AdapNavigationPage *feeds, *articles, *content;
  AdapBreakpoint *breakpoint;

  toggle = gtk_toggle_button_new_with_label ("Show Feeds");
  gtk_button_set_can_shrink (GTK_BUTTON (toggle), TRUE);
  gtk_widget_add_css_class (toggle, "pill");
  gtk_widget_set_visible (toggle, FALSE);

  button = gtk_button_new_with_label ("Open Content");
  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);
  gtk_widget_add_css_class (button, "pill");
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "navigation.push::content");
  gtk_widget_set_visible (button, FALSE);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), toggle);
  gtk_box_append (GTK_BOX (box), button);
  gtk_widget_set_visible (box, FALSE);

  feeds = create_page ("feeds", "Feeds");
  articles = create_page_with_child ("articles", "Articles", box);
  content = create_page ("content", "Content");

  inner_view = adap_navigation_split_view_new ();
  adap_navigation_split_view_set_sidebar_width_fraction (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), 0.355);
  adap_navigation_split_view_set_min_sidebar_width (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), 290);
  adap_navigation_split_view_set_max_sidebar_width (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), 520);
  adap_navigation_split_view_set_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), articles);
  adap_navigation_split_view_set_content (ADAP_NAVIGATION_SPLIT_VIEW (inner_view), content);

  outer_view = adap_overlay_split_view_new ();
  adap_overlay_split_view_set_sidebar_width_fraction (ADAP_OVERLAY_SPLIT_VIEW (outer_view), 0.179);
  adap_overlay_split_view_set_max_sidebar_width (ADAP_OVERLAY_SPLIT_VIEW (outer_view), 260);
  adap_overlay_split_view_set_sidebar (ADAP_OVERLAY_SPLIT_VIEW (outer_view), GTK_WIDGET (feeds));
  adap_overlay_split_view_set_content (ADAP_OVERLAY_SPLIT_VIEW (outer_view), inner_view);

  g_object_bind_property (outer_view, "show-sidebar", toggle, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Feeds");
  adap_window_set_content (ADAP_WINDOW (window), outer_view);
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 1200, 600);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 860sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (box), "visible", TRUE,
                              G_OBJECT (toggle), "visible", TRUE,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 500sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "collapsed", TRUE,
                              G_OBJECT (box), "visible", TRUE,
                              G_OBJECT (toggle), "visible", TRUE,
                              G_OBJECT (button), "visible", TRUE,
                              NULL);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
complex_navigation_cb (void)
{
  GtkWidget *window, *sidebar, *content, *content_button, *view;
  AdapNavigationPage *sidebar_1, *sidebar_2, *content_1, *content_2;
  AdapNavigationPage *sidebar_page, *content_page;
  AdapBreakpoint *breakpoint;

  /* Sidebar */
  sidebar_1 = create_page_with_button ("sidebar", "Sidebar", "Open Page 2", "navigation.push::sidebar-2", NULL);
  sidebar_2 = create_page_with_button ("sidebar-2", "Sidebar 2", "Open Content", "navigation.push::content", &content_button);

  gtk_widget_set_visible (content_button, FALSE);

  sidebar = adap_navigation_view_new ();
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (sidebar), sidebar_1);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (sidebar), sidebar_2);

  sidebar_page = adap_navigation_page_new (sidebar, "");

  /* Content */
  content_1 = create_page_with_button ("content", "Content", "Open Page 2", "navigation.push::content-2", NULL);
  content_2 = create_page ("content-2", "Content 2");

  content = adap_navigation_view_new ();
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (content), content_1);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (content), content_2);

  content_page = adap_navigation_page_new_with_tag (content, "", "content");

  /* Window */
  view = adap_navigation_split_view_new ();
  adap_navigation_split_view_set_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (view), sidebar_page);
  adap_navigation_split_view_set_content (ADAP_NAVIGATION_SPLIT_VIEW (view), content_page);

  breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 400sp"));
  adap_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              G_OBJECT (content_button), "visible", TRUE,
                              NULL);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adap_window_set_content (ADAP_WINDOW (window), view);
  adap_window_add_breakpoint (ADAP_WINDOW (window), breakpoint);
  gtk_widget_set_size_request (window, 360, 200);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

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

  button = gtk_button_new_with_label ("Navigation");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (navigation_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Overlay");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (overlay_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Triple Pane (Mail)");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (triple_pane_mail_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Triple Pane (Feeds)");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (triple_pane_feeds_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Complex Navigation");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (complex_navigation_cb), NULL);
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
  gboolean done = FALSE;

  adap_init ();

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Split Views");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
