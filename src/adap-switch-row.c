/*
 * Copyright (C) 2023 Joshua Lee <lee.son.wai@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-switch-row.h"

/**
 * AdwSwitchRow:
 *
 * A [class@Gtk.ListBoxRow] used to represent two states.
 *
 * <picture>
 *   <source srcset="switch-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="switch-row.png" alt="switch-row">
 * </picture>
 *
 * The `AdwSwitchRow` widget contains a [class@Gtk.Switch] that allows the user
 * to select between two states: "on" or "off". When activated, the row will
 * invert its active state.
 *
 * The user can control the switch by activating the row or by dragging on the
 * switch handle.
 *
 * See [class@Gtk.Switch] for details.
 *
 * Example of an `AdwSwitchRow` UI definition:
 * ```xml
 * <object class="AdwSwitchRow">
 *   <property name="title" translatable="yes">Switch Row</property>
 *   <signal name="notify::active" handler="switch_row_notify_active_cb"/>
 * </object>
 * ```
 *
 * The [property@SwitchRow:active] property should be connected to in order to
 * monitor changes to the active state.
 *
 * ## Accessibility
 *
 * `AdwSwitchRow` uses the `GTK_ACCESSIBLE_ROLE_SWITCH` role.
 *
 * Since: 1.4
 */

struct _AdwSwitchRow
{
  AdwActionRow parent_instance;

  GtkWidget *slider;
};

G_DEFINE_FINAL_TYPE (AdwSwitchRow, adw_switch_row, ADW_TYPE_ACTION_ROW)

enum
{
  PROP_0,
  PROP_ACTIVE,
  PROP_LAST_PROP,
};

static GParamSpec *props[PROP_LAST_PROP];

static void
slider_notify_active_cb (AdwSwitchRow *self)
{
  g_assert (ADW_IS_SWITCH_ROW (self));

  gtk_accessible_update_state (GTK_ACCESSIBLE (self),
                               GTK_ACCESSIBLE_STATE_CHECKED,
                               gtk_switch_get_active (GTK_SWITCH (self->slider)),
                               -1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE]);
}

static void
adw_switch_row_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwSwitchRow *self = ADW_SWITCH_ROW (object);

  switch (prop_id) {
  case PROP_ACTIVE:
    g_value_set_boolean (value, adw_switch_row_get_active (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_switch_row_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwSwitchRow *self = ADW_SWITCH_ROW (object);

  switch (prop_id) {
  case PROP_ACTIVE:
    adw_switch_row_set_active (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_switch_row_dispose (GObject *object)
{
  AdwSwitchRow *self = ADW_SWITCH_ROW (object);

  g_clear_pointer (&self->slider, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_switch_row_parent_class)->dispose (object);
}

static void
adw_switch_row_class_init (AdwSwitchRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_switch_row_get_property;
  object_class->set_property = adw_switch_row_set_property;
  object_class->dispose = adw_switch_row_dispose;

  /**
   * AdwSwitchRow:active: (attributes org.gtk.Property.get=adw_switch_row_get_active org.gtk.Property.set=adw_switch_row_set_active)
   *
   * Whether the switch row is in the "on" or "off" position.
   *
   * Since: 1.4
   */
  props[PROP_ACTIVE] =
    g_param_spec_boolean ("active", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_SWITCH);
}

static void
adw_switch_row_init (AdwSwitchRow *self)
{
  self->slider = gtk_switch_new ();
  gtk_widget_set_valign (self->slider, GTK_ALIGN_CENTER);
  gtk_accessible_update_state (GTK_ACCESSIBLE (self),
                               GTK_ACCESSIBLE_STATE_CHECKED,
                               FALSE,
                               -1);
  gtk_widget_set_can_focus (self->slider, FALSE);
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), TRUE);
  adw_action_row_add_suffix (ADW_ACTION_ROW (self), self->slider);
  adw_action_row_set_activatable_widget (ADW_ACTION_ROW (self), self->slider);

  g_object_bind_property (self, "action-name",
                          self->slider, "action-name",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property (self, "action-target",
                          self->slider, "action-target",
                          G_BINDING_SYNC_CREATE);

  g_signal_connect_swapped (self->slider, "notify::active", G_CALLBACK (slider_notify_active_cb), self);
}

/**
 * adw_switch_row_new:
 *
 * Creates a new `AdwSwitchRow`.
 *
 * Returns: the newly created `AdwSwitchRow`
 *
 * Since: 1.4
 */
GtkWidget *
adw_switch_row_new (void)
{
  return g_object_new (ADW_TYPE_SWITCH_ROW, NULL);
}

/**
 * adw_switch_row_get_active: (attributes org.gtk.Method.get_property=active)
 * @self: a switch row
 *
 * Gets whether @self is in its "on" or "off" position.
 *
 * Returns: whether @self is active or not
 *
 * Since: 1.4
 */
gboolean
adw_switch_row_get_active (AdwSwitchRow *self)
{
  g_return_val_if_fail (ADW_IS_SWITCH_ROW (self), FALSE);

  return gtk_switch_get_active (GTK_SWITCH (self->slider));
}

/**
 * adw_switch_row_set_active: (attributes org.gtk.Method.set_property=active)
 * @self: a switch row
 * @is_active: whether @self should be active
 *
 * Sets whether @self is in its "on" or "off" position
 *
 * Since: 1.4
 */
void
adw_switch_row_set_active (AdwSwitchRow *self,
                           gboolean      is_active)
{
  g_return_if_fail (ADW_IS_SWITCH_ROW (self));

  is_active = !!is_active;

  if (gtk_switch_get_active (GTK_SWITCH (self->slider)) == is_active)
    return;

  gtk_switch_set_active (GTK_SWITCH (self->slider), is_active);
}
