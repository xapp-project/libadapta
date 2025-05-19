#include "adw-overlay-split-view-demo-window.h"

#include <glib/gi18n.h>

struct _AdwOverlaySplitViewDemoWindow
{
  AdwDialog parent_instance;

  AdwOverlaySplitView *split_view;
  GtkToggleButton *start_button;
};

G_DEFINE_FINAL_TYPE (AdwOverlaySplitViewDemoWindow, adw_overlay_split_view_demo_window, ADW_TYPE_DIALOG)

static void
start_button_notify_active_cb (AdwOverlaySplitViewDemoWindow *self)
{
  gboolean start = gtk_toggle_button_get_active (self->start_button);

  adw_overlay_split_view_set_sidebar_position (self->split_view,
                                               start ? GTK_PACK_START : GTK_PACK_END);
}

static void
adw_overlay_split_view_demo_window_class_init (AdwOverlaySplitViewDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/split-views/adw-overlay-split-view-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwOverlaySplitViewDemoWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, AdwOverlaySplitViewDemoWindow, start_button);
  gtk_widget_class_bind_template_callback (widget_class, start_button_notify_active_cb);
}

static void
adw_overlay_split_view_demo_window_init (AdwOverlaySplitViewDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwOverlaySplitViewDemoWindow *
adw_overlay_split_view_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_OVERLAY_SPLIT_VIEW_DEMO_WINDOW, NULL);
}
