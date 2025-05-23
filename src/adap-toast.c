/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-toast-private.h"

#include "adap-marshalers.h"
#include <stdarg.h>

/**
 * AdapToastPriority:
 * @ADAP_TOAST_PRIORITY_NORMAL: the toast will be queued if another toast is
 *   already displayed.
 * @ADAP_TOAST_PRIORITY_HIGH: the toast will be displayed immediately, pushing
 *   the previous toast into the queue instead.
 *
 * [class@Toast] behavior when another toast is already displayed.
 */

/**
 * AdapToast:
 *
 * A helper object for [class@ToastOverlay].
 *
 * Toasts are meant to be passed into [method@ToastOverlay.add_toast] as
 * follows:
 *
 * ```c
 * adap_toast_overlay_add_toast (overlay, adap_toast_new (_("Simple Toast")));
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
 * AdapToast *toast = adap_toast_new (_("Toast with Action"));
 *
 * adap_toast_set_button_label (toast, _("_Example"));
 * adap_toast_set_action_name (toast, "win.example");
 *
 * adap_toast_overlay_add_toast (overlay, toast);
 * ```
 *
 * <picture>
 *   <source srcset="toast-action-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-action.png" alt="toast-action">
 * </picture>
 *
 * ## Modifying toasts
 *
 * Toasts can be modified after they have been shown. For this, an `AdapToast`
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
 *     self->undo_toast = adap_toast_new_format (_("‘%s’ deleted"), ...);
 *
 *     adap_toast_set_priority (self->undo_toast, ADAP_TOAST_PRIORITY_HIGH);
 *     adap_toast_set_button_label (self->undo_toast, _("_Undo"));
 *     adap_toast_set_action_name (self->undo_toast, "toast.undo");
 *
 *     g_signal_connect_swapped (self->undo_toast, "dismissed",
 *                               G_CALLBACK (dismissed_cb), self);
 *
 *     adap_toast_overlay_add_toast (self->toast_overlay, self->undo_toast);
 *
 *     return;
 *   }
 *
 *   title =
 *     g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
 *                                "<span font_features='tnum=1'>%d</span> items deleted",
 *                                n_items), n_items);
 *
 *   adap_toast_set_title (self->undo_toast, title);
 *
 *   // Bump the toast timeout
 *   adap_toast_overlay_add_toast (self->toast_overlay, g_object_ref (self->undo_toast));
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

struct _AdapToast {
  GObject parent_instance;

  char *title;
  char *button_label;
  char *action_name;
  GVariant *action_target;
  AdapToastPriority priority;
  guint timeout;
  GtkWidget *custom_title;
  gboolean use_markup;

  AdapToastOverlay *overlay;
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

G_DEFINE_FINAL_TYPE (AdapToast, adap_toast, G_TYPE_OBJECT)

static void
dismissed_cb (AdapToast *self)
{
  adap_toast_set_overlay (self, NULL);
}

static void
adap_toast_dispose (GObject *object)
{
  AdapToast *self = ADAP_TOAST (object);

  g_clear_object (&self->custom_title);

  G_OBJECT_CLASS (adap_toast_parent_class)->dispose (object);
}

static void
adap_toast_finalize (GObject *object)
{
  AdapToast *self = ADAP_TOAST (object);

  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->button_label, g_free);
  g_clear_pointer (&self->action_name, g_free);
  g_clear_pointer (&self->action_target, g_variant_unref);

  G_OBJECT_CLASS (adap_toast_parent_class)->finalize (object);
}

static void
adap_toast_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  AdapToast *self = ADAP_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adap_toast_get_title (self));
    break;
  case PROP_BUTTON_LABEL:
    g_value_set_string (value, adap_toast_get_button_label (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, adap_toast_get_action_name (self));
    break;
  case PROP_ACTION_TARGET:
    g_value_set_variant (value, adap_toast_get_action_target_value (self));
    break;
  case PROP_PRIORITY:
    g_value_set_enum (value, adap_toast_get_priority (self));
    break;
  case PROP_TIMEOUT:
    g_value_set_uint (value, adap_toast_get_timeout (self));
    break;
  case PROP_CUSTOM_TITLE:
    g_value_set_object (value, adap_toast_get_custom_title (self));
    break;
  case PROP_USE_MARKUP:
    g_value_set_boolean (value, adap_toast_get_use_markup (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  AdapToast *self = ADAP_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    adap_toast_set_title (self, g_value_get_string (value));
    break;
  case PROP_BUTTON_LABEL:
    adap_toast_set_button_label (self, g_value_get_string (value));
    break;
  case PROP_ACTION_NAME:
    adap_toast_set_action_name (self, g_value_get_string (value));
    break;
  case PROP_ACTION_TARGET:
    adap_toast_set_action_target_value (self, g_value_get_variant (value));
    break;
  case PROP_PRIORITY:
    adap_toast_set_priority (self, g_value_get_enum (value));
    break;
  case PROP_TIMEOUT:
    adap_toast_set_timeout (self, g_value_get_uint (value));
    break;
  case PROP_CUSTOM_TITLE:
    adap_toast_set_custom_title (self, g_value_get_object (value));
    break;
  case PROP_USE_MARKUP:
    adap_toast_set_use_markup (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_class_init (AdapToastClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adap_toast_dispose;
  object_class->finalize = adap_toast_finalize;
  object_class->get_property = adap_toast_get_property;
  object_class->set_property = adap_toast_set_property;

  /**
   * AdapToast:title: (attributes org.gtk.Property.get=adap_toast_get_title org.gtk.Property.set=adap_toast_set_title)
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
   * AdapToast:button-label: (attributes org.gtk.Property.get=adap_toast_get_button_label org.gtk.Property.set=adap_toast_set_button_label)
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
   * AdapToast:action-name: (attributes org.gtk.Property.get=adap_toast_get_action_name org.gtk.Property.set=adap_toast_set_action_name)
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
   * AdapToast:action-target: (attributes org.gtk.Property.get=adap_toast_get_action_target_value org.gtk.Property.set=adap_toast_set_action_target_value)
   *
   * The parameter for action invocations.
   */
  props[PROP_ACTION_TARGET] =
    g_param_spec_variant ("action-target", NULL, NULL,
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapToast:priority: (attributes org.gtk.Property.get=adap_toast_get_priority org.gtk.Property.set=adap_toast_set_priority)
   *
   * The priority of the toast.
   *
   * Priority controls how the toast behaves when another toast is already
   * being displayed.
   *
   * If the priority is `ADAP_TOAST_PRIORITY_NORMAL`, the toast will be queued.
   *
   * If the priority is `ADAP_TOAST_PRIORITY_HIGH`, the toast will be displayed
   * immediately, pushing the previous toast into the queue instead.
   */
  props[PROP_PRIORITY] =
    g_param_spec_enum ("priority", NULL, NULL,
                       ADAP_TYPE_TOAST_PRIORITY,
                       ADAP_TOAST_PRIORITY_NORMAL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapToast:timeout: (attributes org.gtk.Property.get=adap_toast_get_timeout org.gtk.Property.set=adap_toast_set_timeout)
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
   * AdapToast:custom-title: (attributes org.gtk.Property.get=adap_toast_get_custom_title org.gtk.Property.set=adap_toast_set_custom_title)
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
   * AdapToast:use-markup: (attributes org.gtk.Property.get=adap_toast_get_use_markup org.gtk.Property.set=adap_toast_set_use_markup)
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
   * AdapToast::dismissed:
   *
   * Emitted when the toast has been dismissed.
   */
  signals[SIGNAL_DISMISSED] =
    g_signal_new ("dismissed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_DISMISSED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapToast::button-clicked:
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
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_BUTTON_CLICKED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);
}

static void
adap_toast_init (AdapToast *self)
{
  self->title = g_strdup ("");
  self->priority = ADAP_TOAST_PRIORITY_NORMAL;
  self->timeout = 5;
  self->custom_title = NULL;
  self->use_markup = TRUE;

  g_signal_connect (self, "dismissed", G_CALLBACK (dismissed_cb), self);
}

/**
 * adap_toast_new:
 * @title: the title to be displayed
 *
 * Creates a new `AdapToast`.
 *
 * The toast will use @title as its title.
 *
 * @title can be marked up with the Pango text markup language.
 *
 * Returns: the new created `AdapToast`
 */
AdapToast *
adap_toast_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADAP_TYPE_TOAST,
                       "title", title,
                       NULL);
}

/**
 * adap_toast_new_format:
 * @format: the formatted string for the toast title
 * @...: the parameters to insert into the format string
 *
 * Creates a new `AdapToast`.
 *
 * The toast will use the format string as its title.
 *
 * See also: [ctor@Toast.new]
 *
 * Returns: the newly created toast object
 *
 * Since: 1.2
 */
AdapToast *
adap_toast_new_format (const char *format,
                      ...)
{
  AdapToast *res;
  va_list args;
  char *title;

  va_start (args, format);
  title = g_strdup_vprintf (format, args);
  va_end (args);

  res = g_object_new (ADAP_TYPE_TOAST,
                      "title", title,
                      NULL);

  g_free (title);

  return res;
}

/**
 * adap_toast_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a toast
 *
 * Gets the title that will be displayed on the toast.
 *
 * If a custom title has been set with [method@Adap.Toast.set_custom_title]
 * the return value will be %NULL.
 *
 * Returns: (nullable): the title
 */
const char *
adap_toast_get_title (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  if (self->custom_title == NULL)
    return self->title;

  return NULL;
}

/**
 * adap_toast_set_title: (attributes org.gtk.Method.set_property=title)
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
adap_toast_set_title (AdapToast   *self,
                     const char *title)
{
  g_return_if_fail (ADAP_IS_TOAST (self));
  g_return_if_fail (title != NULL);

  if (!g_strcmp0 (self->title, title))
    return;

  g_object_freeze_notify (G_OBJECT (self));

  adap_toast_set_custom_title (self, NULL);

  g_set_str (&self->title, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_toast_get_button_label: (attributes org.gtk.Method.get_property=button-label)
 * @self: a toast
 *
 * Gets the label to show on the button.
 *
 * Returns: (nullable): the button label
 */
const char *
adap_toast_get_button_label (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  return self->button_label;
}

/**
 * adap_toast_set_button_label: (attributes org.gtk.Method.set_property=button-label)
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
adap_toast_set_button_label (AdapToast   *self,
                            const char *button_label)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

  if (!g_set_str (&self->button_label, button_label))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BUTTON_LABEL]);
}

/**
 * adap_toast_get_action_name: (attributes org.gtk.Method.get_property=action-name)
 * @self: a toast
 *
 * Gets the name of the associated action.
 *
 * Returns: (nullable): the action name
 */
const char *
adap_toast_get_action_name (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  return self->action_name;
}

/**
 * adap_toast_set_action_name: (attributes org.gtk.Method.set_property=action-name)
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
adap_toast_set_action_name (AdapToast   *self,
                           const char *action_name)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

  if (!g_set_str (&self->action_name, action_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_NAME]);
}

/**
 * adap_toast_get_action_target_value: (attributes org.gtk.Method.get_property=action-target)
 * @self: a toast
 *
 * Gets the parameter for action invocations.
 *
 * Returns: (transfer none) (nullable): the action target
 */
GVariant *
adap_toast_get_action_target_value (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  return self->action_target;
}

/**
 * adap_toast_set_action_target_value: (attributes org.gtk.Method.set_property=action-target)
 * @self: a toast
 * @action_target: (nullable): the action target
 *
 * Sets the parameter for action invocations.
 *
 * If the @action_target variant has a floating reference this function
 * will sink it.
 */
void
adap_toast_set_action_target_value (AdapToast *self,
                                   GVariant *action_target)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

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
 * adap_toast_set_action_target: (skip)
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
adap_toast_set_action_target (AdapToast   *self,
                             const char *format_string,
                             ...)
{
  va_list args;

  va_start (args, format_string);
  adap_toast_set_action_target_value (self,
                                     g_variant_new_va (format_string,
                                                       NULL, &args));
  va_end (args);
}

/**
 * adap_toast_set_detailed_action_name:
 * @self: a toast
 * @detailed_action_name: (nullable): the detailed action name
 *
 * Sets the action name and its parameter.
 *
 * @detailed_action_name is a string in the format accepted by
 * [func@Gio.Action.parse_detailed_name].
 */
void
adap_toast_set_detailed_action_name (AdapToast   *self,
                                    const char *detailed_action_name)
{
  char *name;
  GVariant *target;
  GError *error = NULL;

  g_return_if_fail (ADAP_IS_TOAST (self));

  if (!detailed_action_name) {
    adap_toast_set_action_name (self, NULL);
    adap_toast_set_action_target_value (self, NULL);

    return;
  }

  if (g_action_parse_detailed_name (detailed_action_name, &name, &target, &error)) {
    adap_toast_set_action_name (self, name);
    adap_toast_set_action_target_value (self, target);
  } else {
    g_critical ("Couldn't parse detailed action name: %s", error->message);
  }

  g_clear_error (&error);
  g_clear_pointer (&target, g_variant_unref);
  g_clear_pointer (&name, g_free);
}

/**
 * adap_toast_get_priority: (attributes org.gtk.Method.get_property=priority)
 * @self: a toast
 *
 * Gets priority for @self.
 *
 * Returns: the priority
 */
AdapToastPriority
adap_toast_get_priority (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), ADAP_TOAST_PRIORITY_NORMAL);

  return self->priority;
}

/**
 * adap_toast_set_priority: (attributes org.gtk.Method.set_property=priority)
 * @self: a toast
 * @priority: the priority
 *
 * Sets priority for @self.
 *
 * Priority controls how the toast behaves when another toast is already
 * being displayed.
 *
 * If @priority is `ADAP_TOAST_PRIORITY_NORMAL`, the toast will be queued.
 *
 * If @priority is `ADAP_TOAST_PRIORITY_HIGH`, the toast will be displayed
 * immediately, pushing the previous toast into the queue instead.
 */
void
adap_toast_set_priority (AdapToast         *self,
                        AdapToastPriority  priority)
{
  g_return_if_fail (ADAP_IS_TOAST (self));
  g_return_if_fail (priority >= ADAP_TOAST_PRIORITY_NORMAL &&
                    priority <= ADAP_TOAST_PRIORITY_HIGH);

  if (self->priority == priority)
    return;

  self->priority = priority;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PRIORITY]);
}

/**
 * adap_toast_get_timeout: (attributes org.gtk.Method.get_property=timeout)
 * @self: a toast
 *
 * Gets timeout for @self.
 *
 * Returns: the timeout
 */
guint
adap_toast_get_timeout (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), 0);

  return self->timeout;
}

/**
 * adap_toast_set_timeout: (attributes org.gtk.Method.set_property=timeout)
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
adap_toast_set_timeout (AdapToast *self,
                       guint     timeout)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

  if (self->timeout == timeout)
    return;

  self->timeout = timeout;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMEOUT]);
}

/**
 * adap_toast_get_custom_title: (attributes org.gtk.Method.get_property=custom-title)
 * @self: a toast
 *
 * Gets the custom title widget of @self.
 *
 * Returns: (nullable) (transfer none): the custom title widget
 *
 * Since: 1.2
 */
GtkWidget *
adap_toast_get_custom_title (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  return self->custom_title;
}

/**
 * adap_toast_set_custom_title: (attributes org.gtk.Method.set_property=custom-title)
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
adap_toast_set_custom_title (AdapToast  *self,
                            GtkWidget *widget)
{
  g_return_if_fail (ADAP_IS_TOAST (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  if (widget)
    g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  if (self->custom_title == widget)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  adap_toast_set_title (self, "");

  g_set_object (&self->custom_title, widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CUSTOM_TITLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_toast_dismiss:
 * @self: a toast
 *
 * Dismisses @self.
 *
 * Does nothing if @self has already been dismissed, or hasn't been added to an
 * [class@ToastOverlay].
 */
void
adap_toast_dismiss (AdapToast *self)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

  if (!self->overlay)
    return;

  g_signal_emit (self, signals[SIGNAL_DISMISSED], 0, NULL);
}

AdapToastOverlay *
adap_toast_get_overlay (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), NULL);

  return self->overlay;
}

void
adap_toast_set_overlay (AdapToast        *self,
                       AdapToastOverlay *overlay)
{
  g_return_if_fail (ADAP_IS_TOAST (self));
  g_return_if_fail (overlay == NULL || ADAP_IS_TOAST_OVERLAY (overlay));

  self->overlay = overlay;
}

/**
 * adap_toast_get_use_markup: (attributes org.gtk.Method.get_property=use-markup)
 * @self: a toast
 *
 * Gets whether to use Pango markup for the toast title.
 *
 * Returns: whether the toast uses markup
 *
 * Since: 1.4
 */
gboolean
adap_toast_get_use_markup (AdapToast *self)
{
  g_return_val_if_fail (ADAP_IS_TOAST (self), FALSE);

  return self->use_markup;
}

/**
 * adap_toast_set_use_markup: (attributes org.gtk.Method.set_property=use-markup)
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
adap_toast_set_use_markup (AdapToast *self,
                          gboolean  use_markup)
{
  g_return_if_fail (ADAP_IS_TOAST (self));

  use_markup = !!use_markup;

  if (adap_toast_get_use_markup (self) == use_markup)
    return;

  self->use_markup = use_markup;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_MARKUP]);
}
