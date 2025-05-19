#include "adw-demo-page-carousel.h"

#include <glib/gi18n.h>

struct _AdwDemoPageCarousel
{
  AdwBin parent_instance;

  GtkBox *box;
  AdwCarousel *carousel;
  GtkStack *indicators_stack;
  AdwComboRow *orientation_row;
  AdwComboRow *indicators_row;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageCarousel, adw_demo_page_carousel, ADW_TYPE_BIN)

static char *
get_orientation_name (AdwEnumListItem *item,
                      gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case GTK_ORIENTATION_HORIZONTAL:
    return g_strdup (_("Horizontal"));
  case GTK_ORIENTATION_VERTICAL:
    return g_strdup (_("Vertical"));
  default:
    return NULL;
  }
}

static void
notify_orientation_cb (AdwDemoPageCarousel *self)
{
  GtkOrientation orientation = adw_combo_row_get_selected (self->orientation_row);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->box),
                                  1 - orientation);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->carousel),
                                  orientation);
}

static char *
get_indicators_name (GtkStringObject *value)
{
  const char *style;

  g_assert (GTK_IS_STRING_OBJECT (value));

  style = gtk_string_object_get_string (value);

  if (!g_strcmp0 (style, "dots"))
    return g_strdup (_("Dots"));

  if (!g_strcmp0 (style, "lines"))
    return g_strdup (_("Lines"));

  return NULL;
}

static void
notify_indicators_cb (AdwDemoPageCarousel *self)
{
  GtkStringObject *obj = adw_combo_row_get_selected_item (self->indicators_row);

  gtk_stack_set_visible_child_name (self->indicators_stack,
                                    gtk_string_object_get_string (obj));
}

static void
carousel_return_cb (AdwDemoPageCarousel *self)
{
  adw_carousel_scroll_to (self->carousel,
                          adw_carousel_get_nth_page (self->carousel, 0),
                          TRUE);
}

static void
adw_demo_page_carousel_class_init (AdwDemoPageCarouselClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/carousel/adw-demo-page-carousel.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageCarousel, box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageCarousel, carousel);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageCarousel, indicators_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageCarousel, orientation_row);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageCarousel, indicators_row);
  gtk_widget_class_bind_template_callback (widget_class, get_orientation_name);
  gtk_widget_class_bind_template_callback (widget_class, notify_orientation_cb);
  gtk_widget_class_bind_template_callback (widget_class, get_indicators_name);
  gtk_widget_class_bind_template_callback (widget_class, notify_indicators_cb);

  gtk_widget_class_install_action (widget_class, "carousel.return", NULL, (GtkWidgetActionActivateFunc) carousel_return_cb);
}

static void
adw_demo_page_carousel_init (AdwDemoPageCarousel *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
