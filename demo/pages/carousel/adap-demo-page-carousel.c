#include "adap-demo-page-carousel.h"

#include <glib/gi18n.h>

struct _AdapDemoPageCarousel
{
  AdapBin parent_instance;

  GtkBox *box;
  AdapCarousel *carousel;
  GtkStack *indicators_stack;
  AdapComboRow *orientation_row;
  AdapComboRow *indicators_row;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageCarousel, adap_demo_page_carousel, ADAP_TYPE_BIN)

static char *
get_orientation_name (AdapEnumListItem *item,
                      gpointer         user_data)
{
  switch (adap_enum_list_item_get_value (item)) {
  case GTK_ORIENTATION_HORIZONTAL:
    return g_strdup (_("Horizontal"));
  case GTK_ORIENTATION_VERTICAL:
    return g_strdup (_("Vertical"));
  default:
    return NULL;
  }
}

static void
notify_orientation_cb (AdapDemoPageCarousel *self)
{
  GtkOrientation orientation = adap_combo_row_get_selected (self->orientation_row);

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
notify_indicators_cb (AdapDemoPageCarousel *self)
{
  GtkStringObject *obj = adap_combo_row_get_selected_item (self->indicators_row);

  gtk_stack_set_visible_child_name (self->indicators_stack,
                                    gtk_string_object_get_string (obj));
}

static void
carousel_return_cb (AdapDemoPageCarousel *self)
{
  adap_carousel_scroll_to (self->carousel,
                          adap_carousel_get_nth_page (self->carousel, 0),
                          TRUE);
}

static void
adap_demo_page_carousel_class_init (AdapDemoPageCarouselClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/carousel/adap-demo-page-carousel.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageCarousel, box);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageCarousel, carousel);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageCarousel, indicators_stack);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageCarousel, orientation_row);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageCarousel, indicators_row);
  gtk_widget_class_bind_template_callback (widget_class, get_orientation_name);
  gtk_widget_class_bind_template_callback (widget_class, notify_orientation_cb);
  gtk_widget_class_bind_template_callback (widget_class, get_indicators_name);
  gtk_widget_class_bind_template_callback (widget_class, notify_indicators_cb);

  gtk_widget_class_install_action (widget_class, "carousel.return", NULL, (GtkWidgetActionActivateFunc) carousel_return_cb);
}

static void
adap_demo_page_carousel_init (AdapDemoPageCarousel *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
