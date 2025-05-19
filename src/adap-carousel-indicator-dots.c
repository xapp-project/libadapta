/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-carousel-indicator-dots.h"

#include "adw-animation-util.h"
#include "adw-swipeable.h"
#include "adw-timed-animation.h"

#include <math.h>

#define DOTS_RADIUS 3
#define DOTS_RADIUS_SELECTED 4
#define DOTS_OPACITY 0.3
#define DOTS_OPACITY_SELECTED 0.9
#define DOTS_SPACING 7
#define DOTS_MARGIN 6

/**
 * AdwCarouselIndicatorDots:
 *
 * A dots indicator for [class@Carousel].
 *
 * <picture>
 *   <source srcset="carousel-indicator-dots-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="carousel-indicator-dots.png" alt="carousel-indicator-dots">
 * </picture>
 *
 * The `AdwCarouselIndicatorDots` widget shows a set of dots for each page of a
 * given [class@Carousel]. The dot representing the carousel's active page is
 * larger and more opaque than the others, the transition to the active and
 * inactive state is gradual to match the carousel's position.
 *
 * See also [class@CarouselIndicatorLines].
 *
 * ## CSS nodes
 *
 * `AdwCarouselIndicatorDots` has a single CSS node with name
 * `carouselindicatordots`.
 */

struct _AdwCarouselIndicatorDots
{
  GtkWidget parent_instance;

  AdwCarousel *carousel;
  GtkOrientation orientation;

  AdwAnimation *animation;
  GBinding *duration_binding;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwCarouselIndicatorDots, adw_carousel_indicator_dots, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_CAROUSEL,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_CAROUSEL + 1,
};

static GParamSpec *props[LAST_PROP];

static void
snapshot_dots (GtkWidget      *widget,
               GtkSnapshot    *snapshot,
               GtkOrientation  orientation,
               double          position,
               double         *sizes,
               guint           n_pages)
{
  GdkRGBA color;
  int i, widget_length, widget_thickness;
  double x, y, indicator_length, dot_size, full_size;
  double current_position, remaining_progress;
  graphene_rect_t rect;

  gtk_widget_get_color (widget, &color);
  dot_size = 2 * DOTS_RADIUS_SELECTED + DOTS_SPACING;

  indicator_length = -DOTS_SPACING;
  for (i = 0; i < n_pages; i++)
    indicator_length += dot_size * sizes[i];

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    widget_length = gtk_widget_get_width (widget);
    widget_thickness = gtk_widget_get_height (widget);
  } else {
    widget_length = gtk_widget_get_height (widget);
    widget_thickness = gtk_widget_get_width (widget);
  }

  /* Ensure the indicators are aligned to pixel grid when not animating */
  full_size = round (indicator_length / dot_size) * dot_size;
  if ((widget_length - (int) full_size) % 2 == 0)
    widget_length--;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    x = (widget_length - indicator_length) / 2.0;
    y = widget_thickness / 2;
  } else {
    x = widget_thickness / 2;
    y = (widget_length - indicator_length) / 2.0;
  }

  current_position = 0;
  remaining_progress = 1;

  graphene_rect_init (&rect, -DOTS_RADIUS, -DOTS_RADIUS, DOTS_RADIUS * 2, DOTS_RADIUS * 2);

  for (i = 0; i < n_pages; i++) {
    double progress, radius, opacity;
    GskRoundedRect clip;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      x += dot_size * sizes[i] / 2.0;
    else
      y += dot_size * sizes[i] / 2.0;

    current_position += sizes[i];

    progress = CLAMP (current_position - position, 0, remaining_progress);
    remaining_progress -= progress;

    radius = adw_lerp (DOTS_RADIUS, DOTS_RADIUS_SELECTED, progress) * sizes[i];
    opacity = adw_lerp (DOTS_OPACITY, DOTS_OPACITY_SELECTED, progress) * sizes[i];

    gsk_rounded_rect_init_from_rect (&clip, &rect, radius);

    gtk_snapshot_save (snapshot);
    gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (x, y));
    gtk_snapshot_scale (snapshot, radius / DOTS_RADIUS, radius / DOTS_RADIUS);

    gtk_snapshot_push_rounded_clip (snapshot, &clip);
    gtk_snapshot_push_opacity (snapshot, opacity);

    gtk_snapshot_append_color (snapshot, &color, &rect);

    gtk_snapshot_pop (snapshot);
    gtk_snapshot_pop (snapshot);

    gtk_snapshot_restore (snapshot);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      x += dot_size * sizes[i] / 2.0;
    else
      y += dot_size * sizes[i] / 2.0;
  }
}

static void
animation_cb (double     value,
              GtkWidget *self)
{
  gtk_widget_queue_resize (self);
}

static void
adw_carousel_indicator_dots_measure (GtkWidget      *widget,
                                     GtkOrientation  orientation,
                                     int             for_size,
                                     int            *minimum,
                                     int            *natural,
                                     int            *minimum_baseline,
                                     int            *natural_baseline)
{
  AdwCarouselIndicatorDots *self = ADW_CAROUSEL_INDICATOR_DOTS (widget);
  int size = 0;

  if (orientation == self->orientation) {
    int i, n_points = 0;
    double indicator_length, dot_size;
    double *points = NULL, *sizes;

    if (self->carousel)
      points = adw_swipeable_get_snap_points (ADW_SWIPEABLE (self->carousel), &n_points);

    sizes = g_new0 (double, n_points);

    if (n_points > 0)
      sizes[0] = points[0] + 1;
    for (i = 1; i < n_points; i++)
      sizes[i] = points[i] - points[i - 1];

    dot_size = 2 * DOTS_RADIUS_SELECTED + DOTS_SPACING;
    indicator_length = 0;
    for (i = 0; i < n_points; i++)
      indicator_length += dot_size * sizes[i];

    size = ceil (indicator_length);

    g_free (points);
    g_free (sizes);
  } else {
    size = 2 * DOTS_RADIUS_SELECTED;
  }

  size += 2 * DOTS_MARGIN;

  if (minimum)
    *minimum = size;

  if (natural)
    *natural = size;

  if (minimum_baseline)
    *minimum_baseline = -1;

  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_carousel_indicator_dots_snapshot (GtkWidget   *widget,
                                      GtkSnapshot *snapshot)
{
  AdwCarouselIndicatorDots *self = ADW_CAROUSEL_INDICATOR_DOTS (widget);
  int i, n_points;
  double position;
  double *points, *sizes;

  if (!self->carousel)
    return;

  points = adw_swipeable_get_snap_points (ADW_SWIPEABLE (self->carousel), &n_points);
  position = adw_carousel_get_position (self->carousel);

  if (n_points < 2) {
    g_free (points);

    return;
  }

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
      gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    position = points[n_points - 1] - position;

  sizes = g_new0 (double, n_points);

  sizes[0] = points[0] + 1;
  for (i = 1; i < n_points; i++)
    sizes[i] = points[i] - points[i - 1];

  snapshot_dots (widget, snapshot, self->orientation, position, sizes, n_points);

  g_free (sizes);
  g_free (points);
}

static void
adw_carousel_dispose (GObject *object)
{
  AdwCarouselIndicatorDots *self = ADW_CAROUSEL_INDICATOR_DOTS (object);

  adw_carousel_indicator_dots_set_carousel (self, NULL);

  g_clear_object (&self->animation);

  G_OBJECT_CLASS (adw_carousel_indicator_dots_parent_class)->dispose (object);
}

static void
adw_carousel_indicator_dots_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  AdwCarouselIndicatorDots *self = ADW_CAROUSEL_INDICATOR_DOTS (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    g_value_set_object (value, adw_carousel_indicator_dots_get_carousel (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_carousel_indicator_dots_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  AdwCarouselIndicatorDots *self = ADW_CAROUSEL_INDICATOR_DOTS (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    adw_carousel_indicator_dots_set_carousel (self, g_value_get_object (value));
    break;

  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (orientation != self->orientation) {
        self->orientation = orientation;
        gtk_widget_queue_resize (GTK_WIDGET (self));
        g_object_notify (G_OBJECT (self), "orientation");
      }
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_carousel_indicator_dots_class_init (AdwCarouselIndicatorDotsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_carousel_dispose;
  object_class->get_property = adw_carousel_indicator_dots_get_property;
  object_class->set_property = adw_carousel_indicator_dots_set_property;

  widget_class->measure = adw_carousel_indicator_dots_measure;
  widget_class->snapshot = adw_carousel_indicator_dots_snapshot;

  /**
   * AdwCarouselIndicatorDots:carousel: (attributes org.gtk.Property.get=adw_carousel_indicator_dots_get_carousel org.gtk.Property.set=adw_carousel_indicator_dots_set_carousel)
   *
   * The displayed carousel.
   */
  props[PROP_CAROUSEL] =
    g_param_spec_object ("carousel", NULL, NULL,
                         ADW_TYPE_CAROUSEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "carouselindicatordots");
}

static void
adw_carousel_indicator_dots_init (AdwCarouselIndicatorDots *self)
{
  AdwAnimationTarget *target
    = adw_callback_animation_target_new ((AdwAnimationTargetFunc) animation_cb,
                                         self, NULL);

  self->animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 1, 0, target);
}

/**
 * adw_carousel_indicator_dots_new:
 *
 * Creates a new `AdwCarouselIndicatorDots`.
 *
 * Returns: the newly created `AdwCarouselIndicatorDots`
 */
GtkWidget *
adw_carousel_indicator_dots_new (void)
{
  return g_object_new (ADW_TYPE_CAROUSEL_INDICATOR_DOTS, NULL);
}

/**
 * adw_carousel_indicator_dots_get_carousel: (attributes org.gtk.Method.get_property=carousel)
 * @self: an indicator
 *
 * Gets the displayed carousel.
 *
 * Returns: (nullable) (transfer none): the displayed carousel
 */
AdwCarousel *
adw_carousel_indicator_dots_get_carousel (AdwCarouselIndicatorDots *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL_INDICATOR_DOTS (self), NULL);

  return self->carousel;
}

/**
 * adw_carousel_indicator_dots_set_carousel: (attributes org.gtk.Method.set_property=carousel)
 * @self: an indicator
 * @carousel: (nullable): a carousel
 *
 * Sets the displayed carousel.
 */
void
adw_carousel_indicator_dots_set_carousel (AdwCarouselIndicatorDots *self,
                                          AdwCarousel              *carousel)
{
  g_return_if_fail (ADW_IS_CAROUSEL_INDICATOR_DOTS (self));
  g_return_if_fail (carousel == NULL || ADW_IS_CAROUSEL (carousel));

  if (self->carousel == carousel)
    return;

  adw_animation_reset (self->animation);

  if (self->carousel) {
    g_signal_handlers_disconnect_by_func (self->carousel,
                                          gtk_widget_queue_draw, self);
    g_signal_handlers_disconnect_by_func (self->carousel,
                                          adw_animation_play, self->animation);
    g_clear_object (&self->duration_binding);
  }

  g_set_object (&self->carousel, carousel);

  if (self->carousel) {
    g_signal_connect_object (self->carousel, "notify::position",
                             G_CALLBACK (gtk_widget_queue_draw), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->carousel, "notify::n-pages",
                             G_CALLBACK (adw_animation_play), self->animation,
                             G_CONNECT_SWAPPED);
    self->duration_binding =
      g_object_bind_property (self->carousel, "reveal-duration",
                              self->animation, "duration",
                              G_BINDING_SYNC_CREATE);
  }

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAROUSEL]);
}
