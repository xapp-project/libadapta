/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-indicator-bin-private.h"
#include "adap-view-switcher-button-private.h"

#define TIMEOUT_EXPAND 500

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_NEEDS_ATTENTION,
  PROP_BADGE_NUMBER,

  /* Overridden properties */
  PROP_LABEL,
  PROP_ORIENTATION,

  LAST_PROP = PROP_BADGE_NUMBER + 1,
};

#define MIN_NAT_BUTTON_WIDTH_NARROW 100
#define MIN_NAT_BUTTON_WIDTH_WIDE 120

struct _AdapViewSwitcherButton
{
  GtkToggleButton parent_instance;

  GtkBox *horizontal_box;
  GtkImage *horizontal_image;
  GtkLabel *horizontal_label;
  GtkStack *stack;
  GtkBox *vertical_box;
  GtkImage *vertical_image;
  GtkLabel *vertical_label;

  char *icon_name;
  char *label;
  GtkOrientation orientation;
  gboolean needs_attention;
  guint badge_number;

  guint switch_timer;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapViewSwitcherButton, adap_view_switcher_button, GTK_TYPE_TOGGLE_BUTTON,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static void
switch_timeout_cb (AdapViewSwitcherButton *self)
{
  self->switch_timer = 0;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self), TRUE);
}

static void
update_mnemonic (AdapViewSwitcherButton *self)
{
  g_assert (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  gtk_label_set_mnemonic_widget (self->horizontal_label,
                                 (self->orientation == GTK_ORIENTATION_HORIZONTAL)
                                 ? GTK_WIDGET (self) : NULL);
  gtk_label_set_mnemonic_widget (self->vertical_label,
                                 (self->orientation == GTK_ORIENTATION_VERTICAL)
                                 ? GTK_WIDGET (self) : NULL);
}

static void
drag_enter_cb (AdapViewSwitcherButton *self)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self)))
    return;

  self->switch_timer =
    g_timeout_add_once (TIMEOUT_EXPAND,
                        (GSourceOnceFunc) switch_timeout_cb,
                        self);
  g_source_set_name_by_id (self->switch_timer, "[adap] switch_timeout_cb");
}

static void
drag_leave_cb (AdapViewSwitcherButton *self)
{
  g_clear_handle_id (&self->switch_timer, g_source_remove);
}

static GtkOrientation
get_orientation (AdapViewSwitcherButton *self)
{
  g_return_val_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self), GTK_ORIENTATION_HORIZONTAL);

  return self->orientation;
}

static void
set_orientation (AdapViewSwitcherButton *self,
                 GtkOrientation         orientation)
{
  g_return_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_stack_set_visible_child (self->stack,
                               GTK_WIDGET (self->orientation == GTK_ORIENTATION_VERTICAL ?
                                             self->vertical_box :
                                             self->horizontal_box));

  update_mnemonic (self);
}

static gchar *
get_badge_text (AdapViewSwitcherButton *self,
                guint                  badge_number)
{
  if (badge_number > 999)
    return g_strdup ("999+");

  if (!badge_number)
    return g_strdup ("");

  return g_strdup_printf ("%u", badge_number);
}

static void
adap_view_switcher_button_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adap_view_switcher_button_get_icon_name (self));
    break;
  case PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, adap_view_switcher_button_get_needs_attention (self));
    break;
  case PROP_BADGE_NUMBER:
    g_value_set_uint (value, adap_view_switcher_button_get_badge_number (self));
    break;
  case PROP_LABEL:
    g_value_set_string (value, adap_view_switcher_button_get_label (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adap_view_switcher_button_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adap_view_switcher_button_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_NEEDS_ATTENTION:
    adap_view_switcher_button_set_needs_attention (self, g_value_get_boolean (value));
    break;
  case PROP_BADGE_NUMBER:
    adap_view_switcher_button_set_badge_number (self, g_value_get_uint (value));
    break;
  case PROP_LABEL:
    adap_view_switcher_button_set_label (self, g_value_get_string (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adap_view_switcher_button_dispose (GObject *object)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (object);

  g_clear_handle_id (&self->switch_timer, g_source_remove);

  G_OBJECT_CLASS (adap_view_switcher_button_parent_class)->dispose (object);
}

static void
adap_view_switcher_button_finalize (GObject *object)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (object);

  g_free (self->icon_name);
  g_free (self->label);

  G_OBJECT_CLASS (adap_view_switcher_button_parent_class)->finalize (object);
}

static void
adap_view_switcher_button_measure (GtkWidget      *widget,
                                  GtkOrientation  orientation,
                                  int             for_size,
                                  int            *minimum,
                                  int            *natural,
                                  int            *minimum_baseline,
                                  int            *natural_baseline)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (widget);

  gtk_widget_measure (GTK_WIDGET (self->stack), orientation, for_size,
                      minimum, natural, minimum_baseline, natural_baseline);

  if (orientation != GTK_ORIENTATION_HORIZONTAL)
    return;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    *natural = MAX (*natural, MIN_NAT_BUTTON_WIDTH_WIDE);
  else
    *natural = MAX (*natural, MIN_NAT_BUTTON_WIDTH_NARROW);
}

static void
adap_view_switcher_button_size_allocate (GtkWidget *widget,
                                        int        width,
                                        int        height,
                                        int        baseline)
{
  AdapViewSwitcherButton *self = ADAP_VIEW_SWITCHER_BUTTON (widget);

  gtk_widget_allocate (GTK_WIDGET (self->stack), width, height, baseline, NULL);
}

static void
adap_view_switcher_button_class_init (AdapViewSwitcherButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_view_switcher_button_get_property;
  object_class->set_property = adap_view_switcher_button_set_property;
  object_class->dispose = adap_view_switcher_button_dispose;
  object_class->finalize = adap_view_switcher_button_finalize;

  widget_class->measure = adap_view_switcher_button_measure;
  widget_class->size_allocate = adap_view_switcher_button_size_allocate;

  g_object_class_override_property (object_class,
                                    PROP_LABEL,
                                    "label");

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdapViewSwitcherButton:icon-name: (attributes org.gtk.Property.get=adap_view_switcher_button_get_icon_name org.gtk.Property.set=adap_view_switcher_button_set_icon_name)
   *
   * The icon name representing the view, or `NULL` for no icon.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "text-x-generic-symbolic",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapViewSwitcherButton:needs-attention: (attributes org.gtk.Property.get=adap_view_switcher_button_get_needs_attention org.gtk.Property.set=adap_view_switcher_button_set_needs_attention)
   *
   * Sets a flag specifying whether the view requires the user attention.
   *
   * This is used by the [class@ViewSwitcher] to change the appearance of the
   * corresponding button when a view needs attention and it is not the current
   * one.
   */
  props[PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapViewSwitcherButton:badge-number:
   *
   * A number to display as a badge on the button.
   */
  props[PROP_BADGE_NUMBER] =
    g_param_spec_uint ("badge-number", NULL, NULL,
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /* We probably should set the class's CSS name to "viewswitcherbutton"
   * here, but it doesn't work because GtkCheckButton hardcodes it to "button"
   * on instantiation, and the functions required to override it are private.
   * In the meantime, we can use the "viewswitcher > button" CSS selector as
   * a fairly safe fallback.
   */

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-view-switcher-button.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, horizontal_box);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, horizontal_image);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, horizontal_label);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, stack);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, vertical_box);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, vertical_image);
  gtk_widget_class_bind_template_child (widget_class, AdapViewSwitcherButton, vertical_label);
  gtk_widget_class_bind_template_callback (widget_class, drag_enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, drag_leave_cb);
  gtk_widget_class_bind_template_callback (widget_class, get_badge_text);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TAB);

  g_type_ensure (ADAP_TYPE_INDICATOR_BIN);
}

static void
adap_view_switcher_button_init (AdapViewSwitcherButton *self)
{
  gtk_widget_set_layout_manager (GTK_WIDGET (self), NULL);

  self->icon_name = g_strdup ("image-missing");

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_stack_set_visible_child (GTK_STACK (self->stack), GTK_WIDGET (self->horizontal_box));

  gtk_widget_set_focus_on_click (GTK_WIDGET (self), FALSE);
  update_mnemonic (self);
}

/**
 * adap_view_switcher_button_new:
 *
 * Creates a new `AdapViewSwitcherButton`.
 *
 * Returns: the newly created `AdapViewSwitcherButton`
 */
GtkWidget *
adap_view_switcher_button_new (void)
{
  return g_object_new (ADAP_TYPE_VIEW_SWITCHER_BUTTON, NULL);
}

/**
 * adap_view_switcher_button_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a view switcher button
 *
 * Gets the icon name representing the view.
 *
 * Returns: (nullable): the icon name
 **/
const char *
adap_view_switcher_button_get_icon_name (AdapViewSwitcherButton *self)
{
  g_return_val_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->icon_name;
}

/**
 * adap_view_switcher_button_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a view switcher button
 * @icon_name: (nullable): an icon name
 *
 * Sets the icon name representing the view.
 **/
void
adap_view_switcher_button_set_icon_name (AdapViewSwitcherButton *self,
                                        const char            *icon_name)
{
  g_return_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_strcmp0 (self->icon_name, icon_name))
    return;

  if (icon_name && *icon_name)
    g_set_str (&self->icon_name, icon_name);
  else
    g_set_str (&self->icon_name, "image-missing");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adap_view_switcher_button_get_needs_attention: (attributes org.gtk.Method.get_property=needs-attention)
 * @self: a view switcher button
 *
 * Gets whether the view represented by @self requires the user attention.
 *
 * Returns: whether the view requires the user attention
 **/
gboolean
adap_view_switcher_button_get_needs_attention (AdapViewSwitcherButton *self)
{
  g_return_val_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self), FALSE);

  return self->needs_attention;
}

/**
 * adap_view_switcher_button_set_needs_attention: (attributes org.gtk.Method.set_property=needs-attention)
 * @self: a view switcher button
 * @needs_attention: whether the view needs attention
 *
 * Sets whether the view represented by @self requires the user attention.
 */
void
adap_view_switcher_button_set_needs_attention (AdapViewSwitcherButton *self,
                                              gboolean               needs_attention)
{
  g_return_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  needs_attention = !!needs_attention;

  if (self->needs_attention == needs_attention)
    return;

  self->needs_attention = needs_attention;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION]);
}

/**
 * adap_view_switcher_button_get_badge_number:
 * @self: a view switcher button
 *
 * Gets the badge number.
 *
 * Returns: the badge number
 */
guint
adap_view_switcher_button_get_badge_number (AdapViewSwitcherButton *self)
{
  g_return_val_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self), 0);

  return self->badge_number;
}

/**
 * adap_view_switcher_button_set_badge_number:
 * @self: a view switcher button
 * @badge_number: the new value
 *
 * Sets the badge number.
 */
void
adap_view_switcher_button_set_badge_number (AdapViewSwitcherButton *self,
                                           guint                  badge_number)
{
  g_return_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  if (self->badge_number == badge_number)
    return;

  self->badge_number = badge_number;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BADGE_NUMBER]);
}

/**
 * adap_view_switcher_button_get_label:
 * @self: a view switcher button
 *
 * Gets the label representing the view.
 *
 * Returns: (nullable): the label
 **/
const char *
adap_view_switcher_button_get_label (AdapViewSwitcherButton *self)
{
  g_return_val_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->label;
}

/**
 * adap_view_switcher_button_set_label:
 * @self: a view switcher button
 * @label: (nullable): a label
 *
 * Sets the label representing the view.
 **/
void
adap_view_switcher_button_set_label (AdapViewSwitcherButton *self,
                                    const char            *label)
{
  g_return_if_fail (ADAP_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_set_str (&self->label, label))
    return;

  g_object_notify (G_OBJECT (self), "label");
}
