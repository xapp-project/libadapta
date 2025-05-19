#include <adapta.h>
#include <glib/gi18n.h>

static AdapNavigationPage *
create_static_page (gboolean    header_bar,
                    const char *tag,
                    const char *title,
                    const char *first_label,
                    ...)
{
  GtkWidget *widget;
  va_list args;

  va_start (args, first_label);

  if (first_label) {
    const char *label, *dest;
    GtkWidget *box;

    label = first_label;
    dest = va_arg (args, const char *);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
    gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

    while (label) {
      GtkWidget *btn = gtk_button_new_with_label (label);

      gtk_button_set_can_shrink (GTK_BUTTON (btn), TRUE);
      gtk_widget_add_css_class (btn, "pill");
      gtk_actionable_set_action_name (GTK_ACTIONABLE (btn), "navigation.push");
      gtk_actionable_set_action_target (GTK_ACTIONABLE (btn), "s", dest);
      gtk_box_append (GTK_BOX (box), btn);

      label = va_arg (args, const char *);
      if (!label)
        break;

      dest = va_arg (args, const char *);
    }

    widget = box;
  } else {
    GtkWidget *label = gtk_label_new (title);

    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_widget_add_css_class (label, "title-1");

    widget = label;
  }

  va_end (args);

  if (header_bar) {
    GtkWidget *page = adap_toolbar_view_new ();
    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (page), adap_header_bar_new ());
    adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (page), widget);

    widget = page;
  }

  return adap_navigation_page_new_with_tag (widget, title, tag);
}

static void dynamic_browser_clicked_cb (AdapNavigationView *view,
                                        GObject           *button);

static AdapNavigationPage *
create_dynamic_page (AdapNavigationView *view,
                     gboolean           header_bar,
                     int                page_number)
{
  char *title = g_strdup_printf ("Page %d", page_number);
  GtkWidget *child;
  AdapNavigationPage *page;

  child = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_halign (child, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (child, GTK_ALIGN_CENTER);

  for (int i = 0; i < 2; i++) {
    int dest = page_number * 2 + i;
    char *label;
    GtkWidget *btn;

    label = g_strdup_printf ("Open Page %d", dest);

    btn = gtk_button_new_with_label (label);
    gtk_button_set_can_shrink (GTK_BUTTON (btn), TRUE);
    gtk_widget_add_css_class (btn, "pill");
    gtk_box_append (GTK_BOX (child), btn);
    g_object_set_data (G_OBJECT (btn), "destination", GINT_TO_POINTER (dest));
    g_object_set_data (G_OBJECT (btn), "header-bar", GINT_TO_POINTER (header_bar));
    g_signal_connect_swapped (btn, "clicked", G_CALLBACK (dynamic_browser_clicked_cb), view);

    g_free (label);
  }

  if (header_bar) {
    GtkWidget *toolbar_view = adap_toolbar_view_new ();
    adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), adap_header_bar_new ());
    adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (toolbar_view), child);

    child = toolbar_view;
  }

  page = adap_navigation_page_new (child, title);

  g_free (title);

  return page;
}

static void
dynamic_browser_clicked_cb (AdapNavigationView *view,
                            GObject           *button)
{
  int dest = GPOINTER_TO_INT (g_object_get_data (button, "destination"));
  gboolean header_bar = GPOINTER_TO_INT (g_object_get_data (button, "header-bar"));
  AdapNavigationPage *new_page;

  new_page = create_dynamic_page (view, header_bar, dest);

  adap_navigation_view_push (view, new_page);
}

static void
simple_cb (void)
{
  GtkWidget *window, *view;
  AdapNavigationPage *page_1, *page_2, *page_3, *page_4;

  page_1 = create_static_page (TRUE, "page-1", "Page 1", "Open Page 2", "page-2", "Open Page 3", "page-3", NULL);
  page_2 = create_static_page (TRUE, "page-2", "Page 2", "Open Page 4", "page-4", NULL);
  page_3 = create_static_page (TRUE, "page-3", "Page 3", NULL);
  page_4 = create_static_page (TRUE, "page-4", "Page 4", "Open Page 3", "page-3", NULL);

  view = adap_navigation_view_new ();
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_1);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_2);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_3);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_4);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Simple");
  adap_window_set_content (ADAP_WINDOW (window), view);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  gtk_widget_add_css_class (window, "numeric");

  gtk_window_present (GTK_WINDOW (window));
}

static void
dynamic_cb (void)
{
  GtkWidget *window, *view;
  AdapNavigationPage *page;

  view = adap_navigation_view_new ();
  page = create_dynamic_page (ADAP_NAVIGATION_VIEW (view), TRUE, 1);
  adap_navigation_view_push (ADAP_NAVIGATION_VIEW (view), page);

  window = adap_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Dynamic");
  adap_window_set_content (ADAP_WINDOW (window), view);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  gtk_widget_add_css_class (window, "numeric");

  gtk_window_present (GTK_WINDOW (window));
}

typedef struct {
  GtkWindow *window;
  AdapNavigationView *view;
  GtkWidget *back;
  GtkWidget *forward;
  GSList *forward_stack;
} BrowserData;

static void
free_browser_data (BrowserData *data)
{
  g_slist_free_full (data->forward_stack, g_object_unref);
  g_free (data);
}

static void
browser_popped_cb (BrowserData       *data,
                   AdapNavigationPage *page)
{
  AdapNavigationPage *visible_page, *previous_page;

  visible_page = adap_navigation_view_get_visible_page (data->view);
  previous_page = adap_navigation_view_get_previous_page (data->view, visible_page);

  data->forward_stack = g_slist_prepend (data->forward_stack, g_object_ref (page));

  gtk_widget_set_sensitive (data->forward, TRUE);
  gtk_widget_set_sensitive (data->back, previous_page != NULL);
}

static void
browser_replaced_cb (BrowserData *data)
{
  AdapNavigationPage *visible_page, *previous_page;

  visible_page = adap_navigation_view_get_visible_page (data->view);

  if (visible_page)
    previous_page = adap_navigation_view_get_previous_page (data->view, visible_page);
  else
    previous_page = NULL;

  gtk_widget_set_sensitive (data->forward, data->forward_stack != NULL);
  gtk_widget_set_sensitive (data->back, previous_page != NULL);
}

static void
browser_pushed_cb (BrowserData *data)
{
  AdapNavigationPage *visible_page;

  visible_page = adap_navigation_view_get_visible_page (data->view);

  if (data->forward_stack) {
    AdapNavigationPage *forward_page = data->forward_stack->data;

    if (!g_strcmp0 (adap_navigation_page_get_tag (visible_page),
                    adap_navigation_page_get_tag (forward_page))) {
      data->forward_stack = g_slist_remove (data->forward_stack, forward_page);
      g_object_unref (forward_page);
    } else {
      g_slist_free_full (data->forward_stack, g_object_unref);
      data->forward_stack = NULL;
    }
  }

  gtk_widget_set_sensitive (data->back, TRUE);
  gtk_widget_set_sensitive (data->forward, data->forward_stack != NULL);
}

static AdapNavigationPage *
browser_get_next_page_cb (BrowserData *data)
{
  if (!data->forward_stack)
    return NULL;

  return g_object_ref (data->forward_stack->data);
}

static void
browser_notify_visible_page_cb (BrowserData *data)
{
  AdapNavigationPage *visible_page;
  const char *title;

  visible_page = adap_navigation_view_get_visible_page (data->view);

  if (visible_page)
    title = adap_navigation_page_get_title (visible_page);
  else
    title = "";

  gtk_window_set_title (data->window, title);
}

static void
browser_back_cb (BrowserData *data)
{
  adap_navigation_view_pop (data->view);
}

static void
browser_forward_cb (BrowserData *data)
{
  AdapNavigationPage *page;

  g_assert (data->forward_stack);

  page = data->forward_stack->data;

  adap_navigation_view_push (data->view, page);
}

static void
static_browser_home_cb (BrowserData *data)
{
  const char *home = "page-1";

  g_slist_free_full (data->forward_stack, g_object_unref);
  data->forward_stack = NULL;

  adap_navigation_view_replace_with_tags (data->view, &home, 1);
}

static void
static_browser_cb (void)
{
  GtkWidget *window, *view, *toolbar_view, *header_bar, *back, *forward, *home;
  AdapNavigationPage *page_1, *page_2, *page_3, *page_4;
  BrowserData *data;

  back = gtk_button_new_from_icon_name ("go-previous-symbolic");
  gtk_widget_set_tooltip_text (back, "Back");
  gtk_widget_set_sensitive (back, FALSE);

  forward = gtk_button_new_from_icon_name ("go-next-symbolic");
  gtk_widget_set_tooltip_text (forward, "Forward");
  gtk_widget_set_sensitive (forward, FALSE);

  home = gtk_button_new_from_icon_name ("go-home-symbolic");
  gtk_widget_set_tooltip_text (home, "Home");

  header_bar = adap_header_bar_new ();
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), back);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), forward);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), home);

  page_1 = create_static_page (FALSE, "page-1", "Page 1", "Open Page 2", "page-2", "Open Page 3", "page-3", NULL);
  page_2 = create_static_page (FALSE, "page-2", "Page 2", "Open Page 4", "page-4", NULL);
  page_3 = create_static_page (FALSE, "page-3", "Page 3", NULL);
  page_4 = create_static_page (FALSE, "page-4", "Page 4", "Open Page 3", "page-3", NULL);

  view = adap_navigation_view_new ();
  adap_navigation_view_set_animate_transitions (ADAP_NAVIGATION_VIEW (view), FALSE);
  adap_navigation_view_set_pop_on_escape (ADAP_NAVIGATION_VIEW (view), FALSE);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_1);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_2);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_3);
  adap_navigation_view_add (ADAP_NAVIGATION_VIEW (view), page_4);

  toolbar_view = adap_toolbar_view_new ();
  adap_toolbar_view_set_top_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                      ADAP_TOOLBAR_RAISED);
  adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), header_bar);
  adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (toolbar_view), view);

  window = adap_window_new ();
  adap_window_set_content (ADAP_WINDOW (window), toolbar_view);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  gtk_widget_add_css_class (window, "numeric");

  data = g_new0 (BrowserData, 1);
  data->window = GTK_WINDOW (window);
  data->view = ADAP_NAVIGATION_VIEW (view);
  data->back = back;
  data->forward = forward;

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (browser_pushed_cb), data);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (browser_popped_cb), data);
  g_signal_connect_swapped (view, "replaced", G_CALLBACK (browser_replaced_cb), data);
  g_signal_connect_swapped (view, "get-next-page", G_CALLBACK (browser_get_next_page_cb), data);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (browser_notify_visible_page_cb), data);
  g_signal_connect_swapped (back, "clicked", G_CALLBACK (browser_back_cb), data);
  g_signal_connect_swapped (forward, "clicked", G_CALLBACK (browser_forward_cb), data);
  g_signal_connect_swapped (home, "clicked", G_CALLBACK (static_browser_home_cb), data);
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (free_browser_data), data);

  browser_notify_visible_page_cb (data);

  gtk_window_present (GTK_WINDOW (window));
}

static void
dynamic_browser_home_cb (BrowserData *data)
{
  AdapNavigationPage *home = create_dynamic_page (data->view, FALSE, 1);

  g_slist_free_full (data->forward_stack, g_object_unref);
  data->forward_stack = NULL;

  adap_navigation_view_replace (data->view, &home, 1);
}

static void
dynamic_browser_cb (void)
{
  GtkWidget *window, *view, *toolbar_view, *header_bar, *back, *forward, *home;
  BrowserData *data;

  back = gtk_button_new_from_icon_name ("go-previous-symbolic");
  gtk_widget_set_tooltip_text (back, "Back");
  gtk_widget_set_sensitive (back, FALSE);

  forward = gtk_button_new_from_icon_name ("go-next-symbolic");
  gtk_widget_set_tooltip_text (forward, "Forward");
  gtk_widget_set_sensitive (forward, FALSE);

  home = gtk_button_new_from_icon_name ("go-home-symbolic");
  gtk_widget_set_tooltip_text (home, "Home");

  header_bar = adap_header_bar_new ();
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), back);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), forward);
  adap_header_bar_pack_start (ADAP_HEADER_BAR (header_bar), home);

  view = adap_navigation_view_new ();
  adap_navigation_view_set_animate_transitions (ADAP_NAVIGATION_VIEW (view), FALSE);
  adap_navigation_view_set_pop_on_escape (ADAP_NAVIGATION_VIEW (view), FALSE);

  toolbar_view = adap_toolbar_view_new ();
  adap_toolbar_view_set_top_bar_style (ADAP_TOOLBAR_VIEW (toolbar_view),
                                      ADAP_TOOLBAR_RAISED);
  adap_toolbar_view_add_top_bar (ADAP_TOOLBAR_VIEW (toolbar_view), header_bar);
  adap_toolbar_view_set_content (ADAP_TOOLBAR_VIEW (toolbar_view), view);

  window = adap_window_new ();
  adap_window_set_content (ADAP_WINDOW (window), toolbar_view);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 400);
  gtk_widget_add_css_class (window, "numeric");

  data = g_new0 (BrowserData, 1);
  data->window = GTK_WINDOW (window);
  data->view = ADAP_NAVIGATION_VIEW (view);
  data->back = back;
  data->forward = forward;

  adap_navigation_view_push (ADAP_NAVIGATION_VIEW (view),
                            create_dynamic_page (data->view, FALSE, 1));

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (browser_pushed_cb), data);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (browser_popped_cb), data);
  g_signal_connect_swapped (view, "replaced", G_CALLBACK (browser_replaced_cb), data);
  g_signal_connect_swapped (view, "get-next-page", G_CALLBACK (browser_get_next_page_cb), data);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (browser_notify_visible_page_cb), data);
  g_signal_connect_swapped (back, "clicked", G_CALLBACK (browser_back_cb), data);
  g_signal_connect_swapped (forward, "clicked", G_CALLBACK (browser_forward_cb), data);
  g_signal_connect_swapped (home, "clicked", G_CALLBACK (dynamic_browser_home_cb), data);
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (free_browser_data), data);

  browser_notify_visible_page_cb (data);

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

  button = gtk_button_new_with_label ("Dynamic");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (dynamic_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Static Browser");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (static_browser_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Dynamic Browser");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (dynamic_browser_cb), NULL);
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
  gtk_window_set_title (GTK_WINDOW (window), "Navigation");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
