#include "adap-demo-window.h"

#include <glib/gi18n.h>

#include "pages/about/adap-demo-page-about.h"
#include "pages/animations/adap-demo-page-animations.h"
#include "pages/avatar/adap-demo-page-avatar.h"
#include "pages/banners/adap-demo-page-banners.h"
#include "pages/buttons/adap-demo-page-buttons.h"
#include "pages/carousel/adap-demo-page-carousel.h"
#include "pages/clamp/adap-demo-page-clamp.h"
#include "pages/dialogs/adap-demo-page-dialogs.h"
#include "pages/lists/adap-demo-page-lists.h"
#include "pages/navigation-view/adap-demo-page-navigation-view.h"
#include "pages/split-views/adap-demo-page-split-views.h"
#include "pages/styles/adap-demo-page-styles.h"
#include "pages/tab-view/adap-demo-page-tab-view.h"
#include "pages/toasts/adap-demo-page-toasts.h"
#include "pages/view-switcher/adap-demo-page-view-switcher.h"
#include "pages/welcome/adap-demo-page-welcome.h"

struct _AdapDemoWindow
{
  AdapApplicationWindow parent_instance;

  GtkWidget *color_scheme_button;
  AdapNavigationSplitView *split_view;
  AdapNavigationPage *content_page;
  GtkStack *stack;
  AdapDemoPageToasts *toasts_page;
};

G_DEFINE_FINAL_TYPE (AdapDemoWindow, adap_demo_window, ADAP_TYPE_APPLICATION_WINDOW)

static char *
get_color_scheme_icon_name (gpointer user_data,
                            gboolean dark)
{
  return g_strdup (dark ? "light-mode-symbolic" : "dark-mode-symbolic");
}

static void
color_scheme_button_clicked_cb (AdapDemoWindow *self)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();

  if (adap_style_manager_get_dark (manager))
    adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_FORCE_LIGHT);
  else
    adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_FORCE_DARK);
}

static void
notify_system_supports_color_schemes_cb (AdapDemoWindow *self)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();
  gboolean supports = adap_style_manager_get_system_supports_color_schemes (manager);

  gtk_widget_set_visible (self->color_scheme_button, !supports);

  if (supports)
    adap_style_manager_set_color_scheme (manager, ADAP_COLOR_SCHEME_DEFAULT);
}

static void
notify_visible_child_cb (AdapDemoWindow *self)
{
  GtkWidget *child = gtk_stack_get_visible_child (self->stack);
  GtkStackPage *page = gtk_stack_get_page (self->stack, child);

  adap_navigation_page_set_title (self->content_page,
                                 gtk_stack_page_get_title (page));
  adap_navigation_split_view_set_show_content (self->split_view, TRUE);
}

static void
toast_undo_cb (AdapDemoWindow *self)
{
  adap_demo_page_toasts_undo (self->toasts_page);
}

static void
adap_demo_window_class_init (AdapDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_q, GDK_CONTROL_MASK, "window.close", NULL);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/adap-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapDemoWindow, color_scheme_button);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoWindow, content_page);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoWindow, toasts_page);
  gtk_widget_class_bind_template_callback (widget_class, get_color_scheme_icon_name);
  gtk_widget_class_bind_template_callback (widget_class, color_scheme_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_child_cb);

  gtk_widget_class_install_action (widget_class, "toast.undo", NULL, (GtkWidgetActionActivateFunc) toast_undo_cb);
}

static void
adap_demo_window_init (AdapDemoWindow *self)
{
  AdapStyleManager *manager = adap_style_manager_get_default ();

  g_type_ensure (ADAP_TYPE_DEMO_PAGE_ABOUT);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_ANIMATIONS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_BANNERS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_AVATAR);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_BUTTONS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_CAROUSEL);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_CLAMP);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_DIALOGS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_LISTS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_NAVIGATION_VIEW);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_SPLIT_VIEWS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_STYLES);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_TAB_VIEW);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_TOASTS);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_VIEW_SWITCHER);
  g_type_ensure (ADAP_TYPE_DEMO_PAGE_WELCOME);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (manager,
                           "notify::system-supports-color-schemes",
                           G_CALLBACK (notify_system_supports_color_schemes_cb),
                           self,
                           G_CONNECT_SWAPPED);

  notify_system_supports_color_schemes_cb (self);

  notify_visible_child_cb (self);
}

AdapDemoWindow *
adap_demo_window_new (GtkApplication *application)
{
  return g_object_new (ADAP_TYPE_DEMO_WINDOW, "application", application, NULL);
}
