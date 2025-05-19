#include "adap-tab-view-demo-page.h"

struct _AdapTabViewDemoPage
{
  AdapBin parent_instance;

  GtkEditable *title_entry;

  char *title;
  GIcon *icon;
  int color;

  GIcon *last_icon;
};

G_DEFINE_FINAL_TYPE (AdapTabViewDemoPage, adap_tab_view_demo_page, ADAP_TYPE_BIN)

#define N_COLORS 8

enum {
  PROP_0,
  PROP_TITLE,
  PROP_ICON,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

char **icon_names = NULL;
gsize n_icon_names = 0;

static void
init_icon_names (GtkIconTheme *theme)
{
  if (icon_names)
    return;

  icon_names = gtk_icon_theme_get_icon_names (theme);
  n_icon_names = g_strv_length (icon_names);
}

static GIcon *
get_random_icon (void)
{
  GdkDisplay *display = gdk_display_get_default ();
  GtkIconTheme *theme = gtk_icon_theme_get_for_display (display);
  int index;

  init_icon_names (theme);

  index = g_random_int_range (0, n_icon_names);

  return g_themed_icon_new (icon_names[index]);
}

static int
get_random_color (void)
{
  return g_random_int_range (0, N_COLORS) + 1;
}

static void
set_color (AdapTabViewDemoPage *self,
           int                 color)
{
  if (self->color == color)
    return;

  if (self->color > 0) {
    char *klass = g_strdup_printf ("tab-page-color-%d", self->color);

    gtk_widget_remove_css_class (GTK_WIDGET (self), klass);

    g_free (klass);
  }

  if (color > 0) {
    char *klass = g_strdup_printf ("tab-page-color-%d", color);

    gtk_widget_add_css_class (GTK_WIDGET (self), klass);

    g_free (klass);
  }

  self->color = color;
}

static void
adap_tab_view_demo_page_finalize (GObject *object)
{
  AdapTabViewDemoPage *self = ADAP_TAB_VIEW_DEMO_PAGE (object);

  g_clear_pointer (&self->title, g_free);
  g_clear_object (&self->icon);
  g_clear_object (&self->last_icon);

  G_OBJECT_CLASS (adap_tab_view_demo_page_parent_class)->finalize (object);
}

static void
adap_tab_view_demo_page_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdapTabViewDemoPage *self = ADAP_TAB_VIEW_DEMO_PAGE (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, self->title);
    break;
  case PROP_ICON:
    g_value_set_object (value, self->icon);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_view_demo_page_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdapTabViewDemoPage *self = ADAP_TAB_VIEW_DEMO_PAGE (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_clear_pointer (&self->title, g_free);
    self->title = g_value_dup_string (value);
    break;
  case PROP_ICON:
    g_set_object (&self->icon, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_tab_view_demo_page_class_init (AdapTabViewDemoPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adap_tab_view_demo_page_finalize;
  object_class->get_property = adap_tab_view_demo_page_get_property;
  object_class->set_property = adap_tab_view_demo_page_set_property;

  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_ICON] =
    g_param_spec_object ("icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/tab-view/adap-tab-view-demo-page.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapTabViewDemoPage, title_entry);
}

static void
adap_tab_view_demo_page_init (AdapTabViewDemoPage *self)
{
  self->icon = get_random_icon ();

  set_color (self, get_random_color ());

  gtk_widget_init_template (GTK_WIDGET (self));
}

AdapTabViewDemoPage *
adap_tab_view_demo_page_new (const char *title)
{
  return g_object_new (ADAP_TYPE_TAB_VIEW_DEMO_PAGE,
                       "title", title,
                       NULL);
}

AdapTabViewDemoPage *
adap_tab_view_demo_page_new_duplicate (AdapTabViewDemoPage *self)
{
  g_return_val_if_fail (ADAP_IS_TAB_VIEW_DEMO_PAGE (self), NULL);

  return g_object_new (ADAP_TYPE_TAB_VIEW_DEMO_PAGE,
                       "title", self->title,
                       "icon", self->icon,
                       NULL);
}

void
adap_tab_view_demo_page_refresh_icon (AdapTabViewDemoPage *self)
{
  GIcon *icon;

  g_return_if_fail (ADAP_IS_TAB_VIEW_DEMO_PAGE (self));

  icon = get_random_icon ();

  g_object_set (self, "icon", icon, NULL);

  g_object_unref (icon);
}

void
adap_tab_view_demo_page_set_enable_icon (AdapTabViewDemoPage *self,
                                        gboolean            enable_icon)
{
  g_return_if_fail (ADAP_IS_TAB_VIEW_DEMO_PAGE (self));

  enable_icon = !!enable_icon;

  if (enable_icon) {
    g_object_set (self, "icon", self->last_icon, NULL);
    g_clear_object (&self->last_icon);
  } else {
    self->last_icon = g_object_ref (self->icon);
    g_object_set (self, "icon", NULL, NULL);
  }
}
