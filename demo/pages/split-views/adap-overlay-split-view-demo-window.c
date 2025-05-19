#include "adap-overlay-split-view-demo-window.h"

#include <glib/gi18n.h>

struct _AdapOverlaySplitViewDemoWindow
{
  AdapDialog parent_instance;

  AdapOverlaySplitView *split_view;
  GtkToggleButton *start_button;
};

G_DEFINE_FINAL_TYPE (AdapOverlaySplitViewDemoWindow, adap_overlay_split_view_demo_window, ADAP_TYPE_DIALOG)

static void
start_button_notify_active_cb (AdapOverlaySplitViewDemoWindow *self)
{
  gboolean start = gtk_toggle_button_get_active (self->start_button);

  adap_overlay_split_view_set_sidebar_position (self->split_view,
                                               start ? GTK_PACK_START : GTK_PACK_END);
}

static void
adap_overlay_split_view_demo_window_class_init (AdapOverlaySplitViewDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/split-views/adap-overlay-split-view-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapOverlaySplitViewDemoWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, AdapOverlaySplitViewDemoWindow, start_button);
  gtk_widget_class_bind_template_callback (widget_class, start_button_notify_active_cb);
}

static void
adap_overlay_split_view_demo_window_init (AdapOverlaySplitViewDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdapOverlaySplitViewDemoWindow *
adap_overlay_split_view_demo_window_new (void)
{
  return g_object_new (ADAP_TYPE_OVERLAY_SPLIT_VIEW_DEMO_WINDOW, NULL);
}
