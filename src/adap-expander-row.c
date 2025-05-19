/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-expander-row.h"

#include "adap-action-row.h"
#include "adap-widget-utils-private.h"

/**
 * AdapExpanderRow:
 *
 * A [class@Gtk.ListBoxRow] used to reveal widgets.
 *
 * <picture>
 *   <source srcset="expander-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="expander-row.png" alt="expander-row">
 * </picture>
 *
 * The `AdapExpanderRow` widget allows the user to reveal or hide widgets below
 * it. It also allows the user to enable the expansion of the row, allowing to
 * disable all that the row contains.
 *
 * ## AdapExpanderRow as GtkBuildable
 *
 * The `AdapExpanderRow` implementation of the [iface@Gtk.Buildable] interface
 * supports adding a child as an suffix widget by specifying “suffix” as the
 * “type” attribute of a <child> element.
 *
 * It also supports adding it as a prefix widget by specifying “prefix” as the
 * “type” attribute of a <child> element.
 *
 * ## CSS nodes
 *
 * `AdapExpanderRow` has a main CSS node with name `row` and the `.expander`
 * style class. It has the `.empty` style class when it contains no children.
 *
 * It contains the subnodes `row.header` for its main embedded row,
 * `list.nested` for the list it can expand, and `image.expander-row-arrow` for
 * its arrow.
 */

typedef struct
{
  GtkBox *box;
  GtkBox *suffixes;
  GtkBox *prefixes;
  GtkListBox *list;
  AdapActionRow *action_row;
  GtkSwitch *enable_switch;
  GtkImage *image;

  gboolean expanded;
  gboolean enable_expansion;
  gboolean show_enable_switch;
} AdapExpanderRowPrivate;

static void adap_expander_row_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapExpanderRow, adap_expander_row, ADAP_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (AdapExpanderRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adap_expander_row_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SUBTITLE,
  PROP_ICON_NAME,
  PROP_EXPANDED,
  PROP_ENABLE_EXPANSION,
  PROP_SHOW_ENABLE_SWITCH,
  PROP_TITLE_LINES,
  PROP_SUBTITLE_LINES,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
activate_cb (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv = adap_expander_row_get_instance_private (self);

  adap_expander_row_set_expanded (self, !priv->expanded);
}

static gboolean
keynav_failed_cb (AdapExpanderRow   *self,
                  GtkDirectionType  direction)
{
  GtkWidget *toplevel = GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (self)));

  if (!toplevel)
    return FALSE;

  if (direction != GTK_DIR_UP && direction != GTK_DIR_DOWN)
    return FALSE;

  return gtk_widget_child_focus (toplevel, direction == GTK_DIR_UP ?
                                 GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD);
}

static void
adap_expander_row_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdapExpanderRow *self = ADAP_EXPANDER_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    g_value_set_string (value, adap_expander_row_get_subtitle (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_expander_row_get_icon_name (self));
    break;
  case PROP_EXPANDED:
    g_value_set_boolean (value, adap_expander_row_get_expanded (self));
    break;
  case PROP_ENABLE_EXPANSION:
    g_value_set_boolean (value, adap_expander_row_get_enable_expansion (self));
    break;
  case PROP_SHOW_ENABLE_SWITCH:
    g_value_set_boolean (value, adap_expander_row_get_show_enable_switch (self));
    break;
  case PROP_TITLE_LINES:
    g_value_set_int (value, adap_expander_row_get_title_lines (self));
    break;
  case PROP_SUBTITLE_LINES:
    g_value_set_int (value, adap_expander_row_get_subtitle_lines (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_expander_row_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdapExpanderRow *self = ADAP_EXPANDER_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    adap_expander_row_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_ICON_NAME:
    adap_expander_row_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_EXPANDED:
    adap_expander_row_set_expanded (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_EXPANSION:
    adap_expander_row_set_enable_expansion (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_ENABLE_SWITCH:
    adap_expander_row_set_show_enable_switch (self, g_value_get_boolean (value));
    break;
  case PROP_TITLE_LINES:
    adap_expander_row_set_title_lines (self, g_value_get_int (value));
    break;
  case PROP_SUBTITLE_LINES:
    adap_expander_row_set_subtitle_lines (self, g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_expander_row_class_init (AdapExpanderRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_expander_row_get_property;
  object_class->set_property = adap_expander_row_set_property;

  widget_class->focus = adap_widget_focus_child;
  widget_class->grab_focus = adap_widget_grab_focus_child;

  /**
   * AdapExpanderRow:subtitle: (attributes org.gtk.Property.get=adap_expander_row_get_subtitle org.gtk.Property.set=adap_expander_row_set_subtitle)
   *
   * The subtitle for this row.
   *
   * The subtitle is interpreted as Pango markup unless
   * [property@PreferencesRow:use-markup] is set to `FALSE`.
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:icon-name: (attributes org.gtk.Property.get=adap_expander_row_get_icon_name org.gtk.Property.set=adap_expander_row_set_icon_name)
   *
   * The icon name for this row.
   *
   * Deprecated: 1.3: Use [method@ExpanderRow.add_prefix] to add an icon.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:expanded: (attributes org.gtk.Property.get=adap_expander_row_get_expanded org.gtk.Property.set=adap_expander_row_set_expanded)
   *
   * Whether the row is expanded.
   */
  props[PROP_EXPANDED] =
    g_param_spec_boolean ("expanded", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:enable-expansion: (attributes org.gtk.Property.get=adap_expander_row_get_enable_expansion org.gtk.Property.set=adap_expander_row_set_enable_expansion)
   *
   * Whether expansion is enabled.
   */
  props[PROP_ENABLE_EXPANSION] =
    g_param_spec_boolean ("enable-expansion", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:show-enable-switch: (attributes org.gtk.Property.get=adap_expander_row_get_show_enable_switch org.gtk.Property.set=adap_expander_row_set_show_enable_switch)
   *
   * Whether the switch enabling the expansion is visible.
   */
  props[PROP_SHOW_ENABLE_SWITCH] =
    g_param_spec_boolean ("show-enable-switch", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:title-lines: (attributes org.gtk.Property.get=adap_expander_row_get_title_lines org.gtk.Property.set=adap_expander_row_set_title_lines)
   *
   * The number of lines at the end of which the title label will be ellipsized.
   *
   * If the value is 0, the number of lines won't be limited.
   *
   * Since: 1.3
   */
  props[PROP_TITLE_LINES] =
    g_param_spec_int ("title-lines", NULL, NULL,
                      0, G_MAXINT,
                      0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapExpanderRow:subtitle-lines: (attributes org.gtk.Property.get=adap_expander_row_get_subtitle_lines org.gtk.Property.set=adap_expander_row_set_subtitle_lines)
   *
   * The number of lines at the end of which the subtitle label will be
   * ellipsized.
   *
   * If the value is 0, the number of lines won't be limited.
   *
   * Since: 1.3
   */
  props[PROP_SUBTITLE_LINES] =
    g_param_spec_int ("subtitle-lines", NULL, NULL,
                      0, G_MAXINT,
                      0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-expander-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, action_row);
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, box);
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, suffixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, AdapExpanderRow, enable_switch);
  gtk_widget_class_bind_template_callback (widget_class, activate_cb);
  gtk_widget_class_bind_template_callback (widget_class, keynav_failed_cb);
}

#define NOTIFY(func, prop) \
static void \
func (gpointer this) { \
  g_object_notify_by_pspec (G_OBJECT (this), props[prop]); \
} \

NOTIFY (notify_subtitle_cb, PROP_SUBTITLE);
NOTIFY (notify_icon_name_cb, PROP_ICON_NAME);
NOTIFY (notify_title_lines_cb, PROP_TITLE_LINES);
NOTIFY (notify_subtitle_lines_cb, PROP_SUBTITLE_LINES);

static void
adap_expander_row_init (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv = adap_expander_row_get_instance_private (self);

  priv->prefixes = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  adap_expander_row_set_enable_expansion (self, TRUE);
  adap_expander_row_set_expanded (self, FALSE);

  g_signal_connect_object (priv->action_row, "notify::subtitle", G_CALLBACK (notify_subtitle_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->action_row, "notify::icon-name", G_CALLBACK (notify_icon_name_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->action_row, "notify::title-lines", G_CALLBACK (notify_title_lines_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->action_row, "notify::subtitle-lines", G_CALLBACK (notify_subtitle_lines_cb), self, G_CONNECT_SWAPPED);
}

static void
adap_expander_row_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  AdapExpanderRow *self = ADAP_EXPANDER_ROW (buildable);
  AdapExpanderRowPrivate *priv = adap_expander_row_get_instance_private (self);

  if (!priv->box)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (type && strcmp (type, "action") == 0)
    adap_expander_row_add_suffix (self, GTK_WIDGET (child));
  else if (type && strcmp (type, "suffix") == 0)
    adap_expander_row_add_suffix (self, GTK_WIDGET (child));
  else if (type && strcmp (type, "prefix") == 0)
    adap_expander_row_add_prefix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adap_expander_row_add_row (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_expander_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_expander_row_buildable_add_child;
}

/**
 * adap_expander_row_new:
 *
 * Creates a new `AdapExpanderRow`.
 *
 * Returns: the newly created `AdapExpanderRow`
 */
GtkWidget *
adap_expander_row_new (void)
{
  return g_object_new (ADAP_TYPE_EXPANDER_ROW, NULL);
}

/**
 * adap_expander_row_add_action:
 * @self: an expander row
 * @widget: a widget
 *
 * Adds an action widget to @self.
 *
 * Deprecated: 1.4: Use [method@ExpanderRow.add_suffix] to add a suffix.
 */
void
adap_expander_row_add_action (AdapExpanderRow *self,
                             GtkWidget      *widget)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (self));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adap_expander_row_get_instance_private (self);

  gtk_box_prepend (priv->suffixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->suffixes), TRUE);
}

/**
 * adap_expander_row_add_prefix:
 * @self: an expander row
 * @widget: a widget
 *
 * Adds a prefix widget to @self.
 */
void
adap_expander_row_add_prefix (AdapExpanderRow *self,
                             GtkWidget      *widget)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adap_expander_row_get_instance_private (self);

  if (priv->prefixes == NULL) {
    priv->prefixes = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12));
    adap_action_row_add_prefix (ADAP_ACTION_ROW (priv->action_row), GTK_WIDGET (priv->prefixes));
  }
  gtk_box_append (priv->prefixes, widget);
}

/**
 * adap_expander_row_add_suffix:
 * @self: an expander row
 * @widget: a widget
 *
 * Adds an suffix widget to @self.
 *
 * Since: 1.4
 */
void
adap_expander_row_add_suffix (AdapExpanderRow *self,
                             GtkWidget      *widget)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (self));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adap_expander_row_get_instance_private (self);

  gtk_box_prepend (priv->suffixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->suffixes), TRUE);
}

/**
 * adap_expander_row_add_row:
 * @self: an expander row
 * @child: a widget
 *
 * Adds a widget to @self.
 *
 * The widget will appear in the expanding list below @self.
 */
void
adap_expander_row_add_row (AdapExpanderRow *self,
                          GtkWidget      *child)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adap_expander_row_get_instance_private (self);

  /* When constructing the widget, we want the box to be added as the child of
   * the GtkListBoxRow, as an implementation detail.
   */
  gtk_list_box_append (priv->list, child);

  gtk_widget_remove_css_class (GTK_WIDGET (self), "empty");
}

/**
 * adap_expander_row_remove:
 * @self: an expander row
 * @child: the child to be removed
 *
 * Removes a child from @self.
 */
void
adap_expander_row_remove (AdapExpanderRow *self,
                         GtkWidget      *child)
{
  AdapExpanderRowPrivate *priv;
  GtkWidget *parent;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adap_expander_row_get_instance_private (self);

  parent = gtk_widget_get_parent (child);

  if (parent == GTK_WIDGET (priv->prefixes) || parent == GTK_WIDGET (priv->suffixes)) {
    gtk_box_remove (GTK_BOX (parent), child);
    gtk_widget_set_visible (parent, gtk_widget_get_first_child (parent) != NULL);
  }
  else if (parent == GTK_WIDGET (priv->list) ||
           (GTK_IS_WIDGET (parent) && (gtk_widget_get_parent (parent) == GTK_WIDGET (priv->list)))) {
    gtk_list_box_remove (priv->list, child);

    if (!gtk_widget_get_first_child (GTK_WIDGET (priv->list)))
      gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  }
  else {
    ADAP_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
  }
}

/**
 * adap_expander_row_get_subtitle: (attributes org.gtk.Method.get_property=subtitle)
 * @self: an expander row
 *
 * Gets the subtitle for @self.
 *
 * Returns: the subtitle for @self
 */
const char *
adap_expander_row_get_subtitle (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), NULL);

  priv = adap_expander_row_get_instance_private (self);

  return adap_action_row_get_subtitle (priv->action_row);
}

/**
 * adap_expander_row_set_subtitle: (attributes org.gtk.Method.set_property=subtitle)
 * @self: an expander row
 * @subtitle: the subtitle
 *
 * Sets the subtitle for @self.
 *
 * The subtitle is interpreted as Pango markup unless
 * [property@PreferencesRow:use-markup] is set to `FALSE`.
 */
void
adap_expander_row_set_subtitle (AdapExpanderRow *self,
                               const char     *subtitle)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  adap_action_row_set_subtitle (priv->action_row, subtitle);
}

/**
 * adap_expander_row_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: an expander row
 *
 * Gets the icon name for @self.
 *
 * Returns: (nullable): the icon name for @self
 *
 * Deprecated: 1.3: Use [method@ExpanderRow.add_prefix] to add an icon.
 */
const char *
adap_expander_row_get_icon_name (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), NULL);

  priv = adap_expander_row_get_instance_private (self);

  return adap_action_row_get_icon_name (priv->action_row);
}

/**
 * adap_expander_row_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: an expander row
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 *
 * Deprecated: 1.3: Use [method@ExpanderRow.add_prefix] to add an icon.
 */
void
adap_expander_row_set_icon_name (AdapExpanderRow *self,
                                const char     *icon_name)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  adap_action_row_set_icon_name (priv->action_row, icon_name);
}

/**
 * adap_expander_row_get_expanded: (attributes org.gtk.Method.get_property=expanded)
 * @self: an expander row
 *
 * Gets whether @self is expanded.
 *
 * Returns: whether @self is expanded
 */
gboolean
adap_expander_row_get_expanded (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), FALSE);

  priv = adap_expander_row_get_instance_private (self);

  return priv->expanded;
}

/**
 * adap_expander_row_set_expanded: (attributes org.gtk.Method.set_property=expanded)
 * @self: an expander row
 * @expanded: whether to expand the row
 *
 * Sets whether @self is expanded.
 */
void
adap_expander_row_set_expanded (AdapExpanderRow *self,
                               gboolean        expanded)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  expanded = !!expanded && priv->enable_expansion;

  if (priv->expanded == expanded)
    return;

  priv->expanded = expanded;

  if (expanded)
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED, FALSE);
  else
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED);

  gtk_accessible_update_state (GTK_ACCESSIBLE (priv->action_row),
                               GTK_ACCESSIBLE_STATE_EXPANDED, priv->expanded,
                               -1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPANDED]);
}

/**
 * adap_expander_row_get_enable_expansion: (attributes org.gtk.Method.get_property=enable-expansion)
 * @self: an expander row
 *
 * Gets whether the expansion of @self is enabled.
 *
 * Returns: whether the expansion of @self is enabled.
 */
gboolean
adap_expander_row_get_enable_expansion (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), FALSE);

  priv = adap_expander_row_get_instance_private (self);

  return priv->enable_expansion;
}

/**
 * adap_expander_row_set_enable_expansion: (attributes org.gtk.Method.set_property=enable-expansion)
 * @self: an expander row
 * @enable_expansion: whether to enable the expansion
 *
 * Sets whether the expansion of @self is enabled.
 */
void
adap_expander_row_set_enable_expansion (AdapExpanderRow *self,
                                       gboolean        enable_expansion)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  enable_expansion = !!enable_expansion;

  if (priv->enable_expansion == enable_expansion)
    return;

  priv->enable_expansion = enable_expansion;

  adap_expander_row_set_expanded (self, priv->enable_expansion);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_EXPANSION]);
}

/**
 * adap_expander_row_get_show_enable_switch: (attributes org.gtk.Method.get_property=show-enable-switch)
 * @self: an expander row
 *
 * Gets whether the switch enabling the expansion of @self is visible.
 *
 * Returns: whether the switch enabling the expansion is visible
 */
gboolean
adap_expander_row_get_show_enable_switch (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), FALSE);

  priv = adap_expander_row_get_instance_private (self);

  return priv->show_enable_switch;
}

/**
 * adap_expander_row_set_show_enable_switch: (attributes org.gtk.Method.set_property=show-enable-switch)
 * @self: an expander row
 * @show_enable_switch: whether to show the switch enabling the expansion
 *
 * Sets whether the switch enabling the expansion of @self is visible.
 */
void
adap_expander_row_set_show_enable_switch (AdapExpanderRow *self,
                                         gboolean        show_enable_switch)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  show_enable_switch = !!show_enable_switch;

  if (priv->show_enable_switch == show_enable_switch)
    return;

  priv->show_enable_switch = show_enable_switch;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_ENABLE_SWITCH]);
}

/**
 * adap_expander_row_get_title_lines: (attributes org.gtk.Method.get_property=title-lines)
 * @self: an expander row
 *
 * Gets the number of lines at the end of which the title label will be
 * ellipsized.
 *
 * Returns: the number of lines at the end of which the title label will be
 *   ellipsized
 *
 * Since: 1.3
 */
int
adap_expander_row_get_title_lines (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), 0);

  priv = adap_expander_row_get_instance_private (self);

  return adap_action_row_get_title_lines (priv->action_row);
}

/**
 * adap_expander_row_set_title_lines: (attributes org.gtk.Method.set_property=title-lines)
 * @self: an expander row
 * @title_lines: the number of lines at the end of which the title label will be ellipsized
 *
 * Sets the number of lines at the end of which the title label will be
 * ellipsized.
 *
 * If the value is 0, the number of lines won't be limited.
 *
 * Since: 1.3
 */
void
adap_expander_row_set_title_lines (AdapExpanderRow *self,
                                  int             title_lines)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  adap_action_row_set_title_lines (priv->action_row, title_lines);
}

/**
 * adap_expander_row_get_subtitle_lines: (attributes org.gtk.Method.get_property=subtitle-lines)
 * @self: an expander row
 *
 * Gets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 *
 * Returns: the number of lines at the end of which the subtitle label will be
 *   ellipsized
 *
 * Since: 1.3
 */
int
adap_expander_row_get_subtitle_lines (AdapExpanderRow *self)
{
  AdapExpanderRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_EXPANDER_ROW (self), 0);

  priv = adap_expander_row_get_instance_private (self);

  return adap_action_row_get_subtitle_lines (priv->action_row);
}

/**
 * adap_expander_row_set_subtitle_lines: (attributes org.gtk.Method.set_property=subtitle-lines)
 * @self: an expander row
 * @subtitle_lines: the number of lines at the end of which the subtitle label will be ellipsized
 *
 * Sets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 *
 * If the value is 0, the number of lines won't be limited.
 *
 * Since: 1.3
 */
void
adap_expander_row_set_subtitle_lines (AdapExpanderRow *self,
                                     int             subtitle_lines)
{
  AdapExpanderRowPrivate *priv;

  g_return_if_fail (ADAP_IS_EXPANDER_ROW (self));

  priv = adap_expander_row_get_instance_private (self);

  adap_action_row_set_subtitle_lines (priv->action_row, subtitle_lines);
}
