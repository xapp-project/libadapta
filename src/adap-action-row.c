/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-action-row-private.h"

#include "adap-marshalers.h"
#include "adap-widget-utils-private.h"

/**
 * AdapActionRow:
 *
 * A [class@Gtk.ListBoxRow] used to present actions.
 *
 * <picture>
 *   <source srcset="action-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="action-row.png" alt="action-row">
 * </picture>
 *
 * The `AdapActionRow` widget can have a title, a subtitle and an icon. The row
 * can receive additional widgets at its end, or prefix widgets at its start.
 *
 * It is convenient to present a preference and its related actions.
 *
 * `AdapActionRow` is unactivatable by default, giving it an activatable widget
 * will automatically make it activatable, but unsetting it won't change the
 * row's activatability.
 *
 * ## AdapActionRow as GtkBuildable
 *
 * The `AdapActionRow` implementation of the [iface@Gtk.Buildable] interface
 * supports adding a child at its end by specifying “suffix” or omitting the
 * “type” attribute of a <child> element.
 *
 * It also supports adding a child as a prefix widget by specifying “prefix” as
 * the “type” attribute of a <child> element.
 *
 * ## CSS nodes
 *
 * `AdapActionRow` has a main CSS node with name `row`.
 *
 * It contains the subnode `box.header` for its main horizontal box, and
 * `box.title` for the vertical box containing the title and subtitle labels.
 *
 * It contains subnodes `label.title` and `label.subtitle` representing
 * respectively the title label and subtitle label.
 *
 * `AdapActionRow` can use the
 * [`.property`](style-classes.html#property-rows) style class to emphasize
 * the row subtitle instead of the row title, which is useful for
 * displaying read-only properties.
 */

typedef struct
{
  GtkWidget *header;
  GtkImage *image;
  GtkBox *prefixes;
  GtkLabel *subtitle;
  GtkBox *suffixes;
  GtkLabel *title;
  GtkBox *title_box;

  GtkWidget *previous_parent;

  int title_lines;
  int subtitle_lines;
  gboolean subtitle_selectable;
  GtkWidget *activatable_widget;
  GBinding *activatable_binding;
} AdapActionRowPrivate;

static void adap_action_row_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapActionRow, adap_action_row, ADAP_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (AdapActionRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adap_action_row_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SUBTITLE,
  PROP_ICON_NAME,
  PROP_ACTIVATABLE_WIDGET,
  PROP_TITLE_LINES,
  PROP_SUBTITLE_LINES,
  PROP_SUBTITLE_SELECTABLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
string_is_not_empty (AdapActionRow *self,
                     const char   *string)
{
  return string && string[0];
}

static void
pressed_cb (AdapActionRow *self,
            int           n_press,
            double        x,
            double        y,
            GtkGesture   *gesture)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);
  GtkWidget *picked;
  GtkEditable *delegate;

  picked = gtk_widget_pick (GTK_WIDGET (self), x, y, GTK_PICK_DEFAULT);

  if (picked != GTK_WIDGET (self) &&
      picked != priv->header &&
      picked != GTK_WIDGET (priv->prefixes) &&
      picked != GTK_WIDGET (priv->suffixes)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  if (!GTK_IS_EDITABLE (self)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  delegate = gtk_editable_get_delegate (GTK_EDITABLE (self));

  while (delegate && !GTK_IS_TEXT (delegate))
    delegate = gtk_editable_get_delegate (delegate);

  if (!delegate || !GTK_IS_TEXT (delegate)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gtk_text_grab_focus_without_selecting (GTK_TEXT (delegate));
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
row_activated_cb (AdapActionRow  *self,
                  GtkListBoxRow *row)
{
  /* No need to use GTK_LIST_BOX_ROW() for a pointer comparison. */
  if ((GtkListBoxRow *) self == row)
    adap_action_row_activate (self);
}

static void
parent_cb (AdapActionRow *self)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  if (parent == NULL || !GTK_IS_LIST_BOX (parent))
    return;

  priv->previous_parent = parent;
  g_signal_connect_swapped (parent, "row-activated", G_CALLBACK (row_activated_cb), self);
}

static void
adap_action_row_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdapActionRow *self = ADAP_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    g_value_set_string (value, adap_action_row_get_subtitle (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_action_row_get_icon_name (self));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    g_value_set_object (value, (GObject *) adap_action_row_get_activatable_widget (self));
    break;
  case PROP_SUBTITLE_LINES:
    g_value_set_int (value, adap_action_row_get_subtitle_lines (self));
    break;
  case PROP_TITLE_LINES:
    g_value_set_int (value, adap_action_row_get_title_lines (self));
    break;
  case PROP_SUBTITLE_SELECTABLE:
    g_value_set_boolean (value, adap_action_row_get_subtitle_selectable (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_action_row_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdapActionRow *self = ADAP_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    adap_action_row_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_ICON_NAME:
    adap_action_row_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    adap_action_row_set_activatable_widget (self, (GtkWidget*) g_value_get_object (value));
    break;
  case PROP_SUBTITLE_LINES:
    adap_action_row_set_subtitle_lines (self, g_value_get_int (value));
    break;
  case PROP_TITLE_LINES:
    adap_action_row_set_title_lines (self, g_value_get_int (value));
    break;
  case PROP_SUBTITLE_SELECTABLE:
    adap_action_row_set_subtitle_selectable (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_action_row_dispose (GObject *object)
{
  AdapActionRow *self = ADAP_ACTION_ROW (object);
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  adap_action_row_set_activatable_widget (self, NULL);

  G_OBJECT_CLASS (adap_action_row_parent_class)->dispose (object);
}

static void
adap_action_row_activate_real (AdapActionRow *self)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  if (priv->activatable_widget)
    gtk_widget_mnemonic_activate (priv->activatable_widget, FALSE);

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
adap_action_row_class_init (AdapActionRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_action_row_get_property;
  object_class->set_property = adap_action_row_set_property;
  object_class->dispose = adap_action_row_dispose;

  klass->activate = adap_action_row_activate_real;

  /**
   * AdapActionRow:subtitle: (attributes org.gtk.Property.get=adap_action_row_get_subtitle org.gtk.Property.set=adap_action_row_set_subtitle)
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
   * AdapActionRow:icon-name: (attributes org.gtk.Property.get=adap_action_row_get_icon_name org.gtk.Property.set=adap_action_row_set_icon_name)
   *
   * The icon name for this row.
   *
   * Deprecated: 1.3: Use [method@ActionRow.add_prefix] to add an icon.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdapActionRow:activatable-widget: (attributes org.gtk.Property.get=adap_action_row_get_activatable_widget org.gtk.Property.set=adap_action_row_set_activatable_widget)
   *
   * The widget to activate when the row is activated.
   *
   * The row can be activated either by clicking on it, calling
   * [method@ActionRow.activate], or via mnemonics in the title.
   * See the [property@PreferencesRow:use-underline] property to enable
   * mnemonics.
   *
   * The target widget will be activated by emitting the
   * [signal@Gtk.Widget::mnemonic-activate] signal on it.
   */
  props[PROP_ACTIVATABLE_WIDGET] =
    g_param_spec_object ("activatable-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapActionRow:title-lines: (attributes org.gtk.Property.get=adap_action_row_get_title_lines org.gtk.Property.set=adap_action_row_set_title_lines)
   *
   * The number of lines at the end of which the title label will be ellipsized.
   *
   * If the value is 0, the number of lines won't be limited.
   */
  props[PROP_TITLE_LINES] =
    g_param_spec_int ("title-lines", NULL, NULL,
                      0, G_MAXINT,
                      0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapActionRow:subtitle-lines: (attributes org.gtk.Property.get=adap_action_row_get_subtitle_lines org.gtk.Property.set=adap_action_row_set_subtitle_lines)
   *
   * The number of lines at the end of which the subtitle label will be
   * ellipsized.
   *
   * If the value is 0, the number of lines won't be limited.
   */
  props[PROP_SUBTITLE_LINES] =
    g_param_spec_int ("subtitle-lines", NULL, NULL,
                      0, G_MAXINT,
                      0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapActionRow:subtitle-selectable: (attributes org.gtk.Property.get=adap_action_row_get_subtitle_selectable org.gtk.Property.set=adap_action_row_set_subtitle_selectable)
   *
   * Whether the user can copy the subtitle from the label.
   *
   * See also [property@Gtk.Label:selectable].
   *
   * Since: 1.3
   */
  props[PROP_SUBTITLE_SELECTABLE] =
    g_param_spec_boolean ("subtitle-selectable", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdapActionRow::activated:
   *
   * This signal is emitted after the row has been activated.
   */
  signals[SIGNAL_ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-action-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, header);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, prefixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, subtitle);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, suffixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, title);
  gtk_widget_class_bind_template_child_private (widget_class, AdapActionRow, title_box);
  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
  gtk_widget_class_bind_template_callback (widget_class, pressed_cb);
}

static void
adap_action_row_init (AdapActionRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self, "notify::parent", G_CALLBACK (parent_cb), NULL);
}

static void
adap_action_row_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const char   *type)
{
  AdapActionRow *self = ADAP_ACTION_ROW (buildable);
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  if (!priv->header)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (g_strcmp0 (type, "prefix") == 0)
    adap_action_row_add_prefix (self, GTK_WIDGET (child));
  else if (g_strcmp0 (type, "suffix") == 0)
    adap_action_row_add_suffix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adap_action_row_add_suffix (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_action_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adap_action_row_buildable_add_child;
}

/**
 * adap_action_row_new:
 *
 * Creates a new `AdapActionRow`.
 *
 * Returns: the newly created `AdapActionRow`
 */
GtkWidget *
adap_action_row_new (void)
{
  return g_object_new (ADAP_TYPE_ACTION_ROW, NULL);
}

/**
 * adap_action_row_add_prefix:
 * @self: an action row
 * @widget: a widget
 *
 * Adds a prefix widget to @self.
 */
void
adap_action_row_add_prefix (AdapActionRow *self,
                           GtkWidget    *widget)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adap_action_row_get_instance_private (self);

  gtk_box_prepend (priv->prefixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->prefixes), TRUE);
}

/**
 * adap_action_row_add_suffix:
 * @self: an action row
 * @widget: a widget
 *
 * Adds a suffix widget to @self.
 */
void
adap_action_row_add_suffix (AdapActionRow *self,
                           GtkWidget    *widget)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adap_action_row_get_instance_private (self);

  gtk_box_append (priv->suffixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->suffixes), TRUE);
}

/**
 * adap_action_row_remove:
 * @self: an action row
 * @widget: the child to be removed
 *
 * Removes a child from @self.
 */
void
adap_action_row_remove (AdapActionRow *self,
                       GtkWidget    *child)
{
  AdapActionRowPrivate *priv;
  GtkWidget *parent;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adap_action_row_get_instance_private (self);

  parent = gtk_widget_get_parent (child);

  if (parent == GTK_WIDGET (priv->prefixes) || parent == GTK_WIDGET (priv->suffixes)) {
    gtk_box_remove (GTK_BOX (parent), child);
    gtk_widget_set_visible (parent, gtk_widget_get_first_child (parent) != NULL);
  }
  else {
    ADAP_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
  }
}

/**
 * adap_action_row_get_subtitle: (attributes org.gtk.Method.get_property=subtitle)
 * @self: an action row
 *
 * Gets the subtitle for @self.
 *
 * Returns: (nullable): the subtitle for @self
 */
const char *
adap_action_row_get_subtitle (AdapActionRow *self)
{
  AdapActionRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), NULL);

  priv = adap_action_row_get_instance_private (self);

  return gtk_label_get_text (priv->subtitle);
}

/**
 * adap_action_row_set_subtitle: (attributes org.gtk.Method.set_property=subtitle)
 * @self: an action row
 * @subtitle: the subtitle
 *
 * Sets the subtitle for @self.
 *
 * The subtitle is interpreted as Pango markup unless
 * [property@PreferencesRow:use-markup] is set to `FALSE`.
 */
void
adap_action_row_set_subtitle (AdapActionRow *self,
                             const char   *subtitle)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));

  priv = adap_action_row_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_text (priv->subtitle), subtitle) == 0)
    return;

  gtk_label_set_label (priv->subtitle, subtitle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * adap_action_row_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: an action row
 *
 * Gets the icon name for @self.
 *
 * Returns: (nullable): the icon name for @self
 *
 * Deprecated: 1.3: Use [method@ActionRow.add_prefix] to add an icon.
 */
const char *
adap_action_row_get_icon_name (AdapActionRow *self)
{
  AdapActionRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), NULL);

  priv = adap_action_row_get_instance_private (self);

  return gtk_image_get_icon_name (priv->image);
}

/**
 * adap_action_row_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: an action row
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 *
 * Deprecated: 1.3: Use [method@ActionRow.add_prefix] to add an icon.
 */
void
adap_action_row_set_icon_name (AdapActionRow *self,
                              const char   *icon_name)
{
  AdapActionRowPrivate *priv;
  const char *old_icon_name;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));

  priv = adap_action_row_get_instance_private (self);

  old_icon_name = gtk_image_get_icon_name (priv->image);
  if (g_strcmp0 (old_icon_name, icon_name) == 0)
    return;

  gtk_image_set_from_icon_name (priv->image, icon_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adap_action_row_get_activatable_widget: (attributes org.gtk.Method.get_property=activatable-widget)
 * @self: an action row
 *
 * Gets the widget activated when @self is activated.
 *
 * Returns: (nullable) (transfer none): the activatable widget for @self
 */
GtkWidget *
adap_action_row_get_activatable_widget (AdapActionRow *self)
{
  AdapActionRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), NULL);

  priv = adap_action_row_get_instance_private (self);

  return priv->activatable_widget;
}

static void
activatable_widget_weak_notify (gpointer  data,
                                GObject  *where_the_object_was)
{
  AdapActionRow *self = ADAP_ACTION_ROW (data);
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  priv->activatable_widget = NULL;
  priv->activatable_binding = NULL;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * adap_action_row_set_activatable_widget: (attributes org.gtk.Method.set_property=activatable-widget)
 * @self: an action row
 * @widget: (nullable): the target widget
 *
 * Sets the widget to activate when @self is activated.
 *
 * The row can be activated either by clicking on it, calling
 * [method@ActionRow.activate], or via mnemonics in the title.
 * See the [property@PreferencesRow:use-underline] property to enable mnemonics.
 *
 * The target widget will be activated by emitting the
 * [signal@Gtk.Widget::mnemonic-activate] signal on it.
 */
void
adap_action_row_set_activatable_widget (AdapActionRow *self,
                                       GtkWidget    *widget)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  priv = adap_action_row_get_instance_private (self);

  if (priv->activatable_widget == widget)
    return;

  g_clear_pointer (&priv->activatable_binding, g_binding_unbind);

  if (priv->activatable_widget) {
    gtk_accessible_reset_relation (GTK_ACCESSIBLE (priv->activatable_widget),
                                   GTK_ACCESSIBLE_RELATION_LABELLED_BY);
    gtk_accessible_reset_relation (GTK_ACCESSIBLE (priv->activatable_widget),
                                   GTK_ACCESSIBLE_RELATION_DESCRIBED_BY);

    g_object_weak_unref (G_OBJECT (priv->activatable_widget),
                         activatable_widget_weak_notify,
                         self);
  }

  priv->activatable_widget = widget;

  if (priv->activatable_widget != NULL) {
    g_object_weak_ref (G_OBJECT (priv->activatable_widget),
                       activatable_widget_weak_notify,
                       self);

    priv->activatable_binding =
      g_object_bind_property (widget, "sensitive",
                              self, "activatable",
                              G_BINDING_SYNC_CREATE);

    gtk_accessible_update_relation (GTK_ACCESSIBLE (priv->activatable_widget),
                                    GTK_ACCESSIBLE_RELATION_LABELLED_BY, priv->title, NULL,
                                    GTK_ACCESSIBLE_RELATION_DESCRIBED_BY, priv->subtitle, NULL,
                                    -1);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * adap_action_row_get_title_lines: (attributes org.gtk.Method.get_property=title-lines)
 * @self: an action row
 *
 * Gets the number of lines at the end of which the title label will be
 * ellipsized.
 *
 * Returns: the number of lines at the end of which the title label will be
 *   ellipsized
 */
int
adap_action_row_get_title_lines (AdapActionRow *self)
{
  AdapActionRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), 0);

  priv = adap_action_row_get_instance_private (self);

  return priv->title_lines;
}

/**
 * adap_action_row_set_title_lines: (attributes org.gtk.Method.set_property=title-lines)
 * @self: an action row
 * @title_lines: the number of lines at the end of which the title label will be ellipsized
 *
 * Sets the number of lines at the end of which the title label will be
 * ellipsized.
 *
 * If the value is 0, the number of lines won't be limited.
 */
void
adap_action_row_set_title_lines (AdapActionRow *self,
                                int           title_lines)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (title_lines >= 0);

  priv = adap_action_row_get_instance_private (self);

  if (priv->title_lines == title_lines)
    return;

  priv->title_lines = title_lines;

  gtk_label_set_lines (priv->title, title_lines);
  gtk_label_set_ellipsize (priv->title, title_lines == 0 ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE_LINES]);
}

/**
 * adap_action_row_get_subtitle_lines: (attributes org.gtk.Method.get_property=subtitle-lines)
 * @self: an action row
 *
 * Gets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 *
 * Returns: the number of lines at the end of which the subtitle label will be
 *   ellipsized
 */
int
adap_action_row_get_subtitle_lines (AdapActionRow *self)
{
  AdapActionRowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), 0);

  priv = adap_action_row_get_instance_private (self);

  return priv->subtitle_lines;
}

/**
 * adap_action_row_set_subtitle_lines: (attributes org.gtk.Method.set_property=subtitle-lines)
 * @self: an action row
 * @subtitle_lines: the number of lines at the end of which the subtitle label will be ellipsized
 *
 * Sets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 *
 * If the value is 0, the number of lines won't be limited.
 */
void
adap_action_row_set_subtitle_lines (AdapActionRow *self,
                                   int           subtitle_lines)
{
  AdapActionRowPrivate *priv;

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));
  g_return_if_fail (subtitle_lines >= 0);

  priv = adap_action_row_get_instance_private (self);

  if (priv->subtitle_lines == subtitle_lines)
    return;

  priv->subtitle_lines = subtitle_lines;

  gtk_label_set_lines (priv->subtitle, subtitle_lines);
  gtk_label_set_ellipsize (priv->subtitle, subtitle_lines == 0 ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE_LINES]);
}

/**
 * adap_action_row_get_subtitle_selectable: (attributes org.gtk.Method.get_property=subtitle-selectable)
 * @self: a `AdapActionRow`
 *
 * Gets whether the user can copy the subtitle from the label
 *
 * Returns: whether the user can copy the subtitle from the label
 *
 * Since: 1.3
 */
gboolean
adap_action_row_get_subtitle_selectable (AdapActionRow *self)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  g_return_val_if_fail (ADAP_IS_ACTION_ROW (self), FALSE);

  return priv->subtitle_selectable;
}

/**
 * adap_action_row_set_subtitle_selectable: (attributes org.gtk.Method.set_property=subtitle-selectable)
 * @self: a `AdapActionRow`
 * @subtitle_selectable: `TRUE` if the user can copy the subtitle from the label
 *
 * Sets whether the user can copy the subtitle from the label
 *
 * See also [property@Gtk.Label:selectable].
 *
 * Since: 1.3
 */
void
adap_action_row_set_subtitle_selectable (AdapActionRow *self,
                                        gboolean      subtitle_selectable)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));

  subtitle_selectable = !!subtitle_selectable;

  if (priv->subtitle_selectable == subtitle_selectable)
    return;

  priv->subtitle_selectable = subtitle_selectable;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE_SELECTABLE]);
}

/**
 * adap_action_row_activate:
 * @self: an action row
 *
 * Activates @self.
 */
void
adap_action_row_activate (AdapActionRow *self)
{
  g_return_if_fail (ADAP_IS_ACTION_ROW (self));

  ADAP_ACTION_ROW_GET_CLASS (self)->activate (self);
}

void
adap_action_row_set_expand_suffixes (AdapActionRow *self,
                                    gboolean      expand)
{
  AdapActionRowPrivate *priv = adap_action_row_get_instance_private (self);

  g_return_if_fail (ADAP_IS_ACTION_ROW (self));

  expand = !!expand;

  if (expand) {
    gtk_widget_set_hexpand (GTK_WIDGET (priv->title_box), FALSE);
    gtk_label_set_natural_wrap_mode (priv->title, GTK_NATURAL_WRAP_NONE);
    gtk_label_set_natural_wrap_mode (priv->subtitle, GTK_NATURAL_WRAP_NONE);
  } else {
    gtk_widget_set_hexpand (GTK_WIDGET (priv->title_box), TRUE);
    gtk_label_set_natural_wrap_mode (priv->title, GTK_NATURAL_WRAP_INHERIT);
    gtk_label_set_natural_wrap_mode (priv->subtitle, GTK_NATURAL_WRAP_INHERIT);
  }
}
