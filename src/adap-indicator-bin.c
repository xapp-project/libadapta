/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-indicator-bin-private.h"

#include "adw-gizmo-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwIndicatorBin:
 *
 * A helper object for [class@ViewSwitcherButton].
 *
 * The `AdwIndicatorBin` widget shows an unread indicator over the child widget
 * masking it if they overlap.
 */

struct _AdwIndicatorBin
{
  GtkWidget parent_instance;

  GtkWidget *child;
  gboolean needs_attention;

  GtkWidget *mask;
  GtkWidget *indicator;
  GtkWidget *label;
};

static void adw_indicator_bin_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwIndicatorBin, adw_indicator_bin, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_indicator_bin_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_NEEDS_ATTENTION,
  PROP_BADGE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static gboolean
has_badge (AdwIndicatorBin *self)
{
  const char *text = gtk_label_get_label (GTK_LABEL (self->label));

  return text && text[0];
}

static void
adw_indicator_bin_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *min,
                           int            *nat,
                           int            *min_baseline,
                           int            *nat_baseline)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);

  if (!self->child) {
    if (min)
      *min = 0;
    if (nat)
      *nat = 0;
    if (min_baseline)
      *min_baseline = -1;
    if (nat_baseline)
      *nat_baseline = -1;

    return;
  }

  gtk_widget_measure (self->child, orientation, for_size,
                      min, nat, min_baseline, nat_baseline);
}

static void
adw_indicator_bin_size_allocate (GtkWidget *widget,
                                 int        width,
                                 int        height,
                                 int        baseline)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);
  GtkRequisition mask_size, indicator_size, size;
  float x, y;

  if (self->child)
    gtk_widget_allocate (self->child, width, height, baseline, NULL);

  gtk_widget_get_preferred_size (self->mask, NULL, &mask_size);
  gtk_widget_get_preferred_size (self->indicator, NULL, &indicator_size);

  size.width = MAX (mask_size.width, indicator_size.width);
  size.height = MAX (mask_size.height, indicator_size.height);

  if (size.width > width * 2)
    x = (width - size.width) / 2.0f;
  else if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    x = -size.height / 2.0f;
  else
    x = width - size.width + size.height / 2.0f;

  y = -size.height / 2.0f;

  gtk_widget_allocate (self->mask, size.width, size.height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
  gtk_widget_allocate (self->indicator, size.width, size.height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
}

static void
adw_indicator_bin_snapshot (GtkWidget   *widget,
                            GtkSnapshot *snapshot)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);

  if (!has_badge (self) && !self->needs_attention) {
    if (self->child)
      gtk_widget_snapshot_child (widget, self->child, snapshot);

    return;
  }

  if (self->child) {
    gtk_snapshot_push_mask (snapshot, GSK_MASK_MODE_INVERTED_ALPHA);

    gtk_widget_snapshot_child (widget, self->mask, snapshot);
    gtk_snapshot_pop (snapshot);

    gtk_widget_snapshot_child (widget, self->child, snapshot);
    gtk_snapshot_pop (snapshot);
  }

  gtk_widget_snapshot_child (widget, self->indicator, snapshot);
}

static void
adw_indicator_bin_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_indicator_bin_get_child (self));
    break;

  case PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, adw_indicator_bin_get_needs_attention (self));
    break;

  case PROP_BADGE:
    g_value_set_string (value, adw_indicator_bin_get_badge (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_indicator_bin_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_indicator_bin_set_child (self, g_value_get_object (value));
    break;

  case PROP_NEEDS_ATTENTION:
    adw_indicator_bin_set_needs_attention (self, g_value_get_boolean (value));
    break;

  case PROP_BADGE:
    adw_indicator_bin_set_badge (self, g_value_get_string (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_indicator_bin_dispose (GObject *object)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  g_clear_pointer (&self->child, gtk_widget_unparent);
  g_clear_pointer (&self->mask, gtk_widget_unparent);
  g_clear_pointer (&self->indicator, gtk_widget_unparent);
  self->label = NULL;

  G_OBJECT_CLASS (adw_indicator_bin_parent_class)->dispose (object);
}
static void
adw_indicator_bin_class_init (AdwIndicatorBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_indicator_bin_get_property;
  object_class->set_property = adw_indicator_bin_set_property;
  object_class->dispose = adw_indicator_bin_dispose;

  widget_class->measure = adw_indicator_bin_measure;
  widget_class->size_allocate = adw_indicator_bin_size_allocate;
  widget_class->snapshot = adw_indicator_bin_snapshot;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwIndicatorBin:child:
   *
   * The child widget.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIndicatorBin:needs-attention:
   *
   * Whether the indicator requires attention of the user.
   */
  props[PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIndicatorBin:badge:
   *
   * Additional information for the user.
   */
  props[PROP_BADGE] =
    g_param_spec_string ("badge", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "indicatorbin");
}

static void
adw_indicator_bin_init (AdwIndicatorBin *self)
{
  self->mask = adw_gizmo_new ("mask", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_can_target (self->mask, FALSE);
  gtk_widget_set_parent (self->mask, GTK_WIDGET (self));

  self->indicator = adw_gizmo_new ("indicator", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_can_target (self->indicator, FALSE);
  gtk_widget_set_parent (self->indicator, GTK_WIDGET (self));
  gtk_widget_set_layout_manager (self->indicator, gtk_bin_layout_new ());

  self->label = gtk_label_new (NULL);
  gtk_widget_set_visible (self->label, FALSE);
  gtk_widget_set_parent (self->label, self->indicator);
  gtk_widget_add_css_class (self->label, "numeric");
}

static void
adw_indicator_bin_buildable_add_child (GtkBuildable *buildable,
                                       GtkBuilder   *builder,
                                       GObject      *child,
                                       const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_indicator_bin_set_child (ADW_INDICATOR_BIN (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_indicator_bin_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_indicator_bin_buildable_add_child;
}

/**
 * adw_indicator_bin_new:
 *
 * Creates a new `AdwIndicatorBin`.
 *
 * Returns: the newly created `AdwIndicatorBin`
 */
GtkWidget *
adw_indicator_bin_new (void)
{
  return g_object_new (ADW_TYPE_INDICATOR_BIN, NULL);
}

/**
 * adw_indicator_bin_get_child:
 * @self: an indicator bin
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adw_indicator_bin_get_child (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), NULL);

  return self->child;
}

/**
 * adw_indicator_bin_set_child:
 * @self: an indicator bin
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adw_indicator_bin_set_child (AdwIndicatorBin *self,
                             GtkWidget       *child)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  if (self->child)
    gtk_widget_unparent (self->child);

  self->child = child;

  if (self->child)
    gtk_widget_set_parent (self->child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

gboolean
adw_indicator_bin_get_needs_attention (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), FALSE);

  return self->needs_attention;
}

void
adw_indicator_bin_set_needs_attention (AdwIndicatorBin *self,
                                       gboolean         needs_attention)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));

  needs_attention = !!needs_attention;

  if (self->needs_attention == needs_attention)
    return;

  self->needs_attention = needs_attention;

  if (self->needs_attention)
    gtk_widget_add_css_class (GTK_WIDGET (self), "needs-attention");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "needs-attention");

  gtk_widget_queue_draw (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION]);
}

const char *
adw_indicator_bin_get_badge (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), "");

  return gtk_label_get_label (GTK_LABEL (self->label));
}

void
adw_indicator_bin_set_badge (AdwIndicatorBin *self,
                             const char      *badge)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));

  gtk_label_set_text (GTK_LABEL (self->label), badge);

  if (badge && badge[0])
    gtk_widget_add_css_class (GTK_WIDGET (self), "badge");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "badge");

  gtk_widget_set_visible (self->label, badge && badge[0]);

  gtk_widget_queue_draw (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BADGE]);
}
