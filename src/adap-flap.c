/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-flap.h"

#include <math.h>

#include "adw-animation-util.h"
#include "adw-gizmo-private.h"
#include "adw-shadow-helper-private.h"
#include "adw-spring-animation.h"
#include "adw-swipeable.h"
#include "adw-swipe-tracker-private.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"

/**
 * AdwFlap:
 *
 * An adaptive container acting like a box or an overlay.
 *
 * <picture>
 *   <source srcset="flap-wide-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="flap-wide.png" alt="flap-wide">
 * </picture>
 * <picture>
 *   <source srcset="flap-narrow-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="flap-narrow.png" alt="flap-narrow">
 * </picture>
 *
 * The `AdwFlap` widget can display its children like a [class@Gtk.Box] does or
 * like a [class@Gtk.Overlay] does, according to the
 * [property@Flap:fold-policy] value.
 *
 * `AdwFlap` has at most three children: [property@Flap:content],
 * [property@Flap:flap] and [property@Flap:separator]. Content is the primary
 * child, flap is displayed next to it when unfolded, or overlays it when
 * folded. Flap can be shown or hidden by changing the
 * [property@Flap:reveal-flap] value, as well as via swipe gestures if
 * [property@Flap:swipe-to-open] and/or [property@Flap:swipe-to-close] are set
 * to `TRUE`.
 *
 * Optionally, a separator can be provided, which would be displayed between
 * the content and the flap when there's no shadow to separate them, depending
 * on the transition type.
 *
 * [property@Flap:flap] is transparent by default; add the
 * [`.background`](style-classes.html#background) style class to it if this is
 * unwanted.
 *
 * If [property@Flap:modal] is set to `TRUE`, content becomes completely
 * inaccessible when the flap is revealed while folded.
 *
 * The position of the flap and separator children relative to the content is
 * determined by orientation, as well as the [property@Flap:flap-position]
 * value.
 *
 * Folding the flap will automatically hide the flap widget, and unfolding it
 * will automatically reveal it. If this behavior is not desired, the
 * [property@Flap:locked] property can be used to override it.
 *
 * Common use cases include sidebars, header bars that need to be able to
 * overlap the window content (for example, in fullscreen mode) and bottom
 * sheets.
 *
 * ## AdwFlap as GtkBuildable
 *
 * The `AdwFlap` implementation of the [iface@Gtk.Buildable] interface supports
 * setting the flap child by specifying “flap” as the “type” attribute of a
 * `<child>` element, and separator by specifying “separator”. Specifying
 * “content” child type or omitting it results in setting the content child.
 *
 * ## CSS nodes
 *
 * `AdwFlap` has a single CSS node with name `flap`. The node will get the style
 * classes `.folded` when it is folded, and `.unfolded` when it's not.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */

/**
 * AdwFlapFoldPolicy:
 * @ADW_FLAP_FOLD_POLICY_NEVER: Disable folding, the flap cannot reach narrow
 *   sizes.
 * @ADW_FLAP_FOLD_POLICY_ALWAYS: Keep the flap always folded.
 * @ADW_FLAP_FOLD_POLICY_AUTO: Fold and unfold the flap based on available
 *   space.
 *
 * Describes the possible folding behavior of a [class@Flap] widget.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */

/**
 * AdwFlapTransitionType:
 * @ADW_FLAP_TRANSITION_TYPE_OVER: The flap slides over the content, which is
 *   dimmed. When folded, only the flap can be swiped.
 * @ADW_FLAP_TRANSITION_TYPE_UNDER: The content slides over the flap. Only the
 *   content can be swiped.
 * @ADW_FLAP_TRANSITION_TYPE_SLIDE: The flap slides offscreen when hidden,
 *   neither the flap nor content overlap each other. Both widgets can be
 *   swiped.
 *
 * Describes transitions types of a [class@Flap] widget.
 *
 * It determines the type of animation when transitioning between children in a
 * [class@Flap] widget, as well as which areas can be swiped via
 * [property@Flap:swipe-to-open] and [property@Flap:swipe-to-close].
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */

typedef struct {
  GtkWidget *widget;
  GtkAllocation allocation;
} ChildInfo;

struct _AdwFlap
{
  GtkWidget parent_instance;

  ChildInfo content;
  ChildInfo flap;
  ChildInfo separator;
  GtkWidget *shield;

  AdwFlapFoldPolicy fold_policy;
  AdwFoldThresholdPolicy fold_threshold_policy;
  AdwFlapTransitionType transition_type;
  GtkPackType flap_position;
  gboolean reveal_flap;
  gboolean locked;
  gboolean folded;

  guint fold_duration;
  double fold_progress;
  AdwAnimation *fold_animation;

  double reveal_progress;
  AdwAnimation *reveal_animation;

  gboolean schedule_fold;

  GtkOrientation orientation;

  AdwShadowHelper *shadow_helper;

  gboolean swipe_to_open;
  gboolean swipe_to_close;
  AdwSwipeTracker *tracker;
  gboolean swipe_active;

  gboolean modal;
  GtkEventController *shortcut_controller;
};

static void adw_flap_buildable_init (GtkBuildableIface *iface);
static void adw_flap_swipeable_init (AdwSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwFlap, adw_flap, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_flap_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADW_TYPE_SWIPEABLE, adw_flap_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_FLAP,
  PROP_SEPARATOR,
  PROP_FLAP_POSITION,
  PROP_REVEAL_FLAP,
  PROP_REVEAL_PARAMS,
  PROP_REVEAL_PROGRESS,
  PROP_FOLD_POLICY,
  PROP_FOLD_THRESHOLD_POLICY,
  PROP_FOLD_DURATION,
  PROP_FOLDED,
  PROP_LOCKED,
  PROP_TRANSITION_TYPE,
  PROP_MODAL,
  PROP_SWIPE_TO_OPEN,
  PROP_SWIPE_TO_CLOSE,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_ORIENTATION,
};

static GParamSpec *props[LAST_PROP];

static void
update_swipe_tracker (AdwFlap *self)
{
  gboolean reverse = self->flap_position == GTK_PACK_START;

  if (!self->tracker)
    return;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
      gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    reverse = !reverse;

  adw_swipe_tracker_set_enabled (self->tracker, self->flap.widget &&
                                 (self->swipe_to_open || self->swipe_to_close));
  adw_swipe_tracker_set_reversed (self->tracker, reverse);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->tracker),
                                  self->orientation);
}

static void
set_orientation (AdwFlap        *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_widget_queue_resize (GTK_WIDGET (self));
  update_swipe_tracker (self);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
update_child_visibility (AdwFlap *self)
{
  gboolean visible = self->reveal_progress > 0;

  if (self->flap.widget)
    gtk_widget_set_child_visible (self->flap.widget, visible);

  if (self->separator.widget)
    gtk_widget_set_child_visible (self->separator.widget, visible);

  if (self->fold_policy == ADW_FLAP_FOLD_POLICY_NEVER)
    gtk_widget_queue_resize (GTK_WIDGET (self));
  else
    gtk_widget_queue_allocate (GTK_WIDGET (self));
}


static void
update_shield (AdwFlap *self)
{
  if (self->shield)
    gtk_widget_set_child_visible (self->shield,
                                  self->modal &&
                                  self->fold_progress > 0 &&
                                  self->reveal_progress > 0);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
update_shortcuts (AdwFlap *self)
{
  gtk_event_controller_set_propagation_phase (self->shortcut_controller,
                                              self->modal ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (self->shortcut_controller),
                                     self->modal ? GTK_SHORTCUT_SCOPE_MANAGED : GTK_SHORTCUT_SCOPE_LOCAL);
}

static void
set_reveal_progress (double   progress,
                     AdwFlap *self)
{
  self->reveal_progress = progress;

  update_child_visibility (self);
  update_shield (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_PROGRESS]);
}

static void
fold_animation_value_cb (double   value,
                         AdwFlap *self)
{
  self->fold_progress = value;

  update_shield (self);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
animate_fold (AdwFlap *self)
{
  adw_timed_animation_set_value_from (ADW_TIMED_ANIMATION (self->fold_animation),
                                      self->fold_progress);
  adw_timed_animation_set_value_to (ADW_TIMED_ANIMATION (self->fold_animation),
                                    self->folded ? 1 : 0);

  /* When the flap is completely hidden, we can skip animation */
  adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->fold_animation),
                                    (self->reveal_progress > 0) ? self->fold_duration : 0);

  adw_animation_play (self->fold_animation);
}

static void
reveal_animation_done_cb (AdwFlap *self)
{
  if (self->schedule_fold) {
    self->schedule_fold = FALSE;

    animate_fold (self);
  }

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
animate_reveal (AdwFlap *self,
                double   to,
                double   velocity)
{
  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->reveal_animation),
                                       self->reveal_progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->reveal_animation), to);

  if (!G_APPROX_VALUE (self->reveal_progress, to, DBL_EPSILON))
    adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->reveal_animation),
                                               velocity / adw_swipeable_get_distance (ADW_SWIPEABLE (self)));
  else
    adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->reveal_animation),
                                               velocity);

  adw_animation_play (self->reveal_animation);
}

static void
set_reveal_flap (AdwFlap  *self,
                 gboolean  reveal_flap,
                 double    velocity)
{
  reveal_flap = !!reveal_flap;

  if (self->reveal_flap == reveal_flap)
    return;

  self->reveal_flap = reveal_flap;

  if (!self->swipe_active)
    animate_reveal (self, reveal_flap ? 1 : 0, velocity);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_FLAP]);
}

static void
set_folded (AdwFlap  *self,
            gboolean  folded)
{
  folded = !!folded;

  if (self->folded == folded)
    return;

  self->folded = folded;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

   /* When unlocked, folding should also hide flap. We don't want two concurrent
    * animations in this case, instead only animate reveal and schedule a fold
    * after it finishes, which will be skipped because the flap is fuly hidden.
    * Meanwhile if it's unfolding, animate folding immediately. */
  if (!self->locked && folded)
    self->schedule_fold = TRUE;
  else
    animate_fold (self);

  if (!self->locked)
    set_reveal_flap (self, !self->folded, 0);

  if (folded) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "folded");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "unfolded");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "folded");
    gtk_widget_add_css_class (GTK_WIDGET (self), "unfolded");
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLDED]);
}

static inline GtkPackType
get_start_or_end (AdwFlap *self)
{
  GtkTextDirection direction = gtk_widget_get_direction (GTK_WIDGET (self));
  gboolean is_rtl = direction == GTK_TEXT_DIR_RTL;
  gboolean is_horiz = self->orientation == GTK_ORIENTATION_HORIZONTAL;

  return (is_rtl && is_horiz) ? GTK_PACK_END : GTK_PACK_START;
}

static void
begin_swipe_cb (AdwSwipeTracker *tracker,
                AdwFlap         *self)
{
  if ((G_APPROX_VALUE (self->reveal_progress, 0, DBL_EPSILON) ||
       self->reveal_progress < 0) &&
      !self->swipe_to_open)
    return;

  if ((G_APPROX_VALUE (self->reveal_progress, 1, DBL_EPSILON) ||
       self->reveal_progress > 1) &&
      !self->swipe_to_close)
    return;

  adw_animation_pause (self->reveal_animation);

  self->swipe_active = TRUE;
}

static void
update_swipe_cb (AdwSwipeTracker *tracker,
                 double           progress,
                 AdwFlap         *self)
{
  set_reveal_progress (progress, self);
}

static void
end_swipe_cb (AdwSwipeTracker *tracker,
              double           velocity,
              double           to,
              AdwFlap         *self)
{
  if (!self->swipe_active)
    return;

  self->swipe_active = FALSE;

  if ((to > 0) == self->reveal_flap)
    animate_reveal (self, to, velocity);
  else
    set_reveal_flap (self, to > 0, velocity);
}

static void
released_cb (GtkGestureClick *gesture,
             int              n_press,
             double           x,
             double           y,
             AdwFlap         *self)
{
  adw_flap_set_reveal_flap (self, FALSE);
}

static gboolean
transition_is_content_above_flap (AdwFlap *self)
{
  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
    return FALSE;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    return TRUE;

  default:
    g_assert_not_reached ();
  }
}

static gboolean
transition_should_clip (AdwFlap *self)
{
  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    return FALSE;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
    return TRUE;

  default:
    g_assert_not_reached ();
  }
}

static double
transition_get_content_motion_factor (AdwFlap *self)
{
  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
    return 0;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    return 1;

  default:
    g_assert_not_reached ();
  }
}

static double
transition_get_flap_motion_factor (AdwFlap *self)
{
  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    return 1;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
    return 0;

  default:
    g_assert_not_reached ();
  }
}

static void
restack_children (AdwFlap *self)
{
  if (transition_is_content_above_flap (self)) {
    if (self->flap.widget)
      gtk_widget_insert_before (self->flap.widget, GTK_WIDGET (self), NULL);
    if (self->separator.widget)
      gtk_widget_insert_before (self->separator.widget, GTK_WIDGET (self), NULL);
    if (self->content.widget)
      gtk_widget_insert_before (self->content.widget, GTK_WIDGET (self), NULL);
    if (self->shield)
      gtk_widget_insert_before (self->shield, GTK_WIDGET (self), NULL);
  } else {
    if (self->flap.widget)
      gtk_widget_insert_after (self->flap.widget, GTK_WIDGET (self), NULL);
    if (self->separator.widget)
      gtk_widget_insert_after (self->separator.widget, GTK_WIDGET (self), NULL);
    if (self->shield)
      gtk_widget_insert_after (self->shield, GTK_WIDGET (self), NULL);
    if (self->content.widget)
      gtk_widget_insert_after (self->content.widget, GTK_WIDGET (self), NULL);
  }
}

static void
add_child (AdwFlap   *self,
           ChildInfo *info)
{
  gtk_widget_set_parent (info->widget, GTK_WIDGET (self));

  restack_children (self);
}

static void
remove_child (AdwFlap   *self,
              ChildInfo *info)
{
  gtk_widget_unparent (info->widget);
}

static inline void
get_preferred_size (GtkWidget      *widget,
                    GtkOrientation  orientation,
                    int            *min,
                    int            *nat)
{
  gtk_widget_measure (widget, orientation, -1, min, nat, NULL, NULL);
}

static void
compute_sizes (AdwFlap  *self,
               int       width,
               int       height,
               gboolean  folded,
               gboolean  revealed,
               int      *flap_size,
               int      *content_size,
               int      *separator_size)
{
  gboolean flap_expand, content_expand;
  int total, extra;
  int flap_nat, content_nat;

  if (!self->flap.widget && !self->content.widget)
    return;

  if (self->separator.widget)
    get_preferred_size (self->separator.widget, self->orientation, separator_size, NULL);
  else
    *separator_size = 0;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    total = width;
  else
    total = height;

  if (!self->flap.widget) {
    *content_size = total;
    *flap_size = 0;

    return;
  }

  if (!self->content.widget) {
    *content_size = 0;
    *flap_size = total;

    return;
  }

  get_preferred_size (self->flap.widget, self->orientation, flap_size, &flap_nat);
  get_preferred_size (self->content.widget, self->orientation, content_size, &content_nat);

  flap_expand = gtk_widget_compute_expand (self->flap.widget, self->orientation);
  content_expand = gtk_widget_compute_expand (self->content.widget, self->orientation);

  if (folded) {
    *content_size = total;

    if (flap_expand) {
      *flap_size = total;
    } else {
      get_preferred_size (self->flap.widget, self->orientation, NULL, flap_size);
      *flap_size = MIN (*flap_size, total);
    }

    return;
  }

  if (revealed)
    total -= *separator_size;

  if (flap_expand && content_expand) {
    *flap_size = MAX (total / 2, *flap_size);

    if (!revealed)
      *content_size = total;
    else
      *content_size = total - *flap_size;

    return;
  }

  extra = total - *content_size - *flap_size;

  if (extra > 0 && flap_expand) {
    *flap_size += extra;

    if (!revealed)
      *content_size = total;

    return;
  }

  if (extra > 0 && content_expand) {
    *content_size += extra;
    extra = 0;
  }

  if (extra > 0) {
    GtkRequestedSize sizes[2];

    sizes[0].data = self->flap.widget;
    sizes[0].minimum_size = *flap_size;
    sizes[0].natural_size = flap_nat;

    sizes[1].data = self->content.widget;
    sizes[1].minimum_size = *content_size;
    sizes[1].natural_size = content_nat;

    extra = gtk_distribute_natural_allocation (extra, 2, sizes);

    *flap_size = sizes[0].minimum_size;
    *content_size = sizes[1].minimum_size + extra;
  }

  if (!revealed)
    *content_size = total;
}

static inline void
interpolate_reveal (AdwFlap  *self,
                    int       width,
                    int       height,
                    gboolean  folded,
                    int      *flap_size,
                    int      *content_size,
                    int      *separator_size)
{
  
  if (G_APPROX_VALUE (self->reveal_progress, 0, DBL_EPSILON) || self->reveal_progress < 0) {
    compute_sizes (self, width, height, folded, FALSE, flap_size, content_size, separator_size);
  } else if (G_APPROX_VALUE (self->reveal_progress, 1, DBL_EPSILON) || self->reveal_progress > 1) {
    compute_sizes (self, width, height, folded, TRUE, flap_size, content_size, separator_size);
  } else {
    int flap_revealed, content_revealed, separator_revealed;
    int flap_hidden, content_hidden, separator_hidden;

    compute_sizes (self, width, height, folded, TRUE, &flap_revealed, &content_revealed, &separator_revealed);
    compute_sizes (self, width, height, folded, FALSE, &flap_hidden, &content_hidden, &separator_hidden);

    *flap_size =
      (int) round (adw_lerp (flap_hidden, flap_revealed,
                              self->reveal_progress));
    *content_size =
      (int) round (adw_lerp (content_hidden, content_revealed,
                              self->reveal_progress));
    *separator_size =
      (int) round (adw_lerp (separator_hidden, separator_revealed,
                              self->reveal_progress));
  }
}

static inline void
interpolate_fold (AdwFlap *self,
                  int      width,
                  int      height,
                  int     *flap_size,
                  int     *content_size,
                  int     *separator_size)
{
  if (G_APPROX_VALUE (self->fold_progress, 0, DBL_EPSILON) || self->fold_progress < 0) {
    interpolate_reveal (self, width, height, FALSE, flap_size, content_size, separator_size);
  } else if (G_APPROX_VALUE (self->fold_progress, 1, DBL_EPSILON) || self->fold_progress > 1) {
    interpolate_reveal (self, width, height, TRUE, flap_size, content_size, separator_size);
  } else {
    int flap_folded, content_folded, separator_folded;
    int flap_unfolded, content_unfolded, separator_unfolded;

    interpolate_reveal (self, width, height, TRUE, &flap_folded, &content_folded, &separator_folded);
    interpolate_reveal (self, width, height, FALSE, &flap_unfolded, &content_unfolded, &separator_unfolded);

    *flap_size =
      (int) round (adw_lerp (flap_unfolded, flap_folded,
                              self->fold_progress));
    *content_size =
      (int) round (adw_lerp (content_unfolded, content_folded,
                              self->fold_progress));
    *separator_size =
      (int) round (adw_lerp (separator_unfolded, separator_folded,
                              self->fold_progress));
  }
}

static void
compute_allocation (AdwFlap       *self,
                    int            width,
                    int            height,
                    GtkAllocation *flap_alloc,
                    GtkAllocation *content_alloc,
                    GtkAllocation *separator_alloc)
{
  double distance;
  int content_size, flap_size, separator_size;
  int total, content_pos, flap_pos, separator_pos;
  gboolean content_above_flap = transition_is_content_above_flap (self);

  if (!self->flap.widget && !self->content.widget && !self->separator.widget)
    return;

  content_alloc->x = 0;
  content_alloc->y = 0;
  flap_alloc->x = 0;
  flap_alloc->y = 0;
  separator_alloc->x = 0;
  separator_alloc->y = 0;

  interpolate_fold (self, width, height, &flap_size, &content_size, &separator_size);

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    flap_alloc->width = flap_size;
    content_alloc->width = content_size;
    separator_alloc->width = separator_size;
    flap_alloc->height = content_alloc->height = separator_alloc->height = height;
    total = width;
  } else {
    flap_alloc->height = flap_size;
    content_alloc->height = content_size;
    separator_alloc->height = separator_size;
    flap_alloc->width = content_alloc->width = separator_alloc->width = width;
    total = height;
  }

  if (!self->flap.widget)
    return;

  if (content_above_flap)
    distance = flap_size + separator_size;
  else
    distance = flap_size + separator_size * (1 - self->fold_progress);

  flap_pos = -(int) round ((1 - self->reveal_progress) * transition_get_flap_motion_factor (self) * distance);

  if (content_above_flap) {
    content_pos = (int) round (self->reveal_progress * transition_get_content_motion_factor (self) * distance);
    separator_pos = flap_pos + flap_size;
  } else {
    content_pos = total - content_size + (int) round (self->reveal_progress * self->fold_progress * transition_get_content_motion_factor (self) * distance);
    separator_pos = content_pos - separator_size;
  }

  if (self->flap_position != get_start_or_end (self)) {
    flap_pos = total - flap_pos - flap_size;
    separator_pos = total - separator_pos - separator_size;
    content_pos = total - content_pos - content_size;
  }

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    content_alloc->x = content_pos;
    flap_alloc->x = flap_pos;
    separator_alloc->x = separator_pos;
  } else {
    content_alloc->y = content_pos;
    flap_alloc->y = flap_pos;
    separator_alloc->y = separator_pos;
  }
}

static inline void
allocate_child (AdwFlap   *self,
                ChildInfo *info,
                int        baseline)
{
  if (!info->widget || !gtk_widget_should_layout (info->widget))
    return;

  gtk_widget_size_allocate (info->widget, &info->allocation, baseline);
}

static void
allocate_shadow (AdwFlap *self,
                 int      width,
                 int      height,
                 int      baseline)
{
  double shadow_progress;
  GtkAllocation *shadow_alloc;
  GtkPanDirection shadow_direction;
  int shadow_x = 0, shadow_y = 0;
  gboolean content_above_flap = transition_is_content_above_flap (self);

  if (!self->flap.widget)
    return;

  shadow_alloc = content_above_flap ? &self->content.allocation : &self->flap.allocation;

  if (self->orientation == GTK_ORIENTATION_VERTICAL) {
    if ((self->flap_position == GTK_PACK_START) != content_above_flap) {
      shadow_direction = GTK_PAN_DIRECTION_UP;
      shadow_y = shadow_alloc->y + shadow_alloc->height;
    } else {
      shadow_direction = GTK_PAN_DIRECTION_DOWN;
      shadow_y = shadow_alloc->y - height;
    }
  } else {
    if ((self->flap_position == get_start_or_end (self)) != content_above_flap) {
      shadow_direction = GTK_PAN_DIRECTION_LEFT;
      shadow_x = shadow_alloc->x + shadow_alloc->width;
    } else {
      shadow_direction = GTK_PAN_DIRECTION_RIGHT;
      shadow_x = shadow_alloc->x - width;
    }
  }

  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
    shadow_progress = 1 - MIN (self->reveal_progress, self->fold_progress);
    break;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
    shadow_progress = self->reveal_progress;
    break;

  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    shadow_progress = 1;
    break;

  default:
    g_assert_not_reached ();
  }

  adw_shadow_helper_size_allocate (self->shadow_helper, width, height,
                                   baseline, shadow_x, shadow_y,
                                   shadow_progress, shadow_direction);
}

static void
adw_flap_size_allocate (GtkWidget *widget,
                        int        width,
                        int        height,
                        int        baseline)
{
  AdwFlap *self = ADW_FLAP (widget);

  if (self->fold_policy == ADW_FLAP_FOLD_POLICY_AUTO) {
    GtkRequisition flap_size = { 0, 0 };
    GtkRequisition content_size = { 0, 0 };
    GtkRequisition separator_size = { 0, 0 };

    if (self->fold_threshold_policy == ADW_FOLD_THRESHOLD_POLICY_MINIMUM) {
      if (self->flap.widget)
        gtk_widget_get_preferred_size (self->flap.widget, &flap_size, NULL);
      if (self->content.widget)
        gtk_widget_get_preferred_size (self->content.widget, &content_size, NULL);
      if (self->separator.widget)
        gtk_widget_get_preferred_size (self->separator.widget, &separator_size, NULL);
    } else {
      if (self->flap.widget)
        gtk_widget_get_preferred_size (self->flap.widget, NULL, &flap_size);
      if (self->content.widget)
        gtk_widget_get_preferred_size (self->content.widget, NULL, &content_size);
      if (self->separator.widget)
        gtk_widget_get_preferred_size (self->separator.widget, NULL, &separator_size);
    }

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
      set_folded (self, width < content_size.width + flap_size.width + separator_size.width);
    else
      set_folded (self, height < content_size.height + flap_size.height + separator_size.height);
  }

  compute_allocation (self,
                      width,
                      height,
                      &self->flap.allocation,
                      &self->content.allocation,
                      &self->separator.allocation);

  allocate_child (self, &self->content, baseline);
  allocate_child (self, &self->separator, baseline);
  allocate_child (self, &self->flap, baseline);

  if (gtk_widget_should_layout (self->shield))
    gtk_widget_size_allocate (self->shield, &self->content.allocation, baseline);

  allocate_shadow (self, width, height, baseline);
}

static void
adw_flap_measure (GtkWidget      *widget,
                  GtkOrientation  orientation,
                  int             for_size,
                  int            *minimum,
                  int            *natural,
                  int            *minimum_baseline,
                  int            *natural_baseline)
{
  AdwFlap *self = ADW_FLAP (widget);

  int content_min = 0, content_nat = 0;
  int flap_min = 0, flap_nat = 0;
  int separator_min = 0, separator_nat = 0;
  int min, nat;

  if (self->content.widget)
    get_preferred_size (self->content.widget, orientation, &content_min, &content_nat);

  if (self->flap.widget)
    get_preferred_size (self->flap.widget, orientation, &flap_min, &flap_nat);

  if (self->separator.widget)
    get_preferred_size (self->separator.widget, orientation, &separator_min, &separator_nat);

  if (self->orientation == orientation) {
    double min_progress, nat_progress;

    switch (self->fold_policy) {
    case ADW_FLAP_FOLD_POLICY_NEVER:
      min_progress = (1 - self->fold_progress) * self->reveal_progress;
      nat_progress = min_progress;
      break;

    case ADW_FLAP_FOLD_POLICY_ALWAYS:
      min_progress = 0;
      nat_progress = 0;
      break;

    case ADW_FLAP_FOLD_POLICY_AUTO:
      min_progress = 0;
      nat_progress = self->locked ? self->reveal_progress : 1;
      break;

    default:
      g_assert_not_reached ();
    }

    min = MAX (content_min + (int) round ((flap_min + separator_min) * min_progress), flap_min);
    nat = MAX (content_nat + (int) round ((flap_nat + separator_nat) * nat_progress), flap_nat);
  } else {
    min = MAX (MAX (content_min, flap_min), separator_min);
    nat = MAX (MAX (content_nat, flap_nat), separator_nat);
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
adw_flap_snapshot (GtkWidget   *widget,
                   GtkSnapshot *snapshot)
{
  AdwFlap *self = ADW_FLAP (widget);
  int width, height;
  int shadow_x = 0, shadow_y = 0;
  double shadow_progress;
  gboolean content_above_flap = transition_is_content_above_flap (self);
  GtkAllocation *shadow_alloc;
  gboolean should_clip;

  shadow_alloc = content_above_flap ? &self->content.allocation : &self->flap.allocation;

  width = gtk_widget_get_width (widget);
  height = gtk_widget_get_height (widget);

  if (self->orientation == GTK_ORIENTATION_VERTICAL) {
    if ((self->flap_position == GTK_PACK_START) != content_above_flap)
      shadow_y = shadow_alloc->y + shadow_alloc->height;
    else
      shadow_y = shadow_alloc->y - height;
  } else {
    if ((self->flap_position == get_start_or_end (self)) != content_above_flap)
      shadow_x = shadow_alloc->x + shadow_alloc->width;
    else
      shadow_x = shadow_alloc->x - width;
  }

  switch (self->transition_type) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
    shadow_progress = 1 - MIN (self->reveal_progress, self->fold_progress);
    break;

  case ADW_FLAP_TRANSITION_TYPE_UNDER:
    shadow_progress = self->reveal_progress;
    break;

  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    shadow_progress = 1;
    break;

  default:
    g_assert_not_reached ();
  }

  should_clip = transition_should_clip (self) &&
                shadow_progress < 1 &&
                self->reveal_progress > 0;

  if (should_clip)
    gtk_snapshot_push_clip (snapshot,
                            &GRAPHENE_RECT_INIT (shadow_x,
                                                 shadow_y,
                                                 width,
                                                 height));

  if (!content_above_flap) {
    if (self->content.widget)
      gtk_widget_snapshot_child (widget, self->content.widget, snapshot);

    if (self->separator.widget)
      gtk_widget_snapshot_child (widget, self->separator.widget, snapshot);

    if (should_clip)
      gtk_snapshot_pop (snapshot);
  }

  if (self->flap.widget)
    gtk_widget_snapshot_child (widget, self->flap.widget, snapshot);

  if (content_above_flap) {
    if (self->separator.widget)
      gtk_widget_snapshot_child (widget, self->separator.widget, snapshot);

    if (should_clip)
      gtk_snapshot_pop (snapshot);

    if (self->content.widget)
      gtk_widget_snapshot_child (widget, self->content.widget, snapshot);
  }

  adw_shadow_helper_snapshot (self->shadow_helper, snapshot);
}

static void
adw_flap_direction_changed (GtkWidget        *widget,
                            GtkTextDirection  previous_direction)
{
  AdwFlap *self = ADW_FLAP (widget);

  update_swipe_tracker (self);

  GTK_WIDGET_CLASS (adw_flap_parent_class)->direction_changed (widget,
                                                               previous_direction);
}

static void
adw_flap_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  AdwFlap *self = ADW_FLAP (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_flap_get_content (self));
    break;
  case PROP_FLAP:
    g_value_set_object (value, adw_flap_get_flap (self));
    break;
  case PROP_SEPARATOR:
    g_value_set_object (value, adw_flap_get_separator (self));
    break;
  case PROP_FLAP_POSITION:
    g_value_set_enum (value, adw_flap_get_flap_position (self));
    break;
  case PROP_REVEAL_FLAP:
    g_value_set_boolean (value, adw_flap_get_reveal_flap (self));
    break;
  case PROP_REVEAL_PARAMS:
    g_value_set_boxed (value, adw_flap_get_reveal_params (self));
    break;
  case PROP_REVEAL_PROGRESS:
    g_value_set_double (value, adw_flap_get_reveal_progress (self));
    break;
  case PROP_FOLD_POLICY:
    g_value_set_enum (value, adw_flap_get_fold_policy (self));
    break;
  case PROP_FOLD_THRESHOLD_POLICY:
    g_value_set_enum (value, adw_flap_get_fold_threshold_policy (self));
    break;
  case PROP_FOLD_DURATION:
    g_value_set_uint (value, adw_flap_get_fold_duration (self));
    break;
  case PROP_FOLDED:
    g_value_set_boolean (value, adw_flap_get_folded (self));
    break;
  case PROP_LOCKED:
    g_value_set_boolean (value, adw_flap_get_locked (self));
    break;
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, adw_flap_get_transition_type (self));
    break;
  case PROP_MODAL:
    g_value_set_boolean (value, adw_flap_get_modal (self));
    break;
  case PROP_SWIPE_TO_OPEN:
    g_value_set_boolean (value, adw_flap_get_swipe_to_open (self));
    break;
  case PROP_SWIPE_TO_CLOSE:
    g_value_set_boolean (value, adw_flap_get_swipe_to_close (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_flap_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  AdwFlap *self = ADW_FLAP (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_flap_set_content (self, g_value_get_object (value));
    break;
  case PROP_FLAP:
    adw_flap_set_flap (self, g_value_get_object (value));
    break;
  case PROP_SEPARATOR:
    adw_flap_set_separator (self, g_value_get_object (value));
    break;
  case PROP_FLAP_POSITION:
    adw_flap_set_flap_position (self, g_value_get_enum (value));
    break;
  case PROP_REVEAL_FLAP:
    adw_flap_set_reveal_flap (self, g_value_get_boolean (value));
    break;
  case PROP_REVEAL_PARAMS:
    adw_flap_set_reveal_params (self, g_value_get_boxed (value));
    break;
  case PROP_FOLD_POLICY:
    adw_flap_set_fold_policy (self, g_value_get_enum (value));
    break;
  case PROP_FOLD_THRESHOLD_POLICY:
    adw_flap_set_fold_threshold_policy (self, g_value_get_enum (value));
    break;
  case PROP_FOLD_DURATION:
    adw_flap_set_fold_duration (self, g_value_get_uint (value));
    break;
  case PROP_LOCKED:
    adw_flap_set_locked (self, g_value_get_boolean (value));
    break;
  case PROP_TRANSITION_TYPE:
    adw_flap_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_MODAL:
    adw_flap_set_modal (self, g_value_get_boolean (value));
    break;
  case PROP_SWIPE_TO_OPEN:
    adw_flap_set_swipe_to_open (self, g_value_get_boolean (value));
    break;
  case PROP_SWIPE_TO_CLOSE:
    adw_flap_set_swipe_to_close (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_flap_dispose (GObject *object)
{
  AdwFlap *self = ADW_FLAP (object);

  g_clear_pointer (&self->flap.widget, gtk_widget_unparent);
  g_clear_pointer (&self->separator.widget, gtk_widget_unparent);
  g_clear_pointer (&self->content.widget, gtk_widget_unparent);
  g_clear_pointer (&self->shield, gtk_widget_unparent);

  g_clear_object (&self->shadow_helper);
  g_clear_object (&self->tracker);
  g_clear_object (&self->fold_animation);
  g_clear_object (&self->reveal_animation);

  self->shortcut_controller = NULL;

  G_OBJECT_CLASS (adw_flap_parent_class)->dispose (object);
}

static void
adw_flap_class_init (AdwFlapClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_flap_get_property;
  object_class->set_property = adw_flap_set_property;
  object_class->dispose = adw_flap_dispose;

  widget_class->measure = adw_flap_measure;
  widget_class->size_allocate = adw_flap_size_allocate;
  widget_class->snapshot = adw_flap_snapshot;
  widget_class->direction_changed = adw_flap_direction_changed;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwFlap:content: (attributes org.gtk.Property.get=adw_flap_get_content org.gtk.Property.set=adw_flap_set_content)
   *
   * The content widget.
   *
   * It's always displayed when unfolded, and partially visible when folded.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:flap: (attributes org.gtk.Property.get=adw_flap_get_flap org.gtk.Property.set=adw_flap_set_flap)
   *
   * The flap widget.
   *
   * It's only visible when [property@Flap:reveal-progress] is greater than 0.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FLAP] =
    g_param_spec_object ("flap", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:separator: (attributes org.gtk.Property.get=adw_flap_get_separator org.gtk.Property.set=adw_flap_set_separator)
   *
   * The separator widget.
   *
   * It's displayed between content and flap when there's no shadow to display.
   * When exactly it's visible depends on the [property@Flap:transition-type]
   * value.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_SEPARATOR] =
    g_param_spec_object ("separator", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:flap-position: (attributes org.gtk.Property.get=adw_flap_get_flap_position org.gtk.Property.set=adw_flap_set_flap_position)
   *
   * The flap position.
   *
   * If it's set to `GTK_PACK_START`, the flap is displayed before the content,
   * if `GTK_PACK_END`, it's displayed after the content.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FLAP_POSITION] =
    g_param_spec_enum ("flap-position", NULL, NULL,
                       GTK_TYPE_PACK_TYPE,
                       GTK_PACK_START,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:reveal-flap: (attributes org.gtk.Property.get=adw_flap_get_reveal_flap org.gtk.Property.set=adw_flap_set_reveal_flap)
   *
   * Whether the flap widget is revealed.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_REVEAL_FLAP] =
    g_param_spec_boolean ("reveal-flap", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:reveal-params: (attributes org.gtk.Property.get=adw_flap_get_reveal_params org.gtk.Property.set=adw_flap_set_reveal_params)
   *
   * The reveal animation spring parameters.
   *
   * The default value is equivalent to:
   *
   * ```c
   * adw_spring_params_new (1, 0.5, 500)
   * ```
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_REVEAL_PARAMS] =
    g_param_spec_boxed ("reveal-params", NULL, NULL,
                        ADW_TYPE_SPRING_PARAMS,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:reveal-progress: (attributes org.gtk.Property.get=adw_flap_get_reveal_progress)
   *
   * The current reveal transition progress.
   *
   * 0 means fully hidden, 1 means fully revealed.
   *
   * See [property@Flap:reveal-flap].
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_REVEAL_PROGRESS] =
    g_param_spec_double ("reveal-progress", NULL, NULL,
                          0.0, 1.0, 1.0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:fold-policy: (attributes org.gtk.Property.get=adw_flap_get_fold_policy org.gtk.Property.set=adw_flap_set_fold_policy)
   *
   * The fold policy for the flap.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FOLD_POLICY] =
    g_param_spec_enum ("fold-policy", NULL, NULL,
                       ADW_TYPE_FLAP_FOLD_POLICY,
                       ADW_FLAP_FOLD_POLICY_AUTO,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:fold-threshold-policy: (attributes org.gtk.Property.get=adw_flap_get_fold_threshold_policy org.gtk.Property.set=adw_flap_set_fold_threshold_policy)
   *
   * Determines when the flap will fold.
   *
   * If set to `ADW_FOLD_THRESHOLD_POLICY_MINIMUM`, flap will only fold when
   * the children cannot fit anymore. With `ADW_FOLD_THRESHOLD_POLICY_NATURAL`,
   * it will fold as soon as children don't get their natural size.
   *
   * This can be useful if you have a long ellipsizing label and want to let it
   * ellipsize instead of immediately folding.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FOLD_THRESHOLD_POLICY] =
    g_param_spec_enum ("fold-threshold-policy", NULL, NULL,
                       ADW_TYPE_FOLD_THRESHOLD_POLICY,
                       ADW_FOLD_THRESHOLD_POLICY_MINIMUM,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:fold-duration: (attributes org.gtk.Property.get=adw_flap_get_fold_duration org.gtk.Property.set=adw_flap_set_fold_duration)
   *
   * The fold transition animation duration, in milliseconds.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FOLD_DURATION] =
    g_param_spec_uint ("fold-duration", NULL, NULL,
                       0, G_MAXINT,
                       250,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:folded: (attributes org.gtk.Property.get=adw_flap_get_folded)
   *
   * Whether the flap is currently folded.
   *
   * See [property@Flap:fold-policy].
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_FOLDED] =
    g_param_spec_boolean ("folded", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:locked: (attributes org.gtk.Property.get=adw_flap_get_locked org.gtk.Property.set=adw_flap_set_locked)
   *
   * Whether the flap is locked.
   *
   * If `FALSE`, folding when the flap is revealed automatically closes it, and
   * unfolding it when the flap is not revealed opens it. If `TRUE`,
   * [property@Flap:reveal-flap] value never changes on its own.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_LOCKED] =
    g_param_spec_boolean ("locked", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:transition-type: (attributes org.gtk.Property.get=adw_flap_get_transition_type org.gtk.Property.set=adw_flap_set_transition_type)
   *
   * the type of animation used for reveal and fold transitions.
   *
   * [property@Flap:flap] is transparent by default, which means the content
   * will be seen through it with `ADW_FLAP_TRANSITION_TYPE_OVER` transitions;
   * add the [`.background`](style-classes.html#background) style class to it if
   * this is unwanted.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type", NULL, NULL,
                       ADW_TYPE_FLAP_TRANSITION_TYPE,
                       ADW_FLAP_TRANSITION_TYPE_OVER,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:modal: (attributes org.gtk.Property.get=adw_flap_get_modal org.gtk.Property.set=adw_flap_set_modal)
   *
   * Whether the flap is modal.
   *
   * If `TRUE`, clicking the content widget while flap is revealed, as well as
   * pressing the <kbd>Esc</kbd> key, will close the flap. If `FALSE`, clicks
   * are passed through to the content widget.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_MODAL] =
    g_param_spec_boolean ("modal", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:swipe-to-open: (attributes org.gtk.Property.get=adw_flap_get_swipe_to_open org.gtk.Property.set=adw_flap_set_swipe_to_open)
   *
   * Whether the flap can be opened with a swipe gesture.
   *
   * The area that can be swiped depends on the [property@Flap:transition-type]
   * value.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_SWIPE_TO_OPEN] =
    g_param_spec_boolean ("swipe-to-open", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwFlap:swipe-to-close: (attributes org.gtk.Property.get=adw_flap_get_swipe_to_close org.gtk.Property.set=adw_flap_set_swipe_to_close)
   *
   * Whether the flap can be closed with a swipe gesture.
   *
   * The area that can be swiped depends on the [property@Flap:transition-type]
   * value.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
   */
  props[PROP_SWIPE_TO_CLOSE] =
    g_param_spec_boolean ("swipe-to-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  gtk_widget_class_set_css_name (widget_class, "flap");
}

static gboolean
flap_close_cb (AdwFlap *self)
{
  if (G_APPROX_VALUE (self->reveal_progress, 0, DBL_EPSILON) ||
      self->reveal_progress < 0 ||
      G_APPROX_VALUE (self->fold_progress, 0, DBL_EPSILON) ||
      self->fold_progress < 0)
    return GDK_EVENT_PROPAGATE;

  adw_flap_set_reveal_flap (ADW_FLAP (self), FALSE);

  return GDK_EVENT_STOP;
}

static void
adw_flap_init (AdwFlap *self)
{
  GtkEventController *gesture;
  GtkShortcut *shortcut;
  AdwAnimationTarget *target;

  self->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->flap_position = GTK_PACK_START;
  self->fold_policy = ADW_FLAP_FOLD_POLICY_AUTO;
  self->fold_threshold_policy = ADW_FOLD_THRESHOLD_POLICY_MINIMUM;
  self->transition_type = ADW_FLAP_TRANSITION_TYPE_OVER;
  self->reveal_flap = TRUE;
  self->locked = FALSE;
  self->reveal_progress = 1;
  self->folded = FALSE;
  self->fold_progress = 0;
  self->fold_duration = 250;
  self->modal = TRUE;
  self->swipe_to_open = TRUE;
  self->swipe_to_close = TRUE;

  self->shadow_helper = adw_shadow_helper_new (GTK_WIDGET (self));
  self->tracker = adw_swipe_tracker_new (ADW_SWIPEABLE (self));
  adw_swipe_tracker_set_enabled (self->tracker, FALSE);

  g_signal_connect_object (self->tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self, 0);

  update_swipe_tracker (self);

  self->shield = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_parent (self->shield, GTK_WIDGET (self));

  gesture = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture),
                                              GTK_PHASE_CAPTURE);
  g_signal_connect_object (gesture, "released", G_CALLBACK (released_cb), self, 0);
  gtk_widget_add_controller (self->shield, gesture);

  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape, 0),
                               gtk_callback_action_new ((GtkShortcutFunc) flap_close_cb, NULL, NULL));

  self->shortcut_controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (self->shortcut_controller),
                                        shortcut);
  gtk_widget_add_controller (GTK_WIDGET (self), self->shortcut_controller);

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  gtk_widget_add_css_class (GTK_WIDGET (self), "unfolded");

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              fold_animation_value_cb,
                                              self, NULL);
  self->fold_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 0, 0, target);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              set_reveal_progress,
                                              self, NULL);
  self->reveal_animation =
    adw_spring_animation_new (GTK_WIDGET (self), 0, 0,
                             adw_spring_params_new (1, 0.5, 500), target);
  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->reveal_animation),
                                  TRUE);

  g_signal_connect_swapped (self->reveal_animation, "done",
                            G_CALLBACK (reveal_animation_done_cb), self);

  update_shortcuts (self);
  update_shield (self);
}

static void
adw_flap_add_child (GtkBuildable *buildable,
                    GtkBuilder   *builder,
                    GObject      *child,
                    const char   *type)
{
  if (!g_strcmp0 (type, "content"))
    adw_flap_set_content (ADW_FLAP (buildable), GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "flap"))
    adw_flap_set_flap (ADW_FLAP (buildable), GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "separator"))
    adw_flap_set_separator (ADW_FLAP (buildable), GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_flap_set_content (ADW_FLAP (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_flap_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_flap_add_child;
}

static double
adw_flap_get_distance (AdwSwipeable *swipeable)
{
  AdwFlap *self = ADW_FLAP (swipeable);
  int flap, separator;

  if (!self->flap.widget)
    return 0;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    flap = self->flap.allocation.width;
    separator = self->separator.allocation.width;
  } else {
    flap = self->flap.allocation.height;
    separator = self->separator.allocation.height;
  }

  if (transition_is_content_above_flap (self))
    return flap + separator;

  return flap + separator * (1 - self->fold_progress);
}

static double *
adw_flap_get_snap_points (AdwSwipeable *swipeable,
                          int          *n_snap_points)
{
  AdwFlap *self = ADW_FLAP (swipeable);
  gboolean can_open = self->reveal_progress > 0 || self->swipe_to_open || self->swipe_active;
  gboolean can_close = self->reveal_progress < 1 || self->swipe_to_close || self->swipe_active;
  double *points;

  if (!can_open && !can_close)
    return NULL;

  if (can_open && can_close) {
    points = g_new0 (double, 2);

    if (n_snap_points)
      *n_snap_points = 2;

    points[0] = 0;
    points[1] = 1;

    return points;
  }

  points = g_new0 (double, 1);

  if (n_snap_points)
    *n_snap_points = 1;

  points[0] = can_open ? 1 : 0;

  return points;
}

static double
adw_flap_get_progress (AdwSwipeable *swipeable)
{
  AdwFlap *self = ADW_FLAP (swipeable);

  return self->reveal_progress;
}

static double
adw_flap_get_cancel_progress (AdwSwipeable *swipeable)
{
  AdwFlap *self = ADW_FLAP (swipeable);

  return round (self->reveal_progress);
}

static void
adw_flap_get_swipe_area (AdwSwipeable           *swipeable,
                         AdwNavigationDirection  navigation_direction,
                         gboolean                is_drag,
                         GdkRectangle           *rect)
{
  AdwFlap *self = ADW_FLAP (swipeable);
  GtkAllocation *alloc;
  int width, height;
  double flap_factor, content_factor;
  gboolean content_above_flap;

  if (!self->flap.widget) {
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;

    return;
  }

  width = gtk_widget_get_width (GTK_WIDGET (self));
  height = gtk_widget_get_height (GTK_WIDGET (self));

  content_above_flap = transition_is_content_above_flap (self);
  flap_factor = transition_get_flap_motion_factor (self);
  content_factor = transition_get_content_motion_factor (self);

  if (!is_drag ||
      ((G_APPROX_VALUE (flap_factor, 1, DBL_EPSILON) || flap_factor > 1) &&
       (G_APPROX_VALUE (content_factor, 1, DBL_EPSILON) || content_factor > 1)) ||
      (self->fold_progress < 1 && flap_factor > 0)) {
    rect->x = 0;
    rect->y = 0;
    rect->width = width;
    rect->height = height;

    return;
  }

  alloc = content_above_flap
    ? &self->content.allocation
    : &self->flap.allocation;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (alloc->x <= 0) {
      rect->x = 0;
      rect->width = MAX (alloc->width + alloc->x, ADW_SWIPE_BORDER);
    } else if (alloc->x + alloc->width >= width) {
      rect->width = MAX (width - alloc->x, ADW_SWIPE_BORDER);
      rect->x = width - rect->width;
    } else {
      g_assert_not_reached ();
    }

    rect->y = alloc->y;
    rect->height = alloc->height;
  } else {
    if (alloc->y <= 0) {
      rect->y = 0;
      rect->height = MAX (alloc->height + alloc->y, ADW_SWIPE_BORDER);
    } else if (alloc->y + alloc->height >= height) {
      rect->height = MAX (height - alloc->y, ADW_SWIPE_BORDER);
      rect->y = height - rect->height;
    } else {
      g_assert_not_reached ();
    }

    rect->x = alloc->x;
    rect->width = alloc->width;
  }
}

static void
adw_flap_swipeable_init (AdwSwipeableInterface *iface)
{
  iface->get_distance = adw_flap_get_distance;
  iface->get_snap_points = adw_flap_get_snap_points;
  iface->get_progress = adw_flap_get_progress;
  iface->get_cancel_progress = adw_flap_get_cancel_progress;
  iface->get_swipe_area = adw_flap_get_swipe_area;
}

/**
 * adw_flap_new:
 *
 * Creates a new `AdwFlap`.
 *
 * Returns: the newly created `AdwFlap`
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
GtkWidget *
adw_flap_new (void)
{
  return g_object_new (ADW_TYPE_FLAP, NULL);
}

/**
 * adw_flap_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: a flap
 *
 * Gets the content widget for @self.
 *
 * Returns: (transfer none) (nullable): the content widget for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
GtkWidget *
adw_flap_get_content (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), NULL);

  return self->content.widget;
}

/**
 * adw_flap_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: a flap
 * @content: (nullable): the content widget
 *
 * Sets the content widget for @self.
 *
 * It's always displayed when unfolded, and partially visible when folded.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_content (AdwFlap   *self,
                      GtkWidget *content)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  if (self->content.widget == content)
    return;

  if (self->content.widget)
    remove_child (self, &self->content);

  self->content.widget = content;

  if (self->content.widget)
    add_child (self, &self->content);

  update_child_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_flap_get_flap: (attributes org.gtk.Method.get_property=flap)
 * @self: a flap
 *
 * Gets the flap widget for @self.
 *
 * Returns: (transfer none) (nullable): the flap widget for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
GtkWidget *
adw_flap_get_flap (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), NULL);

  return self->flap.widget;
}

/**
 * adw_flap_set_flap: (attributes org.gtk.Method.set_property=flap)
 * @self: a flap
 * @flap: (nullable): the flap widget
 *
 * Sets the flap widget for @self.
 *
 * It's only visible when [property@Flap:reveal-progress] is greater than 0.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_flap (AdwFlap   *self,
                   GtkWidget *flap)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (flap == NULL || GTK_IS_WIDGET (flap));

  if (flap)
    g_return_if_fail (gtk_widget_get_parent (flap) == NULL);

  if (self->flap.widget == flap)
    return;

  if (self->flap.widget)
    remove_child (self, &self->flap);

  self->flap.widget = flap;

  if (self->flap.widget)
    add_child (self, &self->flap);

  update_swipe_tracker (self);
  update_child_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FLAP]);
}

/**
 * adw_flap_get_separator: (attributes org.gtk.Method.get_property=separator)
 * @self: a flap
 *
 * Gets the separator widget for @self.
 *
 * Returns: (transfer none) (nullable): the separator widget for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
GtkWidget *
adw_flap_get_separator (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), NULL);

  return self->separator.widget;
}

/**
 * adw_flap_set_separator: (attributes org.gtk.Method.set_property=separator)
 * @self: a flap
 * @separator: (nullable): the separator widget
 *
 * Sets the separator widget for @self.
 *
 * It's displayed between content and flap when there's no shadow to display.
 * When exactly it's visible depends on the [property@Flap:transition-type]
 * value.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_separator (AdwFlap   *self,
                        GtkWidget *separator)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (separator == NULL || GTK_IS_WIDGET (separator));

  if (separator)
    g_return_if_fail (gtk_widget_get_parent (separator) == NULL);

  if (self->separator.widget == separator)
    return;

  if (self->separator.widget)
    remove_child (self, &self->separator);

  self->separator.widget = separator;

  if (self->separator.widget)
    add_child (self, &self->separator);

  update_child_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SEPARATOR]);
}

/**
 * adw_flap_get_flap_position: (attributes org.gtk.Method.get_property=flap-position)
 * @self: a flap
 *
 * Gets the flap position for @self.
 *
 * Returns: the flap position for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
GtkPackType
adw_flap_get_flap_position (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), GTK_PACK_START);

  return self->flap_position;
}

/**
 * adw_flap_set_flap_position: (attributes org.gtk.Method.set_property=flap-position)
 * @self: a flap
 * @position: the new value
 *
 * Sets the flap position for @self.
 *
 * If it's set to `GTK_PACK_START`, the flap is displayed before the content,
 * if `GTK_PACK_END`, it's displayed after the content.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_flap_position (AdwFlap     *self,
                            GtkPackType  position)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (position <= GTK_PACK_END);

  if (self->flap_position == position)
    return;

  self->flap_position = position;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FLAP_POSITION]);
}

/**
 * adw_flap_get_reveal_flap: (attributes org.gtk.Method.get_property=reveal-flap)
 * @self: a flap
 *
 * Gets whether the flap widget is revealed for @self.
 *
 * Returns: `TRUE` if the flap widget is revealed
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_reveal_flap (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->reveal_flap;
}

/**
 * adw_flap_set_reveal_flap: (attributes org.gtk.Method.set_property=reveal-flap)
 * @self: a flap
 * @reveal_flap: whether to reveal the flap widget
 *
 * Sets whether the flap widget is revealed for @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_reveal_flap (AdwFlap  *self,
                          gboolean  reveal_flap)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  set_reveal_flap (self, reveal_flap, 0);
}

/**
 * adw_flap_get_reveal_params: (attributes org.gtk.Method.get_property=reveal-params)
 * @self: a flap
 *
 * Gets the reveal animation spring parameters for @self.
 *
 * Returns: the reveal animation parameters
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
AdwSpringParams *
adw_flap_get_reveal_params (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), NULL);

  return adw_spring_animation_get_spring_params (ADW_SPRING_ANIMATION (self->reveal_animation));
}

/**
 * adw_flap_set_reveal_params: (attributes org.gtk.Method.set_property=reveal-params)
 * @self: a flap
 * @params: the new parameters
 *
 * Sets the reveal animation spring parameters for @self.
 *
 * The default value is equivalent to:
 *
 * ```c
 * adw_spring_params_new (1, 0.5, 500)
 * ```
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_reveal_params (AdwFlap         *self,
                            AdwSpringParams *params)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (params != NULL);

  if (adw_flap_get_reveal_params (self) == params)
    return;

  adw_spring_animation_set_spring_params (ADW_SPRING_ANIMATION (self->reveal_animation),
                                          params);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_PARAMS]);
}

/**
 * adw_flap_get_reveal_progress: (attributes org.gtk.Method.get_property=reveal-progress)
 * @self: a flap
 *
 * Gets the current reveal progress for @self.
 *
 * 0 means fully hidden, 1 means fully revealed.
 *
 * See [property@Flap:reveal-flap].
 *
 * Returns: the current reveal progress for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
double
adw_flap_get_reveal_progress (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), 0.0);

  return self->reveal_progress;
}

/**
 * adw_flap_get_fold_policy: (attributes org.gtk.Method.get_property=fold-policy)
 * @self: a flap
 *
 * Gets the fold policy for @self.
 *
 * Returns: the fold policy for @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
AdwFlapFoldPolicy
adw_flap_get_fold_policy (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), ADW_FLAP_FOLD_POLICY_NEVER);

  return self->fold_policy;
}

/**
 * adw_flap_set_fold_policy: (attributes org.gtk.Method.set_property=fold-policy)
 * @self: a flap
 * @policy: the fold policy
 *
 * Sets the fold policy for @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_fold_policy (AdwFlap           *self,
                          AdwFlapFoldPolicy  policy)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (policy <= ADW_FLAP_FOLD_POLICY_AUTO);

  if (self->fold_policy == policy)
    return;

  self->fold_policy = policy;

  switch (self->fold_policy) {
  case ADW_FLAP_FOLD_POLICY_NEVER:
    set_folded (self, FALSE);
    break;

  case ADW_FLAP_FOLD_POLICY_ALWAYS:
    set_folded (self, TRUE);
    break;

  case ADW_FLAP_FOLD_POLICY_AUTO:
    gtk_widget_queue_allocate (GTK_WIDGET (self));
    break;

  default:
    g_assert_not_reached ();
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLD_POLICY]);
}

/**
 * adw_flap_get_fold_threshold_policy: (attributes org.gtk.Method.get_property=fold-threshold-policy)
 * @self: a flap
 *
 * Gets the fold threshold policy for @self.
 *
 * Returns: the fold threshold policy
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
AdwFoldThresholdPolicy
adw_flap_get_fold_threshold_policy (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), ADW_FOLD_THRESHOLD_POLICY_MINIMUM);

  return self->fold_threshold_policy;
}

/**
 * adw_flap_set_fold_threshold_policy: (attributes org.gtk.Method.set_property=fold-threshold-policy)
 * @self: a flap
 * @policy: the policy to use
 *
 * Sets the fold threshold policy for @self.
 *
 * If set to `ADW_FOLD_THRESHOLD_POLICY_MINIMUM`, flap will only fold when the
 * children cannot fit anymore. With `ADW_FOLD_THRESHOLD_POLICY_NATURAL`, it
 * will fold as soon as children don't get their natural size.
 *
 * This can be useful if you have a long ellipsizing label and want to let it
 * ellipsize instead of immediately folding.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_fold_threshold_policy (AdwFlap                *self,
                                    AdwFoldThresholdPolicy  policy)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (policy <= ADW_FOLD_THRESHOLD_POLICY_NATURAL);

  if (self->fold_threshold_policy == policy)
    return;

  self->fold_threshold_policy = policy;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLD_THRESHOLD_POLICY]);
}

/**
 * adw_flap_get_fold_duration: (attributes org.gtk.Method.get_property=fold-duration)
 * @self: a flap
 *
 * Gets the fold transition animation duration for @self, in milliseconds.
 *
 * Returns: the fold transition duration
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
guint
adw_flap_get_fold_duration (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), 0);

  return self->fold_duration;
}

/**
 * adw_flap_set_fold_duration: (attributes org.gtk.Method.set_property=fold-duration)
 * @self: a flap
 * @duration: the new duration, in milliseconds
 *
 * Sets the fold transition animation duration for @self, in milliseconds.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_fold_duration (AdwFlap *self,
                            guint    duration)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  if (self->fold_duration == duration)
    return;

  self->fold_duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLD_DURATION]);
}

/**
 * adw_flap_get_folded: (attributes org.gtk.Method.get_property=folded)
 * @self: a flap
 *
 * Gets whether @self is currently folded.
 *
 * See [property@Flap:fold-policy].
 *
 * Returns: `TRUE` if @self is currently folded
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_folded (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->folded;
}

/**
 * adw_flap_get_locked: (attributes org.gtk.Method.get_property=locked)
 * @self: a flap
 *
 * Gets whether @self is locked.
 *
 * Returns: `TRUE` if @self is locked
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_locked (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->locked;
}

/**
 * adw_flap_set_locked: (attributes org.gtk.Method.set_property=locked)
 * @self: a flap
 * @locked: the new value
 *
 * Sets whether @self is locked.
 *
 * If `FALSE`, folding when the flap is revealed automatically closes it, and
 * unfolding it when the flap is not revealed opens it. If `TRUE`,
 * [property@Flap:reveal-flap] value never changes on its own.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_locked (AdwFlap  *self,
                     gboolean  locked)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  locked = !!locked;

  if (self->locked == locked)
    return;

  self->locked = locked;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LOCKED]);
}

/**
 * adw_flap_get_transition_type: (attributes org.gtk.Method.get_property=transition-type)
 * @self: a flap
 *
 * Gets the type of animation used for reveal and fold transitions in @self.
 *
 * Returns: the current transition type of @self
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
AdwFlapTransitionType
adw_flap_get_transition_type (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), ADW_FLAP_TRANSITION_TYPE_OVER);

  return self->transition_type;
}

/**
 * adw_flap_set_transition_type: (attributes org.gtk.Method.set_property=transition-type)
 * @self: a flap
 * @transition_type: the new transition type
 *
 * Sets the type of animation used for reveal and fold transitions in @self.
 *
 * [property@Flap:flap] is transparent by default, which means the content will
 * be seen through it with `ADW_FLAP_TRANSITION_TYPE_OVER` transitions; add the
 * [`.background`](style-classes.html#background) style class to it if this is
 * unwanted.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_transition_type (AdwFlap               *self,
                              AdwFlapTransitionType  transition_type)
{
  g_return_if_fail (ADW_IS_FLAP (self));
  g_return_if_fail (transition_type <= ADW_FLAP_TRANSITION_TYPE_SLIDE);

  if (self->transition_type == transition_type)
    return;

  self->transition_type = transition_type;

  restack_children (self);

  if (self->reveal_progress > 0 || (self->fold_progress > 0 && self->fold_progress < 1))
    gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_TYPE]);
}

/**
 * adw_flap_get_modal: (attributes org.gtk.Method.get_property=modal)
 * @self: a flap
 *
 * Gets whether @self is modal.
 *
 * Returns: `TRUE` if @self is modal
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_modal (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->modal;
}

/**
 * adw_flap_set_modal: (attributes org.gtk.Method.set_property=modal)
 * @self: a flap
 * @modal: whether @self is modal
 *
 * Sets whether @self is modal.
 *
 * If `TRUE`, clicking the content widget while flap is revealed, as well as
 * pressing the <kbd>Esc</kbd> key, will close the flap. If `FALSE`, clicks are
 * passed through to the content widget.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_modal (AdwFlap  *self,
                    gboolean  modal)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  modal = !!modal;

  if (self->modal == modal)
    return;

  self->modal = modal;

  update_shortcuts (self);
  update_shield (self);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODAL]);
}

/**
 * adw_flap_get_swipe_to_open: (attributes org.gtk.Method.get_property=swipe-to-open)
 * @self: a flap
 *
 * Gets whether @self can be opened with a swipe gesture.
 *
 * Returns: `TRUE` if @self can be opened with a swipe gesture
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_swipe_to_open (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->swipe_to_open;
}

/**
 * adw_flap_set_swipe_to_open: (attributes org.gtk.Method.set_property=swipe-to-open)
 * @self: a flap
 * @swipe_to_open: whether @self can be opened with a swipe gesture
 *
 * Sets whether @self can be opened with a swipe gesture.
 *
 * The area that can be swiped depends on the [property@Flap:transition-type]
 * value.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_swipe_to_open (AdwFlap  *self,
                            gboolean  swipe_to_open)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  swipe_to_open = !!swipe_to_open;

  if (self->swipe_to_open == swipe_to_open)
    return;

  self->swipe_to_open = swipe_to_open;

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SWIPE_TO_OPEN]);
}

/**
 * adw_flap_get_swipe_to_close: (attributes org.gtk.Method.get_property=swipe-to-close)
 * @self: a flap
 *
 * Gets whether @self can be closed with a swipe gesture.
 *
 * Returns: `TRUE` if @self can be closed with a swipe gesture
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
gboolean
adw_flap_get_swipe_to_close (AdwFlap *self)
{
  g_return_val_if_fail (ADW_IS_FLAP (self), FALSE);

  return self->swipe_to_close;
}

/**
 * adw_flap_set_swipe_to_close: (attributes org.gtk.Method.set_property=swipe-to-close)
 * @self: a flap
 * @swipe_to_close: whether @self can be closed with a swipe gesture
 *
 * Sets whether @self can be closed with a swipe gesture.
 *
 * The area that can be swiped depends on the [property@Flap:transition-type]
 * value.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwflap)
 */
void
adw_flap_set_swipe_to_close (AdwFlap  *self,
                             gboolean  swipe_to_close)
{
  g_return_if_fail (ADW_IS_FLAP (self));

  swipe_to_close = !!swipe_to_close;

  if (self->swipe_to_close == swipe_to_close)
    return;

  self->swipe_to_close = swipe_to_close;

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SWIPE_TO_CLOSE]);
}
