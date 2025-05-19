/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-split-button.h"

#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

/**
 * AdwSplitButton:
 *
 * A combined button and dropdown widget.
 *
 * <picture>
 *   <source srcset="split-button-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="split-button.png" alt="split-button">
 * </picture>
 *
 * `AdwSplitButton` is typically used to present a set of actions in a menu,
 * but allow access to one of them with a single click.
 *
 * The API is very similar to [class@Gtk.Button] and [class@Gtk.MenuButton], see
 * their documentation for details.
 *
 * ## CSS nodes
 *
 * ```
 * splitbutton[.image-button][.text-button]
 * ├── button
 * │   ╰── <content>
 * ├── separator
 * ╰── menubutton
 *     ╰── button.toggle
 *         ╰── arrow
 * ```
 *
 * `AdwSplitButton`'s CSS node is called `splitbutton`. It contains the css
 * nodes: `button`, `separator`, `menubutton`. See [class@Gtk.MenuButton]
 * documentation for the `menubutton` contents.
 *
 * The main CSS node will contain the `.image-button` or `.text-button` style
 * classes matching the button contents. The nested button nodes will never
 * contain them.
 *
 * ## Accessibility
 *
 * `AdwSplitButton` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 */

enum {
  PROP_0,
  PROP_LABEL,
  PROP_USE_UNDERLINE,
  PROP_ICON_NAME,
  PROP_CHILD,
  PROP_CAN_SHRINK,
  PROP_MENU_MODEL,
  PROP_POPOVER,
  PROP_DIRECTION,
  PROP_DROPDOWN_TOOLTIP,

  /* actionable properties */
  PROP_ACTION_NAME,
  PROP_ACTION_TARGET,
  LAST_PROP = PROP_ACTION_NAME
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CLICKED,
  SIGNAL_ACTIVATE,
  SIGNAL_LAST_SIGNAL
};

static guint signals[SIGNAL_LAST_SIGNAL];

struct _AdwSplitButton
{
  GtkWidget parent_instance;

  GtkWidget *button;
  GtkWidget *menu_button;
  GtkWidget *arrow_button;
  GtkWidget *separator;

  guint disposed : 1;
  guint has_dropdown_tooltip : 1;
};

static void adw_split_button_actionable_init (GtkActionableInterface *iface);
static void adw_split_button_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSplitButton, adw_split_button, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACTIONABLE, adw_split_button_actionable_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_split_button_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
update_state (AdwSplitButton *self)
{
  GtkStateFlags flags;
  gboolean keyboard_activating;

  if (self->disposed)
    return;

  flags = gtk_widget_get_state_flags (self->button) |
          gtk_widget_get_state_flags (self->arrow_button);

  keyboard_activating =
    gtk_widget_has_css_class (self->button, "keyboard-activating") ||
    gtk_widget_has_css_class (self->arrow_button, "keyboard-activating");

  if (flags & GTK_STATE_FLAG_ACTIVE || keyboard_activating)
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_ACTIVE, FALSE);
  else
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_ACTIVE);

  if (flags & GTK_STATE_FLAG_CHECKED)
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED, FALSE);
  else
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED);
}

static void
update_style_classes (AdwSplitButton *self)
{
  const char *label = gtk_button_get_label (GTK_BUTTON (self->button));
  const char *icon_name = gtk_button_get_icon_name (GTK_BUTTON (self->button));

  if (icon_name && icon_name[0])
    gtk_widget_add_css_class (GTK_WIDGET (self), "image-button");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "image-button");

  if (label && label[0])
    gtk_widget_add_css_class (GTK_WIDGET (self), "text-button");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "text-button");

  gtk_widget_remove_css_class (self->button, "text-button");
  gtk_widget_remove_css_class (self->button, "image-button");

  gtk_widget_remove_css_class (self->arrow_button, "image-button");
}

static void
clicked_cb (AdwSplitButton *self)
{
  g_signal_emit (self, signals[SIGNAL_CLICKED], 0);
}

static void
activate_cb (AdwSplitButton *self)
{
  g_signal_emit_by_name (self->button, "activate");
}

static void
adw_split_button_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (object);

  switch (prop_id) {
  case PROP_LABEL:
    g_value_set_string (value, adw_split_button_get_label (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_split_button_get_use_underline (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_split_button_get_icon_name (self));
    break;
  case PROP_CHILD:
    g_value_set_object (value, adw_split_button_get_child (self));
    break;
  case PROP_CAN_SHRINK:
    g_value_set_boolean (value, adw_split_button_get_can_shrink (self));
    break;
  case PROP_MENU_MODEL:
    g_value_set_object (value, adw_split_button_get_menu_model (self));
    break;
  case PROP_POPOVER:
    g_value_set_object (value, adw_split_button_get_popover (self));
    break;
  case PROP_DIRECTION:
    g_value_set_enum (value, adw_split_button_get_direction (self));
    break;
  case PROP_DROPDOWN_TOOLTIP:
    g_value_set_string (value, adw_split_button_get_dropdown_tooltip (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, gtk_actionable_get_action_name (GTK_ACTIONABLE (self)));
    break;
  case PROP_ACTION_TARGET:
    g_value_set_variant (value, gtk_actionable_get_action_target_value (GTK_ACTIONABLE (self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_split_button_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (object);

  switch (prop_id) {
  case PROP_LABEL:
    adw_split_button_set_label (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    adw_split_button_set_use_underline (self, g_value_get_boolean (value));
    break;
  case PROP_ICON_NAME:
    adw_split_button_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_CHILD:
    adw_split_button_set_child (self, g_value_get_object (value));
    break;
  case PROP_CAN_SHRINK:
    adw_split_button_set_can_shrink (self, g_value_get_boolean (value));
    break;
  case PROP_MENU_MODEL:
    adw_split_button_set_menu_model (self, g_value_get_object (value));
    break;
  case PROP_POPOVER:
    adw_split_button_set_popover (self, g_value_get_object (value));
    break;
  case PROP_DIRECTION:
    adw_split_button_set_direction (self, g_value_get_enum (value));
    break;
  case PROP_DROPDOWN_TOOLTIP:
    adw_split_button_set_dropdown_tooltip (self, g_value_get_string (value));
    break;
  case PROP_ACTION_NAME:
    gtk_actionable_set_action_name (GTK_ACTIONABLE (self), g_value_get_string (value));
    break;
  case PROP_ACTION_TARGET:
    gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self), g_value_get_variant (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_split_button_dispose (GObject *object)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (object);

  self->disposed = TRUE;

  g_clear_pointer (&self->button, gtk_widget_unparent);
  g_clear_pointer (&self->menu_button, gtk_widget_unparent);
  g_clear_pointer (&self->separator, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_split_button_parent_class)->dispose (object);
}

static void
adw_split_button_class_init (AdwSplitButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_split_button_get_property;
  object_class->set_property = adw_split_button_set_property;
  object_class->dispose = adw_split_button_dispose;

  widget_class->focus = adw_widget_focus_child;
  widget_class->grab_focus = adw_widget_grab_focus_child;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwSplitButton:label: (attributes org.gtk.Property.get=adw_split_button_get_label org.gtk.Property.set=adw_split_button_set_label)
   *
   * The label for the button.
   *
   * Setting the label will set [property@SplitButton:icon-name] and
   * [property@SplitButton:child] to `NULL`.
   */
  props[PROP_LABEL] =
    g_param_spec_string ("label", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:use-underline: (attributes org.gtk.Property.get=adw_split_button_get_use_underline org.gtk.Property.set=adw_split_button_set_use_underline)
   *
   * Whether an underline in the text indicates a mnemonic.
   *
   * See [property@SplitButton:label].
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:icon-name: (attributes org.gtk.Property.get=adw_split_button_get_icon_name org.gtk.Property.set=adw_split_button_set_icon_name)
   *
   * The name of the icon used to automatically populate the button.
   *
   * Setting the icon name will set [property@SplitButton:label] and
   * [property@SplitButton:child] to `NULL`.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:child: (attributes org.gtk.Property.get=adw_split_button_get_child org.gtk.Property.set=adw_split_button_set_child)
   *
   * The child widget.
   *
   * Setting the child widget will set [property@SplitButton:label] and
   * [property@SplitButton:icon-name] to `NULL`.
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:can-shrink: (attributes org.gtk.Property.get=adw_split_button_get_can_shrink org.gtk.Property.set=adw_split_button_set_can_shrink)
   *
   * Whether the button can be smaller than the natural size of its contents.
   *
   * If set to `TRUE`, the label will ellipsize.
   *
   * See [property@Gtk.Button:can-shrink] and
   * [property@Gtk.MenuButton:can-shrink].
   *
   * Since: 1.4
   */
  props[PROP_CAN_SHRINK] =
    g_param_spec_boolean ("can-shrink", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:menu-model: (attributes org.gtk.Property.get=adw_split_button_get_menu_model org.gtk.Property.set=adw_split_button_set_menu_model)
   *
   * The `GMenuModel` from which the popup will be created.
   *
   * If the menu model is `NULL`, the dropdown is disabled.
   *
   * A [class@Gtk.Popover] will be created from the menu model with
   * [ctor@Gtk.PopoverMenu.new_from_model]. Actions will be connected as
   * documented for this function.
   *
   * If [property@SplitButton:popover] is already set, it will be dissociated
   * from the button, and the property is set to `NULL`.
   */
  props[PROP_MENU_MODEL] =
    g_param_spec_object ("menu-model", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:popover: (attributes org.gtk.Property.get=adw_split_button_get_popover org.gtk.Property.set=adw_split_button_set_popover)
   *
   * The `GtkPopover` that will be popped up when the dropdown is clicked.
   *
   * If the popover is `NULL`, the dropdown is disabled.
   *
   * If [property@SplitButton:menu-model] is set, the menu model is dissociated
   * from the button, and the property is set to `NULL`.
   */
  props[PROP_POPOVER] =
    g_param_spec_object ("popover", NULL, NULL,
                         GTK_TYPE_POPOVER,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:direction: (attributes org.gtk.Property.get=adw_split_button_get_direction org.gtk.Property.set=adw_split_button_set_direction)
   *
   * The direction in which the popup will be popped up.
   *
   * The dropdown arrow icon will point at the same direction.
   *
   * If the does not fit in the available space in the given direction, GTK will
   * try its best to keep it inside the screen and fully visible.
   *
   * If you pass `GTK_ARROW_NONE`, it's equivalent to `GTK_ARROW_DOWN`.
   */
  props[PROP_DIRECTION] =
    g_param_spec_enum ("direction", NULL, NULL,
                       GTK_TYPE_ARROW_TYPE,
                       GTK_ARROW_DOWN,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSplitButton:dropdown-tooltip: (attributes org.gtk.Property.get=adw_split_button_get_dropdown_tooltip org.gtk.Property.set=adw_split_button_set_dropdown_tooltip)
   *
   * The tooltip of the dropdown button.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * Since: 1.2
   */
  props[PROP_DROPDOWN_TOOLTIP] =
    g_param_spec_string ("dropdown-tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class, PROP_ACTION_NAME, "action-name");
  g_object_class_override_property (object_class, PROP_ACTION_TARGET, "action-target");

  /**
   * AdwSplitButton::clicked:
   *
   * Emitted when the button has been activated (pressed and released).
   */
  signals[SIGNAL_CLICKED] =
    g_signal_new ("clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLICKED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwSplitButton::activate:
   *
   * Emitted to animate press then release.
   *
   * This is an action signal. Applications should never connect to this signal,
   * but use the [signal@SplitButton::clicked] signal.
   */
  signals[SIGNAL_ACTIVATE] =
    g_signal_new ("activate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_set_activate_signal (widget_class, signals[SIGNAL_ACTIVATE]);

  g_signal_override_class_handler ("activate",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (activate_cb));

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "splitbutton");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

#define NOTIFY(func, prop) \
static void \
func (GObject *object) { \
  g_object_notify_by_pspec (object, props[prop]); \
}

NOTIFY (notify_use_underline_cb, PROP_USE_UNDERLINE);
NOTIFY (notify_menu_model_cb, PROP_MENU_MODEL);
NOTIFY (notify_popover_cb, PROP_POPOVER);
NOTIFY (notify_direction_cb, PROP_DIRECTION);

static void
notify_action_name_cb (GObject *object)
{
  g_object_notify (object, "action-name");
}

static void
notify_action_target_cb (GObject *object)
{
  g_object_notify (object, "action-target");
}

static void
adw_split_button_init (AdwSplitButton *self)
{
  gtk_widget_set_hexpand (GTK_WIDGET (self), FALSE);

  self->button = gtk_button_new ();
  gtk_widget_set_parent (self->button, GTK_WIDGET (self));
  gtk_widget_set_hexpand (self->button, TRUE);
  gtk_accessible_update_relation (GTK_ACCESSIBLE (self->button),
                                  GTK_ACCESSIBLE_RELATION_LABELLED_BY, self, NULL,
                                  GTK_ACCESSIBLE_RELATION_DESCRIBED_BY, self, NULL,
                                  -1);

  self->separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_parent (self->separator, GTK_WIDGET (self));

  self->menu_button = gtk_menu_button_new ();
  gtk_widget_set_parent (self->menu_button, GTK_WIDGET (self));
  gtk_widget_set_tooltip_text (self->menu_button, _("More Options"));

  /* FIXME: This is iffy, but we don't have any other way to do it */
  self->arrow_button = gtk_widget_get_first_child (self->menu_button);

  g_signal_connect_swapped (self->button, "clicked", G_CALLBACK (clicked_cb), self);

  g_signal_connect_swapped (self->button, "notify::css-classes", G_CALLBACK (update_state), self);
  g_signal_connect_swapped (self->button, "state-flags-changed", G_CALLBACK (update_state), self);
  g_signal_connect_swapped (self->arrow_button, "notify::css-classes", G_CALLBACK (update_state), self);
  g_signal_connect_swapped (self->arrow_button, "state-flags-changed", G_CALLBACK (update_state), self);

  g_signal_connect_swapped (self->button, "notify::use-underline", G_CALLBACK (notify_use_underline_cb), self);
  g_signal_connect_swapped (self->button, "notify::action-name", G_CALLBACK (notify_action_name_cb), self);
  g_signal_connect_swapped (self->button, "notify::action-target", G_CALLBACK (notify_action_target_cb), self);
  g_signal_connect_swapped (self->menu_button, "notify::menu-model", G_CALLBACK (notify_menu_model_cb), self);
  g_signal_connect_swapped (self->menu_button, "notify::popover", G_CALLBACK (notify_popover_cb), self);
  g_signal_connect_swapped (self->menu_button, "notify::direction", G_CALLBACK (notify_direction_cb), self);

  update_style_classes (self);
}

static const char *
adw_split_button_get_action_name (GtkActionable *actionable)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (actionable);

  return gtk_actionable_get_action_name (GTK_ACTIONABLE (self->button));
}

static void
adw_split_button_set_action_name (GtkActionable *actionable,
                                  const char    *action_name)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (actionable);

  gtk_actionable_set_action_name (GTK_ACTIONABLE (self->button), action_name);
}

static GVariant *
adw_split_button_get_action_target_value (GtkActionable *actionable)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (actionable);

  return gtk_actionable_get_action_target_value (GTK_ACTIONABLE (self->button));
}

static void
adw_split_button_set_action_target_value (GtkActionable *actionable,
                                          GVariant      *action_target)
{
  AdwSplitButton *self = ADW_SPLIT_BUTTON (actionable);

  gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self->button), action_target);
}

static void
adw_split_button_actionable_init (GtkActionableInterface *iface)
{
  iface->get_action_name = adw_split_button_get_action_name;
  iface->set_action_name = adw_split_button_set_action_name;
  iface->get_action_target_value = adw_split_button_get_action_target_value;
  iface->set_action_target_value = adw_split_button_set_action_target_value;
}

static void
adw_split_button_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  if (GTK_IS_POPOVER (child))
    adw_split_button_set_popover (ADW_SPLIT_BUTTON (buildable), GTK_POPOVER (child));
  else if (GTK_IS_WIDGET (child))
    adw_split_button_set_child (ADW_SPLIT_BUTTON (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_split_button_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_split_button_buildable_add_child;
}

/**
 * adw_split_button_new:
 *
 * Creates a new `AdwSplitButton`.
 *
 * Returns: the newly created `AdwSplitButton`
 */
GtkWidget *
adw_split_button_new (void)
{
  return g_object_new (ADW_TYPE_SPLIT_BUTTON, NULL);
}

/**
 * adw_split_button_get_label: (attributes org.gtk.Method.get_property=label)
 * @self: a split button
 *
 * Gets the label for @self.
 *
 * Returns: (nullable): the label for @self
 */
const char *
adw_split_button_get_label (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  return gtk_button_get_label (GTK_BUTTON (self->button));
}

/**
 * adw_split_button_set_label: (attributes org.gtk.Method.set_property=label)
 * @self: a split button
 * @label: the label to set
 *
 * Sets the label for @self.
 *
 * Setting the label will set [property@SplitButton:icon-name] and
 * [property@SplitButton:child] to `NULL`.
 */
void
adw_split_button_set_label (AdwSplitButton *self,
                            const char     *label)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));
  g_return_if_fail (label != NULL);

  if (!g_strcmp0 (label, adw_split_button_get_label (self)))
    return;

  g_object_freeze_notify (G_OBJECT (self));
  if (adw_split_button_get_icon_name (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
  if (adw_split_button_get_child (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);

  gtk_button_set_label (GTK_BUTTON (self->button), label);
  update_style_classes (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_split_button_get_use_underline: (attributes org.gtk.Method.get_property=use-underline)
 * @self: a split button
 *
 * Gets whether an underline in the text indicates a mnemonic.
 *
 * Returns: whether an underline in the text indicates a mnemonic
 */
gboolean
adw_split_button_get_use_underline (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), FALSE);

  return gtk_button_get_use_underline (GTK_BUTTON (self->button));
}

/**
 * adw_split_button_set_use_underline: (attributes org.gtk.Method.set_property=use-underline)
 * @self: a split button
 * @use_underline: whether an underline in the text indicates a mnemonic
 *
 * Sets whether an underline in the text indicates a mnemonic.
 *
 * See [property@SplitButton:label].
 */
void
adw_split_button_set_use_underline (AdwSplitButton *self,
                                    gboolean        use_underline)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  use_underline = !!use_underline;

  if (use_underline == adw_split_button_get_use_underline (self))
    return;

  gtk_button_set_use_underline (GTK_BUTTON (self->button), use_underline);
}

/**
 * adw_split_button_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a split button
 *
 * Gets the name of the icon used to automatically populate the button.
 *
 * Returns: (nullable): the icon name
 */
const char *
adw_split_button_get_icon_name (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  return gtk_button_get_icon_name (GTK_BUTTON (self->button));
}

/**
 * adw_split_button_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a split button
 * @icon_name: the icon name to set
 *
 * Sets the name of the icon used to automatically populate the button.
 *
 * Setting the icon name will set [property@SplitButton:label] and
 * [property@SplitButton:child] to `NULL`.
 */
void
adw_split_button_set_icon_name (AdwSplitButton *self,
                                const char     *icon_name)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));
  g_return_if_fail (icon_name != NULL);

  if (!g_strcmp0 (icon_name, adw_split_button_get_icon_name (self)))
    return;

  g_object_freeze_notify (G_OBJECT (self));
  if (adw_split_button_get_label (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
  if (adw_split_button_get_child (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);

  gtk_button_set_icon_name (GTK_BUTTON (self->button), icon_name);

  update_style_classes (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_split_button_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a split button
 *
 * Gets the child widget.
 *
 * Returns: (transfer none) (nullable): the child widget
 */
GtkWidget *
adw_split_button_get_child (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  return gtk_button_get_child (GTK_BUTTON (self->button));
}

/**
 * adw_split_button_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a split button
 * @child: (nullable): the new child widget
 *
 * Sets the child widget.
 *
 * Setting the child widget will set [property@SplitButton:label] and
 * [property@SplitButton:icon-name] to `NULL`.
 */
void
adw_split_button_set_child (AdwSplitButton *self,
                            GtkWidget      *child)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (child == adw_split_button_get_child (self))
    return;

  g_object_freeze_notify (G_OBJECT (self));
  if (adw_split_button_get_label (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
  if (adw_split_button_get_icon_name (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);

  gtk_button_set_child (GTK_BUTTON (self->button), child);

  update_style_classes (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_split_button_get_can_shrink: (attributes org.gtk.Method.get_property=can-shrink)
 * @self: a split button
 *
 * gets whether the button can be smaller than the natural size of its contents.
 *
 * Returns: whether the button can shrink
 *
 * Since: 1.4
 */
gboolean
adw_split_button_get_can_shrink (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), FALSE);

  return gtk_button_get_can_shrink (GTK_BUTTON (self->button));
}

/**
 * adw_split_button_set_can_shrink: (attributes org.gtk.Method.set_property=can-shrink)
 * @self: a split button
 * @can_shrink: whether the button can shrink
 *
 * Sets whether the button can be smaller than the natural size of its contents.
 *
 * If set to `TRUE`, the label will ellipsize.
 *
 * See [method@Gtk.Button.set_can_shrink] and
 * [method@Gtk.MenuButton.set_can_shrink].
 *
 * Since: 1.4
 */
void
adw_split_button_set_can_shrink (AdwSplitButton *self,
                                 gboolean        can_shrink)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  can_shrink = !!can_shrink;

  if (can_shrink == adw_split_button_get_can_shrink (self))
    return;

  gtk_button_set_can_shrink (GTK_BUTTON (self->button), can_shrink);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SHRINK]);
}

/**
 * adw_split_button_get_menu_model: (attributes org.gtk.Method.get_property=menu-model)
 * @self: a split button
 *
 * Gets the menu model from which the popup will be created.
 *
 * Returns: (transfer none) (nullable): the menu model
 */
GMenuModel *
adw_split_button_get_menu_model (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  return gtk_menu_button_get_menu_model (GTK_MENU_BUTTON (self->menu_button));
}

/**
 * adw_split_button_set_menu_model: (attributes org.gtk.Method.set_property=menu-model)
 * @self: a split button
 * @menu_model: (nullable): the menu model
 *
 * Sets the menu model from which the popup will be created.
 *
 * If the menu model is `NULL`, the dropdown is disabled.
 *
 * A [class@Gtk.Popover] will be created from the menu model with
 * [ctor@Gtk.PopoverMenu.new_from_model]. Actions will be connected as
 * documented for this function.
 *
 * If [property@SplitButton:popover] is already set, it will be dissociated from
 * the button, and the property is set to `NULL`.
 */
void
adw_split_button_set_menu_model (AdwSplitButton *self,
                                 GMenuModel     *menu_model)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  if (menu_model == adw_split_button_get_menu_model (self))
    return;

  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->menu_button), menu_model);
}

/**
 * adw_split_button_get_popover: (attributes org.gtk.Method.get_property=popover)
 * @self: a split button
 *
 * Gets the popover that will be popped up when the dropdown is clicked.
 *
 * Returns: (transfer none) (nullable): the popover
 */
GtkPopover *
adw_split_button_get_popover (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  return gtk_menu_button_get_popover (GTK_MENU_BUTTON (self->menu_button));
}

/**
 * adw_split_button_set_popover: (attributes org.gtk.Method.set_property=popover)
 * @self: a split button
 * @popover: (nullable): the popover
 *
 * Sets the popover that will be popped up when the dropdown is clicked.
 *
 * If the popover is `NULL`, the dropdown is disabled.
 *
 * If [property@SplitButton:menu-model] is set, the menu model is dissociated
 * from the button, and the property is set to `NULL`.
 */
void
adw_split_button_set_popover (AdwSplitButton *self,
                              GtkPopover     *popover)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  if (popover == adw_split_button_get_popover (self))
    return;

  gtk_menu_button_set_popover (GTK_MENU_BUTTON (self->menu_button), GTK_WIDGET (popover));
}

/**
 * adw_split_button_get_direction: (attributes org.gtk.Method.get_property=direction)
 * @self: a split button
 *
 * Gets the direction in which the popup will be popped up.
 *
 * Returns: the direction
 */
GtkArrowType
adw_split_button_get_direction (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), GTK_ARROW_DOWN);

  return gtk_menu_button_get_direction (GTK_MENU_BUTTON (self->menu_button));
}

/**
 * adw_split_button_set_direction: (attributes org.gtk.Method.set_property=direction)
 * @self: a split button
 * @direction: the direction
 *
 * Sets the direction in which the popup will be popped up.
 *
 * The dropdown arrow icon will point at the same direction.
 *
 * If the does not fit in the available space in the given direction, GTK will
 * try its best to keep it inside the screen and fully visible.
 *
 * If you pass `GTK_ARROW_NONE`, it's equivalent to `GTK_ARROW_DOWN`.
 */
void
adw_split_button_set_direction (AdwSplitButton *self,
                                GtkArrowType    direction)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  if (direction == adw_split_button_get_direction (self))
    return;

  gtk_menu_button_set_direction (GTK_MENU_BUTTON (self->menu_button), direction);

  update_style_classes (self);
}

/**
 * adw_split_button_get_dropdown_tooltip: (attributes org.gtk.Method.get_property=dropdown-tooltip)
 * @self: a split button
 *
 * Gets the tooltip of the dropdown button of @self.
 *
 * Returns: (transfer none): the dropdown tooltip of @self
 *
 * Since: 1.2
 */
const char *
adw_split_button_get_dropdown_tooltip (AdwSplitButton *self)
{
  g_return_val_if_fail (ADW_IS_SPLIT_BUTTON (self), NULL);

  if (!self->has_dropdown_tooltip)
    return "";

  return gtk_widget_get_tooltip_markup (self->menu_button);
}

/**
 * adw_split_button_set_dropdown_tooltip: (attributes org.gtk.Method.set_property=dropdown-tooltip)
 * @self: a split button
 * @tooltip: the dropdown tooltip of @self
 *
 * Sets the tooltip of the dropdown button of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * Since: 1.2
 */
void
adw_split_button_set_dropdown_tooltip (AdwSplitButton *self,
                                       const char     *tooltip)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));
  g_return_if_fail (tooltip != NULL);

  if (!g_strcmp0 (tooltip, adw_split_button_get_dropdown_tooltip (self)))
    return;

  self->has_dropdown_tooltip = tooltip && *tooltip;

  if (self->has_dropdown_tooltip)
    gtk_widget_set_tooltip_markup (self->menu_button, tooltip);
  else
    gtk_widget_set_tooltip_text (self->menu_button, _("More Options"));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DROPDOWN_TOOLTIP]);
}

/**
 * adw_split_button_popup:
 * @self: a split button
 *
 * Pops up the menu.
 */
void
adw_split_button_popup (AdwSplitButton *self)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  gtk_menu_button_popup (GTK_MENU_BUTTON (self->menu_button));
}

/**
 * adw_split_button_popdown:
 * @self: a split button
 *
 * Dismisses the menu.
 */
void
adw_split_button_popdown (AdwSplitButton *self)
{
  g_return_if_fail (ADW_IS_SPLIT_BUTTON (self));

  gtk_menu_button_popdown (GTK_MENU_BUTTON (self->menu_button));
}
