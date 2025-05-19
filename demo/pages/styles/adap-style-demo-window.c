#include "adw-style-demo-window.h"

#include <glib/gi18n.h>

struct _AdwStyleDemoWindow
{
  AdwDialog parent_instance;

  gboolean progress;

  AdwDialog *status_page_window;
  AdwDialog *sidebar_window;
  AdwNavigationSplitView *split_view;
};

G_DEFINE_FINAL_TYPE (AdwStyleDemoWindow, adw_style_demo_window, ADW_TYPE_DIALOG)

enum {
  PROP_0,
  PROP_DEVEL,
  PROP_PROGRESS,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
status_page_cb (GtkWidget  *sender,
                const char *name,
                GVariant   *param)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (sender);

  adw_dialog_present (self->status_page_window, GTK_WIDGET (self));
}

static void
sidebar_cb (GtkWidget  *sender,
            const char *name,
            GVariant   *param)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (sender);

  adw_dialog_present (self->sidebar_window, GTK_WIDGET (self));
}

static void
dummy_cb (GtkWidget  *sender,
          const char *name,
          GVariant   *param)
{
}

static gboolean
get_devel_style (AdwStyleDemoWindow *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WIDGET (root))
    return FALSE;

  return gtk_widget_has_css_class (GTK_WIDGET (root), "devel");
}

static void
set_devel_style (AdwStyleDemoWindow *self,
                 gboolean            devel)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WIDGET (root))
    return;

  if (devel)
    gtk_widget_add_css_class (GTK_WIDGET (root), "devel");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (root), "devel");
}

static void
sidebar_forward_cb (AdwStyleDemoWindow *self)
{
  adw_navigation_split_view_set_show_content (self->split_view, TRUE);
}

static void
adw_style_demo_window_root (GtkWidget *widget)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (widget);

  GTK_WIDGET_CLASS (adw_style_demo_window_parent_class)->root (widget);

  if (get_devel_style (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVEL]);
}

static void
adw_style_demo_window_unroot (GtkWidget *widget)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (widget);
  gboolean has_devel = get_devel_style (self);

  GTK_WIDGET_CLASS (adw_style_demo_window_parent_class)->unroot (widget);

  if (has_devel)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVEL]);
}

static void
adw_style_demo_window_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_DEVEL:
    g_value_set_boolean (value, get_devel_style (self));
    break;
  case PROP_PROGRESS:
    g_value_set_boolean (value, self->progress);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_window_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_DEVEL:
    set_devel_style (self, g_value_get_boolean (value));
    break;
  case PROP_PROGRESS:
    self->progress = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_window_class_init (AdwStyleDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_style_demo_window_get_property;
  object_class->set_property = adw_style_demo_window_set_property;
  widget_class->root = adw_style_demo_window_root;
  widget_class->unroot = adw_style_demo_window_unroot;

  props[PROP_DEVEL] =
    g_param_spec_boolean ("devel", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_PROGRESS] =
    g_param_spec_boolean ("progress", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/styles/adw-style-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, status_page_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, sidebar_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, split_view);
  gtk_widget_class_bind_template_callback (widget_class, sidebar_forward_cb);

  gtk_widget_class_install_property_action (widget_class, "style.devel", "devel");
  gtk_widget_class_install_property_action (widget_class, "style.progress", "progress");
  gtk_widget_class_install_action (widget_class, "style.status-page", NULL, status_page_cb);
  gtk_widget_class_install_action (widget_class, "style.sidebar", NULL, sidebar_cb);
  gtk_widget_class_install_action (widget_class, "style.dummy", NULL, dummy_cb);
}

static void
adw_style_demo_window_init (AdwStyleDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwStyleDemoWindow *
adw_style_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_STYLE_DEMO_WINDOW, NULL);
}
