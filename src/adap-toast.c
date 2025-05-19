/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-toast-private.h"

#include "adw-marshalers.h"
#include <stdarg.h>

/**
 * AdwToastPriority:
 * @ADW_TOAST_PRIORITY_NORMAL: the toast will be queued if another toast is
 *   already displayed.
 * @ADW_TOAST_PRIORITY_HIGH: the toast will be displayed immediately, pushing
 *   the previous toast into the queue instead.
 *
 * [class@Toast] behavior when another toast is already displayed.
 */

/**
 * AdwToast:
 *
 * A helper object for [class@ToastOverlay].
 *
 * Toasts are meant to be passed into [method@ToastOverlay.add_toast] as
 * follows:
 *
 * ```c
 * adw_toast_overlay_add_toast (overlay, adw_toast_new (_("Simple Toast")));
 * ```
 *
 * <picture>
 *   <source srcset="toast-simple-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-simple.png" alt="toast-simple">
 * </picture>
 *
 * Toasts always have a close button. They emit the [signal@Toast::dismissed]
 * signal when disappearing.
 *
 * [property@Toast:timeout] determines how long the toast stays on screen, while
 * [property@Toast:priority] determines how it behaves if another toast is
 * already being displayed.
 *
 * Toast titles use Pango markup by default, set [property@Toast:use-markup] to
 * `FALSE` if this is unwanted.
 *
 * [property@Toast:custom-title] can be used to replace the title label with a
 * custom widget.
 *
 * ## Actions
 *
 * Toasts can have one button on them, with a label and an attached
 * [iface@Gio.Action].
 *
 * ```c
 * AdwToast *toast = adw_toast_new (_("Toast with Action"));
 *
 * adw_toast_set_button_label (toast, _("_Example"));
 * adw_toast_set_action_name (toast, "win.example");
 *
 * adw_toast_overlay_add_toast (overlay, toast);
 * ```
 *
 * <picture>
 *   <source srcset="toast-action-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-action.png" alt="toast-action">
 * </picture>
 *
 * ## Modifying toasts
 *
 * Toasts can be modified after they have been shown. For this, an `AdwToast`
 * reference must be kept around while the toast is visible.
 *
 * A common use case for this is using toasts as undo prompts that stack with
 * each other, allowing to batch undo the last deleted items:
 *
 * ```c
 *
 * static void
 * toast_undo_cb (GtkWidget  *sender,
 *                const char *action,
 *                GVariant   *param)
 * {
 *   // Undo the deletion
 * }
 *
 * static void
 * dismissed_cb (MyWindow *self)
 * {
 *   self->undo_toast = NULL;
 *
 *   // Permanently delete the items
 * }
 *
 * static void
 * delete_item (MyWindow *self,
 *              MyItem   *item)
 * {
 *   g_autofree char *title = NULL;
 *   int n_items;
 *
 *   // Mark the item as waiting for deletion
 *   n_items = ... // The number of waiting items
 *
 *   if (!self->undo_toast) {
 *     self->undo_toast = adw_toast_new_format (_("‘%s’ deleted"), ...);
 *
 *     adw_toast_set_priority (self->undo_toast, ADW_TOAST_PRIORITY_HIGH);
 *     adw_toast_set_button_label (self->undo_toast, _("_Undo"));
 *     adw_toast_set_action_name (self->undo_toast, "toast.undo");
 *
 *     g_signal_connect_swapped (self->undo_toast, "dismissed",
 *                               G_CALLBACK (dismissed_cb), self);
 *
 *     adw_toast_overlay_add_toast (self->toast_overlay, self->undo_toast);
 *
 *     return;
 *   }
 *
 *   title =
 *     g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
 *                                "<span font_features='tnum=1'>%d</span> items deleted",
 *                                n_items), n_items);
 *
 *   adw_toast_set_title (self->undo_toast, title);
 *
 *   // Bump the toast timeout
 *   adw_toast_overlay_add_toast (self->toast_overlay, g_object_ref (self->undo_toast));
 * }
 *
 * static void
 * my_window_class_init (MyWindowClass *klass)
 * {
 *   GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
 *
 *   gtk_widget_class_install_action (widget_class, "toast.undo", NULL, toast_undo_cb);
 * }
 * ```
 *
 * <picture>
 *   <source srcset="toast-undo-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-undo.png" alt="toast-undo">
 * </picture>
 */

struct _AdwToast {
  GObject parent_instance;

  char *title;
  char *button_label;
  char *action_name;
  GVariant *action_target;
  AdwToastPriority priority;
  guint timeout;
  GtkWidget *custom_title;
  gboolean use_markup;

  AdwToastOverlay *overlay;
};

enum {
  PROP_0,
  PROP_TITLE,
  PROP_BUTTON_LABEL,
  PROP_ACTION_NAME,
  PROP_ACTION_TARGET,
  PROP_PRIORITY,
  PROP_TIMEOUT,
  PROP_CUSTOM_TITLE,
  PROP_USE_MARKUP,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_DISMISSED,
  SIGNAL_BUTTON_CLICKED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdwToast, adw_toast, G_TYPE_OBJECT)

static void
dismissed_cb (AdwToast *self)
{
  adw_toast_set_overlay (self, NULL);
}

static void
adw_toast_dispose (GObject *object)
{
  AdwToast *self = ADW_TOAST (object);

  g_clear_object (&self->custom_title);

  G_OBJECT_CLASS (adw_toast_parent_class)->dispose (object);
}

static void
adw_toast_finalize (GObject *object)
{
  AdwToast *self = ADW_TOAST (object);

  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->button_label, g_free);
  g_clear_pointer (&self->action_name, g_free);
  g_clear_pointer (&self->action_target, g_variant_unref);

  G_OBJECT_CLASS (adw_toast_parent_class)->finalize (object);
}

static void
adw_toast_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  AdwToast *self = ADW_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_toast_get_title (self));
    break;
  case PROP_BUTTON_LABEL:
    g_value_set_string (value, adw_toast_get_button_label (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, adw_toast_get_action_name (self));
    break;
  case PROP_ACTION_TARGET:
    g_value_set_variant (value, adw_toast_get_action_target_value (self));
    break;
  case PROP_PRIORITY:
    g_value_set_enum (value, adw_toast_get_priority (self));
    break;
  case PROP_TIMEOUT:
    g_value_set_uint (value, adw_toast_get_timeout (self));
    break;
  case PROP_CUSTOM_TITLE:
    g_value_set_object (value, adw_toast_get_custom_title (self));
    break;
  case PROP_USE_MARKUP:
    g_value_set_boolean (value, adw_toast_get_use_markup (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  AdwToast *self = ADW_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_toast_set_title (self, g_value_get_string (value));
    break;
  case PROP_BUTTON_LABEL:
    adw_toast_set_button_label (self, g_value_get_string (value));
    break;
  case PROP_ACTION_NAME:
    adw_toast_set_action_name (self, g_value_get_string (value));
    break;
  case PROP_ACTION_TARGET:
    adw_toast_set_action_target_value (self, g_value_get_variant (value));
    break;
  case PROP_PRIORITY:
    adw_toast_set_priority (self, g_value_get_enum (value));
    break;
  case PROP_TIMEOUT:
    adw_toast_set_timeout (self, g_value_get_uint (value));
    break;
  case PROP_CUSTOM_TITLE:
    adw_toast_set_custom_title (self, g_value_get_object (value));
    break;
  case PROP_USE_MARKUP:
    adw_toast_set_use_markup (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_class_init (AdwToastClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_toast_dispose;
  object_class->finalize = adw_toast_finalize;
  object_class->get_property = adw_toast_get_property;
  object_class->set_property = adw_toast_set_property;

  /**
   * AdwToast:title: (attributes org.gtk.Property.get=adw_toast_get_title org.gtk.Property.set=adw_toast_set_title)
   *
   * The title of the toast.
   *
   * The title can be marked up with the Pango text markup language.
   *
   * Setting a title will unset [property@Toast:custom-title].
   *
   * If [property@Toast:custom-title] is set, it will be used instead.
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:button-label: (attributes org.gtk.Property.get=adw_toast_get_button_label org.gtk.Property.set=adw_toast_set_button_label)
   *
   * The label to show on the button.
   *
   * Underlines in the button text can be used to indicate a mnemonic.
   *
   * If set to `NULL`, the button won't be shown.
   *
   * See [property@Toast:action-name].
   */
  props[PROP_BUTTON_LABEL] =
    g_param_spec_string ("button-label", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:action-name: (attributes org.gtk.Property.get=adw_toast_get_action_name org.gtk.Property.set=adw_toast_set_action_name)
   *
   * The name of the associated action.
   *
   * It will be activated when clicking the button.
   *
   * See [property@Toast:action-target].
   */
  props[PROP_ACTION_NAME] =
    g_param_spec_string ("action-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:action-target: (attributes org.gtk.Property.get=adw_toast_get_action_target_value org.gtk.Property.set=adw_toast_set_action_target_value)
   *
   * The parameter for action invocations.
   */
  props[PROP_ACTION_TARGET] =
    g_param_spec_variant ("action-target", NULL, NULL,
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:priority: (attributes org.gtk.Property.get=adw_toast_get_priority org.gtk.Property.set=adw_toast_set_priority)
   *
   * The priority of the toast.
   *
   * Priority controls how the toast behaves when another toast is already
   * being displayed.
   *
   * If the priority is `ADW_TOAST_PRIORITY_NORMAL`, the toast will be queued.
   *
   * If the priority is `ADW_TOAST_PRIORITY_HIGH`, the toast will be displayed
   * immediately, pushing the previous toast into the queue instead.
   */
  props[PROP_PRIORITY] =
    g_param_spec_enum ("priority", NULL, NULL,
                       ADW_TYPE_TOAST_PRIORITY,
                       ADW_TOAST_PRIORITY_NORMAL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:timeout: (attributes org.gtk.Property.get=adw_toast_get_timeout org.gtk.Property.set=adw_toast_set_timeout)
   *
   * The timeout of the toast, in seconds.
   *
   * If timeout is 0, the toast is displayed indefinitely until manually
   * dismissed.
   *
   * Toasts cannot disappear while being hovered, pressed (on touchscreen), or
   * have keyboard focus inside them.
   */
  props[PROP_TIMEOUT] =
    g_param_spec_uint ("timeout", NULL, NULL,
                       0, G_MAXUINT, 5,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:custom-title: (attributes org.gtk.Property.get=adw_toast_get_custom_title org.gtk.Property.set=adw_toast_set_custom_title)
   *
   * The custom title widget.
   *
   * It will be displayed instead of the title if set. In this case,
   * [property@Toast:title] is ignored.
   *
   * Setting a custom title will unset [property@Toast:title].
   *
   * Since: 1.2
   */
  props[PROP_CUSTOM_TITLE] =
    g_param_spec_object ("custom-title", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:use-markup: (attributes org.gtk.Property.get=adw_toast_get_use_markup org.gtk.Property.set=adw_toast_set_use_markup)
   *
   * Whether to use Pango markup for the toast title.
   *
   * See also [func@Pango.parse_markup].
   *
   * Since: 1.4
   */
  props[PROP_USE_MARKUP] =
    g_param_spec_boolean ("use-markup", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwToast::dismissed:
   *
   * Emitted when the toast has been dismissed.
   */
  signals[SIGNAL_DISMISSED] =
    g_signal_new ("dismissed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_DISMISSED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwToast::button-clicked:
   *
   * Emitted after the button has been clicked.
   *
   * It can be used as an alternative to setting an action.
   *
   * Since: 1.2
   */
  signals[SIGNAL_BUTTON_CLICKED] =
    g_signal_new ("button-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_BUTTON_CLICKED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);
}

static void
adw_toast_init (AdwToast *self)
{
  self->title = g_strdup ("");
  self->priority = ADW_TOAST_PRIORITY_NORMAL;
  self->timeout = 5;
  self->custom_title = NULL;
  self->use_markup = TRUE;

  g_signal_connect (self, "dismissed", G_CALLBACK (dismissed_cb), self);
}

/**
 * adw_toast_new:
 * @title: the title to be displayed
 *
 * Creates a new `AdwToast`.
 *
 * The toast will use @title as its title.
 *
 * @title can be marked up with the Pango text markup language.
 *
 * Returns: the new created `AdwToast`
 */
AdwToast *
adw_toast_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADW_TYPE_TOAST,
                       "title", title,
                       NULL);
}

/**
 * adw_toast_new_format:
 * @format: the formatted string for the toast title
 * @...: the parameters to insert into the format string
 *
 * Creates a new `AdwToast`.
 *
 * The toast will use the format string as its title.
 *
 * See also: [ctor@Toast.new]
 *
 * Returns: the newly created toast object
 *
 * Since: 1.2
 */
AdwToast *
adw_toast_new_format (const char *format,
                      ...)
{
  AdwToast *res;
  va_list args;
  char *title;

  va_start (args, format);
  title = g_strdup_vprintf (format, args);
  va_end (args);

  res = g_object_new (ADW_TYPE_TOAST,
                      "title", title,
                      NULL);

  g_free (title);

  return res;
}

/**
 * adw_toast_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a toast
 *
 * Gets the title that will be displayed on the toast.
 *
 * If a custom title has been set with [method@Adw.Toast.set_custom_title]
 * the return value will be %NULL.
 *
 * Returns: (nullable): the title
 */
const char *
adw_toast_get_title (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  if (self->custom_title == NULL)
    return self->title;

  return NULL;
}

/**
 * adw_toast_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a toast
 * @title: a title
 *
 * Sets the title that will be displayed on the toast.
 *
 * The title can be marked up with the Pango text markup language.
 *
 * Setting a title will unset [property@Toast:custom-title].
 *
 * If [property@Toast:custom-title] is set, it will be used instead.
 */
void
adw_toast_set_title (AdwToast   *self,
                     const char *title)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (title != NULL);

  if (!g_strcmp0 (self->title, title))
    return;

  g_object_freeze_notify (G_OBJECT (self));

  adw_toast_set_custom_title (self, NULL);

  g_set_str (&self->title, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_toast_get_button_label: (attributes org.gtk.Method.get_property=button-label)
 * @self: a toast
 *
 * Gets the label to show on the button.
 *
 * Returns: (nullable): the button label
 */
const char *
adw_toast_get_button_label (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->button_label;
}

/**
 * adw_toast_set_button_label: (attributes org.gtk.Method.set_property=button-label)
 * @self: a toast
 * @button_label: (nullable): a button label
 *
 * Sets the label to show on the button.
 *
 * Underlines in the button text can be used to indicate a mnemonic.
 *
 * If set to `NULL`, the button won't be shown.
 *
 * See [property@Toast:action-name].
 */
void
adw_toast_set_button_label (AdwToast   *self,
                            const char *button_label)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!g_set_str (&self->button_label, button_label))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BUTTON_LABEL]);
}

/**
 * adw_toast_get_action_name: (attributes org.gtk.Method.get_property=action-name)
 * @self: a toast
 *
 * Gets the name of the associated action.
 *
 * Returns: (nullable): the action name
 */
const char *
adw_toast_get_action_name (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->action_name;
}

/**
 * adw_toast_set_action_name: (attributes org.gtk.Method.set_property=action-name)
 * @self: a toast
 * @action_name: (nullable): the action name
 *
 * Sets the name of the associated action.
 *
 * It will be activated when clicking the button.
 *
 * See [property@Toast:action-target].
 */
void
adw_toast_set_action_name (AdwToast   *self,
                           const char *action_name)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!g_set_str (&self->action_name, action_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_NAME]);
}

/**
 * adw_toast_get_action_target_value: (attributes org.gtk.Method.get_property=action-target)
 * @self: a toast
 *
 * Gets the parameter for action invocations.
 *
 * Returns: (transfer none) (nullable): the action target
 */
GVariant *
adw_toast_get_action_target_value (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->action_target;
}

/**
 * adw_toast_set_action_target_value: (attributes org.gtk.Method.set_property=action-target)
 * @self: a toast
 * @action_target: (nullable): the action target
 *
 * Sets the parameter for action invocations.
 *
 * If the @action_target variant has a floating reference this function
 * will sink it.
 */
void
adw_toast_set_action_target_value (AdwToast *self,
                                   GVariant *action_target)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (action_target == self->action_target)
    return;

  if (action_target && self->action_target &&
      g_variant_equal (action_target, self->action_target))
    return;

  g_clear_pointer (&self->action_target, g_variant_unref);
  if (action_target != NULL)
    self->action_target = g_variant_ref_sink (action_target);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_TARGET]);
}

/**
 * adw_toast_set_action_target: (skip)
 * @self: a toast
 * @format_string: (nullable): a variant format string
 * @...: arguments appropriate for @target_format
 *
 * Sets the parameter for action invocations.
 *
 * This is a convenience function that calls [ctor@GLib.Variant.new] for
 * @format_string and uses the result to call
 * [method@Toast.set_action_target_value].
 *
 * If you are setting a string-valued target and want to set
 * the action name at the same time, you can use
 * [method@Toast.set_detailed_action_name].
 */
void
adw_toast_set_action_target (AdwToast   *self,
                             const char *format_string,
                             ...)
{
  va_list args;

  va_start (args, format_string);
  adw_toast_set_action_target_value (self,
                                     g_variant_new_va (format_string,
                                                       NULL, &args));
  va_end (args);
}

/**
 * adw_toast_set_detailed_action_name:
 * @self: a toast
 * @detailed_action_name: (nullable): the detailed action name
 *
 * Sets the action name and its parameter.
 *
 * @detailed_action_name is a string in the format accepted by
 * [func@Gio.Action.parse_detailed_name].
 */
void
adw_toast_set_detailed_action_name (AdwToast   *self,
                                    const char *detailed_action_name)
{
  char *name;
  GVariant *target;
  GError *error = NULL;

  g_return_if_fail (ADW_IS_TOAST (self));

  if (!detailed_action_name) {
    adw_toast_set_action_name (self, NULL);
    adw_toast_set_action_target_value (self, NULL);

    return;
  }

  if (g_action_parse_detailed_name (detailed_action_name, &name, &target, &error)) {
    adw_toast_set_action_name (self, name);
    adw_toast_set_action_target_value (self, target);
  } else {
    g_critical ("Couldn't parse detailed action name: %s", error->message);
  }

  g_clear_error (&error);
  g_clear_pointer (&target, g_variant_unref);
  g_clear_pointer (&name, g_free);
}

/**
 * adw_toast_get_priority: (attributes org.gtk.Method.get_property=priority)
 * @self: a toast
 *
 * Gets priority for @self.
 *
 * Returns: the priority
 */
AdwToastPriority
adw_toast_get_priority (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), ADW_TOAST_PRIORITY_NORMAL);

  return self->priority;
}

/**
 * adw_toast_set_priority: (attributes org.gtk.Method.set_property=priority)
 * @self: a toast
 * @priority: the priority
 *
 * Sets priority for @self.
 *
 * Priority controls how the toast behaves when another toast is already
 * being displayed.
 *
 * If @priority is `ADW_TOAST_PRIORITY_NORMAL`, the toast will be queued.
 *
 * If @priority is `ADW_TOAST_PRIORITY_HIGH`, the toast will be displayed
 * immediately, pushing the previous toast into the queue instead.
 */
void
adw_toast_set_priority (AdwToast         *self,
                        AdwToastPriority  priority)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (priority >= ADW_TOAST_PRIORITY_NORMAL &&
                    priority <= ADW_TOAST_PRIORITY_HIGH);

  if (self->priority == priority)
    return;

  self->priority = priority;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PRIORITY]);
}

/**
 * adw_toast_get_timeout: (attributes org.gtk.Method.get_property=timeout)
 * @self: a toast
 *
 * Gets timeout for @self.
 *
 * Returns: the timeout
 */
guint
adw_toast_get_timeout (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), 0);

  return self->timeout;
}

/**
 * adw_toast_set_timeout: (attributes org.gtk.Method.set_property=timeout)
 * @self: a toast
 * @timeout: the timeout
 *
 * Sets timeout for @self.
 *
 * If @timeout is 0, the toast is displayed indefinitely until manually
 * dismissed.
 *
 * Toasts cannot disappear while being hovered, pressed (on touchscreen), or
 * have keyboard focus inside them.
 */
void
adw_toast_set_timeout (AdwToast *self,
                       guint     timeout)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (self->timeout == timeout)
    return;

  self->timeout = timeout;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMEOUT]);
}

/**
 * adw_toast_get_custom_title: (attributes org.gtk.Method.get_property=custom-title)
 * @self: a toast
 *
 * Gets the custom title widget of @self.
 *
 * Returns: (nullable) (transfer none): the custom title widget
 *
 * Since: 1.2
 */
GtkWidget *
adw_toast_get_custom_title (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->custom_title;
}

/**
 * adw_toast_set_custom_title: (attributes org.gtk.Method.set_property=custom-title)
 * @self: a toast
 * @widget: (nullable): the custom title widget
 *
 * Sets the custom title widget of @self.
 *
 * It will be displayed instead of the title if set. In this case,
 * [property@Toast:title] is ignored.
 *
 * Setting a custom title will unset [property@Toast:title].
 *
 * Since: 1.2
 */
void
adw_toast_set_custom_title (AdwToast  *self,
                            GtkWidget *widget)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  if (widget)
    g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  if (self->custom_title == widget)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  adw_toast_set_title (self, "");

  g_set_object (&self->custom_title, widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CUSTOM_TITLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_toast_dismiss:
 * @self: a toast
 *
 * Dismisses @self.
 *
 * Does nothing if @self has already been dismissed, or hasn't been added to an
 * [class@ToastOverlay].
 */
void
adw_toast_dismiss (AdwToast *self)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!self->overlay)
    return;

  g_signal_emit (self, signals[SIGNAL_DISMISSED], 0, NULL);
}

AdwToastOverlay *
adw_toast_get_overlay (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->overlay;
}

void
adw_toast_set_overlay (AdwToast        *self,
                       AdwToastOverlay *overlay)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (overlay == NULL || ADW_IS_TOAST_OVERLAY (overlay));

  self->overlay = overlay;
}

/**
 * adw_toast_get_use_markup: (attributes org.gtk.Method.get_property=use-markup)
 * @self: a toast
 *
 * Gets whether to use Pango markup for the toast title.
 *
 * Returns: whether the toast uses markup
 *
 * Since: 1.4
 */
gboolean
adw_toast_get_use_markup (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), FALSE);

  return self->use_markup;
}

/**
 * adw_toast_set_use_markup: (attributes org.gtk.Method.set_property=use-markup)
 * @self: a toast
 * @use_markup: whether to use markup
 *
 * Whether to use Pango markup for the toast title.
 *
 * See also [func@Pango.parse_markup].
 *
 * Since: 1.4
 */
void
adw_toast_set_use_markup (AdwToast *self,
                          gboolean  use_markup)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  use_markup = !!use_markup;

  if (adw_toast_get_use_markup (self) == use_markup)
    return;

  self->use_markup = use_markup;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_MARKUP]);
}
