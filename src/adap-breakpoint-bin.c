/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adap-breakpoint-bin-private.h"

#include "adap-breakpoint-private.h"
#include "adap-widget-utils-private.h"

/**
 * AdapBreakpointBin:
 *
 * A widget that changes layout based on available size.
 *
 * <picture>
 *   <source srcset="breakpoint-bin-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="breakpoint-bin.png" alt="breakpoint-bin">
 * </picture>
 *
 * `AdapBreakpointBin` provides a way to use breakpoints without [class@Window],
 * [class@ApplicationWindow] or [class@Dialog]. It can be useful for limiting
 * breakpoints to a single page and similar purposes. Most applications
 * shouldn't need it.
 *
 * `AdapBreakpointBin` is similar to [class@Bin]. It has one child, set via the
 * [property@BreakpointBin:child] property.
 *
 * When `AdapBreakpointBin` is resized, its child widget can rearrange its layout
 * at specific thresholds.
 *
 * The thresholds and layout changes are defined via [class@Breakpoint] objects.
 * They can be added using [method@BreakpointBin.add_breakpoint].
 *
 * Each breakpoint has a condition, specifying the bin's size and/or aspect
 * ratio, and setters that automatically set object properties when that
 * happens. The [signal@Breakpoint::apply] and [signal@Breakpoint::unapply] can
 * be used instead for more complex scenarios.
 *
 * Breakpoints are only allowed to modify widgets inside the `AdapBreakpointBin`,
 * but not on the `AdapBreakpointBin` itself or any other widgets.
 *
 * If multiple breakpoints can be used for the current size, the last one is
 * always picked. The current breakpoint can be tracked using the
 * [property@BreakpointBin:current-breakpoint] property.
 *
 * If none of the breakpoints can be used, that property will be set to `NULL`,
 * and the original property values will be used instead.
 *
 * ## Minimum Size
 *
 * Adding a breakpoint to `AdapBreakpointBin` will result in it having no minimum
 * size. The [property@Gtk.Widget:width-request] and
 * [property@Gtk.Widget:height-request] properties must always be set when using
 * breakpoints, indicating the smallest size you want to support.
 *
 * The minimum size and breakpoint conditions must be carefully selected so that
 * the child widget completely fits. If it doesn't, it will overflow and a
 * warning message will be printed.
 *
 * When choosing minimum size, consider translations and text scale factor
 * changes. Make sure to leave enough space for text labels, and enable
 * ellipsizing or wrapping if they might not fit.
 *
 * For [class@Gtk.Label] this can be done via [property@Gtk.Label:ellipsize], or
 * via [property@Gtk.Label:wrap] together with [property@Gtk.Label:wrap-mode].
 *
 * For buttons, use [property@Gtk.Button:can-shrink],
 * [property@Gtk.MenuButton:can-shrink], [property@Adap.SplitButton:can-shrink],
 * or [property@Adap.ButtonContent:can-shrink].
 *
 * ## Example
 *
 * ```c
 * GtkWidget *bin, *child;
 * AdapBreakpoint *breakpoint;
 *
 * bin = adap_breakpoint_bin_new ();
 * gtk_widget_set_size_request (bin, 150, 150);
 *
 * child = gtk_label_new ("Wide");
 * gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
 * gtk_widget_add_css_class (child, "title-1");
 * adap_breakpoint_bin_set_child (ADAP_BREAKPOINT_BIN (bin), child);
 *
 * breakpoint = adap_breakpoint_new (adap_breakpoint_condition_parse ("max-width: 200px"));
 * adap_breakpoint_add_setters (breakpoint,
 *                             G_OBJECT (child), "label", "Narrow",
 *                             NULL);
 * adap_breakpoint_bin_add_breakpoint (ADAP_BREAKPOINT_BIN (bin), breakpoint);
 * ```
 *
 * The bin has a single label inside it, displaying "Wide". When the bin's width
 * is smaller than or equal to 200px, it changes to "Narrow".
 *
 * ## `AdapBreakpointBin` as `GtkBuildable`
 *
 * `AdapBreakpointBin` allows adding `AdapBreakpoint` objects as children.
 *
 * Example of an `AdapBreakpointBin` UI definition:
 *
 * ```xml
 * <object class="AdapBreakpointBin">
 *   <property name="width-request">150</property>
 *   <property name="height-request">150</property>
 *   <property name="child">
 *     <object class="GtkLabel" id="child">
 *       <property name="label">Wide</property>
 *       <property name="ellipsize">end</property>
 *       <style>
 *         <class name="title-1"/>
 *       </style>
 *     </object>
 *   </property>
 *   <child>
 *     <object class="AdapBreakpoint">
 *       <condition>max-width: 200px</condition>
 *       <setter object="child" property="label">Narrow</setter>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * See [class@Breakpoint] documentation for details.
 *
 * Since: 1.4
 */

typedef struct {
  gboolean grab_focus;
  GtkDirectionType direction;
} DelayedFocus;

typedef struct
{
  GtkWidget *child;

  GList *breakpoints;
  AdapBreakpoint *current_breakpoint;

  GskRenderNode *old_node;
  gboolean first_allocation;
  guint tick_cb_id;

  gboolean block_warnings;
  GtkWidget *warning_widget;
  gboolean enable_min_size_warnings;
  gboolean enable_overflow_warnings;

  gboolean pass_through;

  int natural_width;
  int natural_height;

  GArray *delayed_focus;
} AdapBreakpointBinPrivate;

static void adap_breakpoint_bin_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapBreakpointBin, adap_breakpoint_bin, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdapBreakpointBin)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_breakpoint_bin_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_CURRENT_BREAKPOINT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
allocate_child (AdapBreakpointBin *self,
                int               width,
                int               height,
                int               baseline)
{
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);
  int min_width, min_height;

  if (priv->old_node)
    return;

  if (!priv->child)
    return;

  if (!priv->block_warnings && priv->breakpoints && priv->enable_min_size_warnings) {
    int window_width, window_height;
    GtkWidget *warning_widget;

    if (priv->warning_widget)
      warning_widget = priv->warning_widget;
    else
      warning_widget = GTK_WIDGET (self);

    gtk_widget_get_size_request (warning_widget,
                                 &window_width, &window_height);

    if (window_width <= 0 && window_height <= 0)
      g_warning ("%s %p does not have a minimum size, set the 'width-request' "
                 "and 'height-request' properties to specify it",
                 G_OBJECT_TYPE_NAME (warning_widget), warning_widget);
    else if (window_width <= 0)
      g_warning ("%s %p does not have a minimum width, set the "
                 "'width-request' property to specify it",
                 G_OBJECT_TYPE_NAME (warning_widget), warning_widget);
    else if (window_height <= 0)
      g_warning ("%s %p does not have a minimum height, set the "
                 "'height-request' property to specify it",
                 G_OBJECT_TYPE_NAME (warning_widget), warning_widget);
  }

  gtk_widget_measure (priv->child, GTK_ORIENTATION_HORIZONTAL, -1,
                      &min_width, NULL, NULL, NULL);
  gtk_widget_measure (priv->child, GTK_ORIENTATION_VERTICAL, -1,
                      &min_height, NULL, NULL, NULL);

  if (width >= min_width && height >= min_height) {
    gtk_widget_allocate (priv->child, width, height, baseline, NULL);

    return;
  }

  if (!priv->block_warnings && priv->enable_overflow_warnings) {
    GtkWidget *warning_widget;

    if (priv->warning_widget)
      warning_widget = priv->warning_widget;
    else
      warning_widget = GTK_WIDGET (self);

    if (min_width > width && min_height > height)
      g_warning ("%s %p exceeds %s size: requested %d×%d px, %d×%d px available",
                 G_OBJECT_TYPE_NAME (priv->child), priv->child,
                 G_OBJECT_TYPE_NAME (warning_widget), min_width, min_height, width, height);
    else if (min_width > width)
      g_warning ("%s %p exceeds %s width: requested %d px, %d px available",
                 G_OBJECT_TYPE_NAME (priv->child), priv->child,
                 G_OBJECT_TYPE_NAME (warning_widget), min_width, width);
    else
      g_warning ("%s %p exceeds %s height: requested %d px, %d px available",
                 G_OBJECT_TYPE_NAME (priv->child), priv->child,
                 G_OBJECT_TYPE_NAME (warning_widget), min_height, height);
  }

  width = MAX (width, min_width);
  height = MAX (height, min_height);

  gtk_widget_allocate (priv->child, width, height, baseline, NULL);
}

static gboolean
breakpoint_changed_tick_cb (GtkWidget        *widget,
                            GdkFrameClock    *frame_clock,
                            AdapBreakpointBin *self)
{
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);
  int i;

  priv->tick_cb_id = 0;
  g_clear_pointer (&priv->old_node, gsk_render_node_unref);
  gtk_widget_set_child_visible (priv->child, TRUE);
  gtk_widget_queue_resize (GTK_WIDGET (self));

  for (i = 0; i < priv->delayed_focus->len; i++) {
    DelayedFocus *focus = &g_array_index (priv->delayed_focus, DelayedFocus, i);

    if (focus->grab_focus)
      gtk_widget_grab_focus (GTK_WIDGET (widget));
    else
      adap_widget_focus_child (GTK_WIDGET (widget), focus->direction);
  }

  g_array_remove_range (priv->delayed_focus, 0, priv->delayed_focus->len);

  return G_SOURCE_REMOVE;
}

static void
breakpoint_notify_condition_cb (AdapBreakpointBin *self)
{
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static gboolean
adap_breakpoint_bin_contains (GtkWidget *widget,
                             double     x,
                             double     y)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->pass_through)
    return FALSE;

  return GTK_WIDGET_CLASS (adap_breakpoint_bin_parent_class)->contains (widget, x, y);
}

static void
adap_breakpoint_bin_snapshot (GtkWidget   *widget,
                             GtkSnapshot *snapshot)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->old_node) {
    gtk_snapshot_append_node (snapshot, priv->old_node);
    return;
  }

  GTK_WIDGET_CLASS (adap_breakpoint_bin_parent_class)->snapshot (GTK_WIDGET (self),
                                                                snapshot);
}

static GtkSizeRequestMode
adap_breakpoint_bin_get_request_mode (GtkWidget *widget)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->breakpoints)
    return GTK_SIZE_REQUEST_CONSTANT_SIZE;

  if (priv->child)
    return gtk_widget_get_request_mode (priv->child);

  return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
adap_breakpoint_bin_measure (GtkWidget      *widget,
                            GtkOrientation  orientation,
                            int             for_size,
                            int            *minimum,
                            int            *natural,
                            int            *minimum_baseline,
                            int            *natural_baseline)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);
  int min = 0, nat = 0;

  if (priv->child) {
    gtk_widget_measure (priv->child, orientation, for_size,
                        &min, &nat, NULL, NULL);

    if (priv->breakpoints)
      min = 0;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (priv->natural_width >= 0)
      nat = MAX (min, priv->natural_width);
    else
      nat = MAX (min, nat);
  } else {
    if (priv->natural_height >= 0)
      nat = MAX (min, priv->natural_height);
    else
      nat = MAX (min, nat);
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adap_breakpoint_bin_size_allocate (GtkWidget *widget,
                                  int        width,
                                  int        height,
                                  int        baseline)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);
  GList *l;
  GtkSnapshot *snapshot;
  AdapBreakpoint *new_breakpoint = NULL;
  GtkSettings *settings;

  if (!priv->child)
    return;

  settings = gtk_widget_get_settings (widget);

  for (l = priv->breakpoints; l; l = l->next) {
    AdapBreakpoint *breakpoint = l->data;

    if (adap_breakpoint_check_condition (breakpoint, settings, width, height)) {
      new_breakpoint = breakpoint;
      break;
    }
  }

  if (new_breakpoint == priv->current_breakpoint) {
    allocate_child (self, width, height, baseline);
    priv->first_allocation = FALSE;

    return;
  }

  if (!priv->first_allocation) {
    priv->block_warnings = TRUE;
    allocate_child (self, width, height, baseline);
    priv->block_warnings = FALSE;

    snapshot = gtk_snapshot_new ();
    adap_breakpoint_bin_snapshot (widget, snapshot);

    priv->old_node = gtk_snapshot_free_to_node (snapshot);

    gtk_widget_set_child_visible (priv->child, FALSE);
  }

  adap_breakpoint_transition (priv->current_breakpoint, new_breakpoint);

  priv->current_breakpoint = new_breakpoint;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_BREAKPOINT]);

  if (priv->first_allocation) {
    priv->block_warnings = TRUE;
    allocate_child (self, width, height, baseline);
    priv->block_warnings = FALSE;
    priv->first_allocation = FALSE;
  } else {
    priv->tick_cb_id = gtk_widget_add_tick_callback (widget,
                                                     (GtkTickCallback) breakpoint_changed_tick_cb,
                                                     self, NULL);
  }
}

static void
adap_breakpoint_bin_map (GtkWidget *widget)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  priv->first_allocation = TRUE;

  GTK_WIDGET_CLASS (adap_breakpoint_bin_parent_class)->map (GTK_WIDGET (self));
}

static gboolean
adap_breakpoint_bin_focus (GtkWidget        *widget,
                          GtkDirectionType  direction)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->tick_cb_id > 0) {
    DelayedFocus focus;

    focus.grab_focus = FALSE;
    focus.direction = direction;

    g_array_append_val (priv->delayed_focus, focus);

    return FALSE;
  }

  return adap_widget_focus_child (widget, direction);
}

static gboolean
adap_breakpoint_bin_grab_focus (GtkWidget *widget)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (widget);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->tick_cb_id > 0) {
    DelayedFocus focus;

    focus.grab_focus = TRUE;
    focus.direction = 0;

    g_array_append_val (priv->delayed_focus, focus);

    return FALSE;
  }

  return adap_widget_grab_focus_child (widget);
}

static void
adap_breakpoint_bin_dispose (GObject *object)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (object);
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  g_clear_pointer (&priv->child, gtk_widget_unparent);

  if (priv->tick_cb_id) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->tick_cb_id);
    priv->tick_cb_id = 0;
  }

  if (priv->breakpoints) {
    g_list_free_full (priv->breakpoints, g_object_unref);
    priv->breakpoints = NULL;
  }

  g_clear_pointer (&priv->delayed_focus, g_array_unref);

  G_OBJECT_CLASS (adap_breakpoint_bin_parent_class)->dispose (object);
}

static void
adap_breakpoint_bin_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_breakpoint_bin_get_child (self));
    break;
  case PROP_CURRENT_BREAKPOINT:
    g_value_set_object (value, adap_breakpoint_bin_get_current_breakpoint (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_breakpoint_bin_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdapBreakpointBin *self = ADAP_BREAKPOINT_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_breakpoint_bin_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_breakpoint_bin_class_init (AdapBreakpointBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_breakpoint_bin_dispose;
  object_class->get_property = adap_breakpoint_bin_get_property;
  object_class->set_property = adap_breakpoint_bin_set_property;

  widget_class->contains = adap_breakpoint_bin_contains;
  widget_class->get_request_mode = adap_breakpoint_bin_get_request_mode;
  widget_class->measure = adap_breakpoint_bin_measure;
  widget_class->size_allocate = adap_breakpoint_bin_size_allocate;
  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->snapshot = adap_breakpoint_bin_snapshot;
  widget_class->map = adap_breakpoint_bin_map;
  widget_class->focus = adap_breakpoint_bin_focus;
  widget_class->grab_focus = adap_breakpoint_bin_grab_focus;

  /**
   * AdapBreakpointBin:child: (attributes org.gtk.Property.get=adap_breakpoint_bin_get_child org.gtk.Property.set=adap_breakpoint_bin_set_child)
   *
   * The child widget.
   *
   * Since: 1.4
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapBreakpointBin:current-breakpoint: (attributes org.gtk.Property.get=adap_breakpoint_bin_get_current_breakpoint)
   *
   * The current breakpoint.
   *
   * Since: 1.4
   */
  props[PROP_CURRENT_BREAKPOINT] =
    g_param_spec_object ("current-breakpoint", NULL, NULL,
                         ADAP_TYPE_BREAKPOINT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adap_breakpoint_bin_init (AdapBreakpointBin *self)
{
  AdapBreakpointBinPrivate *priv = adap_breakpoint_bin_get_instance_private (self);

  priv->natural_width = -1;
  priv->natural_height = -1;
  priv->enable_min_size_warnings = TRUE;
  priv->enable_overflow_warnings = TRUE;

  priv->delayed_focus = g_array_new (FALSE, FALSE, sizeof (DelayedFocus));

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);
}

static void
adap_breakpoint_bin_buildable_add_child (GtkBuildable *buildable,
                                        GtkBuilder   *builder,
                                        GObject      *child,
                                        const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_breakpoint_bin_set_child (ADAP_BREAKPOINT_BIN (buildable), GTK_WIDGET (child));
  else if (ADAP_IS_BREAKPOINT (child))
    adap_breakpoint_bin_add_breakpoint (ADAP_BREAKPOINT_BIN (buildable),
                                       g_object_ref (ADAP_BREAKPOINT (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_breakpoint_bin_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_breakpoint_bin_buildable_add_child;
}

/**
 * adap_breakpoint_bin_new:
 *
 * Creates a new `AdapBreakpointBin`.
 *
 * Returns: the newly created `AdapBreakpointBin`
 *
 * Since: 1.4
 */
GtkWidget *
adap_breakpoint_bin_new (void)
{
  return g_object_new (ADAP_TYPE_BREAKPOINT_BIN, NULL);
}

/**
 * adap_breakpoint_bin_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a breakpoint bin
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.4
 */
GtkWidget *
adap_breakpoint_bin_get_child (AdapBreakpointBin *self)
{
  AdapBreakpointBinPrivate *priv;

  g_return_val_if_fail (ADAP_IS_BREAKPOINT_BIN (self), NULL);

  priv = adap_breakpoint_bin_get_instance_private (self);

  return priv->child;
}

/**
 * adap_breakpoint_bin_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a breakpoint bin
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.4
 */
void
adap_breakpoint_bin_set_child (AdapBreakpointBin *self,
                              GtkWidget        *child)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adap_breakpoint_bin_get_instance_private (self);

  if (priv->child == child)
    return;

  if (priv->child)
    gtk_widget_unparent (priv->child);

  priv->child = child;

  if (child) {
    gtk_widget_set_parent (child, GTK_WIDGET (self));

    if (priv->old_node)
      gtk_widget_set_child_visible (child, FALSE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adap_breakpoint_bin_add_breakpoint:
 * @self: a breakpoint bin
 * @breakpoint: (transfer full): the breakpoint to add
 *
 * Adds @breakpoint to @self.
 *
 * Since: 1.4
 */
void
adap_breakpoint_bin_add_breakpoint (AdapBreakpointBin *self,
                                   AdapBreakpoint    *breakpoint)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));
  g_return_if_fail (ADAP_IS_BREAKPOINT (breakpoint));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->breakpoints = g_list_prepend (priv->breakpoints, breakpoint);

  breakpoint_notify_condition_cb (self);

  g_signal_connect_swapped (breakpoint, "notify::condition",
                            G_CALLBACK (breakpoint_notify_condition_cb), self);
}

/**
 * adap_breakpoint_bin_remove_breakpoint:
 * @self: a breakpoint bin
 * @breakpoint: a breakpoint to remove
 *
 * Removes @breakpoint from @self.
 *
 * Since: 1.5
 */
void
adap_breakpoint_bin_remove_breakpoint (AdapBreakpointBin *self,
                                      AdapBreakpoint    *breakpoint)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));
  g_return_if_fail (ADAP_IS_BREAKPOINT (breakpoint));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->breakpoints = g_list_remove (priv->breakpoints, breakpoint);

  breakpoint_notify_condition_cb (self);

  g_signal_handlers_disconnect_by_func (breakpoint, breakpoint_notify_condition_cb, self);
}

/**
 * adap_breakpoint_bin_get_current_breakpoint: (attributes org.gtk.Method.get_property=current-breakpoint)
 * @self: a breakpoint bin
 *
 * Gets the current breakpoint.
 *
 * Returns: (nullable) (transfer none): the current breakpoint
 *
 * Since: 1.4
 */
AdapBreakpoint *
adap_breakpoint_bin_get_current_breakpoint (AdapBreakpointBin *self)
{
  AdapBreakpointBinPrivate *priv;

  g_return_val_if_fail (ADAP_IS_BREAKPOINT_BIN (self), NULL);

  priv = adap_breakpoint_bin_get_instance_private (self);

  return priv->current_breakpoint;
}

void
adap_breakpoint_bin_set_warnings (AdapBreakpointBin *self,
                                 gboolean          min_size_warnings,
                                 gboolean          overflow_warnings)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->enable_min_size_warnings = !!min_size_warnings;
  priv->enable_overflow_warnings = !!overflow_warnings;
}

void
adap_breakpoint_bin_set_warning_widget (AdapBreakpointBin *self,
                                       GtkWidget        *warning_widget)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->warning_widget = warning_widget;
}

gboolean
adap_breakpoint_bin_has_breakpoints (AdapBreakpointBin *self)
{
  AdapBreakpointBinPrivate *priv;

  g_return_val_if_fail (ADAP_IS_BREAKPOINT_BIN (self), FALSE);

  priv = adap_breakpoint_bin_get_instance_private (self);

  return !!priv->breakpoints;
}

void
adap_breakpoint_bin_set_pass_through (AdapBreakpointBin *self,
                                     gboolean          pass_through)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->pass_through = !!pass_through;
}

void
adap_breakpoint_bin_set_natural_size (AdapBreakpointBin *self,
                                     int               width,
                                     int               height)
{
  AdapBreakpointBinPrivate *priv;

  g_return_if_fail (ADAP_IS_BREAKPOINT_BIN (self));

  priv = adap_breakpoint_bin_get_instance_private (self);

  priv->natural_width = width;
  priv->natural_height = height;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}
