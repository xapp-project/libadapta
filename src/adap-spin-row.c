/*
 * Copyright 2022 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-spin-row.h"

#include "adap-action-row-private.h"
#include "adap-marshalers.h"
#include "adap-widget-utils-private.h"

#include <math.h>

#define MAX_DIGITS 20

/**
 * AdapSpinRow:
 *
 * An [class@ActionRow] with an embedded spin button.
 *
 * <picture>
 *   <source srcset="spin-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="spin-row.png" alt="spin-row">
 * </picture>
 *
 * Example of an `AdapSpinRow` UI definition:
 *
 * ```xml
 * <object class="AdapSpinRow">
 *   <property name="title" translatable="yes">Spin Row</property>
 *   <property name="adjustment">
 *     <object class="GtkAdjustment">
 *       <property name="lower">0</property>
 *       <property name="upper">100</property>
 *       <property name="value">50</property>
 *       <property name="page-increment">10</property>
 *       <property name="step-increment">1</property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * See [class@Gtk.SpinButton] for details.
 *
 * ## CSS nodes
 *
 * `AdapSpinRow` has the same structure as [class@ActionRow], as well as the
 * `.spin` style class on the main node.
 *
 * ## Accessibility
 *
 * `AdapSpinRow` uses an internal `GtkSpinButton` with the
 * `GTK_ACCESSIBLE_ROLE_SPIN_BUTTON` role.
 *
 * Since: 1.4
 */

struct _AdapSpinRow
{
  AdapActionRow parent_instance;

  GtkWidget *spin_button;
};

static void adap_spin_row_accessible_init (GtkAccessibleInterface *iface);
static void adap_spin_row_editable_init (GtkEditableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapSpinRow, adap_spin_row, ADAP_TYPE_ACTION_ROW,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, adap_spin_row_accessible_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, adap_spin_row_editable_init))

enum {
  PROP_0,
  PROP_ADJUSTMENT,
  PROP_CLIMB_RATE,
  PROP_DIGITS,
  PROP_NUMERIC,
  PROP_SNAP_TO_TICKS,
  PROP_UPDATE_POLICY,
  PROP_VALUE,
  PROP_WRAP,
  PROP_LAST_PROP,
};

static GParamSpec *props[PROP_LAST_PROP];

enum {
  SIGNAL_INPUT,
  SIGNAL_OUTPUT,
  SIGNAL_WRAPPED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
boolean_handled_accumulator (GSignalInvocationHint *ihint,
                             GValue                *return_accu,
                             const GValue          *handler_return,
                             gpointer               dummy)
{
  gboolean signal_handled;

  signal_handled = g_value_get_boolean (handler_return);
  g_value_set_boolean (return_accu, signal_handled);

  return !signal_handled;
}

static void
spin_button_state_flags_changed_cb (AdapSpinRow *self)
{
  GtkStateFlags flags = gtk_widget_get_state_flags (self->spin_button);

  if (flags & GTK_STATE_FLAG_FOCUS_WITHIN)
    gtk_widget_add_css_class (GTK_WIDGET (self), "focused");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "focused");
}

static gboolean
spin_button_keynav_failed_cb (AdapSpinRow       *self,
                              GtkDirectionType  direction)
{
  if (direction == GTK_DIR_LEFT || direction == GTK_DIR_RIGHT)
    return gtk_widget_child_focus (GTK_WIDGET (self), direction);

  return GDK_EVENT_PROPAGATE;
}

static int
spin_button_input_cb (AdapSpinRow *self,
                      double     *new_value)
{
  int return_value;

  g_signal_emit (self, signals[SIGNAL_INPUT], 0, new_value, &return_value);

  return return_value;
}

static gboolean
spin_button_output_cb (AdapSpinRow *self)
{
  gboolean return_value;

  g_signal_emit (self, signals[SIGNAL_OUTPUT], 0, &return_value);

  return return_value;
}

static void
spin_button_wrapped_cb (AdapSpinRow *self)
{
  g_signal_emit (self, signals[SIGNAL_WRAPPED], 0);
}

static void
spin_button_notify_value_cb (AdapSpinRow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static gboolean
adap_spin_row_grab_focus (GtkWidget *widget)
{
  AdapSpinRow *self = ADAP_SPIN_ROW (widget);

  return gtk_widget_grab_focus (self->spin_button);
}

static void
adap_spin_row_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdapSpinRow *self = ADAP_SPIN_ROW (object);

  if (gtk_editable_delegate_get_property (object, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_ADJUSTMENT:
    g_value_set_object (value, adap_spin_row_get_adjustment (self));
    break;
  case PROP_CLIMB_RATE:
    g_value_set_double (value, adap_spin_row_get_climb_rate (self));
    break;
  case PROP_DIGITS:
    g_value_set_uint (value, adap_spin_row_get_digits (self));
    break;
  case PROP_NUMERIC:
    g_value_set_boolean (value, adap_spin_row_get_numeric (self));
    break;
  case PROP_SNAP_TO_TICKS:
    g_value_set_boolean (value, adap_spin_row_get_snap_to_ticks (self));
    break;
  case PROP_UPDATE_POLICY:
    g_value_set_enum (value, adap_spin_row_get_update_policy (self));
    break;
  case PROP_VALUE:
    g_value_set_double (value, adap_spin_row_get_value (self));
    break;
  case PROP_WRAP:
    g_value_set_boolean (value, adap_spin_row_get_wrap (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_spin_row_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdapSpinRow *self = ADAP_SPIN_ROW (object);

  if (gtk_editable_delegate_set_property (object, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_ADJUSTMENT:
    adap_spin_row_set_adjustment (self, g_value_get_object (value));
    break;
  case PROP_CLIMB_RATE:
    adap_spin_row_set_climb_rate (self, g_value_get_double (value));
    break;
  case PROP_DIGITS:
    adap_spin_row_set_digits (self, g_value_get_uint (value));
    break;
  case PROP_NUMERIC:
    adap_spin_row_set_numeric (self, g_value_get_boolean (value));
    break;
  case PROP_SNAP_TO_TICKS:
    adap_spin_row_set_snap_to_ticks (self, g_value_get_boolean (value));
    break;
  case PROP_UPDATE_POLICY:
    adap_spin_row_set_update_policy (self, g_value_get_enum (value));
    break;
  case PROP_VALUE:
    adap_spin_row_set_value (self, g_value_get_double (value));
    break;
  case PROP_WRAP:
    adap_spin_row_set_wrap (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_spin_row_class_init (AdapSpinRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adap_spin_row_get_property;
  object_class->set_property = adap_spin_row_set_property;

  widget_class->focus = adap_widget_focus_child;
  widget_class->grab_focus = adap_spin_row_grab_focus;

  /**
   * AdapSpinRow:adjustment: (attributes org.gtk.Property.get=adap_spin_row_get_adjustment org.gtk.Property.set=adap_spin_row_set_adjustment)
   *
   * The adjustment that holds the value of the spin row.
   *
   * Since: 1.4
   */
  props[PROP_ADJUSTMENT] =
    g_param_spec_object ("adjustment", NULL, NULL,
                         GTK_TYPE_ADJUSTMENT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:climb-rate: (attributes org.gtk.Property.get=adap_spin_row_get_climb_rate org.gtk.Property.set=adap_spin_row_set_climb_rate)
   *
   * The acceleration rate when you hold down a button or key.
   *
   * Since: 1.4
   */
  props[PROP_CLIMB_RATE] =
    g_param_spec_double ("climb-rate", NULL, NULL,
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:digits: (attributes org.gtk.Property.get=adap_spin_row_get_digits org.gtk.Property.set=adap_spin_row_set_digits)
   *
   * The number of decimal places to display.
   *
   * Since: 1.4
   */
  props[PROP_DIGITS] =
    g_param_spec_uint ("digits", NULL, NULL,
                       0, MAX_DIGITS, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:numeric: (attributes org.gtk.Property.get=adap_spin_row_get_numeric org.gtk.Property.set=adap_spin_row_set_numeric)
   *
   * Whether non-numeric characters should be ignored.
   *
   * Since: 1.4
   */
  props[PROP_NUMERIC] =
    g_param_spec_boolean ("numeric", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:snap-to-ticks: (attributes org.gtk.Property.get=adap_spin_row_get_snap_to_ticks org.gtk.Property.set=adap_spin_row_set_snap_to_ticks)
   *
   * Whether invalid values are snapped to the nearest step increment.
   *
   * Since: 1.4
   */
  props[PROP_SNAP_TO_TICKS] =
    g_param_spec_boolean ("snap-to-ticks", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:update-policy: (attributes org.gtk.Property.get=adap_spin_row_get_update_policy org.gtk.Property.set=adap_spin_row_set_update_policy)
   *
   * The policy for updating the spin row.
   *
   * The options are always, or only when the value is invalid.
   *
   * Since: 1.4
   */
  props[PROP_UPDATE_POLICY] =
    g_param_spec_enum ("update-policy", NULL, NULL,
                       GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY,
                       GTK_UPDATE_ALWAYS,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:value: (attributes org.gtk.Property.get=adap_spin_row_get_value org.gtk.Property.set=adap_spin_row_set_value)
   *
   * The current value.
   *
   * Since: 1.4
   */
  props[PROP_VALUE] =
    g_param_spec_double ("value", NULL, NULL,
                         -G_MAXDOUBLE, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapSpinRow:wrap: (attributes org.gtk.Property.get=adap_spin_row_get_wrap org.gtk.Property.set=adap_spin_row_set_wrap)
   *
   * Whether the spin row should wrap upon reaching its limits.
   *
   * Since: 1.4
   */
  props[PROP_WRAP] =
    g_param_spec_boolean ("wrap", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_editable_install_properties (object_class, PROP_LAST_PROP);

  /**
   * AdapSpinRow::input:
   * @self: a spin row
   * @new_value: (out) (type double): return location for the new value
   *
   * Emitted to convert the user's input into a double value.
   *
   * The signal handler is expected to use [method@Gtk.Editable.get_text] to
   * retrieve the text of the spinbutton and set new_value to the new value.
   *
   * The default conversion uses [func@GLib.strtod].
   *
   * See [signal@Gtk.SpinButton::input].
   *
   * Returns: `TRUE` for a successful conversion, `FALSE` if the input was not
   *   handled, and `GTK_INPUT_ERROR` if the conversion failed.
   *
   * Since: 1.4
   */
  signals[SIGNAL_INPUT] =
    g_signal_new ("input",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_INT__POINTER,
                  G_TYPE_INT, 1,
                  G_TYPE_POINTER);
  g_signal_set_va_marshaller (signals[SIGNAL_INPUT],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_INT__POINTERv);

  /**
   * AdapSpinRow::output:
   *
   * Emitted to tweak the formatting of the value for display.
   *
   * See [signal@Gtk.SpinButton::output].
   *
   * Returns: `TRUE` if the value has been displayed
   *
   * Since: 1.4
   */
  signals[SIGNAL_OUTPUT] =
    g_signal_new ("output",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  boolean_handled_accumulator,
                  NULL,
                  adap_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_OUTPUT],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_BOOLEAN__VOIDv);

  /**
   * AdapSpinRow::wrapped:
   *
   * Emitted right after the spinbutton wraps.
   *
   * See [signal@Gtk.SpinButton::wrapped].
   *
   * Since: 1.4
   */
  signals[SIGNAL_WRAPPED] =
    g_signal_new ("wrapped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  g_signal_set_va_marshaller (signals[SIGNAL_WRAPPED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-spin-row.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapSpinRow, spin_button);

  gtk_widget_class_bind_template_callback (widget_class, spin_button_state_flags_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_keynav_failed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_input_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_output_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_wrapped_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_notify_value_cb);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_PRESENTATION);
}

static void
adap_spin_row_init (AdapSpinRow *self)
{
  GListModel *controllers;
  guint i, n;

  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_editable_init_delegate (GTK_EDITABLE (self));

  adap_action_row_set_expand_suffixes (ADAP_ACTION_ROW (self), TRUE);

  /* XXX: We shouldn't be doing this, but there's no way to prevent scrolling
   * on the spin button at the moment */
  controllers = gtk_widget_observe_controllers (self->spin_button);

  n = g_list_model_get_n_items (controllers);
  for (i = 0; i < n; i++) {
    GtkEventController *controller = g_list_model_get_item (controllers, i);

    if (GTK_IS_EVENT_CONTROLLER_SCROLL (controller))
      gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_NONE);

    g_object_unref (controller);
  }

  g_object_unref (controllers);
}

static gboolean
adap_spin_row_accessible_get_platform_state (GtkAccessible              *accessible,
                                            GtkAccessiblePlatformState  state)
{
  return gtk_editable_delegate_get_accessible_platform_state (GTK_EDITABLE (accessible), state);
}

static void
adap_spin_row_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_platform_state = adap_spin_row_accessible_get_platform_state;
}

static GtkEditable *
adap_spin_row_editable_get_delegate (GtkEditable *editable)
{
  AdapSpinRow *self = ADAP_SPIN_ROW (editable);

  return GTK_EDITABLE (self->spin_button);
}

void
adap_spin_row_editable_init (GtkEditableInterface *iface)
{
  iface->get_delegate = adap_spin_row_editable_get_delegate;
}

/**
 * adap_spin_row_new:
 * @adjustment: (nullable): the adjustment that this spin row should use
 * @climb_rate: the rate the value changes when holding a button or key
 * @digits: the number of decimal places to display
 *
 * Creates a new `AdapSpinRow`.
 *
 * Returns: the newly created `AdapSpinRow`
 *
 * Since: 1.4
 */
GtkWidget *
adap_spin_row_new (GtkAdjustment *adjustment,
                  double         climb_rate,
                  guint          digits)
{
  g_return_val_if_fail (adjustment == NULL || GTK_IS_ADJUSTMENT (adjustment), NULL);
  g_return_val_if_fail (climb_rate >= 0, NULL);

  return g_object_new (ADAP_TYPE_SPIN_ROW,
                       "adjustment", adjustment,
                       "climb-rate", climb_rate,
                       "digits", digits,
                       NULL);
}

/**
 * adap_spin_row_new_with_range:
 * @min: minimum allowable value
 * @max: maximum allowable value
 * @step: increment added or subtracted by spinning the widget
 *
 * Creates a new `AdapSpinRow` with the given properties.
 *
 * This is a convenience constructor that allows creation of a numeric
 * `AdapSpinRow` without manually creating an adjustment. The value is initially
 * set to the minimum value and a page increment of 10 * @step is the default.
 * The precision of the spin row is equivalent to the precisions of @step.
 *
 * ::: note
 *     The way in which the precision is derived works best if @step is a power
 *     of ten. If the resulting precision is not suitable for your needs, use
 *     [method@SpinRow.set_digits] to correct it.
 *
 * Returns: the new `AdapSpinRow`
 *
 * Since: 1.4
 */
GtkWidget *
adap_spin_row_new_with_range (double min,
                             double max,
                             double step)
{

  GtkAdjustment *adjustment;
  int digits;

  g_return_val_if_fail (min <= max, NULL);
  g_return_val_if_fail (!G_APPROX_VALUE (step, 0, DBL_EPSILON), NULL);

  adjustment = gtk_adjustment_new (min, min, max, step, 10 * step, 0);

  if (fabs (step) >= 1) {
    digits = 0;
  } else {
    digits = abs ((int) floor (log10 (fabs (step))));
    if (digits > MAX_DIGITS)
      digits = MAX_DIGITS;
  }

  return g_object_new (ADAP_TYPE_SPIN_ROW,
                       "adjustment", adjustment,
                       "climb-rate", step,
                       "digits", digits,
                       "numeric", TRUE,
                       NULL);
}

/**
 * adap_spin_row_configure:
 * @self: a spin row
 * @adjustment: (nullable): the adjustment that this spin row should use
 * @climb_rate: the new climb rate
 * @digits: the number of decimal places to display
 *
 * Changes the properties of an existing spin row.
 *
 * The adjustment, climb rate, and number of decimal places are updated
 * accordingly.
 *
 * Since: 1.4
 */
void
adap_spin_row_configure (AdapSpinRow    *self,
                        GtkAdjustment *adjustment,
                        double         climb_rate,
                        guint          digits)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));
  g_return_if_fail (adjustment == NULL || GTK_IS_ADJUSTMENT (adjustment));
  g_return_if_fail (climb_rate >= 0);

  g_object_freeze_notify (G_OBJECT (self));

  adap_spin_row_set_adjustment (self, adjustment);
  adap_spin_row_set_climb_rate (self, climb_rate);
  adap_spin_row_set_digits (self, digits);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_spin_row_get_adjustment: (attributes org.gtk.Method.get_property=adjustment)
 * @self: a spin row
 *
 * Gets the adjustment that holds the value for the spin row.
 *
 * Returns: (transfer none): the adjustment that holds the spin row's value
 *
 * Since: 1.4
 */
GtkAdjustment *
adap_spin_row_get_adjustment (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), NULL);

  /* Even though the setter is nullable, this will always return a valid
   * adjustment */
  return gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_adjustment: (attributes org.gtk.Method.set_property=adjustment)
 * @self: a spin row
 * @adjustment: (nullable): an adjustment
 *
 * Sets the adjustment that holds the value for the spin row.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_adjustment (AdapSpinRow    *self,
                             GtkAdjustment *adjustment)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));
  g_return_if_fail (adjustment == NULL || GTK_IS_ADJUSTMENT (adjustment));

  if (adjustment == adap_spin_row_get_adjustment (self))
    return;

  gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (self->spin_button), adjustment);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ADJUSTMENT]);
}

/**
 * adap_spin_row_get_climb_rate: (attributes org.gtk.Method.get_property=climb-rate)
 * @self: a spin row
 *
 * Gets the acceleration rate when you hold down a button or key.
 *
 * Returns: the acceleration rate when you hold down a button or key
 *
 * Since: 1.4
 */
double
adap_spin_row_get_climb_rate (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), 0.0);

  return gtk_spin_button_get_climb_rate (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_climb_rate: (attributes org.gtk.Method.set_property=climb-rate)
 * @self: a spin row
 * @climb_rate: the acceleration rate when you hold down a button or key
 *
 * Sets the acceleration rate when you hold down a button or key.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_climb_rate (AdapSpinRow *self,
                             double      climb_rate)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));
  g_return_if_fail (climb_rate >= 0);

  if (G_APPROX_VALUE (climb_rate, adap_spin_row_get_climb_rate (self), DBL_EPSILON))
    return;

  gtk_spin_button_set_climb_rate (GTK_SPIN_BUTTON (self->spin_button), climb_rate);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CLIMB_RATE]);
}

/**
 * adap_spin_row_get_digits: (attributes org.gtk.Method.get_property=digits)
 * @self: a spin row
 *
 * Gets the number of decimal places to display.
 *
 * Returns: the number of decimal places to display
 *
 * Since: 1.4
 */
guint
adap_spin_row_get_digits (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), 0);

  return gtk_spin_button_get_digits (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_digits: (attributes org.gtk.Method.set_property=digits)
 * @self: a spin row
 * @digits: the number of decimal places to display
 *
 * Sets the number of decimal places to display.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_digits (AdapSpinRow *self,
                         guint       digits)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  if (digits == adap_spin_row_get_digits (self))
    return;

  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (self->spin_button), digits);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DIGITS]);
}

/**
 * adap_spin_row_get_numeric: (attributes org.gtk.Method.get_property=numeric)
 * @self: a spin row
 *
 * Gets whether non-numeric characters should be ignored.
 *
 * Returns: whether non-numeric characters should be ignored.
 *
 * Since: 1.4
 */
gboolean
adap_spin_row_get_numeric (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_numeric (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_numeric: (attributes org.gtk.Method.set_property=numeric)
 * @self: a spin row
 * @numeric: whether non-numeric characters should be ignored
 *
 * Sets whether non-numeric characters should be ignored.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_numeric (AdapSpinRow *self,
                          gboolean    numeric)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  numeric = !!numeric;

  if (numeric == adap_spin_row_get_numeric (self))
    return;

  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (self->spin_button), numeric);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NUMERIC]);
}

/**
 * adap_spin_row_get_snap_to_ticks: (attributes org.gtk.Method.get_property=snap-to-ticks)
 * @self: a spin row
 *
 * Gets whether invalid values are snapped to nearest step increment.
 *
 * Returns: whether invalid values are snapped to the nearest step increment
 *
 * Since: 1.4
 */
gboolean
adap_spin_row_get_snap_to_ticks (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_snap_to_ticks (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_snap_to_ticks: (attributes org.gtk.Method.set_property=snap-to-ticks)
 * @self: a spin row
 * @snap_to_ticks: whether invalid values are snapped to the nearest step increment
 *
 * Sets whether invalid values are snapped to the nearest step increment.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_snap_to_ticks (AdapSpinRow *self,
                                gboolean    snap_to_ticks)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  snap_to_ticks = !!snap_to_ticks;

  if (snap_to_ticks == adap_spin_row_get_snap_to_ticks (self))
    return;

  gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (self->spin_button), snap_to_ticks);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SNAP_TO_TICKS]);
}

/**
 * adap_spin_row_get_update_policy: (attributes org.gtk.Method.get_property=update-policy)
 * @self: a spin row
 *
 * Gets the policy for updating the spin row.
 *
 * Returns: the policy for updating the spin row
 *
 * Since: 1.4
 */
GtkSpinButtonUpdatePolicy
adap_spin_row_get_update_policy (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), GTK_UPDATE_ALWAYS);

  return gtk_spin_button_get_update_policy (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_update_policy: (attributes org.gtk.Method.set_property=update-policy)
 * @self: a spin row
 * @policy: the policy for updating the spin row
 *
 * Sets the policy for updating the spin row.
 *
 * The options are always, or only when the value is invalid.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_update_policy (AdapSpinRow                *self,
                                GtkSpinButtonUpdatePolicy  policy)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  if (policy == adap_spin_row_get_update_policy (self))
    return;

  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (self->spin_button), policy);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UPDATE_POLICY]);
}

/**
 * adap_spin_row_get_value: (attributes org.gtk.Method.get_property=value)
 * @self: a spin row
 *
 * Gets the current value.
 *
 * Returns: the current value
 *
 * Since: 1.4
 */
double
adap_spin_row_get_value (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), 0.0);

  return gtk_spin_button_get_value (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_value: (attributes org.gtk.Method.set_property=value)
 * @self: a spin row
 * @value: a new value
 *
 * Sets the current value.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_value (AdapSpinRow *self,
                        double      value)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  if (G_APPROX_VALUE (value, adap_spin_row_get_value (self), DBL_EPSILON))
    return;

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_button), value);
}

/**
 * adap_spin_row_get_wrap: (attributes org.gtk.Method.get_property=wrap)
 * @self: a spin row
 *
 * Gets whether the spin row should wrap upon reaching its limits.
 *
 * Returns: whether the spin row should wrap upon reaching its limits
 *
 * Since: 1.4
 */
gboolean
adap_spin_row_get_wrap (AdapSpinRow *self)
{
  g_return_val_if_fail (ADAP_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_wrap (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_wrap: (attributes org.gtk.Method.set_property=wrap)
 * @self: a spin row
 * @wrap: whether the spin row should wrap upon reaching its limits
 *
 * Sets whether the spin row should wrap upon reaching its limits.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_wrap (AdapSpinRow *self,
                       gboolean    wrap)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  wrap = !!wrap;

  if (wrap == adap_spin_row_get_wrap (self))
    return;

  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (self->spin_button), wrap);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP]);
}

/**
 * adap_spin_row_update:
 * @self: a spin row
 *
 * Manually force an update of the spin row.
 *
 * Since: 1.4
 */
void
adap_spin_row_update (AdapSpinRow *self)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  gtk_spin_button_update (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adap_spin_row_set_range:
 * @self: a spin row
 * @min: minimum allowable value
 * @max: maximum allowable value
 *
 * Sets the minimum and maximum allowable values for @self.
 *
 * If the current value is outside this range, it will be adjusted
 * to fit within the range, otherwise it will remain unchanged.
 *
 * Since: 1.4
 */
void
adap_spin_row_set_range (AdapSpinRow *self,
                        double      min,
                        double      max)
{
  g_return_if_fail (ADAP_IS_SPIN_ROW (self));

  gtk_spin_button_set_range (GTK_SPIN_BUTTON (self->spin_button), min, max);
}
