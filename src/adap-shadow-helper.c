/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-shadow-helper-private.h"

#include "adw-gizmo-private.h"

struct _AdwShadowHelper
{
  GObject parent_instance;

  GtkWidget *widget;

  GtkWidget *dimming;
  GtkWidget *shadow;
  GtkWidget *border;
  GtkWidget *outline;
};

G_DEFINE_FINAL_TYPE (AdwShadowHelper, adw_shadow_helper, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_WIDGET,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
adw_shadow_helper_constructed (GObject *object)
{
  AdwShadowHelper *self = ADW_SHADOW_HELPER (object);

  self->dimming = adw_gizmo_new_with_role ("dimming", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                           NULL, NULL, NULL, NULL, NULL, NULL);
  self->shadow = adw_gizmo_new_with_role ("shadow", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                          NULL, NULL, NULL, NULL, NULL, NULL);
  self->border = adw_gizmo_new_with_role ("border", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                          NULL, NULL, NULL, NULL, NULL, NULL);
  self->outline = adw_gizmo_new_with_role ("outline", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                           NULL, NULL, NULL, NULL, NULL, NULL);

  gtk_widget_set_visible (self->dimming, FALSE);
  gtk_widget_set_visible (self->shadow, FALSE);
  gtk_widget_set_visible (self->border, FALSE);
  gtk_widget_set_visible (self->outline, FALSE);

  gtk_widget_set_can_target (self->dimming, FALSE);
  gtk_widget_set_can_target (self->shadow, FALSE);
  gtk_widget_set_can_target (self->border, FALSE);
  gtk_widget_set_can_target (self->outline, FALSE);

  gtk_widget_set_parent (self->dimming, self->widget);
  gtk_widget_set_parent (self->shadow, self->widget);
  gtk_widget_set_parent (self->border, self->widget);
  gtk_widget_set_parent (self->outline, self->widget);

  G_OBJECT_CLASS (adw_shadow_helper_parent_class)->constructed (object);
}

static void
adw_shadow_helper_dispose (GObject *object)
{
  AdwShadowHelper *self = ADW_SHADOW_HELPER (object);

  g_clear_pointer (&self->dimming, gtk_widget_unparent);
  g_clear_pointer (&self->shadow, gtk_widget_unparent);
  g_clear_pointer (&self->border, gtk_widget_unparent);
  g_clear_pointer (&self->outline, gtk_widget_unparent);
  self->widget = NULL;

  G_OBJECT_CLASS (adw_shadow_helper_parent_class)->dispose (object);
}

static void
adw_shadow_helper_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwShadowHelper *self = ADW_SHADOW_HELPER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    g_value_set_object (value, self->widget);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shadow_helper_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwShadowHelper *self = ADW_SHADOW_HELPER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    self->widget = g_value_get_object (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shadow_helper_class_init (AdwShadowHelperClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_shadow_helper_constructed;
  object_class->dispose = adw_shadow_helper_dispose;
  object_class->get_property = adw_shadow_helper_get_property;
  object_class->set_property = adw_shadow_helper_set_property;

  /**
   * AdwShadowHelper:widget:
   *
   * The widget the shadow will be drawn for. Must not be `NULL`
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_shadow_helper_init (AdwShadowHelper *self)
{
}

/**
 * adw_shadow_helper_new:
 *
 * Creates a new `AdwShadowHelper`.
 *
 * Returns: the newly created `AdwShadowHelper`
 */
AdwShadowHelper *
adw_shadow_helper_new (GtkWidget *widget)
{
  return g_object_new (ADW_TYPE_SHADOW_HELPER,
                       "widget", widget,
                       NULL);
}

static void
set_style_classes (AdwShadowHelper *self,
                   GtkPanDirection  direction)
{
  const char *classes[2];

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
    classes[0] = "left";
    break;
  case GTK_PAN_DIRECTION_RIGHT:
    classes[0] = "right";
    break;
  case GTK_PAN_DIRECTION_UP:
    classes[0] = "up";
    break;
  case GTK_PAN_DIRECTION_DOWN:
    classes[0] = "down";
    break;
  default:
    g_assert_not_reached ();
  }
  classes[1] = NULL;

  gtk_widget_set_css_classes (self->dimming, classes);
  gtk_widget_set_css_classes (self->shadow, classes);
  gtk_widget_set_css_classes (self->border, classes);
  gtk_widget_set_css_classes (self->outline, classes);
}

void
adw_shadow_helper_size_allocate (AdwShadowHelper *self,
                                 int              width,
                                 int              height,
                                 int              baseline,
                                 int              x,
                                 int              y,
                                 double           progress,
                                 GtkPanDirection  direction)
{
  double distance, remaining_distance;
  double shadow_opacity;
  int shadow_size, border_size, outline_size;
  GtkOrientation orientation;

  set_style_classes (self, direction);

  gtk_widget_allocate (self->dimming, width, height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
  case GTK_PAN_DIRECTION_RIGHT:
    distance = width;
    orientation = GTK_ORIENTATION_HORIZONTAL;
    break;
  case GTK_PAN_DIRECTION_UP:
  case GTK_PAN_DIRECTION_DOWN:
    distance = height;
    orientation = GTK_ORIENTATION_VERTICAL;
    break;
  default:
    g_assert_not_reached ();
  }

  gtk_widget_set_visible (self->dimming, progress < 1);
  gtk_widget_set_visible (self->shadow, progress < 1);
  gtk_widget_set_visible (self->border, progress < 1);
  gtk_widget_set_visible (self->outline, progress < 1);

  gtk_widget_measure (self->shadow, orientation, -1, &shadow_size, NULL, NULL, NULL);
  gtk_widget_measure (self->border, orientation, -1, &border_size, NULL, NULL, NULL);
  gtk_widget_measure (self->outline, orientation, -1, &outline_size, NULL, NULL, NULL);

  remaining_distance = (1 - progress) * (double) distance;
  if (remaining_distance < shadow_size)
    shadow_opacity = (remaining_distance / shadow_size);
  else
    shadow_opacity = 1;

  gtk_widget_set_opacity (self->dimming, 1 - progress);
  gtk_widget_set_opacity (self->shadow, shadow_opacity);

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
    gtk_widget_allocate (self->shadow, shadow_size, MAX (height, shadow_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
    gtk_widget_allocate (self->border, border_size, MAX (height, border_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
    gtk_widget_allocate (self->outline, outline_size, MAX (height, outline_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x - outline_size, y)));
    break;
  case GTK_PAN_DIRECTION_RIGHT:
    gtk_widget_allocate (self->shadow, shadow_size, MAX (height, shadow_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x + width - shadow_size, y)));
    gtk_widget_allocate (self->border, border_size, MAX (height, border_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x + width - border_size, y)));
    gtk_widget_allocate (self->outline, outline_size, MAX (height, outline_size), baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x + width, y)));
    break;
  case GTK_PAN_DIRECTION_UP:
    gtk_widget_allocate (self->shadow, MAX (width, shadow_size), shadow_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
    gtk_widget_allocate (self->border, MAX (width, border_size), border_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
    gtk_widget_allocate (self->outline, MAX (width, outline_size), outline_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y - outline_size)));
    break;
  case GTK_PAN_DIRECTION_DOWN:
    gtk_widget_allocate (self->shadow, MAX (width, shadow_size), shadow_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y + height - shadow_size)));
    gtk_widget_allocate (self->border, MAX (width, border_size), border_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y + height - border_size)));
    gtk_widget_allocate (self->outline, MAX (width, outline_size), outline_size, baseline,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y + height)));
    break;
  default:
    g_assert_not_reached ();
  }
}

void
adw_shadow_helper_snapshot (AdwShadowHelper *self,
                            GtkSnapshot     *snapshot)
{
  if (!gtk_widget_get_child_visible (self->dimming))
    return;

  gtk_widget_snapshot_child (self->widget, self->dimming, snapshot);
  gtk_widget_snapshot_child (self->widget, self->shadow, snapshot);
  gtk_widget_snapshot_child (self->widget, self->border, snapshot);
  gtk_widget_snapshot_child (self->widget, self->outline, snapshot);
}
