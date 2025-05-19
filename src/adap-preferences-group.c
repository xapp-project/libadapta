/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-preferences-group-private.h"

#include "adw-preferences-row.h"
#include "adw-widget-utils-private.h"

/**
 * AdwPreferencesGroup:
 *
 * A group of preference rows.
 *
 * <picture>
 *   <source srcset="preferences-group-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="preferences-group.png" alt="preferences-group">
 * </picture>
 *
 * An `AdwPreferencesGroup` represents a group or tightly related preferences,
 * which in turn are represented by [class@PreferencesRow].
 *
 * To summarize the role of the preferences it gathers, a group can have both a
 * title and a description. The title will be used by [class@PreferencesDialog]
 * to let the user look for a preference.
 *
 * ## AdwPreferencesGroup as GtkBuildable
 *
 * The `AdwPreferencesGroup` implementation of the [iface@Gtk.Buildable] interface
 * supports adding [class@PreferencesRow]s to the list by omitting "type". If "type"
 * is omitted and the widget isn't a [class@PreferencesRow] the child is added to
 * a box below the list.
 *
 * When the "type" attribute of a child is `header-suffix`, the child
 * is set as the suffix on the end of the title and description.
 *
 * ## CSS nodes
 *
 * `AdwPreferencesGroup` has a single CSS node with name `preferencesgroup`.
 *
 * ## Accessibility
 *
 * `AdwPreferencesGroup` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 */

typedef struct
{
  GtkWidget *box;
  GtkLabel *description;
  GtkListBox *listbox;
  GtkBox *listbox_box;
  GtkLabel *title;
  GtkBox *header_box;
  GtkWidget *header_suffix;

  GListModel *rows;
} AdwPreferencesGroupPrivate;

static void adw_preferences_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwPreferencesGroup, adw_preferences_group, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdwPreferencesGroup)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_preferences_group_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_HEADER_SUFFIX,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
update_title_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->title),
                          gtk_label_get_text (priv->title) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->title), "") != 0);
}

static void
update_description_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->description),
                          gtk_label_get_text (priv->description) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->description), "") != 0);
}

static void
update_listbox_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  /* We must wait until the listbox has been built and added. */

  if (priv->rows == NULL)
    return;

  gtk_widget_set_visible (GTK_WIDGET (priv->listbox),
                          g_list_model_get_n_items (priv->rows) > 0);
}

static gboolean
is_single_line (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  if (gtk_widget_get_visible (GTK_WIDGET (priv->description)))
    return FALSE;

  if (priv->header_suffix)
    return TRUE;

  if (gtk_widget_get_visible (GTK_WIDGET (priv->title)))
    return TRUE;

  return FALSE;
}

static void
update_header_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->header_box),
                          gtk_widget_get_visible (GTK_WIDGET (priv->title)) ||
                          gtk_widget_get_visible (GTK_WIDGET (priv->description)) ||
                          priv->header_suffix != NULL);

  if (is_single_line (self))
    gtk_widget_add_css_class (GTK_WIDGET (priv->header_box), "single-line");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (priv->header_box), "single-line");
}

static gboolean
listbox_keynav_failed_cb (AdwPreferencesGroup *self,
                          GtkDirectionType     direction)
{
  GtkWidget *toplevel = GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (self)));

  if (!toplevel)
    return FALSE;

  if (direction != GTK_DIR_UP && direction != GTK_DIR_DOWN)
    return FALSE;

  return gtk_widget_child_focus (toplevel, direction == GTK_DIR_UP ?
                                 GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD);
}

static gboolean
row_has_title (AdwPreferencesRow *row,
               gpointer           user_data)
{
  const char *title;

  g_assert (ADW_IS_PREFERENCES_ROW (row));

  if (!gtk_widget_get_visible (GTK_WIDGET (row)))
    return FALSE;

  title = adw_preferences_row_get_title (row);

  return title && *title;
}

static void
adw_preferences_group_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_preferences_group_get_title (self));
    break;
  case PROP_DESCRIPTION:
    g_value_set_string (value, adw_preferences_group_get_description (self));
    break;
  case PROP_HEADER_SUFFIX:
    g_value_set_object (value, adw_preferences_group_get_header_suffix (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_group_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_preferences_group_set_title (self, g_value_get_string (value));
    break;
  case PROP_DESCRIPTION:
    adw_preferences_group_set_description (self, g_value_get_string (value));
    break;
  case PROP_HEADER_SUFFIX:
    adw_preferences_group_set_header_suffix (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_group_dispose (GObject *object)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  g_clear_object (&priv->rows);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_PREFERENCES_GROUP);

  G_OBJECT_CLASS (adw_preferences_group_parent_class)->dispose (object);
}

static void
adw_preferences_group_class_init (AdwPreferencesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_preferences_group_get_property;
  object_class->set_property = adw_preferences_group_set_property;
  object_class->dispose = adw_preferences_group_dispose;

  widget_class->compute_expand = adw_widget_compute_expand;
  widget_class->focus = adw_widget_focus_child;

  /**
   * AdwPreferencesGroup:title: (attributes org.gtk.Property.get=adw_preferences_group_get_title org.gtk.Property.set=adw_preferences_group_set_title)
   *
   * The title for this group of preferences.
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesGroup:description: (attributes org.gtk.Property.get=adw_preferences_group_get_description org.gtk.Property.set=adw_preferences_group_set_description)
   *
   * The description for this group of preferences.
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesGroup:header-suffix: (attributes org.gtk.Property.get=adw_preferences_group_get_header_suffix org.gtk.Property.set=adw_preferences_group_set_header_suffix)
   *
   * The header suffix widget.
   *
   * Displayed above the list, next to the title and description.
   *
   * Suffixes are commonly used to show a button or a spinner for the whole
   * group.
   *
   * Since: 1.1
   */
  props[PROP_HEADER_SUFFIX] =
    g_param_spec_object ("header-suffix", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-preferences-group.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, box);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, description);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, listbox);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, listbox_box);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, title);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, header_box);
  gtk_widget_class_bind_template_callback (widget_class, listbox_keynav_failed_cb);

  gtk_widget_class_set_css_name (widget_class, "preferencesgroup");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
adw_preferences_group_init (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  update_description_visibility (self);
  update_title_visibility (self);
  update_listbox_visibility (self);
  update_header_visibility (self);

  priv->rows = gtk_widget_observe_children (GTK_WIDGET (priv->listbox));

  g_signal_connect_object (priv->rows, "items-changed",
                           G_CALLBACK (update_listbox_visibility), self,
                           G_CONNECT_SWAPPED);
}

static void
adw_preferences_group_buildable_add_child (GtkBuildable *buildable,
                                           GtkBuilder   *builder,
                                           GObject      *child,
                                           const char   *type)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (buildable);
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  if (g_strcmp0 (type, "header-suffix") == 0 && GTK_IS_WIDGET (child))
    adw_preferences_group_set_header_suffix (self, GTK_WIDGET (child));
  else if (priv->box && GTK_IS_WIDGET (child))
    adw_preferences_group_add (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_preferences_group_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_preferences_group_buildable_add_child;
}

/**
 * adw_preferences_group_new:
 *
 * Creates a new `AdwPreferencesGroup`.
 *
 * Returns: the newly created `AdwPreferencesGroup`
 */
GtkWidget *
adw_preferences_group_new (void)
{
  return g_object_new (ADW_TYPE_PREFERENCES_GROUP, NULL);
}

/**
 * adw_preferences_group_add:
 * @self: a preferences group
 * @child: the widget to add
 *
 * Adds a child to @self.
 */
void
adw_preferences_group_add (AdwPreferencesGroup *self,
                           GtkWidget           *child)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adw_preferences_group_get_instance_private (self);

  if (ADW_IS_PREFERENCES_ROW (child))
    gtk_list_box_append (priv->listbox, child);
  else
    gtk_box_append (priv->listbox_box, child);
}

/**
 * adw_preferences_group_remove:
 * @self: a preferences group
 * @child: the child to remove
 *
 * Removes a child from @self.
 */
void
adw_preferences_group_remove (AdwPreferencesGroup *self,
                              GtkWidget           *child)
{
  AdwPreferencesGroupPrivate *priv;
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adw_preferences_group_get_instance_private (self);

  parent = gtk_widget_get_parent (child);

  if (parent == GTK_WIDGET (priv->listbox))
    gtk_list_box_remove (priv->listbox, child);
  else if (parent == GTK_WIDGET (priv->listbox_box))
    gtk_box_remove (priv->listbox_box, child);
  else
    ADW_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
}

/**
 * adw_preferences_group_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a preferences group
 *
 * Gets the title of @self.
 *
 * Returns: the title of @self
 */
const char *
adw_preferences_group_get_title (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->title);
}

/**
 * adw_preferences_group_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a preferences group
 * @title: the title
 *
 * Sets the title for @self.
 */
void
adw_preferences_group_set_title (AdwPreferencesGroup *self,
                                 const char          *title)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));

  priv = adw_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->title), title) == 0)
    return;

  gtk_label_set_label (priv->title, title);
  update_title_visibility (self);
  update_header_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_preferences_group_get_description: (attributes org.gtk.Method.get_property=description)
 * @self: a preferences group
 *
 * Gets the description of @self.
 *
 * Returns: (nullable): the description of @self
 */
const char *
adw_preferences_group_get_description (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->description);
}

/**
 * adw_preferences_group_set_description: (attributes org.gtk.Method.set_property=description)
 * @self: a preferences group
 * @description: (nullable): the description
 *
 * Sets the description for @self.
 */
void
adw_preferences_group_set_description (AdwPreferencesGroup *self,
                                       const char          *description)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));

  priv = adw_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->description), description) == 0)
    return;

  gtk_label_set_label (priv->description, description);
  update_description_visibility (self);
  update_header_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

/**
 * adw_preferences_group_get_header_suffix: (attributes org.gtk.Method.get_property=header-suffix)
 * @self: a `AdwPreferencesGroup`
 *
 * Gets the suffix for @self's header.
 *
 * Returns: (nullable) (transfer none): the suffix for @self's header.
 *
 * Since: 1.1
 */
GtkWidget *
adw_preferences_group_get_header_suffix (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  return priv->header_suffix;
}

/**
 * adw_preferences_group_set_header_suffix: (attributes org.gtk.Method.set_property=header-suffix)
 * @self: a `AdwPreferencesGroup`
 * @suffix: (nullable): the suffix to set
 *
 * Sets the suffix for @self's header.
 *
 * Displayed above the list, next to the title and description.
 *
 * Suffixes are commonly used to show a button or a spinner for the whole group.
 *
 * Since: 1.1
 */
void
adw_preferences_group_set_header_suffix (AdwPreferencesGroup *self,
                                         GtkWidget           *suffix)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (suffix == NULL || GTK_IS_WIDGET (suffix));

  if (suffix)
    g_return_if_fail (gtk_widget_get_parent (suffix) == NULL);

  priv = adw_preferences_group_get_instance_private (self);

  if (suffix == priv->header_suffix)
    return;

  if (priv->header_suffix)
    gtk_box_remove (priv->header_box, priv->header_suffix);

  priv->header_suffix = suffix;

  if (priv->header_suffix)
    gtk_box_append (priv->header_box, priv->header_suffix);

  update_header_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HEADER_SUFFIX]);
}

/**
 * adw_preferences_group_get_rows:
 * @self: a preferences group
 *
 * Gets a [iface@Gio.ListModel] that contains the rows of the group.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a list model for the group's rows
 */
GListModel *
adw_preferences_group_get_rows (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;
  GtkCustomFilter *filter;
  GListModel *model;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  filter = gtk_custom_filter_new ((GtkCustomFilterFunc) row_has_title, NULL, NULL);
  model = gtk_widget_observe_children (GTK_WIDGET (priv->listbox));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (filter)));

  return model;
}
