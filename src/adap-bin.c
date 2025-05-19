/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adap-bin.h"

#include "adap-widget-utils-private.h"

/**
 * AdapBin:
 *
 * A widget with one child.
 *
 * <picture>
 *   <source srcset="bin-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="bin.png" alt="bin">
 * </picture>
 *
 * The `AdapBin` widget has only one child, set with the [property@Bin:child]
 * property.
 *
 * It is useful for deriving subclasses, since it provides common code needed
 * for handling a single child widget.
 */

typedef struct
{
  GtkWidget *child;
} AdapBinPrivate;

static void adap_bin_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapBin, adap_bin, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdapBin)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_bin_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adap_bin_dispose (GObject *object)
{
  AdapBin *self = ADAP_BIN (object);
  AdapBinPrivate *priv = adap_bin_get_instance_private (self);

  g_clear_pointer (&priv->child, gtk_widget_unparent);

  G_OBJECT_CLASS (adap_bin_parent_class)->dispose (object);
}

static void
adap_bin_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  AdapBin *self = ADAP_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_bin_get_child (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_bin_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  AdapBin *self = ADAP_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_bin_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_bin_class_init (AdapBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_bin_dispose;
  object_class->get_property = adap_bin_get_property;
  object_class->set_property = adap_bin_set_property;

  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->focus = adap_widget_focus_child;

  /**
   * AdapBin:child: (attributes org.gtk.Property.get=adap_bin_get_child org.gtk.Property.set=adap_bin_set_child)
   *
   * The child widget of the `AdapBin`.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
adap_bin_init (AdapBin *self)
{
}

static void
adap_bin_buildable_add_child (GtkBuildable *buildable,
                             GtkBuilder   *builder,
                             GObject      *child,
                             const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_bin_set_child (ADAP_BIN (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_bin_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_bin_buildable_add_child;
}

/**
 * adap_bin_new:
 *
 * Creates a new `AdapBin`.
 *
 * Returns: the new created `AdapBin`
 */
GtkWidget *
adap_bin_new (void)
{
  return g_object_new (ADAP_TYPE_BIN, NULL);
}

/**
 * adap_bin_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a bin
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adap_bin_get_child (AdapBin *self)
{
  AdapBinPrivate *priv;

  g_return_val_if_fail (ADAP_IS_BIN (self), NULL);

  priv = adap_bin_get_instance_private (self);

  return priv->child;
}

/**
 * adap_bin_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a bin
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adap_bin_set_child (AdapBin    *self,
                   GtkWidget *child)
{
  AdapBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BIN (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adap_bin_get_instance_private (self);

  if (priv->child == child)
    return;

  if (priv->child)
    gtk_widget_unparent (priv->child);

  priv->child = child;

  if (priv->child)
    gtk_widget_set_parent (priv->child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}
