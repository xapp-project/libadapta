/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adap-button-content.h"

#include "adap-split-button.h"

/**
 * AdapButtonContent:
 *
 * A helper widget for creating buttons.
 *
 * <picture>
 *   <source srcset="button-content-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="button-content.png" alt="button-content">
 * </picture>
 *
 * `AdapButtonContent` is a box-like widget with an icon and a label.
 *
 * It's intended to be used as a direct child of [class@Gtk.Button],
 * [class@Gtk.MenuButton] or [class@SplitButton], when they need to have both an
 * icon and a label, as follows:
 *
 * ```xml
 * <object class="GtkButton">
 *   <property name="child">
 *     <object class="AdapButtonContent">
 *       <property name="icon-name">document-open-symbolic</property>
 *       <property name="label" translatable="yes">_Open</property>
 *       <property name="use-underline">True</property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * `AdapButtonContent` handles style classes and connecting the mnemonic to the
 * button automatically.
 *
 * ## CSS nodes
 *
 * ```
 * buttoncontent
 * ╰── box
 *     ├── image
 *     ╰── label
 * ```
 *
 * `AdapButtonContent`'s CSS node is called `buttoncontent`. It contains a `box`
 * subnode that serves as a container for the  `image` and `label` nodes.
 *
 * When inside a `GtkButton` or `AdapSplitButton`, the button will receive the
 * `.image-text-button` style class. When inside a `GtkMenuButton`, the
 * internal `GtkButton` will receive it instead.
 *
 * ## Accessibility
 *
 * `AdapButtonContent` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 */

struct _AdapButtonContent {
  GtkWidget parent_instance;

  GtkWidget *box;
  GtkWidget *icon;
  GtkWidget *label;

  char *icon_name;
  gboolean can_shrink;

  GtkWidget *button;
};

G_DEFINE_FINAL_TYPE (AdapButtonContent, adap_button_content, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_LABEL,
  PROP_USE_UNDERLINE,
  PROP_CAN_SHRINK,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static inline GtkWidget *
find_parent_button (AdapButtonContent *self)
{
  return gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_BUTTON);
}

static void
adap_button_content_root (GtkWidget *widget)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (widget);

  GTK_WIDGET_CLASS (adap_button_content_parent_class)->root (widget);

  gtk_label_set_mnemonic_widget (GTK_LABEL (self->label),
                                 find_parent_button (self));

  self->button = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_BUTTON);

  /* For AdapSplitButton we want to style the split button widget and not the
   * button inside. */
  if (ADAP_IS_SPLIT_BUTTON (gtk_widget_get_parent (self->button)))
    self->button = gtk_widget_get_parent (self->button);

  gtk_widget_add_css_class (self->button, "image-text-button");
}

static void
adap_button_content_unroot (GtkWidget *widget)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (widget);

  gtk_label_set_mnemonic_widget (GTK_LABEL (self->label), NULL);

  if (self->button) {
    gtk_widget_remove_css_class (self->button, "image-text-button");

    self->button = NULL;
  }

  GTK_WIDGET_CLASS (adap_button_content_parent_class)->unroot (widget);
}

static void
adap_button_content_dispose (GObject *object)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (object);

  self->icon = NULL;
  self->label = NULL;
  g_clear_pointer (&self->box, gtk_widget_unparent);

  G_OBJECT_CLASS (adap_button_content_parent_class)->dispose (object);
}

static void
adap_button_content_finalize (GObject *object)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (object);

  g_clear_pointer (&self->icon_name, g_free);

  G_OBJECT_CLASS (adap_button_content_parent_class)->finalize (object);
}

static void
adap_button_content_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_button_content_get_icon_name (self));
    break;
  case PROP_LABEL:
    g_value_set_string (value, adap_button_content_get_label (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adap_button_content_get_use_underline (self));
    break;
  case PROP_CAN_SHRINK:
    g_value_set_boolean (value, adap_button_content_get_can_shrink (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_button_content_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdapButtonContent *self = ADAP_BUTTON_CONTENT (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adap_button_content_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_LABEL:
    adap_button_content_set_label (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    adap_button_content_set_use_underline (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SHRINK:
    adap_button_content_set_can_shrink (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_button_content_class_init (AdapButtonContentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_button_content_dispose;
  object_class->finalize = adap_button_content_finalize;
  object_class->get_property = adap_button_content_get_property;
  object_class->set_property = adap_button_content_set_property;

  widget_class->root = adap_button_content_root;
  widget_class->unroot = adap_button_content_unroot;

  /**
   * AdapButtonContent:icon-name: (attributes org.gtk.Property.get=adap_button_content_get_icon_name org.gtk.Property.set=adap_button_content_set_icon_name)
   *
   * The name of the displayed icon.
   *
   * If empty, the icon is not shown.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapButtonContent:label: (attributes org.gtk.Property.get=adap_button_content_get_label org.gtk.Property.set=adap_button_content_set_label)
   *
   * The displayed label.
   */
  props[PROP_LABEL] =
    g_param_spec_string ("label", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapButtonContent:use-underline: (attributes org.gtk.Property.get=adap_button_content_get_use_underline org.gtk.Property.set=adap_button_content_set_use_underline)
   *
   * Whether an underline in the text indicates a mnemonic.
   *
   * The mnemonic can be used to activate the parent button.
   *
   * See [property@ButtonContent:label].
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapButtonContent:can-shrink: (attributes org.gtk.Property.get=adap_button_content_get_can_shrink org.gtk.Property.set=adap_button_content_set_can_shrink)
   *
   * Whether the button can be smaller than the natural size of its contents.
   *
   * If set to `TRUE`, the label will ellipsize.
   *
   * See [property@Gtk.Button:can-shrink].
   *
   * Since: 1.4
   */
  props[PROP_CAN_SHRINK] =
    g_param_spec_boolean ("can-shrink", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "buttoncontent");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adap_button_content_init (AdapButtonContent *self)
{
  self->icon_name = g_strdup ("");

  gtk_widget_set_hexpand (GTK_WIDGET (self), FALSE);

  self->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign (self->box, GTK_ALIGN_CENTER);

  self->icon = g_object_new (GTK_TYPE_IMAGE,
                             "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                             NULL);
  gtk_image_set_from_icon_name (GTK_IMAGE (self->icon), "image-missing");
  gtk_widget_set_valign (self->icon, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand (self->icon, TRUE);

  self->label = gtk_label_new (NULL);
  gtk_widget_set_hexpand (self->label, TRUE);
  gtk_widget_set_visible (self->label, FALSE);

  gtk_box_append (GTK_BOX (self->box), self->icon);
  gtk_box_append (GTK_BOX (self->box), self->label);
  gtk_widget_set_parent (self->box, GTK_WIDGET (self));
}

/**
 * adap_button_content_new:
 *
 * Creates a new `AdapButtonContent`.
 *
 * Returns: the new created `AdapButtonContent`
 */
GtkWidget *
adap_button_content_new (void)
{
  return g_object_new (ADAP_TYPE_BUTTON_CONTENT, NULL);
}

/**
 * adap_button_content_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a button content
 *
 * Gets the name of the displayed icon.
 *
 * Returns: the icon name
 */
const char *
adap_button_content_get_icon_name (AdapButtonContent *self)
{
  g_return_val_if_fail (ADAP_IS_BUTTON_CONTENT (self), NULL);

  return self->icon_name;
}

/**
 * adap_button_content_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a button content
 * @icon_name: the new icon name
 *
 * Sets the name of the displayed icon.
 *
 * If empty, the icon is not shown.
 */
void
adap_button_content_set_icon_name (AdapButtonContent *self,
                                  const char       *icon_name)
{
  g_return_if_fail (ADAP_IS_BUTTON_CONTENT (self));
  g_return_if_fail (icon_name != NULL);

  if (!g_set_str (&self->icon_name, icon_name))
    return;

  if (!icon_name[0])
    icon_name = "image-missing";

  gtk_image_set_from_icon_name (GTK_IMAGE (self->icon), icon_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adap_button_content_get_label: (attributes org.gtk.Method.get_property=label)
 * @self: a button content
 *
 * Gets the displayed label.
 *
 * Returns: the label
 */
const char *
adap_button_content_get_label (AdapButtonContent *self)
{
  g_return_val_if_fail (ADAP_IS_BUTTON_CONTENT (self), NULL);

  return gtk_label_get_label (GTK_LABEL (self->label));
}

/**
 * adap_button_content_set_label: (attributes org.gtk.Method.set_property=label)
 * @self: a button content
 * @label: the new label
 *
 * Sets the displayed label.
 */
void
adap_button_content_set_label (AdapButtonContent *self,
                              const char       *label)
{
  g_return_if_fail (ADAP_IS_BUTTON_CONTENT (self));
  g_return_if_fail (label != NULL);

  if (!g_strcmp0 (label, adap_button_content_get_label (self)))
    return;

  gtk_label_set_label (GTK_LABEL (self->label), label);

  gtk_widget_set_visible (self->label, label[0]);
  gtk_widget_set_hexpand (self->icon, !label[0]);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
}

/**
 * adap_button_content_get_use_underline: (attributes org.gtk.Method.get_property=use-underline)
 * @self: a button content
 *
 * Gets whether an underline in the text indicates a mnemonic.
 *
 * Returns: whether an underline in the text indicates a mnemonic
 */
gboolean
adap_button_content_get_use_underline (AdapButtonContent *self)
{
  g_return_val_if_fail (ADAP_IS_BUTTON_CONTENT (self), FALSE);

  return gtk_label_get_use_underline (GTK_LABEL (self->label));
}

/**
 * adap_button_content_set_use_underline: (attributes org.gtk.Method.set_property=use-underline)
 * @self: a button content
 * @use_underline: whether an underline in the text indicates a mnemonic
 *
 * Sets whether an underline in the text indicates a mnemonic.
 *
 * The mnemonic can be used to activate the parent button.
 *
 * See [property@ButtonContent:label].
 */
void
adap_button_content_set_use_underline (AdapButtonContent *self,
                                      gboolean          use_underline)
{
  g_return_if_fail (ADAP_IS_BUTTON_CONTENT (self));

  use_underline = !!use_underline;

  if (use_underline == adap_button_content_get_use_underline (self))
    return;

  gtk_label_set_use_underline (GTK_LABEL (self->label), use_underline);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

/**
 * adap_button_content_get_can_shrink: (attributes org.gtk.Method.get_property=can-shrink)
 * @self: a button content
 *
 * gets whether the button can be smaller than the natural size of its contents.
 *
 * Returns: whether the button can shrink
 *
 * Since: 1.4
 */
gboolean
adap_button_content_get_can_shrink (AdapButtonContent *self)
{
  g_return_val_if_fail (ADAP_IS_BUTTON_CONTENT (self), FALSE);

  return gtk_label_get_ellipsize (GTK_LABEL (self->label)) != PANGO_ELLIPSIZE_NONE;
}

/**
 * adap_button_content_set_can_shrink: (attributes org.gtk.Method.set_property=can-shrink)
 * @self: a button content
 * @can_shrink: whether the button can shrink
 *
 * Sets whether the button can be smaller than the natural size of its contents.
 *
 * If set to `TRUE`, the label will ellipsize.
 *
 * See [method@Gtk.Button.set_can_shrink].
 *
 * Since: 1.4
 */
void
adap_button_content_set_can_shrink (AdapButtonContent *self,
                                   gboolean          can_shrink)
{
  g_return_if_fail (ADAP_IS_BUTTON_CONTENT (self));

  can_shrink = !!can_shrink;

  if (can_shrink == adap_button_content_get_can_shrink (self))
    return;

  gtk_label_set_ellipsize (GTK_LABEL (self->label),
                           can_shrink ? PANGO_ELLIPSIZE_END
                                      : PANGO_ELLIPSIZE_NONE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SHRINK]);
}

