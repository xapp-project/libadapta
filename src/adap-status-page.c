/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-status-page.h"

#include "adap-widget-utils-private.h"

/**
 * AdapStatusPage:
 *
 * A page used for empty/error states and similar use-cases.
 *
 * <picture>
 *   <source srcset="status-page-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="status-page.png" alt="status-page">
 * </picture>
 *
 * The `AdapStatusPage` widget can have an icon, a title, a description and a
 * custom widget which is displayed below them.
 *
 * ## CSS nodes
 *
 * `AdapStatusPage` has a main CSS node with name `statuspage`.
 *
 * `AdapStatusPage` can use the
 * [`.compact`](style-classes.html#compact-status-page) style class for when it
 * needs to fit into a small space such a sidebar or a popover.
 */

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_PAINTABLE,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_CHILD,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

struct _AdapStatusPage
{
  GtkWidget parent_instance;

  GtkWidget *scrolled_window;
  GtkBox *toplevel_box;
  GtkImage *image;
  char *icon_name;
  GdkPaintable *paintable;
  GtkLabel *title_label;
  GtkLabel *description_label;

  GtkWidget *user_widget;
};

static void adap_status_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapStatusPage, adap_status_page, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_status_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static gboolean
has_image (AdapStatusPage *self,
           const char    *icon_name,
           GdkPaintable  *paintable)
{
  return paintable || (icon_name && icon_name[0]);
}

static gboolean
string_is_not_empty (AdapStatusPage *self,
                     const char    *string)
{
  return string && string[0];
}

static void
adap_status_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdapStatusPage *self = ADAP_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_status_page_get_icon_name (self));
    break;

  case PROP_PAINTABLE:
    g_value_set_object (value, adap_status_page_get_paintable (self));
    break;

  case PROP_TITLE:
    g_value_set_string (value, adap_status_page_get_title (self));
    break;

  case PROP_DESCRIPTION:
    g_value_set_string (value, adap_status_page_get_description (self));
    break;

  case PROP_CHILD:
    g_value_set_object (value, adap_status_page_get_child (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_status_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdapStatusPage *self = ADAP_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adap_status_page_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_PAINTABLE:
    adap_status_page_set_paintable (self, g_value_get_object (value));
    break;

  case PROP_TITLE:
    adap_status_page_set_title (self, g_value_get_string (value));
    break;

  case PROP_DESCRIPTION:
    adap_status_page_set_description (self, g_value_get_string (value));
    break;

  case PROP_CHILD:
    adap_status_page_set_child (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_status_page_dispose (GObject *object)
{
  AdapStatusPage *self = ADAP_STATUS_PAGE (object);

  adap_status_page_set_child (self, NULL);

  self->user_widget = NULL;

  gtk_widget_dispose_template (GTK_WIDGET (self), ADAP_TYPE_STATUS_PAGE);

  G_OBJECT_CLASS (adap_status_page_parent_class)->dispose (object);
}

static void
adap_status_page_finalize (GObject *object)
{
  AdapStatusPage *self = ADAP_STATUS_PAGE (object);

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adap_status_page_parent_class)->finalize (object);
}

static void
adap_status_page_class_init (AdapStatusPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_status_page_get_property;
  object_class->set_property = adap_status_page_set_property;
  object_class->dispose = adap_status_page_dispose;
  object_class->finalize = adap_status_page_finalize;

  widget_class->compute_expand = adap_widget_compute_expand;

  /**
   * AdapStatusPage:icon-name: (attributes org.gtk.Property.get=adap_status_page_get_icon_name org.gtk.Property.set=adap_status_page_set_icon_name)
   *
   * The name of the icon to be used.
   *
   * Changing this will set [property@StatusPage:paintable] to `NULL`.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapStatusPage:paintable: (attributes org.gtk.Property.get=adap_status_page_get_paintable org.gtk.Property.set=adap_status_page_set_paintable)
   *
   * The paintable to be used.
   *
   * Changing this will set [property@StatusPage:icon-name] to `NULL`.
   */
  props[PROP_PAINTABLE] =
    g_param_spec_object ("paintable", NULL, NULL,
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapStatusPage:title: (attributes org.gtk.Property.get=adap_status_page_get_title org.gtk.Property.set=adap_status_page_set_title)
   *
   * The title to be displayed below the icon.
   * 
   * It is not parsed as Pango markup.
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapStatusPage:description: (attributes org.gtk.Property.get=adap_status_page_get_description org.gtk.Property.set=adap_status_page_set_description)
   *
   * The description markup to be displayed below the title.
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapStatusPage:child: (attributes org.gtk.Property.get=adap_status_page_get_child org.gtk.Property.set=adap_status_page_set_child)
   *
   * The child widget.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-status-page.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapStatusPage, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, AdapStatusPage, toplevel_box);
  gtk_widget_class_bind_template_child (widget_class, AdapStatusPage, image);
  gtk_widget_class_bind_template_child (widget_class, AdapStatusPage, title_label);
  gtk_widget_class_bind_template_child (widget_class, AdapStatusPage, description_label);
  gtk_widget_class_bind_template_callback (widget_class, has_image);
  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "statuspage");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adap_status_page_init (AdapStatusPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
adap_status_page_buildable_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const char   *type)
{
  AdapStatusPage *self = ADAP_STATUS_PAGE (buildable);

  if (!self->scrolled_window && GTK_IS_WIDGET (child))
    gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (buildable));
  else if (GTK_IS_WIDGET (child))
    adap_status_page_set_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_status_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_status_page_buildable_add_child;
}


/**
 * adap_status_page_new:
 *
 * Creates a new `AdapStatusPage`.
 *
 * Returns: the newly created `AdapStatusPage`
 */
GtkWidget *
adap_status_page_new (void)
{
  return g_object_new (ADAP_TYPE_STATUS_PAGE, NULL);
}

/**
 * adap_status_page_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a status page
 *
 * Gets the icon name for @self.
 *
 * Returns: (nullable): the icon name
 */
const char *
adap_status_page_get_icon_name (AdapStatusPage *self)
{
  g_return_val_if_fail (ADAP_IS_STATUS_PAGE (self), NULL);

  return self->icon_name;
}

/**
 * adap_status_page_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a status page
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 *
 * Changing this will set [property@StatusPage:paintable] to `NULL`.
 */
void
adap_status_page_set_icon_name (AdapStatusPage *self,
                               const char    *icon_name)
{
  g_return_if_fail (ADAP_IS_STATUS_PAGE (self));

  if (g_strcmp0 (self->icon_name, icon_name) == 0)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (self->paintable) {
    g_clear_object (&self->paintable);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAINTABLE]);
  }

  g_set_str (&self->icon_name, icon_name);
  gtk_image_set_from_icon_name (self->image, self->icon_name);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_status_page_get_paintable: (attributes org.gtk.Method.get_property=paintable)
 * @self: a status page
 *
 * Gets the paintable for @self.
 *
 * Returns: (nullable) (transfer none): the paintable
 */
GdkPaintable *
adap_status_page_get_paintable (AdapStatusPage *self)
{
  g_return_val_if_fail (ADAP_IS_STATUS_PAGE (self), NULL);

  return self->paintable;
}

/**
 * adap_status_page_set_paintable: (attributes org.gtk.Method.set_property=paintable)
 * @self: a status page
 * @paintable: (nullable): the paintable
 *
 * Sets the paintable for @self.
 *
 * Changing this will set [property@StatusPage:icon-name] to `NULL`.
 */
void
adap_status_page_set_paintable (AdapStatusPage *self,
                               GdkPaintable  *paintable)
{
  g_return_if_fail (ADAP_IS_STATUS_PAGE (self));
  g_return_if_fail (paintable == NULL || GDK_IS_PAINTABLE (paintable));

  if (self->paintable == paintable)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (self->icon_name) {
    g_clear_pointer (&self->icon_name, g_free);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
  }

  g_set_object (&self->paintable, paintable);
  gtk_image_set_from_paintable (self->image, self->paintable);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAINTABLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_status_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a status page
 *
 * Gets the title for @self.
 *
 * Returns: the title
 */
const char *
adap_status_page_get_title (AdapStatusPage *self)
{
  g_return_val_if_fail (ADAP_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->title_label);
}

/**
 * adap_status_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a status page
 * @title: the title
 *
 * Sets the title for @self.
 *
 * The title is displayed below the icon. It is not parsed as Pango markup.
 */
void
adap_status_page_set_title (AdapStatusPage *self,
                           const char    *title)
{
  g_return_if_fail (ADAP_IS_STATUS_PAGE (self));

  if (g_strcmp0 (title, adap_status_page_get_title (self)) == 0)
    return;

  gtk_label_set_label (self->title_label, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adap_status_page_get_description: (attributes org.gtk.Method.get_property=description)
 * @self: a status page
 *
 * Gets the description markup for @self.
 *
 * Returns: (nullable): the description
 */
const char *
adap_status_page_get_description (AdapStatusPage *self)
{
  g_return_val_if_fail (ADAP_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->description_label);
}

/**
 * adap_status_page_set_description: (attributes org.gtk.Method.set_property=description)
 * @self: a status page
 * @description: (nullable): the description
 *
 * Sets the description markup for @self.
 *
 * The description is displayed below the title. It is parsed as Pango markup.
 */
void
adap_status_page_set_description (AdapStatusPage *self,
                                 const char    *description)
{
  g_return_if_fail (ADAP_IS_STATUS_PAGE (self));

  if (g_strcmp0 (description, adap_status_page_get_description (self)) == 0)
    return;

  gtk_label_set_label (self->description_label, description);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

/**
 * adap_status_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a status page
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adap_status_page_get_child (AdapStatusPage *self)
{
  g_return_val_if_fail (ADAP_IS_STATUS_PAGE (self), NULL);

  return self->user_widget;
}

/**
 * adap_status_page_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a status page
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adap_status_page_set_child (AdapStatusPage *self,
                           GtkWidget     *child)
{
  g_return_if_fail (ADAP_IS_STATUS_PAGE (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (child == self->user_widget)
    return;

  if (self->user_widget)
    gtk_box_remove (self->toplevel_box, self->user_widget);

  self->user_widget = child;

  if (self->user_widget)
    gtk_box_append (self->toplevel_box, self->user_widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}
