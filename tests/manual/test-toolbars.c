#include <adapta.h>

static GtkWidget *
create_content (void)
{
  GtkWidget *box, *toolbar_view, *tab_view, *stack;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /* Left column */

  toolbar_view = adap_toolbar_view_new ();
  gtk_widget_set_hexpand (toolbar_view, TRUE);
  gtk_widget_set_size_request (toolbar_view, 360, -1);
  gtk_box_append (GTK_BOX (box), toolbar_view);

  adap_toolbar_view_set_top_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                      ADAP_TOOLBAR_RAISED);

  adap_toolbar_view_set_bottom_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                         ADAP_TOOLBAR_RAISED);

  /* Contents */
  {
    AdapTabPage *page;

    tab_view = GTK_WIDGET (adap_tab_view_new ());

    page = adap_tab_view_add_page (ADAP_TAB_VIEW (tab_view),
                                  gtk_label_new ("Page 1"), NULL);
    adap_tab_page_set_title (page, "Page 1");

    page = adap_tab_view_add_page (ADAP_TAB_VIEW (tab_view),
                                  gtk_label_new ("Page 2"), NULL);
    adap_tab_page_set_title (page, "Page 2");

    adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (toolbar_view), tab_view);
  }

  /* Header bar .default-decoration */
  {
    GtkWidget *headerbar = gtk_header_bar_new ();
    gtk_widget_add_css_class (headerbar, "default-decoration");

    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), headerbar);
  }

  /* Menu bar */
  {
    GtkWidget *menubar;
    GMenu *menu, *submenu;

    menu = g_menu_new ();

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "File", G_MENU_MODEL (submenu));

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "Edit", G_MENU_MODEL (submenu));
    g_object_unref (submenu);

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "View", G_MENU_MODEL (submenu));

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "Help", G_MENU_MODEL (submenu));

    menubar = gtk_popover_menu_bar_new_from_model (G_MENU_MODEL (menu));

    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), menubar);

    g_object_unref (menu);
  }

  /* .toolbar */
  {
    GtkWidget *toolbar, *button, *content, *spacer;

    toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (toolbar, "toolbar");

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("document-new-symbolic"));

    content = adap_button_content_new ();
    adap_button_content_set_icon_name (ADAP_BUTTON_CONTENT (content),
                                      "document-open-symbolic");
    adap_button_content_set_label (ADAP_BUTTON_CONTENT (content), "Open");

    button = gtk_button_new ();
    gtk_button_set_child (GTK_BUTTON (button), content);
    gtk_box_append (GTK_BOX (toolbar), button);

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("document-save-symbolic"));

    spacer = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class (spacer, "spacer");
    gtk_box_append (GTK_BOX (toolbar), spacer);

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("edit-undo-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("edit-redo-symbolic"));

    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), toolbar);
  }

  /* Tab bar */
  {
    GtkWidget *tabbar, *button;

    tabbar = GTK_WIDGET (adap_tab_bar_new ());
    adap_tab_bar_set_view (ADAP_TAB_BAR (tabbar), ADAP_TAB_VIEW (tab_view));

    button = gtk_button_new_from_icon_name ("pan-down-symbolic");
    gtk_widget_add_css_class (button, "flat");
    adap_tab_bar_set_start_action_widget (ADAP_TAB_BAR (tabbar), button);

    button = gtk_button_new_from_icon_name ("pan-down-symbolic");
    gtk_widget_add_css_class (button, "flat");
    adap_tab_bar_set_end_action_widget (ADAP_TAB_BAR (tabbar), button);

    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), tabbar);
  }

  /* .toolbar */
  {
    GtkWidget *clamp;

    clamp = adap_clamp_new ();
    gtk_widget_add_css_class (clamp, "toolbar");
    adap_clamp_set_maximum_size (ADAP_CLAMP (clamp), 400);
    adap_clamp_set_child (ADAP_CLAMP (clamp), gtk_entry_new ());

    adap_toolbar_view_add_bottom_bar (ADAP_TOOLBAR_VIEW (toolbar_view), clamp);
  }

  /* .toolbar */
  {
    GtkWidget *toolbar, *clamp, *button;

    toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (toolbar, "toolbar");
    gtk_box_set_homogeneous (GTK_BOX (toolbar), TRUE);

    button = adap_tab_button_new ();
    adap_tab_button_set_view (ADAP_TAB_BUTTON (button), ADAP_TAB_VIEW (tab_view));

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("go-previous-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("go-next-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("starred-symbolic"));
    gtk_box_append (GTK_BOX (toolbar), button);
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("open-menu-symbolic"));

    clamp = adap_clamp_new ();
    adap_clamp_set_maximum_size (ADAP_CLAMP (clamp), 400);
    adap_clamp_set_child (ADAP_CLAMP (clamp), toolbar);

    adap_toolbar_view_add_bottom_bar (ADAP_TOOLBAR_VIEW (toolbar_view), clamp);
  }

  gtk_box_append (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_VERTICAL));

  /* Right column */

  toolbar_view = adap_toolbar_view_new ();
  gtk_widget_set_hexpand (toolbar_view, TRUE);
  gtk_widget_set_size_request (toolbar_view, 360, -1);
  gtk_box_append (GTK_BOX (box), toolbar_view);

  adap_toolbar_view_set_top_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                      ADAP_TOOLBAR_RAISED);

  adap_toolbar_view_set_bottom_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                         ADAP_TOOLBAR_RAISED);

  /* Contents */
  {
    AdapViewStackPage *page;

    stack = adap_view_stack_new ();

    page = adap_view_stack_add_titled_with_icon (ADAP_VIEW_STACK (stack),
                                                gtk_label_new ("Page 1"),
                                                NULL,
                                                "Page 1",
                                                "emblem-system-symbolic");
    page = adap_view_stack_add_titled_with_icon (ADAP_VIEW_STACK (stack),
                                                gtk_label_new ("Page 2"),
                                                NULL,
                                                "Page 2",
                                                "emblem-system-symbolic");
    adap_view_stack_page_set_needs_attention (page, TRUE);
    adap_view_stack_page_set_badge_number (page, 3);
    page = adap_view_stack_add_titled_with_icon (ADAP_VIEW_STACK (stack),
                                                gtk_label_new ("Page 3"),
                                                NULL,
                                                "Page 3",
                                                "emblem-system-symbolic");

    adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (toolbar_view), stack);
  }

  /* Header bar */
  {
    GtkWidget *headerbar = gtk_header_bar_new ();
    gtk_header_bar_pack_start (GTK_HEADER_BAR (headerbar),
                               gtk_button_new_from_icon_name ("edit-find-symbolic"));
    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar),
                             gtk_button_new_from_icon_name ("open-menu-symbolic"));

    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), headerbar);
  }

  /* Search bar */
  {
    GtkWidget *entry, *clamp, *searchbar;

    entry = gtk_search_entry_new ();

    clamp = adap_clamp_new ();
    adap_clamp_set_maximum_size (ADAP_CLAMP (clamp), 400);
    adap_clamp_set_child (ADAP_CLAMP (clamp), entry);

    searchbar = gtk_search_bar_new ();
    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (searchbar), TRUE);
    gtk_search_bar_set_child (GTK_SEARCH_BAR (searchbar), clamp);
    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), searchbar);
  }

  /* Action bar */
  {
    GtkWidget *actionbar;

    actionbar = gtk_action_bar_new ();
    gtk_action_bar_set_revealed (GTK_ACTION_BAR (actionbar), TRUE);

    gtk_action_bar_pack_start (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_with_label ("Export"));
    gtk_action_bar_pack_start (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_with_label ("Link"));
    gtk_action_bar_pack_end (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_from_icon_name ("view-more-symbolic"));

    adap_toolbar_view_add_bottom_bar (ADAP_TOOLBAR_VIEW (toolbar_view), actionbar);
  }

  /* Switcher bar */
  {
    GtkWidget *switcher;

    switcher = adap_view_switcher_bar_new ();
    adap_view_switcher_bar_set_reveal (ADAP_VIEW_SWITCHER_BAR (switcher), TRUE);
    adap_view_switcher_bar_set_stack (ADAP_VIEW_SWITCHER_BAR (switcher),
                                     ADAP_VIEW_STACK (stack));

    adap_toolbar_view_add_bottom_bar (ADAP_TOOLBAR_VIEW (toolbar_view), switcher);
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

  window = adap_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Toolbars");
  adap_window_set_content (ADAP_WINDOW (window), create_content ());
  gtk_window_set_default_size (GTK_WINDOW (window), 720, 400);
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
