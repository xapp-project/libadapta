/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-clamp.h"

#include "adap-clamp-layout.h"
#include "adap-enums.h"
#include "adap-length-unit.h"
#include "adap-widget-utils-private.h"

/**
 * AdapClamp:
 *
 * A widget constraining its child to a given size.
 *
 * <picture>
 *   <source srcset="clamp-wide-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="clamp-wide.png" alt="clamp-wide">
 * </picture>
 * <picture>
 *   <source srcset="clamp-narrow-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="clamp-narrow.png" alt="clamp-narrow">
 * </picture>
 *
 * The `AdapClamp` widget constrains the size of the widget it contains to a
 * given maximum size. It will constrain the width if it is horizontal, or the
 * height if it is vertical. The expansion of the child from its minimum to its
 * maximum size is eased out for a smooth transition.
 *
 * If the child requires more than the requested maximum size, it will be
 * allocated the minimum size it can fit in instead.
 *
 * `AdapClamp` can scale with the text scale factor, use the
 * [property@Clamp:unit] property to enable that behavior.
 *
 * See also: [class@ClampLayout], [class@ClampScrollable].
 *
 * ## CSS nodes
 *
 * `AdapClamp` has a single CSS node with name `clamp`.
 */

enum {
  PROP_0,
  PROP_CHILD,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,
  PROP_UNIT,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_UNIT + 1,
};

struct _AdapClamp
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

static void adap_clamp_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapClamp, adap_clamp, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_clamp_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
set_orientation (AdapClamp       *self,
                 GtkOrientation  orientation)
{
  GtkLayoutManager *layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  gtk_orientable_set_orientation (GTK_ORIENTABLE (layout), orientation);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adap_clamp_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  AdapClamp *self = ADAP_CLAMP (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_clamp_get_child (self));
    break;
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, adap_clamp_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, adap_clamp_get_tightening_threshold (self));
    break;
  case PROP_UNIT:
    g_value_set_enum (value, adap_clamp_get_unit (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_clamp_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  AdapClamp *self = ADAP_CLAMP (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_clamp_set_child (self, g_value_get_object (value));
    break;
  case PROP_MAXIMUM_SIZE:
    adap_clamp_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    adap_clamp_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_UNIT:
    adap_clamp_set_unit (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_clamp_dispose (GObject *object)
{
  AdapClamp *self = ADAP_CLAMP (object);

  g_clear_pointer (&self->child, gtk_widget_unparent);

  G_OBJECT_CLASS (adap_clamp_parent_class)->dispose (object);
}

static void
adap_clamp_class_init (AdapClampClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_clamp_get_property;
  object_class->set_property = adap_clamp_set_property;
  object_class->dispose = adap_clamp_dispose;

  widget_class->compute_expand = adap_widget_compute_expand;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdapClamp:child: (attributes org.gtk.Property.get=adap_clamp_get_child org.gtk.Property.set=adap_clamp_set_child)
   *
   * The child widget of the `AdapClamp`.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapClamp:maximum-size: (attributes org.gtk.Property.get=adap_clamp_get_maximum_size org.gtk.Property.set=adap_clamp_set_maximum_size)
   *
   * The maximum size allocated to the child.
   *
   * It is the width if the clamp is horizontal, or the height if it is vertical.
   */
  props[PROP_MAXIMUM_SIZE] =
    g_param_spec_int ("maximum-size", NULL, NULL,
                      0, G_MAXINT, 600,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapClamp:tightening-threshold: (attributes org.gtk.Property.get=adap_clamp_get_tightening_threshold org.gtk.Property.set=adap_clamp_set_tightening_threshold)
   *
   * The size above which the child is clamped.
   *
   * Starting from this size, the clamp will tighten its grip on the child,
   * slowly allocating less and less of the available size up to the maximum
   * allocated size. Below that threshold and below the maximum size, the child
   * will be allocated all the available size.
   *
   * If the threshold is greater than the maximum size to allocate to the child,
   * the child will be allocated all the size up to the maximum.
   * If the threshold is lower than the minimum size to allocate to the child,
   * that size will be used as the tightening threshold.
   *
   * Effectively, tightening the grip on the child before it reaches its maximum
   * size makes transitions to and from the maximum size smoother when resizing.
   */
  props[PROP_TIGHTENING_THRESHOLD] =
    g_param_spec_int ("tightening-threshold", NULL, NULL,
                      0, G_MAXINT, 400,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapClamp:unit: (attributes org.gtk.Property.get=adap_clamp_get_unit org.gtk.Property.set=adap_clamp_set_unit)
   *
   * The length unit for maximum size and tightening threshold.
   *
   * Allows the sizes to vary depending on the text scale factor.
   *
   * Since: 1.4
   */
  props[PROP_UNIT] =
    g_param_spec_enum ("unit", NULL, NULL,
                       ADAP_TYPE_LENGTH_UNIT,
                       ADAP_LENGTH_UNIT_SP,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, ADAP_TYPE_CLAMP_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "clamp");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adap_clamp_init (AdapClamp *self)
{
}

static void
adap_clamp_buildable_add_child (GtkBuildable *buildable,
                               GtkBuilder   *builder,
                               GObject      *child,
                               const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_clamp_set_child (ADAP_CLAMP (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_clamp_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_clamp_buildable_add_child;
}

/**
 * adap_clamp_new:
 *
 * Creates a new `AdapClamp`.
 *
 * Returns: the newly created `AdapClamp`
 */
GtkWidget *
adap_clamp_new (void)
{
  return g_object_new (ADAP_TYPE_CLAMP, NULL);
}

/**
 * adap_clamp_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a clamp
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adap_clamp_get_child (AdapClamp  *self)
{
  g_return_val_if_fail (ADAP_IS_CLAMP (self), NULL);

  return self->child;
}

/**
 * adap_clamp_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a clamp
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adap_clamp_set_child (AdapClamp  *self,
                     GtkWidget *child)
{
  g_return_if_fail (ADAP_IS_CLAMP (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  g_clear_pointer (&self->child, gtk_widget_unparent);

  self->child = child;

  if (child)
    gtk_widget_set_parent (child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adap_clamp_get_maximum_size: (attributes org.gtk.Method.get_property=maximum-size)
 * @self: a clamp
 *
 * Gets the maximum size allocated to the child.
 *
 * Returns: the maximum size to allocate to the child
 */
int
adap_clamp_get_maximum_size (AdapClamp *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP (self), 0);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_maximum_size (layout);
}

/**
 * adap_clamp_set_maximum_size: (attributes org.gtk.Method.set_property=maximum-size)
 * @self: a clamp
 * @maximum_size: the maximum size
 *
 * Sets the maximum size allocated to the child.
 *
 * It is the width if the clamp is horizontal, or the height if it is vertical.
 */
void
adap_clamp_set_maximum_size (AdapClamp *self,
                            int       maximum_size)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP (self));

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_maximum_size (layout) == maximum_size)
    return;

  adap_clamp_layout_set_maximum_size (layout, maximum_size);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * adap_clamp_get_tightening_threshold: (attributes org.gtk.Method.get_property=tightening-threshold)
 * @self: a clamp
 *
 * Gets the size above which the child is clamped.
 *
 * Returns: the size above which the child is clamped
 */
int
adap_clamp_get_tightening_threshold (AdapClamp *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP (self), 0);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_tightening_threshold (layout);
}

/**
 * adap_clamp_set_tightening_threshold: (attributes org.gtk.Method.set_property=tightening-threshold)
 * @self: a clamp
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size above which the child is clamped.
 *
 * Starting from this size, the clamp will tighten its grip on the child, slowly
 * allocating less and less of the available size up to the maximum allocated
 * size. Below that threshold and below the maximum size, the child will be
 * allocated all the available size.
 *
 * If the threshold is greater than the maximum size to allocate to the child,
 * the child will be allocated all the size up to the maximum. If the threshold
 * is lower than the minimum size to allocate to the child, that size will be
 * used as the tightening threshold.
 *
 * Effectively, tightening the grip on the child before it reaches its maximum
 * size makes transitions to and from the maximum size smoother when resizing.
 */
void
adap_clamp_set_tightening_threshold (AdapClamp *self,
                                    int       tightening_threshold)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP (self));

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_tightening_threshold (layout) == tightening_threshold)
    return;

  adap_clamp_layout_set_tightening_threshold (layout, tightening_threshold);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}

/**
 * adap_clamp_get_unit: (attributes org.gtk.Method.get_property=unit)
 * @self: a clamp
 *
 * Gets the length unit for maximum size and tightening threshold.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdapLengthUnit
adap_clamp_get_unit (AdapClamp *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP (self), ADAP_LENGTH_UNIT_PX);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_unit (layout);
}

/**
 * adap_clamp_set_unit: (attributes org.gtk.Method.set_property=unit)
 * @self: a clamp
 * @unit: the length unit
 *
 * Sets the length unit for maximum size and tightening threshold.
 *
 * Allows the sizes to vary depending on the text scale factor.
 *
 * Since: 1.4
 */
void
adap_clamp_set_unit (AdapClamp      *self,
                    AdapLengthUnit  unit)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP (self));
  g_return_if_fail (unit >= ADAP_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADAP_LENGTH_UNIT_SP);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_unit (layout) == unit)
    return;

  adap_clamp_layout_set_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UNIT]);
}
