/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-toast-overlay.h"

#include "adap-animation-util.h"
#include "adap-easing.h"
#include "adap-timed-animation.h"
#include "adap-toast-private.h"
#include "adap-toast-widget-private.h"
#include "adap-widget-utils-private.h"

#define SHOW_DURATION 300
#define HIDE_DURATION 300
#define REPLACE_DURATION 500
#define SCALE_AMOUNT 0.05
#define NATURAL_WIDTH 450

/**
 * AdapToastOverlay:
 *
 * A widget showing toasts above its content.
 *
 * <picture>
 *   <source srcset="toast-overlay-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-overlay.png" alt="toast-overlay">
 * </picture>
 *
 * Much like [class@Gtk.Overlay], `AdapToastOverlay` is a container with a single
 * main child, on top of which it can display a [class@Toast], overlaid.
 * Toasts can be shown with [method@ToastOverlay.add_toast].
 *
 * See [class@Toast] for details.
 *
 * ## CSS nodes
 *
 * ```
 * toastoverlay
 * ├── [child]
 * ├── toast
 * ┊   ├── widget
 * ┊   │   ├── [label.heading]
 *     │   ╰── [custom title]
 *     ├── [button]
 *     ╰── button.circular.flat
 * ```
 *
 * `AdapToastOverlay`'s CSS node is called `toastoverlay`. It contains the child,
 * as well as zero or more `toast` subnodes.
 *
 * Each of the `toast` nodes contains a `widget` subnode, optionally a `button`
 * subnode, and another `button` subnode with `.circular` and `.flat` style
 * classes.
 *
 * The `widget` subnode contains a `label` subnode with the `.heading` style
 * class, or a custom widget provided by the application.
 *
 * ## Accessibility
 *
 * `AdapToastOverlay` uses the `GTK_ACCESSIBLE_ROLE_TAB_GROUP` role.
 */

typedef struct {
  AdapToastOverlay *overlay;

  AdapToast *toast;
  GtkWidget *widget;

  AdapAnimation *show_animation;
  AdapAnimation *hide_animation;

  gulong shown_id;
  gulong dismissed_id;
  gboolean postponing;
} ToastInfo;

struct _AdapToastOverlay {
  GtkWidget parent_instance;

  GtkWidget *child;

  GQueue *queue;
  ToastInfo *current_toast;
  GList *hiding_toasts;
};

enum {
  PROP_0,
  PROP_CHILD,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void adap_toast_overlay_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapToastOverlay, adap_toast_overlay, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                               adap_toast_overlay_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
free_toast_info (ToastInfo *info)
{
  if (info->shown_id && info->show_animation)
    g_signal_handler_disconnect (info->show_animation, info->shown_id);
  if (info->dismissed_id && info->toast)
    g_signal_handler_disconnect (info->toast, info->dismissed_id);

  g_clear_object (&info->show_animation);
  g_clear_object (&info->hide_animation);
  g_clear_pointer (&info->widget, gtk_widget_unparent);
  g_clear_object (&info->toast);

  g_free (info);
}

static void
dismiss_and_free_toast_info (ToastInfo *info)
{
  g_signal_handler_disconnect (info->toast, info->dismissed_id);
  info->dismissed_id = 0;

  adap_toast_dismiss (info->toast);

  free_toast_info (info);
}

static void
hide_value_cb (double     value,
               ToastInfo *info)
{
  value = adap_easing_ease (ADAP_EASE_OUT_CUBIC, value);
  gtk_widget_set_opacity (info->widget, value);

  gtk_widget_queue_allocate (GTK_WIDGET (info->overlay));
}

static void
hide_done_cb (ToastInfo *info)
{
  AdapToastOverlay *self = info->overlay;

  self->hiding_toasts = g_list_remove (self->hiding_toasts, info);

  /* We don't want to free the toast just yet, just remove the widget as we'll
   * make a new one later */
  if (info->postponing && info->dismissed_id) {
    g_clear_object (&info->show_animation);
    g_clear_object (&info->hide_animation);
    g_clear_pointer (&info->widget, gtk_widget_unparent);
    info->postponing = FALSE;

    return;
  }

  free_toast_info (info);
}

static void
show_done_cb (ToastInfo *info)
{
  g_clear_object (&info->show_animation);
}

static void show_toast (AdapToastOverlay *self,
                        ToastInfo       *info);

static void
hide_current_toast (AdapToastOverlay *self)
{
  ToastInfo *info = self->current_toast;
  AdapAnimationTarget *target;

  self->hiding_toasts = g_list_append (self->hiding_toasts, info);
  self->current_toast = NULL;

  gtk_widget_set_can_target (GTK_WIDGET (info->widget), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (info->widget), FALSE);

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc)
                                              hide_value_cb,
                                              info, NULL);
  info->hide_animation =
    adap_timed_animation_new (GTK_WIDGET (self), 1, 0, HIDE_DURATION, target);

  g_signal_connect_swapped (info->hide_animation, "done",
                            G_CALLBACK (hide_done_cb), info);

  adap_animation_play (info->hide_animation);
}

static void
dismissed_cb (ToastInfo *info)
{
  AdapToastOverlay *self = info->overlay;

  if (info->hide_animation && !info->postponing)
    return;

  /* Protect against repeat emissions */
  if (info->dismissed_id) {
    g_signal_handler_disconnect (info->toast, info->dismissed_id);
    info->dismissed_id = 0;
  }

  if (info == self->current_toast) {
    ToastInfo *next_toast;

    hide_current_toast (self);

    next_toast = g_queue_pop_head (self->queue);

    if (next_toast)
      show_toast (self, next_toast);
  } else {
    g_queue_remove (self->queue, info);

    adap_toast_set_overlay (ADAP_TOAST (info->toast), NULL);
    if (!info->hide_animation)
      free_toast_info (info);
  }
}

static void
show_toast_animation_cb (double     value,
                         GtkWidget *self)
{
  gtk_widget_queue_allocate (self);
}

static void
show_toast (AdapToastOverlay *self,
            ToastInfo       *info)
{
  AdapAnimationTarget *target;

  g_assert (!info->widget);

  self->current_toast = info;

  info->widget = adap_toast_widget_new (info->toast);
  gtk_widget_insert_before (info->widget, GTK_WIDGET (self), NULL);

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc)
                                              show_toast_animation_cb,
                                              self, NULL);
  info->show_animation =
    adap_timed_animation_new (GTK_WIDGET (self), 0, 1,
                             self->hiding_toasts ? REPLACE_DURATION : SHOW_DURATION,
                             target);

  info->shown_id = g_signal_connect_swapped (info->show_animation, "done",
                                             G_CALLBACK (show_done_cb), info);

  adap_animation_play (info->show_animation);
}

static int
bump_sort_func (ToastInfo *compare,
                AdapToast  *toast,
                gpointer   user_data)
{
  if (adap_toast_get_priority (compare->toast) == ADAP_TOAST_PRIORITY_HIGH)
    return -1;

  return 1;
}

static int
find_toast_func (ToastInfo *info,
                 AdapToast  *toast)
{
  if (info && info->toast == toast)
    return 0;

  return 1;
}

static void
bump_toast (AdapToastOverlay *self,
            AdapToast        *toast)
{
  GList *link;
  ToastInfo *info;

  /* Remove it from the queue, then reinsert in the right location */
  link = g_queue_find_custom (self->queue, toast,
                              (GCompareFunc) find_toast_func);

  g_assert (link);

  info = link->data;
  g_queue_remove (self->queue, info);

  if (adap_toast_get_priority (toast) == ADAP_TOAST_PRIORITY_HIGH)
    g_queue_push_head (self->queue, info);
  else
    g_queue_insert_sorted (self->queue, info,
                           (GCompareDataFunc) bump_sort_func,
                           NULL);
}

static gboolean
dismiss_cb (AdapToastOverlay *self,
            GVariant        *args)
{
  if (self->current_toast) {
    adap_toast_dismiss (self->current_toast->toast);

    return GDK_EVENT_STOP;
  }

  return GDK_EVENT_PROPAGATE;
}

static void
adap_toast_overlay_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *minimum,
                           int            *natural,
                           int            *minimum_baseline,
                           int            *natural_baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    int child_min = 0;
    int child_nat = 0;
    int child_min_baseline = -1;
    int child_nat_baseline = -1;

    if (!gtk_widget_should_layout (child))
      continue;

    gtk_widget_measure (child, orientation, for_size,
                        &child_min, &child_nat,
                        &child_min_baseline, &child_nat_baseline);

    *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);

    if (child_min_baseline > -1)
      *minimum_baseline = MAX (*minimum_baseline, child_min_baseline);
    if (child_nat_baseline > -1)
      *natural_baseline = MAX (*natural_baseline, child_nat_baseline);
  }
}

static void
allocate_toast (AdapToastOverlay *self,
                ToastInfo       *info,
                int              width,
                int              height)
{
  GtkRequisition size;
  GskTransform *transform;
  float x, y;

  gtk_widget_get_preferred_size (info->widget, NULL, &size);

  if (adap_toast_widget_get_button_visible (ADAP_TOAST_WIDGET (info->widget)))
    size.width = MIN (MAX (size.width, NATURAL_WIDTH), width);
  else
    size.width = MIN (size.width, width);

  size.height = MIN (size.height, height);

  x = (width - size.width) / 2;
  y = height - size.height;
  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y));

  if (info->show_animation) {
    float value = adap_animation_get_value (info->show_animation);
    float offset = adap_lerp (size.height, 0.0f, value);

    transform = gsk_transform_translate (transform,
                                         &GRAPHENE_POINT_INIT (0, offset));
  }

  if (info->hide_animation) {
    float value = adap_animation_get_value (info->hide_animation);

    x = size.width / 2.0f;
    y = size.height / 2.0f;

    value = adap_lerp (1 - SCALE_AMOUNT, 1, value);
    transform = gsk_transform_translate (transform,
                                         &GRAPHENE_POINT_INIT (x, y));
    transform = gsk_transform_scale (transform, value, value);
    transform = gsk_transform_translate (transform,
                                         &GRAPHENE_POINT_INIT (-x, -y));
  }

  gtk_widget_allocate (info->widget, size.width, size.height, -1, transform);
}

static void
adap_toast_overlay_size_allocate (GtkWidget *widget,
                                 int        width,
                                 int        height,
                                 int        baseline)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (widget);
  GList *l;

  if (self->child && gtk_widget_should_layout (self->child))
    gtk_widget_allocate (self->child, width, height, baseline, NULL);

  for (l = self->hiding_toasts; l; l = l->next)
    allocate_toast (self, l->data, width, height);

  if (self->current_toast)
    allocate_toast (self, self->current_toast, width, height);
}

static void
adap_toast_overlay_dispose (GObject *object)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (object);

  adap_toast_overlay_set_child (self, NULL);

  g_clear_list (&self->hiding_toasts, (GDestroyNotify) free_toast_info);
  g_clear_pointer (&self->current_toast, dismiss_and_free_toast_info);
  g_queue_foreach (self->queue, (GFunc) dismiss_and_free_toast_info, NULL);

  G_OBJECT_CLASS (adap_toast_overlay_parent_class)->dispose (object);
}

static void
adap_toast_overlay_finalize (GObject *object)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (object);

  g_queue_free (self->queue);

  G_OBJECT_CLASS (adap_toast_overlay_parent_class)->finalize (object);
}

static void
adap_toast_overlay_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_toast_overlay_get_child (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_overlay_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_toast_overlay_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_overlay_class_init (AdapToastOverlayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_toast_overlay_dispose;
  object_class->finalize = adap_toast_overlay_finalize;
  object_class->get_property = adap_toast_overlay_get_property;
  object_class->set_property = adap_toast_overlay_set_property;

  widget_class->compute_expand = adap_widget_compute_expand;
  widget_class->get_request_mode = adap_widget_get_request_mode;
  widget_class->measure = adap_toast_overlay_measure;
  widget_class->size_allocate = adap_toast_overlay_size_allocate;

  /**
   * AdapToastOverlay:child: (attributes org.gtk.Property.get=adap_toast_overlay_get_child org.gtk.Property.set=adap_toast_overlay_set_child)
   *
   * The child widget.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "toastoverlay");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0, (GtkShortcutFunc) dismiss_cb, NULL);
}

static void
adap_toast_overlay_init (AdapToastOverlay *self)
{
  self->queue = g_queue_new ();

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);
}

static void
adap_toast_overlay_buildable_add_child (GtkBuildable *buildable,
                                       GtkBuilder   *builder,
                                       GObject      *child,
                                       const char   *type)
{
  AdapToastOverlay *self = ADAP_TOAST_OVERLAY (buildable);

  if (!type && GTK_IS_WIDGET (child))
    adap_toast_overlay_set_child (self, GTK_WIDGET (child));
  else if (!type && ADAP_IS_TOAST (child))
    adap_toast_overlay_add_toast (self, g_object_ref (ADAP_TOAST (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_toast_overlay_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_toast_overlay_buildable_add_child;
}

/**
 * adap_toast_overlay_new:
 *
 * Creates a new `AdapToastOverlay`.
 *
 * Returns: the new created `AdapToastOverlay`
 */
GtkWidget *
adap_toast_overlay_new (void)
{
  return g_object_new (ADAP_TYPE_TOAST_OVERLAY, NULL);
}

/**
 * adap_toast_overlay_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a toast overlay
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adap_toast_overlay_get_child (AdapToastOverlay *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST_OVERLAY (self), NULL);

  return self->child;
}

/**
 * adap_toast_overlay_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a toast overlay
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adap_toast_overlay_set_child (AdapToastOverlay *self,
                             GtkWidget       *child)
{
  g_return_if_fail (ADAP_IS_TOAST_OVERLAY (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  if (self->child)
    gtk_widget_unparent (self->child);

  self->child = child;

  if (self->child)
    gtk_widget_insert_after (self->child, GTK_WIDGET (self), NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adap_toast_overlay_add_toast:
 * @self: a toast overlay
 * @toast: (transfer full): a toast
 *
 * Displays @toast.
 *
 * Only one toast can be shown at a time; if a toast is already being displayed,
 * either @toast or the original toast will be placed in a queue, depending on
 * the priority of @toast. See [property@Toast:priority].
 *
 * If called on a toast that's already displayed, its timeout will be reset.
 *
 * If called on a toast currently in the queue, the toast will be bumped
 * forward to be shown as soon as possible.
 */
void
adap_toast_overlay_add_toast (AdapToastOverlay *self,
                             AdapToast        *toast)
{
  ToastInfo *info;
  AdapToastOverlay *overlay;

  g_return_if_fail (ADAP_IS_TOAST_OVERLAY (self));
  g_return_if_fail (ADAP_IS_TOAST (toast));

  overlay = adap_toast_get_overlay (toast);

  /* If the toast has been added already and is being shown, reset its
   * timeout. Otherwise, bump it forward in the queue. */
  if (overlay == self) {
    if (self->current_toast && self->current_toast->toast == toast)
      adap_toast_widget_reset_timeout (ADAP_TOAST_WIDGET (self->current_toast->widget));
    else
      bump_toast (self, toast);

    /* Unref, as we don't actually take the ownership */
    g_object_unref (toast);
    return;
  }

  if (overlay) {
    g_critical ("Adding toast '%s', but it has already been added to a "
                "different AdapToastOverlay", adap_toast_get_title (toast));

    g_object_unref (toast);
    return;
  }

  adap_toast_set_overlay (toast, self);

  info = g_new0 (ToastInfo, 1);
  info->overlay = self;
  info->toast = toast;
  info-> dismissed_id =
    g_signal_connect_swapped (info->toast, "dismissed",
                              G_CALLBACK (dismissed_cb), info);

  if (!self->current_toast) {
    show_toast (self, info);

    return;
  }

  switch (adap_toast_get_priority (toast)) {
  case ADAP_TOAST_PRIORITY_NORMAL:
    g_queue_push_tail (self->queue, info);
    break;

  case ADAP_TOAST_PRIORITY_HIGH:
    self->current_toast->postponing = TRUE;
    g_queue_push_head (self->queue, self->current_toast);

    hide_current_toast (self);
    show_toast (self, info);
    break;

  default:
    g_assert_not_reached ();
  }
}
