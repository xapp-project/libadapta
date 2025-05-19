/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-clamp-scrollable.h"

#include "adap-clamp-layout.h"
#include "adap-enums.h"
#include "adap-length-unit.h"
#include "adap-widget-utils-private.h"

/**
 * AdapClampScrollable:
 *
 * A scrollable [class@Clamp].
 *
 * `AdapClampScrollable` is a variant of [class@Clamp] that implements the
 * [iface@Gtk.Scrollable] interface.
 *
 * The primary use case for `AdapClampScrollable` is clamping
 * [class@Gtk.ListView].
 *
 * See also: [class@ClampLayout].
 */

enum {
  PROP_0,
  PROP_CHILD,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,
  PROP_UNIT,

  /* Overridden properties */
  PROP_ORIENTATION,
  PROP_HADJUSTMENT,
  PROP_VADJUSTMENT,
  PROP_HSCROLL_POLICY,
  PROP_VSCROLL_POLICY,

  LAST_PROP = PROP_UNIT + 1,
};

struct _AdapClampScrollable
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkOrientation orientation;

  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;
  GtkScrollablePolicy hscroll_policy;
  GtkScrollablePolicy vscroll_policy;

  GBinding *hadjustment_binding;
  GBinding *vadjustment_binding;
  GBinding *hscroll_policy_binding;
  GBinding *vscroll_policy_binding;
};

static GParamSpec *props[LAST_PROP];

static void adap_clamp_scrollable_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapClampScrollable, adap_clamp_scrollable, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_clamp_scrollable_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
set_orientation (AdapClampScrollable *self,
                 GtkOrientation      orientation)
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
set_hadjustment (AdapClampScrollable *self,
                 GtkAdjustment      *adjustment)
{
  if (self->hadjustment == adjustment)
    return;

  self->hadjustment = adjustment;

  g_object_notify (G_OBJECT (self), "hadjustment");
}

static void
set_vadjustment (AdapClampScrollable *self,
                 GtkAdjustment      *adjustment)
{
  if (self->vadjustment == adjustment)
    return;

  self->vadjustment = adjustment;

  g_object_notify (G_OBJECT (self), "vadjustment");
}

static void
set_hscroll_policy (AdapClampScrollable  *self,
                    GtkScrollablePolicy  policy)
{
  if (self->hscroll_policy == policy)
    return;

  self->hscroll_policy = policy;

  g_object_notify (G_OBJECT (self), "hscroll-policy");
}

static void
set_vscroll_policy (AdapClampScrollable  *self,
                    GtkScrollablePolicy  policy)
{
  if (self->vscroll_policy == policy)
    return;

  self->vscroll_policy = policy;

  g_object_notify (G_OBJECT (self), "vscroll-policy");
}

static void
adap_clamp_scrollable_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdapClampScrollable *self = ADAP_CLAMP_SCROLLABLE (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_clamp_scrollable_get_child (self));
    break;
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, adap_clamp_scrollable_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, adap_clamp_scrollable_get_tightening_threshold (self));
    break;
  case PROP_UNIT:
    g_value_set_enum (value, adap_clamp_scrollable_get_unit (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  case PROP_HADJUSTMENT:
    g_value_set_object (value, self->hadjustment);
    break;
  case PROP_VADJUSTMENT:
    g_value_set_object (value, self->vadjustment);
    break;
  case PROP_HSCROLL_POLICY:
    g_value_set_enum (value, self->hscroll_policy);
    break;
  case PROP_VSCROLL_POLICY:
    g_value_set_enum (value, self->vscroll_policy);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_clamp_scrollable_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdapClampScrollable *self = ADAP_CLAMP_SCROLLABLE (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_clamp_scrollable_set_child (self, g_value_get_object (value));
    break;
  case PROP_MAXIMUM_SIZE:
    adap_clamp_scrollable_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    adap_clamp_scrollable_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_UNIT:
    adap_clamp_scrollable_set_unit (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  case PROP_HADJUSTMENT:
    set_hadjustment (self, g_value_get_object (value));
    break;
  case PROP_VADJUSTMENT:
    set_vadjustment (self, g_value_get_object (value));
    break;
  case PROP_HSCROLL_POLICY:
    set_hscroll_policy (self, g_value_get_enum (value));
    break;
  case PROP_VSCROLL_POLICY:
    set_vscroll_policy (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_clamp_scrollable_dispose (GObject *object)
{
  AdapClampScrollable *self = ADAP_CLAMP_SCROLLABLE (object);

  adap_clamp_scrollable_set_child (self, NULL);

  G_OBJECT_CLASS (adap_clamp_scrollable_parent_class)->dispose (object);
}

static void
adap_clamp_scrollable_class_init (AdapClampScrollableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_clamp_scrollable_get_property;
  object_class->set_property = adap_clamp_scrollable_set_property;
  object_class->dispose = adap_clamp_scrollable_dispose;

  widget_class->compute_expand = adap_widget_compute_expand;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_override_property (object_class,
                                    PROP_HADJUSTMENT,
                                    "hadjustment");

  g_object_class_override_property (object_class,
                                    PROP_VADJUSTMENT,
                                    "vadjustment");

  g_object_class_override_property (object_class,
                                    PROP_HSCROLL_POLICY,
                                    "hscroll-policy");

  g_object_class_override_property (object_class,
                                    PROP_VSCROLL_POLICY,
                                    "vscroll-policy");

  /**
   * AdapClampScrollable:child: (attributes org.gtk.Property.get=adap_clamp_scrollable_get_child org.gtk.Property.set=adap_clamp_scrollable_set_child)
   *
   * The child widget of the `AdapClampScrollable`.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapClampScrollable:maximum-size: (attributes org.gtk.Property.get=adap_clamp_scrollable_get_maximum_size org.gtk.Property.set=adap_clamp_scrollable_set_maximum_size)
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
   * AdapClampScrollable:tightening-threshold: (attributes org.gtk.Property.get=adap_clamp_scrollable_get_tightening_threshold org.gtk.Property.set=adap_clamp_scrollable_set_tightening_threshold)
   *
   * The size above which the child is clamped.
   *
   * Starting from this size, the clamp will tighten its grip on the child,
   * slowly allocating less and less of the available size up to the maximum
   * allocated size. Below that threshold and below the maximum width, the child
   * will be allocated all the available size.
   *
   * If the threshold is greater than the maximum size to allocate to the child,
   * the child will be allocated all the width up to the maximum.
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
   * AdapClampScrollable:unit: (attributes org.gtk.Property.get=adap_clamp_scrollable_get_unit org.gtk.Property.set=adap_clamp_scrollable_set_unit)
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
}

static void
adap_clamp_scrollable_init (AdapClampScrollable *self)
{
}

static void
adap_clamp_scrollable_buildable_add_child (GtkBuildable *buildable,
                                          GtkBuilder   *builder,
                                          GObject      *child,
                                          const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_clamp_scrollable_set_child (ADAP_CLAMP_SCROLLABLE (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_clamp_scrollable_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_clamp_scrollable_buildable_add_child;
}

/**
 * adap_clamp_scrollable_new:
 *
 * Creates a new `AdapClampScrollable`.
 *
 * Returns: the newly created `AdapClampScrollable`
 */
GtkWidget *
adap_clamp_scrollable_new (void)
{
  return g_object_new (ADAP_TYPE_CLAMP_SCROLLABLE, NULL);
}

/**
 * adap_clamp_scrollable_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a clamp scrollable
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adap_clamp_scrollable_get_child (AdapClampScrollable *self)
{
  g_return_val_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self), NULL);

  return self->child;
}

/**
 * adap_clamp_scrollable_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a clamp scrollable
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adap_clamp_scrollable_set_child (AdapClampScrollable *self,
                                GtkWidget          *child)
{
  g_return_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  if (self->child) {
    g_clear_pointer (&self->hadjustment_binding, g_binding_unbind);
    g_clear_pointer (&self->vadjustment_binding, g_binding_unbind);
    g_clear_pointer (&self->hscroll_policy_binding, g_binding_unbind);
    g_clear_pointer (&self->vscroll_policy_binding, g_binding_unbind);

    gtk_widget_unparent (self->child);
  }

  self->child = child;

  if (child) {
    gtk_widget_set_parent (child, GTK_WIDGET (self));

    self->hadjustment_binding =
      g_object_bind_property (self, "hadjustment",
                              child, "hadjustment",
                              G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
    self->vadjustment_binding =
      g_object_bind_property (self, "vadjustment",
                              child, "vadjustment",
                              G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
    self->hscroll_policy_binding =
      g_object_bind_property (self, "hscroll-policy",
                              child, "hscroll-policy",
                              G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
    self->vscroll_policy_binding =
      g_object_bind_property (self, "vscroll-policy",
                              child, "vscroll-policy",
                              G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adap_clamp_scrollable_get_maximum_size: (attributes org.gtk.Method.get_property=maximum-size)
 * @self: a clamp scrollable
 *
 * Gets the maximum size allocated to the child.
 *
 * Returns: the maximum size to allocate to the child
 */
int
adap_clamp_scrollable_get_maximum_size (AdapClampScrollable *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self), 0);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_maximum_size (layout);
}

/**
 * adap_clamp_scrollable_set_maximum_size: (attributes org.gtk.Method.set_property=maximum-size)
 * @self: a clamp scrollable
 * @maximum_size: the maximum size
 *
 * Sets the maximum size allocated to the child.
 *
 * It is the width if the clamp is horizontal, or the height if it is vertical.
 */
void
adap_clamp_scrollable_set_maximum_size (AdapClampScrollable *self,
                                       int                 maximum_size)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self));

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_maximum_size (layout) == maximum_size)
    return;

  adap_clamp_layout_set_maximum_size (layout, maximum_size);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * adap_clamp_scrollable_get_tightening_threshold: (attributes org.gtk.Method.get_property=tightening-threshold)
 * @self: a clamp scrollable
 *
 * Gets the size above which the child is clamped.
 *
 * Returns: the size above which the child is clamped
 */
int
adap_clamp_scrollable_get_tightening_threshold (AdapClampScrollable *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self), 0);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_tightening_threshold (layout);
}

/**
 * adap_clamp_scrollable_set_tightening_threshold: (attributes org.gtk.Method.set_property=tightening-threshold)
 * @self: a clamp scrollable
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size above which the child is clamped.
 *
 * Starting from this size, the clamp will tighten its grip on the child, slowly
 * allocating less and less of the available size up to the maximum allocated
 * size. Below that threshold and below the maximum width, the child will be
 * allocated all the available size.
 *
 * If the threshold is greater than the maximum size to allocate to the child,
 * the child will be allocated all the width up to the maximum. If the threshold
 * is lower than the minimum size to allocate to the child, that size will be
 * used as the tightening threshold.
 *
 * Effectively, tightening the grip on the child before it reaches its maximum
 * size makes transitions to and from the maximum size smoother when resizing.
 */
void
adap_clamp_scrollable_set_tightening_threshold (AdapClampScrollable *self,
                                               int                 tightening_threshold)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self));

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_tightening_threshold (layout) == tightening_threshold)
    return;

  adap_clamp_layout_set_tightening_threshold (layout, tightening_threshold);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}

/**
 * adap_clamp_scrollable_get_unit: (attributes org.gtk.Method.get_property=unit)
 * @self: a clamp scrollable
 *
 * Gets the length unit for maximum size and tightening threshold.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdapLengthUnit
adap_clamp_scrollable_get_unit (AdapClampScrollable *self)
{
  AdapClampLayout *layout;

  g_return_val_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self), ADAP_LENGTH_UNIT_PX);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adap_clamp_layout_get_unit (layout);
}

/**
 * adap_clamp_scrollable_set_unit: (attributes org.gtk.Method.set_property=unit)
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
adap_clamp_scrollable_set_unit (AdapClampScrollable *self,
                               AdapLengthUnit       unit)
{
  AdapClampLayout *layout;

  g_return_if_fail (ADAP_IS_CLAMP_SCROLLABLE (self));
  g_return_if_fail (unit >= ADAP_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADAP_LENGTH_UNIT_SP);

  layout = ADAP_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adap_clamp_layout_get_unit (layout) == unit)
    return;

  adap_clamp_layout_set_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UNIT]);
}
