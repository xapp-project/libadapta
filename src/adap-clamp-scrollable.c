/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-clamp-scrollable.h"

#include "adw-clamp-layout.h"
#include "adw-enums.h"
#include "adw-length-unit.h"
#include "adw-widget-utils-private.h"

/**
 * AdwClampScrollable:
 *
 * A scrollable [class@Clamp].
 *
 * `AdwClampScrollable` is a variant of [class@Clamp] that implements the
 * [iface@Gtk.Scrollable] interface.
 *
 * The primary use case for `AdwClampScrollable` is clamping
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

struct _AdwClampScrollable
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

static void adw_clamp_scrollable_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwClampScrollable, adw_clamp_scrollable, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_clamp_scrollable_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
set_orientation (AdwClampScrollable *self,
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
set_hadjustment (AdwClampScrollable *self,
                 GtkAdjustment      *adjustment)
{
  if (self->hadjustment == adjustment)
    return;

  self->hadjustment = adjustment;

  g_object_notify (G_OBJECT (self), "hadjustment");
}

static void
set_vadjustment (AdwClampScrollable *self,
                 GtkAdjustment      *adjustment)
{
  if (self->vadjustment == adjustment)
    return;

  self->vadjustment = adjustment;

  g_object_notify (G_OBJECT (self), "vadjustment");
}

static void
set_hscroll_policy (AdwClampScrollable  *self,
                    GtkScrollablePolicy  policy)
{
  if (self->hscroll_policy == policy)
    return;

  self->hscroll_policy = policy;

  g_object_notify (G_OBJECT (self), "hscroll-policy");
}

static void
set_vscroll_policy (AdwClampScrollable  *self,
                    GtkScrollablePolicy  policy)
{
  if (self->vscroll_policy == policy)
    return;

  self->vscroll_policy = policy;

  g_object_notify (G_OBJECT (self), "vscroll-policy");
}

static void
adw_clamp_scrollable_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdwClampScrollable *self = ADW_CLAMP_SCROLLABLE (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_clamp_scrollable_get_child (self));
    break;
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, adw_clamp_scrollable_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, adw_clamp_scrollable_get_tightening_threshold (self));
    break;
  case PROP_UNIT:
    g_value_set_enum (value, adw_clamp_scrollable_get_unit (self));
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
adw_clamp_scrollable_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdwClampScrollable *self = ADW_CLAMP_SCROLLABLE (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_clamp_scrollable_set_child (self, g_value_get_object (value));
    break;
  case PROP_MAXIMUM_SIZE:
    adw_clamp_scrollable_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    adw_clamp_scrollable_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_UNIT:
    adw_clamp_scrollable_set_unit (self, g_value_get_enum (value));
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
adw_clamp_scrollable_dispose (GObject *object)
{
  AdwClampScrollable *self = ADW_CLAMP_SCROLLABLE (object);

  adw_clamp_scrollable_set_child (self, NULL);

  G_OBJECT_CLASS (adw_clamp_scrollable_parent_class)->dispose (object);
}

static void
adw_clamp_scrollable_class_init (AdwClampScrollableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_clamp_scrollable_get_property;
  object_class->set_property = adw_clamp_scrollable_set_property;
  object_class->dispose = adw_clamp_scrollable_dispose;

  widget_class->compute_expand = adw_widget_compute_expand;

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
   * AdwClampScrollable:child: (attributes org.gtk.Property.get=adw_clamp_scrollable_get_child org.gtk.Property.set=adw_clamp_scrollable_set_child)
   *
   * The child widget of the `AdwClampScrollable`.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwClampScrollable:maximum-size: (attributes org.gtk.Property.get=adw_clamp_scrollable_get_maximum_size org.gtk.Property.set=adw_clamp_scrollable_set_maximum_size)
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
   * AdwClampScrollable:tightening-threshold: (attributes org.gtk.Property.get=adw_clamp_scrollable_get_tightening_threshold org.gtk.Property.set=adw_clamp_scrollable_set_tightening_threshold)
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
   * AdwClampScrollable:unit: (attributes org.gtk.Property.get=adw_clamp_scrollable_get_unit org.gtk.Property.set=adw_clamp_scrollable_set_unit)
   *
   * The length unit for maximum size and tightening threshold.
   *
   * Allows the sizes to vary depending on the text scale factor.
   *
   * Since: 1.4
   */
  props[PROP_UNIT] =
    g_param_spec_enum ("unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_SP,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, ADW_TYPE_CLAMP_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "clamp");
}

static void
adw_clamp_scrollable_init (AdwClampScrollable *self)
{
}

static void
adw_clamp_scrollable_buildable_add_child (GtkBuildable *buildable,
                                          GtkBuilder   *builder,
                                          GObject      *child,
                                          const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_clamp_scrollable_set_child (ADW_CLAMP_SCROLLABLE (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_clamp_scrollable_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_clamp_scrollable_buildable_add_child;
}

/**
 * adw_clamp_scrollable_new:
 *
 * Creates a new `AdwClampScrollable`.
 *
 * Returns: the newly created `AdwClampScrollable`
 */
GtkWidget *
adw_clamp_scrollable_new (void)
{
  return g_object_new (ADW_TYPE_CLAMP_SCROLLABLE, NULL);
}

/**
 * adw_clamp_scrollable_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a clamp scrollable
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adw_clamp_scrollable_get_child (AdwClampScrollable *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_SCROLLABLE (self), NULL);

  return self->child;
}

/**
 * adw_clamp_scrollable_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a clamp scrollable
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adw_clamp_scrollable_set_child (AdwClampScrollable *self,
                                GtkWidget          *child)
{
  g_return_if_fail (ADW_IS_CLAMP_SCROLLABLE (self));
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
 * adw_clamp_scrollable_get_maximum_size: (attributes org.gtk.Method.get_property=maximum-size)
 * @self: a clamp scrollable
 *
 * Gets the maximum size allocated to the child.
 *
 * Returns: the maximum size to allocate to the child
 */
int
adw_clamp_scrollable_get_maximum_size (AdwClampScrollable *self)
{
  AdwClampLayout *layout;

  g_return_val_if_fail (ADW_IS_CLAMP_SCROLLABLE (self), 0);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_clamp_layout_get_maximum_size (layout);
}

/**
 * adw_clamp_scrollable_set_maximum_size: (attributes org.gtk.Method.set_property=maximum-size)
 * @self: a clamp scrollable
 * @maximum_size: the maximum size
 *
 * Sets the maximum size allocated to the child.
 *
 * It is the width if the clamp is horizontal, or the height if it is vertical.
 */
void
adw_clamp_scrollable_set_maximum_size (AdwClampScrollable *self,
                                       int                 maximum_size)
{
  AdwClampLayout *layout;

  g_return_if_fail (ADW_IS_CLAMP_SCROLLABLE (self));

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adw_clamp_layout_get_maximum_size (layout) == maximum_size)
    return;

  adw_clamp_layout_set_maximum_size (layout, maximum_size);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * adw_clamp_scrollable_get_tightening_threshold: (attributes org.gtk.Method.get_property=tightening-threshold)
 * @self: a clamp scrollable
 *
 * Gets the size above which the child is clamped.
 *
 * Returns: the size above which the child is clamped
 */
int
adw_clamp_scrollable_get_tightening_threshold (AdwClampScrollable *self)
{
  AdwClampLayout *layout;

  g_return_val_if_fail (ADW_IS_CLAMP_SCROLLABLE (self), 0);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_clamp_layout_get_tightening_threshold (layout);
}

/**
 * adw_clamp_scrollable_set_tightening_threshold: (attributes org.gtk.Method.set_property=tightening-threshold)
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
adw_clamp_scrollable_set_tightening_threshold (AdwClampScrollable *self,
                                               int                 tightening_threshold)
{
  AdwClampLayout *layout;

  g_return_if_fail (ADW_IS_CLAMP_SCROLLABLE (self));

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adw_clamp_layout_get_tightening_threshold (layout) == tightening_threshold)
    return;

  adw_clamp_layout_set_tightening_threshold (layout, tightening_threshold);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}

/**
 * adw_clamp_scrollable_get_unit: (attributes org.gtk.Method.get_property=unit)
 * @self: a clamp scrollable
 *
 * Gets the length unit for maximum size and tightening threshold.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdwLengthUnit
adw_clamp_scrollable_get_unit (AdwClampScrollable *self)
{
  AdwClampLayout *layout;

  g_return_val_if_fail (ADW_IS_CLAMP_SCROLLABLE (self), ADW_LENGTH_UNIT_PX);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_clamp_layout_get_unit (layout);
}

/**
 * adw_clamp_scrollable_set_unit: (attributes org.gtk.Method.set_property=unit)
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
adw_clamp_scrollable_set_unit (AdwClampScrollable *self,
                               AdwLengthUnit       unit)
{
  AdwClampLayout *layout;

  g_return_if_fail (ADW_IS_CLAMP_SCROLLABLE (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adw_clamp_layout_get_unit (layout) == unit)
    return;

  adw_clamp_layout_set_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UNIT]);
}
