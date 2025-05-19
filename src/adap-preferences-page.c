/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-preferences-page-private.h"

#include "adap-preferences-group-private.h"
#include "adap-widget-utils-private.h"

/**
 * AdapPreferencesPage:
 *
 * A page from [class@PreferencesDialog].
 *
 * <picture>
 *   <source srcset="preferences-page-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="preferences-page.png" alt="preferences-page">
 * </picture>
 *
 * The `AdapPreferencesPage` widget gathers preferences groups into a single page
 * of a preferences window.
 *
 * ## CSS nodes
 *
 * `AdapPreferencesPage` has a single CSS node with name `preferencespage`.
 *
 * ## Accessibility
 *
 * `AdapPreferencesPage` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 */

typedef struct
{
  GtkBox *box;
  GtkLabel *description;
  GtkWidget *scrolled_window;

  char *icon_name;
  char *title;

  char *name;

  gboolean use_underline;
} AdapPreferencesPagePrivate;

static void adap_preferences_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapPreferencesPage, adap_preferences_page, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdapPreferencesPage)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adap_preferences_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_NAME,
  PROP_USE_UNDERLINE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static gboolean
is_visible_group (GtkWidget *widget,
                  gpointer   user_data)
{
  return ADAP_IS_PREFERENCES_GROUP (widget) &&
         gtk_widget_get_visible (widget);
}

static void
adap_preferences_page_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdapPreferencesPage *self = ADAP_PREFERENCES_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_preferences_page_get_icon_name (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, adap_preferences_page_get_title (self));
    break;
  case PROP_DESCRIPTION:
    g_value_set_string (value, adap_preferences_page_get_description (self));
    break;
  case PROP_NAME:
    g_value_set_string (value, adap_preferences_page_get_name (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adap_preferences_page_get_use_underline (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_preferences_page_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdapPreferencesPage *self = ADAP_PREFERENCES_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adap_preferences_page_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_TITLE:
    adap_preferences_page_set_title (self, g_value_get_string (value));
    break;
  case PROP_DESCRIPTION:
    adap_preferences_page_set_description (self, g_value_get_string (value));
    break;
  case PROP_NAME:
    adap_preferences_page_set_name (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    adap_preferences_page_set_use_underline (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_preferences_page_dispose (GObject *object)
{
  gtk_widget_dispose_template (GTK_WIDGET (object), ADAP_TYPE_PREFERENCES_PAGE);

  G_OBJECT_CLASS (adap_preferences_page_parent_class)->dispose (object);
}

static void
adap_preferences_page_finalize (GObject *object)
{
  AdapPreferencesPage *self = ADAP_PREFERENCES_PAGE (object);
  AdapPreferencesPagePrivate *priv = adap_preferences_page_get_instance_private (self);

  g_clear_pointer (&priv->icon_name, g_free);
  g_clear_pointer (&priv->title, g_free);
  g_clear_pointer (&priv->name, g_free);

  G_OBJECT_CLASS (adap_preferences_page_parent_class)->finalize (object);
}

static void
adap_preferences_page_class_init (AdapPreferencesPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_preferences_page_get_property;
  object_class->set_property = adap_preferences_page_set_property;
  object_class->dispose = adap_preferences_page_dispose;
  object_class->finalize = adap_preferences_page_finalize;

  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->focus = adap_widget_focus_child;

  /**
   * AdapPreferencesPage:icon-name: (attributes org.gtk.Property.get=adap_preferences_page_get_icon_name org.gtk.Property.set=adap_preferences_page_set_icon_name)
   *
   * The icon name for this page.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesPage:title: (attributes org.gtk.Property.get=adap_preferences_page_get_title org.gtk.Property.set=adap_preferences_page_set_title)
   *
   * The title for this page.
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesPage:description: (attributes org.gtk.Property.get=adap_preferences_page_get_description org.gtk.Property.set=adap_preferences_page_set_description)
   *
   * The description to be displayed at the top of the page.
   * 
   * Since: 1.4
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesPage:name: (attributes org.gtk.Property.get=adap_preferences_page_get_name org.gtk.Property.set=adap_preferences_page_set_name)
   *
   * The name of this page.
   */
  props[PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesPage:use-underline: (attributes org.gtk.Property.get=adap_preferences_page_get_use_underline org.gtk.Property.set=adap_preferences_page_set_use_underline)
   *
   * Whether an embedded underline in the title indicates a mnemonic.
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-preferences-page.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesPage, box);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesPage, description);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesPage, scrolled_window);

  gtk_widget_class_set_css_name (widget_class, "preferencespage");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
adap_preferences_page_init (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv = adap_preferences_page_get_instance_private (self);

  priv->title = g_strdup ("");

  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
adap_preferences_page_buildable_add_child (GtkBuildable *buildable,
                                          GtkBuilder   *builder,
                                          GObject      *child,
                                          const char   *type)
{
  AdapPreferencesPage *self = ADAP_PREFERENCES_PAGE (buildable);
  AdapPreferencesPagePrivate *priv = adap_preferences_page_get_instance_private (self);

  if (priv->box && ADAP_IS_PREFERENCES_GROUP (child))
    adap_preferences_page_add (self, ADAP_PREFERENCES_GROUP (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_preferences_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adap_preferences_page_buildable_add_child;
}

/**
 * adap_preferences_page_new:
 *
 * Creates a new `AdapPreferencesPage`.
 *
 * Returns: the newly created `AdapPreferencesPage`
 */
GtkWidget *
adap_preferences_page_new (void)
{
  return g_object_new (ADAP_TYPE_PREFERENCES_PAGE, NULL);
}

/**
 * adap_preferences_page_add:
 * @self: a preferences page
 * @group: the group to add
 *
 * Adds a preferences group to @self.
 */
void
adap_preferences_page_add (AdapPreferencesPage  *self,
                          AdapPreferencesGroup *group)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));
  g_return_if_fail (ADAP_IS_PREFERENCES_GROUP (group));

  priv = adap_preferences_page_get_instance_private (self);

  gtk_box_append (priv->box, GTK_WIDGET (group));
}

/**
 * adap_preferences_page_remove:
 * @self: a preferences page
 * @group: the group to remove
 *
 * Removes a group from @self.
 */
void
adap_preferences_page_remove (AdapPreferencesPage  *self,
                             AdapPreferencesGroup *group)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));
  g_return_if_fail (ADAP_IS_PREFERENCES_GROUP (group));

  priv = adap_preferences_page_get_instance_private (self);

  if (gtk_widget_get_parent (GTK_WIDGET (group)) == GTK_WIDGET (priv->box))
    gtk_box_remove (priv->box, GTK_WIDGET (group));
  else
    ADAP_CRITICAL_CANNOT_REMOVE_CHILD (self, group);
}

/**
 * adap_preferences_page_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a preferences page
 *
 * Gets the icon name for @self.
 *
 * Returns: (nullable): the icon name for @self
 */
const char *
adap_preferences_page_get_icon_name (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), NULL);

  priv = adap_preferences_page_get_instance_private (self);

  return priv->icon_name;
}

/**
 * adap_preferences_page_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a preferences page
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 */
void
adap_preferences_page_set_icon_name (AdapPreferencesPage *self,
                                    const char         *icon_name)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);

  if (!g_set_str (&priv->icon_name, icon_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adap_preferences_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a preferences page
 *
 * Gets the title of @self.
 *
 * Returns: the title of @self.
 */
const char *
adap_preferences_page_get_title (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), NULL);

  priv = adap_preferences_page_get_instance_private (self);

  return priv->title;
}

/**
 * adap_preferences_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a preferences page
 * @title: the title
 *
 * Sets the title of @self.
 */
void
adap_preferences_page_set_title (AdapPreferencesPage *self,
                                const char         *title)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);

  if (!g_set_str (&priv->title, title ? title : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adap_preferences_page_get_description: (attributes org.gtk.Method.get_property=description)
 * @self: a preferences page
 *
 * Gets the description of @self.
 *
 * Returns: the description of @self.
 * 
 * Since: 1.4
 */
const char *
adap_preferences_page_get_description (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), NULL);

  priv = adap_preferences_page_get_instance_private (self);

  return gtk_label_get_text (priv->description);
}

/**
 * adap_preferences_page_set_description: (attributes org.gtk.Method.set_property=description)
 * @self: a preferences page
 * @description: the description
 *
 * Sets the description of @self.
 * 
 * The description is displayed at the top of the page.
 * 
 * Since: 1.4
 */
void
adap_preferences_page_set_description (AdapPreferencesPage *self,
                                      const char         *description)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->description), description) == 0)
    return;

  gtk_label_set_label (priv->description, description);
  gtk_widget_set_visible (GTK_WIDGET (priv->description),
                          description && *description);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

/**
 * adap_preferences_page_get_name: (attributes org.gtk.Method.get_property=name)
 * @self: a preferences page
 *
 * Gets the name of @self.
 *
 * Returns: (nullable): the name of @self
 */
const char *
adap_preferences_page_get_name (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), NULL);

  priv = adap_preferences_page_get_instance_private (self);

  return priv->name;
}

/**
 * adap_preferences_page_set_name: (attributes org.gtk.Method.set_property=name)
 * @self: a preferences page
 * @name: (nullable): the name
 *
 * Sets the name of @self.
 */
void
adap_preferences_page_set_name (AdapPreferencesPage *self,
                               const char         *name)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);

  if (!g_set_str (&priv->name, name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);
}

/**
 * adap_preferences_page_get_use_underline: (attributes org.gtk.Method.get_property=use-underline)
 * @self: a preferences page
 *
 * Gets whether an embedded underline in the title indicates a mnemonic.
 *
 * Returns: whether an embedded underline in the title indicates a mnemonic
 */
gboolean
adap_preferences_page_get_use_underline (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), FALSE);

  priv = adap_preferences_page_get_instance_private (self);

  return priv->use_underline;
}

/**
 * adap_preferences_page_set_use_underline: (attributes org.gtk.Method.set_property=use-underline)
 * @self: a preferences page
 * @use_underline: `TRUE` if underlines in the text indicate mnemonics
 *
 * Sets whether an embedded underline in the title indicates a mnemonic.
 */
void
adap_preferences_page_set_use_underline (AdapPreferencesPage *self,
                                        gboolean           use_underline)
{
  AdapPreferencesPagePrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);

  use_underline = !!use_underline;

  if (priv->use_underline == use_underline)
    return;

  priv->use_underline = use_underline;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

static GListModel *
preferences_group_to_rows (AdapPreferencesGroup *group)
{
  g_object_unref (group);

  return adap_preferences_group_get_rows (group);
}

/**
 * adap_preferences_page_get_rows:
 * @self: a preferences page
 *
 * Gets a [iface@Gio.ListModel] that contains the rows of the page.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a list model for the page's rows
 */
GListModel *
adap_preferences_page_get_rows (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;
  GListModel *model;
  GtkCustomFilter *filter;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_PAGE (self), NULL);

  priv = adap_preferences_page_get_instance_private (self);

  filter = gtk_custom_filter_new ((GtkCustomFilterFunc) is_visible_group, NULL, NULL);

  model = gtk_widget_observe_children (GTK_WIDGET (priv->box));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (filter)));
  model = G_LIST_MODEL (gtk_map_list_model_new (model,
                                                (GtkMapListModelMapFunc) preferences_group_to_rows,
                                                NULL,
                                                NULL));

  return G_LIST_MODEL (gtk_flatten_list_model_new (model));
}

/**
 * adap_preferences_page_scroll_to_top:
 * @self: a preferences page
 *
 * Scrolls the scrolled window of @self to the top.
 *
 * Since: 1.3
 */
void
adap_preferences_page_scroll_to_top (AdapPreferencesPage *self)
{
  AdapPreferencesPagePrivate *priv;
  GtkAdjustment *adjustment;

  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (self));

  priv = adap_preferences_page_get_instance_private (self);
  adjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (priv->scrolled_window));

  gtk_adjustment_set_value (adjustment, gtk_adjustment_get_lower (adjustment));
}
