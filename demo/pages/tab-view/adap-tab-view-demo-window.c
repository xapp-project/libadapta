#include "adap-tab-view-demo-window.h"

#include <glib/gi18n.h>

#include "adap-tab-view-demo-page.h"

struct _AdapTabViewDemoWindow
{
  AdapWindow parent_instance;
  AdapTabView *view;
  AdapTabBar *tab_bar;
  AdapTabOverview *tab_overview;

  GActionMap *tab_action_group;

  AdapTabPage *menu_page;
  gboolean in_dispose;
};

G_DEFINE_FINAL_TYPE (AdapTabViewDemoWindow, adap_tab_view_demo_window, ADAP_TYPE_WINDOW)

static void
window_new (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       user_data)
{
  AdapTabViewDemoWindow *window = adap_tab_view_demo_window_new ();

  adap_tab_view_demo_window_prepopulate (window);

  gtk_window_present (GTK_WINDOW (window));
}

static gboolean
text_to_tooltip (GBinding     *binding,
                 const GValue *input,
                 GValue       *output,
                 gpointer      user_data)
{
  const char *title = g_value_get_string (input);
  char *tooltip = g_markup_printf_escaped (_("Elaborate tooltip for <b>%s</b>"), title);

  g_value_take_string (output, tooltip);

  return TRUE;
}

static AdapTabPage *
add_page (AdapTabViewDemoWindow *self,
          AdapTabPage           *parent,
          AdapTabViewDemoPage   *content)
{
  AdapTabPage *page;

  page = adap_tab_view_add_page (self->view, GTK_WIDGET (content), parent);

  g_object_bind_property (content, "title",
                          page, "title",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (content, "title",
                               page, "tooltip",
                               G_BINDING_SYNC_CREATE,
                               text_to_tooltip, NULL,
                               NULL, NULL);
  g_object_bind_property (content, "icon",
                          page, "icon",
                          G_BINDING_SYNC_CREATE);

  adap_tab_page_set_indicator_activatable (page, TRUE);
  adap_tab_page_set_thumbnail_xalign (page, 0.5);
  adap_tab_page_set_thumbnail_yalign (page, 0.5);

  return page;
}

static AdapTabPage *
create_tab_cb (AdapTabViewDemoWindow *self)
{
  char *title;
  AdapTabPage *page;
  AdapTabViewDemoPage *content;
  static int next_page = 1;

  title = g_strdup_printf (_("Tab %d"), next_page);

  content = adap_tab_view_demo_page_new (title);
  page = add_page (self, NULL, content);

  next_page++;

  g_free (title);

  return page;
}

static void
tab_new (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  AdapTabPage *page = create_tab_cb (self);
  GtkWidget *content = adap_tab_page_get_child (page);

  adap_tab_view_set_selected_page (self->view, page);

  gtk_widget_grab_focus (content);
}

static AdapTabPage *
get_current_page (AdapTabViewDemoWindow *self)
{
  if (self->menu_page)
    return self->menu_page;

  return adap_tab_view_get_selected_page (self->view);
}

static void
tab_pin (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_set_page_pinned (self->view, get_current_page (self), TRUE);
}

static void
tab_unpin (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_set_page_pinned (self->view, get_current_page (self), FALSE);
}

static void
tab_close (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_close_page (self->view, get_current_page (self));
}

static void
tab_close_other (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_close_other_pages (self->view, get_current_page (self));
}

static void
tab_close_before (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_close_pages_before (self->view, get_current_page (self));
}

static void
tab_close_after (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  adap_tab_view_close_pages_after (self->view, get_current_page (self));
}

static void
tab_move_to_new_window (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);

  AdapTabViewDemoWindow *window = adap_tab_view_demo_window_new ();

  adap_tab_view_transfer_page (self->view,
                              self->menu_page,
                              window->view,
                              0);

  gtk_window_present (GTK_WINDOW (window));
}

static void
tab_change_needs_attention (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  gboolean need_attention = g_variant_get_boolean (parameter);

  adap_tab_page_set_needs_attention (get_current_page (self), need_attention);
  g_simple_action_set_state (action, g_variant_new_boolean (need_attention));
}

static void
tab_change_loading (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  gboolean loading = g_variant_get_boolean (parameter);

  adap_tab_page_set_loading (get_current_page (self), loading);
  g_simple_action_set_state (action, g_variant_new_boolean (loading));
}

static GIcon *
get_indicator_icon (AdapTabPage *page)
{
  gboolean muted;

  muted = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (page),
                                              "adap-tab-view-demo-muted"));

  if (muted)
    return g_themed_icon_new ("tab-audio-muted-symbolic");
  else
    return g_themed_icon_new ("tab-audio-playing-symbolic");
}

static char *
get_indicator_tooltip (AdapTabPage *page)
{
  gboolean muted;

  muted = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (page),
                                              "adap-tab-view-demo-muted"));

  if (muted)
    return g_strdup (_("Unmute Tab"));
  else
    return g_strdup (_("Mute Tab"));
}

static void
tab_change_indicator (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  gboolean indicator = g_variant_get_boolean (parameter);
  GIcon *icon = NULL;
  char *tooltip = NULL;

  if (indicator) {
    icon = get_indicator_icon (get_current_page (self));
    tooltip = get_indicator_tooltip (get_current_page (self));
  } else {
    tooltip = g_strdup ("");
  }

  adap_tab_page_set_indicator_icon (get_current_page (self), icon);
  adap_tab_page_set_indicator_tooltip (get_current_page (self), tooltip);
  g_simple_action_set_state (action, g_variant_new_boolean (indicator));

  g_clear_pointer (&tooltip, g_free);
  g_clear_object (&icon);
}

static void
tab_change_icon (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  gboolean enable_icon = g_variant_get_boolean (parameter);
  AdapTabPage *page = get_current_page (self);
  GtkWidget *child = adap_tab_page_get_child (page);

  adap_tab_view_demo_page_set_enable_icon (ADAP_TAB_VIEW_DEMO_PAGE (child),
                                          enable_icon);

  g_simple_action_set_state (action, g_variant_new_boolean (enable_icon));
}

static void
tab_refresh_icon (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  AdapTabPage *page = get_current_page (self);
  GtkWidget *child = adap_tab_page_get_child (page);

  adap_tab_view_demo_page_refresh_icon (ADAP_TAB_VIEW_DEMO_PAGE (child));
}

static void
tab_duplicate (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (user_data);
  AdapTabPage *parent = get_current_page (self);
  GtkWidget *parent_content = adap_tab_page_get_child (parent);
  AdapTabViewDemoPage *content;
  AdapTabPage *page;

  content = adap_tab_view_demo_page_new_duplicate (ADAP_TAB_VIEW_DEMO_PAGE (parent_content));
  page = add_page (self, parent, content);

  adap_tab_page_set_indicator_icon (page, adap_tab_page_get_indicator_icon (parent));
  adap_tab_page_set_indicator_tooltip (page, adap_tab_page_get_indicator_tooltip (parent));
  adap_tab_page_set_loading (page, adap_tab_page_get_loading (parent));
  adap_tab_page_set_needs_attention (page, adap_tab_page_get_needs_attention (parent));

  g_object_set_data (G_OBJECT (page),
                     "adap-tab-view-demo-muted",
                     g_object_get_data (G_OBJECT (parent),
                                        "adap-tab-view-demo-muted"));

  adap_tab_view_set_selected_page (self->view, page);
}

static GActionEntry action_entries[] = {
  { "window-new", window_new },
  { "tab-new", tab_new },
};

static GActionEntry tab_action_entries[] = {
  { "pin", tab_pin },
  { "unpin", tab_unpin },
  { "close", tab_close },
  { "close-other", tab_close_other },
  { "close-before", tab_close_before },
  { "close-after", tab_close_after },
  { "move-to-new-window", tab_move_to_new_window },
  { "needs-attention", NULL, NULL, "false", tab_change_needs_attention },
  { "loading", NULL, NULL, "false", tab_change_loading },
  { "indicator", NULL, NULL, "false", tab_change_indicator },
  { "icon", NULL, NULL, "false", tab_change_icon },
  { "refresh-icon", tab_refresh_icon },
  { "duplicate", tab_duplicate },
};

static inline void
set_tab_action_enabled (AdapTabViewDemoWindow *self,
                        const char           *name,
                        gboolean              enabled)
{
  GAction *action = g_action_map_lookup_action (self->tab_action_group, name);

  g_assert (G_IS_SIMPLE_ACTION (action));

  g_simple_action_set_enabled (G_SIMPLE_ACTION (action),
                               enabled);
}

static inline void
set_tab_action_state (AdapTabViewDemoWindow *self,
                      const char           *name,
                      gboolean              state)
{
  GAction *action = g_action_map_lookup_action (self->tab_action_group, name);

  g_assert (G_IS_SIMPLE_ACTION (action));

  g_simple_action_set_state (G_SIMPLE_ACTION (action),
                             g_variant_new_boolean (state));
}

static void
page_detached_cb (AdapTabViewDemoWindow *self,
                  AdapTabPage           *page)
{
  if (self->in_dispose)
    return;

  if (!adap_tab_view_get_n_pages (self->view) &&
      !adap_tab_overview_get_open (self->tab_overview))
    gtk_window_close (GTK_WINDOW (self));
}

static void
setup_menu_cb (AdapTabViewDemoWindow *self,
               AdapTabPage           *page,
               AdapTabView           *view)
{
  AdapTabPage *prev = NULL;
  gboolean can_close_before = TRUE, can_close_after = TRUE;
  gboolean pinned = FALSE, prev_pinned;
  gboolean has_icon = FALSE;
  guint n_pages, pos;

  self->menu_page = page;

  n_pages = adap_tab_view_get_n_pages (self->view);

  if (page) {
    pos = adap_tab_view_get_page_position (self->view, page);

    if (pos > 0)
      prev = adap_tab_view_get_nth_page (self->view, pos - 1);

    pinned = adap_tab_page_get_pinned (page);
    prev_pinned = prev && adap_tab_page_get_pinned (prev);

    can_close_before = !pinned && prev && !prev_pinned;
    can_close_after = pos < n_pages - 1;

    has_icon = adap_tab_page_get_icon (page) != NULL;
  }

  set_tab_action_enabled (self, "pin", !page || !pinned);
  set_tab_action_enabled (self, "unpin", !page || pinned);
  set_tab_action_enabled (self, "close", !page || !pinned);
  set_tab_action_enabled (self, "close-before", can_close_before);
  set_tab_action_enabled (self, "close-after", can_close_after);
  set_tab_action_enabled (self, "close-other", can_close_before || can_close_after);
  set_tab_action_enabled (self, "move-to-new-window", !page || (!pinned && n_pages > 1));
  set_tab_action_enabled (self, "refresh-icon", has_icon);

  if (page) {
    set_tab_action_state (self, "icon", has_icon);
    set_tab_action_state (self, "loading", adap_tab_page_get_loading (page));
    set_tab_action_state (self, "needs-attention", adap_tab_page_get_needs_attention (page));
    set_tab_action_state (self, "indicator", adap_tab_page_get_indicator_icon (page) != NULL);
  }
}

static AdapTabView *
create_window_cb (AdapTabViewDemoWindow *self)
{
  AdapTabViewDemoWindow *window = adap_tab_view_demo_window_new ();

  gtk_window_present (GTK_WINDOW (window));

  return window->view;
}

static void
indicator_activated_cb (AdapTabViewDemoWindow *self,
                        AdapTabPage           *page)
{
  GIcon *icon;
  char *tooltip;
  gboolean muted;

  muted = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (page),
                                              "adap-tab-view-demo-muted"));

  g_object_set_data (G_OBJECT (page),
                     "adap-tab-view-demo-muted",
                     GINT_TO_POINTER (!muted));

  icon = get_indicator_icon (page);
  tooltip = get_indicator_tooltip (page);

  adap_tab_page_set_indicator_icon (page, icon);
  adap_tab_page_set_indicator_tooltip (page, tooltip);

  g_object_unref (icon);
  g_free (tooltip);
}

static gboolean
extra_drag_drop_cb (AdapTabViewDemoWindow *self,
                    AdapTabPage           *page,
                    GValue               *value)
{
  adap_tab_page_set_title (page,  g_value_get_string (value));

  return GDK_EVENT_STOP;
}

static void
adap_tab_view_demo_window_dispose (GObject *object)
{
  AdapTabViewDemoWindow *self = ADAP_TAB_VIEW_DEMO_WINDOW (object);

  self->in_dispose = TRUE;

  g_clear_object (&self->tab_action_group);

  G_OBJECT_CLASS (adap_tab_view_demo_window_parent_class)->dispose (object);
}

static void
adap_tab_view_demo_window_class_init (AdapTabViewDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_tab_view_demo_window_dispose;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/tab-view/adap-tab-view-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapTabViewDemoWindow, view);
  gtk_widget_class_bind_template_child (widget_class, AdapTabViewDemoWindow, tab_bar);
  gtk_widget_class_bind_template_child (widget_class, AdapTabViewDemoWindow, tab_overview);
  gtk_widget_class_bind_template_callback (widget_class, page_detached_cb);
  gtk_widget_class_bind_template_callback (widget_class, setup_menu_cb);
  gtk_widget_class_bind_template_callback (widget_class, create_tab_cb);
  gtk_widget_class_bind_template_callback (widget_class, create_window_cb);
  gtk_widget_class_bind_template_callback (widget_class, indicator_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_drop_cb);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_t, GDK_CONTROL_MASK, "win.tab-new", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_n, GDK_CONTROL_MASK, "win.window-new", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_w, GDK_CONTROL_MASK, "tab.close", NULL);
}

static void
adap_tab_view_demo_window_init (AdapTabViewDemoWindow *self)
{
  GActionMap *action_map;
  GdkDisplay *display;
  AdapStyleManager *style_manager;

  gtk_widget_init_template (GTK_WIDGET (self));

  action_map = G_ACTION_MAP (g_simple_action_group_new ());
  g_action_map_add_action_entries (action_map,
                                   action_entries,
                                   G_N_ELEMENTS (action_entries),
                                   self);
  gtk_widget_insert_action_group (GTK_WIDGET (self),
                                  "win",
                                  G_ACTION_GROUP (action_map));

  self->tab_action_group = G_ACTION_MAP (g_simple_action_group_new ());
  g_action_map_add_action_entries (self->tab_action_group,
                                   tab_action_entries,
                                   G_N_ELEMENTS (tab_action_entries),
                                   self);

  gtk_widget_insert_action_group (GTK_WIDGET (self),
                                  "tab",
                                  G_ACTION_GROUP (self->tab_action_group));

  adap_tab_bar_setup_extra_drop_target (self->tab_bar,
                                       GDK_ACTION_COPY,
                                       (GType[1]) { G_TYPE_STRING }, 1);
  adap_tab_overview_setup_extra_drop_target (self->tab_overview,
                                            GDK_ACTION_COPY,
                                            (GType[1]) { G_TYPE_STRING }, 1);

  display = gtk_widget_get_display (GTK_WIDGET (self));
  style_manager = adap_style_manager_get_for_display (display);

  g_signal_connect_object (style_manager, "notify::dark",
                           G_CALLBACK (adap_tab_view_invalidate_thumbnails),
                           self->view,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (style_manager, "notify::high-contrast",
                           G_CALLBACK (adap_tab_view_invalidate_thumbnails),
                           self->view,
                           G_CONNECT_SWAPPED);
}

AdapTabViewDemoWindow *
adap_tab_view_demo_window_new (void)
{
  return g_object_new (ADAP_TYPE_TAB_VIEW_DEMO_WINDOW, NULL);
}

void
adap_tab_view_demo_window_prepopulate (AdapTabViewDemoWindow *self)
{
  tab_new (NULL, NULL, self);
  tab_new (NULL, NULL, self);
  tab_new (NULL, NULL, self);

  adap_tab_view_invalidate_thumbnails (self->view);
}
