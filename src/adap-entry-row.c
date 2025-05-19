/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-entry-row-private.h"

#include "adw-animation-private.h"
#include "adw-animation-util.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"

#define EMPTY_ANIMATION_DURATION 150
#define TITLE_SPACING 3

/**
 * AdwEntryRow:
 *
 * A [class@Gtk.ListBoxRow] with an embedded text entry.
 *
 * <picture>
 *   <source srcset="entry-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="entry-row.png" alt="entry-row">
 * </picture>
 *
 * `AdwEntryRow` has a title that doubles as placeholder text. It shows an icon
 * indicating that it's editable and can receive additional widgets before or
 * after the editable part.
 *
 * If [property@EntryRow:show-apply-button] is set to `TRUE`, `AdwEntryRow` can
 * show an apply button when editing its contents. This can be useful if
 * changing its contents can result in an expensive operation, such as network
 * activity.
 *
 * `AdwEntryRow` provides only minimal API and should be used with the
 * [iface@Gtk.Editable] API.
 *
 * See also [class@PasswordEntryRow].
 *
 * ## AdwEntryRow as GtkBuildable
 *
 * The `AdwEntryRow` implementation of the [iface@Gtk.Buildable] interface
 * supports adding a child at its end by specifying “suffix” or omitting the
 * “type” attribute of a <child> element.
 *
 * It also supports adding a child as a prefix widget by specifying “prefix” as
 * the “type” attribute of a <child> element.
 *
 * ## CSS nodes
 *
 * `AdwEntryRow` has a single CSS node with name `row` and the `.entry` style
 * class.
 *
 * Since: 1.2
 */

typedef struct
{
  GtkWidget *header;
  GtkWidget *text;
  GtkWidget *title;
  GtkWidget *empty_title;
  GtkWidget *editable_area;
  GtkWidget *edit_icon;
  GtkWidget *apply_button;
  GtkWidget *indicator;
  GtkBox *suffixes;
  GtkBox *prefixes;
  GSignalGroup *buffer_signals;

  gboolean empty;
  double empty_progress;
  AdwAnimation *empty_animation;

  gboolean editing;
  gboolean show_apply_button;
  gboolean text_changed;
  gboolean show_indicator;
  gboolean activates_default;
} AdwEntryRowPrivate;

static void adw_entry_row_buildable_init (GtkBuildableIface *iface);
static void adw_entry_row_editable_init (GtkEditableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwEntryRow, adw_entry_row, ADW_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (AdwEntryRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_entry_row_buildable_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, adw_entry_row_editable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SHOW_APPLY_BUTTON,
  PROP_INPUT_HINTS,
  PROP_INPUT_PURPOSE,
  PROP_ATTRIBUTES,
  PROP_ENABLE_EMOJI_COMPLETION,
  PROP_ACTIVATES_DEFAULT,
  PROP_TEXT_LENGTH,
  PROP_LAST_PROP,
};

static GParamSpec *props[PROP_LAST_PROP];

enum {
  SIGNAL_APPLY,
  SIGNAL_ENTRY_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
empty_animation_value_cb (double       value,
                          AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  priv->empty_progress = value;

  gtk_widget_queue_allocate (priv->editable_area);

  gtk_widget_set_opacity (priv->text, value);
  gtk_widget_set_opacity (priv->title, value);
  gtk_widget_set_opacity (priv->empty_title, 1 - value);
}

static gboolean
is_text_focused (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  GtkStateFlags flags = gtk_widget_get_state_flags (priv->text);

  return !!(flags & GTK_STATE_FLAG_FOCUS_WITHIN);
}

static void
update_empty (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (GTK_TEXT (priv->text));
  gboolean focused = is_text_focused (self);
  gboolean editable = gtk_editable_get_editable (GTK_EDITABLE (priv->text));
  gboolean empty = gtk_entry_buffer_get_length (buffer) == 0;

  gtk_widget_set_visible (priv->edit_icon, !priv->text_changed && (!priv->editing || !editable));
  gtk_widget_set_sensitive (priv->edit_icon, editable);
  gtk_widget_set_visible (priv->indicator, priv->editing && priv->show_indicator);
  gtk_widget_set_visible (priv->apply_button, priv->text_changed);

  priv->empty = empty && !(focused && editable) && !priv->text_changed;

  gtk_widget_queue_allocate (priv->editable_area);

  adw_timed_animation_set_value_from (ADW_TIMED_ANIMATION (priv->empty_animation),
                                      priv->empty_progress);
  adw_timed_animation_set_value_to (ADW_TIMED_ANIMATION (priv->empty_animation),
                                    priv->empty ? 0 : 1);
  adw_animation_play (priv->empty_animation);
}

static void
text_changed_cb (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  if (priv->show_apply_button && priv->editing)
    priv->text_changed = TRUE;

  update_empty (self);
}

static void
text_state_flags_changed_cb (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  priv->editing = is_text_focused (self);

  if (priv->editing)
    gtk_widget_add_css_class (GTK_WIDGET (self), "focused");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "focused");

  update_empty (self);
}

static gboolean
text_keynav_failed_cb (AdwEntryRow      *self,
                       GtkDirectionType  direction)
{
  if (direction == GTK_DIR_LEFT || direction == GTK_DIR_RIGHT)
    return gtk_widget_child_focus (GTK_WIDGET (self), direction);

  return GDK_EVENT_PROPAGATE;
}

static void
pressed_cb (GtkGesture  *gesture,
            int          n_press,
            double       x,
            double       y,
            AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  GtkWidget *picked;

  picked = gtk_widget_pick (GTK_WIDGET (self), x, y, GTK_PICK_DEFAULT);

  if (picked != GTK_WIDGET (self) &&
      picked != priv->header &&
      picked != priv->indicator &&
      picked != GTK_WIDGET (priv->prefixes) &&
      picked != GTK_WIDGET (priv->suffixes)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  gtk_text_grab_focus_without_selecting (GTK_TEXT (priv->text));

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
apply_button_clicked_cb (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  if (gtk_widget_has_focus (priv->apply_button))
    gtk_widget_grab_focus (GTK_WIDGET (self));

  priv->text_changed = FALSE;
  update_empty (self);

  g_signal_emit (self, signals[SIGNAL_APPLY], 0);
}

static void
text_activated_cb (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  if (gtk_widget_get_visible (priv->apply_button)) {
    apply_button_clicked_cb (self);
  } else {
    if (priv->activates_default)
      gtk_widget_activate_default (GTK_WIDGET (self));

    g_signal_emit (self, signals[SIGNAL_ENTRY_ACTIVATED], 0);
  }
}

static void
on_length_changed (AdwEntryRow    *self,
                   GtkEntryBuffer *buffer)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TEXT_LENGTH]);
}

static void
measure_editable_area (GtkWidget      *widget,
                       GtkOrientation  orientation,
                       int             for_size,
                       int            *minimum,
                       int            *natural,
                       int            *minimum_baseline,
                       int            *natural_baseline)
{
  AdwEntryRow *self = g_object_get_data (G_OBJECT (widget), "row");
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  int text_min = 0, text_nat = 0;
  int title_min = 0, title_nat = 0;
  int empty_min = 0, empty_nat = 0;

  gtk_widget_measure (priv->text, orientation, for_size,
                      &text_min, &text_nat, NULL, NULL);
  gtk_widget_measure (priv->title, orientation, for_size,
                      &title_min, &title_nat, NULL, NULL);
  gtk_widget_measure (priv->empty_title, orientation, for_size,
                      &empty_min, &empty_nat, NULL, NULL);

  if (minimum)
    *minimum = MAX (text_min + TITLE_SPACING + title_min, empty_min);

  if (natural)
    *natural = MAX (text_nat + TITLE_SPACING + title_nat, empty_nat);

  if (minimum_baseline)
    *minimum_baseline = -1;

  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_editable_area (GtkWidget *widget,
                        int        width,
                        int        height,
                        int        baseline)
{
  AdwEntryRow *self = g_object_get_data (G_OBJECT (widget), "row");
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  GskTransform *transform;
  int empty_height = 0, title_height = 0, text_height = 0, text_baseline = -1;
  float empty_scale, title_scale, title_offset;

  gtk_widget_measure (priv->title, GTK_ORIENTATION_VERTICAL, width,
                      NULL, &title_height, NULL, NULL);
  gtk_widget_measure (priv->empty_title, GTK_ORIENTATION_VERTICAL, width,
                      NULL, &empty_height, NULL, NULL);
  gtk_widget_measure (priv->text, GTK_ORIENTATION_VERTICAL, width,
                      NULL, &text_height, NULL, &text_baseline);

  empty_scale = (float) adw_lerp (1.0, (double) title_height / empty_height, priv->empty_progress);
  title_scale = (float) adw_lerp ((double) empty_height / title_height, 1.0, priv->empty_progress);
  title_offset = (float) adw_lerp ((double) (height - empty_height) / 2.0,
                                   (double) (height - title_height - text_height - TITLE_SPACING) / 2.0,
                                   priv->empty_progress);

  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, title_offset));
  if (is_rtl)
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (width, 0));
  transform = gsk_transform_scale (transform, empty_scale, empty_scale);
  if (is_rtl)
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-width, 0));
  gtk_widget_allocate (priv->empty_title, width, empty_height, -1, transform);

  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, title_offset));
  if (is_rtl)
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (width, 0));
  transform = gsk_transform_scale (transform, title_scale, title_scale);
  if (is_rtl)
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-width, 0));
  gtk_widget_allocate (priv->title, width, title_height, -1, transform);

  text_baseline += (int) ((double) (height + title_height - text_height + TITLE_SPACING) / 2.0);
  gtk_widget_allocate (priv->text, width, height, text_baseline, NULL);
}

static gboolean
adw_entry_row_grab_focus (GtkWidget *widget)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (widget);
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  return gtk_widget_grab_focus (priv->text);
}

static void
adw_entry_row_get_property (GObject     *object,
                            guint        prop_id,
                            GValue      *value,
                            GParamSpec  *pspec)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (object);

  if (gtk_editable_delegate_get_property (object, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_SHOW_APPLY_BUTTON:
    g_value_set_boolean (value, adw_entry_row_get_show_apply_button (self));
    break;
  case PROP_INPUT_HINTS:
    g_value_set_flags (value, adw_entry_row_get_input_hints (self));
    break;
  case PROP_INPUT_PURPOSE:
    g_value_set_enum (value, adw_entry_row_get_input_purpose (self));
    break;
  case PROP_ATTRIBUTES:
    g_value_set_boxed (value, adw_entry_row_get_attributes (self));
    break;
  case PROP_ENABLE_EMOJI_COMPLETION:
    g_value_set_boolean (value, adw_entry_row_get_enable_emoji_completion (self));
    break;
  case PROP_ACTIVATES_DEFAULT:
    g_value_set_boolean (value, adw_entry_row_get_activates_default (self));
    break;
  case PROP_TEXT_LENGTH:
    g_value_set_uint (value, adw_entry_row_get_text_length (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_entry_row_set_property (GObject       *object,
                            guint          prop_id,
                            const GValue  *value,
                            GParamSpec    *pspec)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (object);

  if (gtk_editable_delegate_set_property (object, prop_id, value, pspec))
  {
    switch (prop_id) {
    case PROP_LAST_PROP + GTK_EDITABLE_PROP_EDITABLE:
      update_empty (self);
      break;
    default:;
    }
    return;
  }

  switch (prop_id) {
  case PROP_SHOW_APPLY_BUTTON:
    adw_entry_row_set_show_apply_button (self, g_value_get_boolean (value));
    break;
  case PROP_INPUT_HINTS:
    adw_entry_row_set_input_hints (self, g_value_get_flags (value));
    break;
  case PROP_INPUT_PURPOSE:
    adw_entry_row_set_input_purpose (self, g_value_get_enum (value));
    break;
  case PROP_ATTRIBUTES:
    adw_entry_row_set_attributes (self, g_value_get_boxed (value));
    break;
  case PROP_ENABLE_EMOJI_COMPLETION:
    adw_entry_row_set_enable_emoji_completion (self, g_value_get_boolean (value));
    break;
  case PROP_ACTIVATES_DEFAULT:
    adw_entry_row_set_activates_default (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_entry_row_dispose (GObject *object)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (object);
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  g_clear_object (&priv->empty_animation);

  if (priv->text)
    gtk_editable_finish_delegate (GTK_EDITABLE (self));

  G_OBJECT_CLASS (adw_entry_row_parent_class)->dispose (object);
}

static void
adw_entry_row_class_init (AdwEntryRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_entry_row_get_property;
  object_class->set_property = adw_entry_row_set_property;
  object_class->dispose = adw_entry_row_dispose;

  widget_class->focus = adw_widget_focus_child;
  widget_class->grab_focus = adw_entry_row_grab_focus;

  /**
   * AdwEntryRow:show-apply-button: (attributes org.gtk.Property.get=adw_entry_row_get_show_apply_button org.gtk.Property.set=adw_entry_row_set_show_apply_button)
   *
   * Whether to show the apply button.
   *
   * When set to `TRUE`, typing text in the entry will reveal an apply button.
   * Clicking it or pressing the <kbd>Enter</kbd> key will hide the button and
   * emit the [signal@EntryRow::apply] signal.
   *
   * This is useful if changing the entry contents can trigger an expensive
   * operation, e.g. network activity, to avoid triggering it after typing every
   * character.
   *
   * Since: 1.2
   */
  props[PROP_SHOW_APPLY_BUTTON] =
    g_param_spec_boolean ("show-apply-button", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:input-hints: (attributes org.gtk.Property.get=adw_entry_row_get_input_hints org.gtk.Property.set=adw_entry_row_set_input_hints)
   *
   * Additional input hints for the entry row.
   *
   * Input hints allow input methods to fine-tune their behavior.
   *
   * See also: [property@Adw.EntryRow:input-purpose]
   *
   * Since: 1.2
   */
  props[PROP_INPUT_HINTS] =
    g_param_spec_flags ("input-hints", NULL, NULL,
                        GTK_TYPE_INPUT_HINTS,
                        GTK_INPUT_HINT_NONE,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:input-purpose: (attributes org.gtk.Property.get=adw_entry_row_get_input_purpose org.gtk.Property.set=adw_entry_row_set_input_purpose)
   *
   * The input purpose of the entry row.
   *
   * The input purpose can be used by input methods to adjust their behavior.
   *
   * Since: 1.2
   */
  props[PROP_INPUT_PURPOSE] =
    g_param_spec_enum ("input-purpose", NULL, NULL,
                       GTK_TYPE_INPUT_PURPOSE,
                       GTK_INPUT_PURPOSE_FREE_FORM,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:attributes: (attributes org.gtk.Property.get=adw_entry_row_get_attributes org.gtk.Property.set=adw_entry_row_set_attributes)
   *
   * A list of Pango attributes to apply to the text of the embedded entry.
   *
   * The [struct@Pango.Attribute]'s `start_index` and `end_index` must refer to
   * the [class@Gtk.EntryBuffer] text, i.e. without the preedit string.
   *
   * Since: 1.2
   */
  props[PROP_ATTRIBUTES] =
    g_param_spec_boxed ("attributes", NULL, NULL,
                        PANGO_TYPE_ATTR_LIST,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:enable-emoji-completion: (attributes org.gtk.Property.get=adw_entry_row_get_enable_emoji_completion org.gtk.Property.set=adw_entry_row_set_enable_emoji_completion)
   *
   * Whether to suggest emoji replacements on the entry row.
   *
   * Emoji replacement is done with :-delimited names, like `:heart:`.
   *
   * Since: 1.2
   */
  props[PROP_ENABLE_EMOJI_COMPLETION] =
    g_param_spec_boolean ("enable-emoji-completion", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:activates-default: (attributes org.gtk.Property.get=adw_entry_row_get_activates_default org.gtk.Property.set=adw_entry_row_set_activates_default)
   *
   * Whether activating the embedded entry can activate the default widget.
   *
   * Since: 1.2
   */
  props[PROP_ACTIVATES_DEFAULT] =
    g_param_spec_boolean ("activates-default", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwEntryRow:text-length: (attributes org.gtk.Property.get=adw_entry_row_get_text_length)
   *
   * The length of the text in the entry row.
   *
   * Since: 1.5
   */
  props[PROP_TEXT_LENGTH] =
    g_param_spec_uint ("text-length", NULL, NULL,
                       0, G_MAXUINT16, 0,
                       G_PARAM_READABLE);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_editable_install_properties (object_class, PROP_LAST_PROP);

  /**
   * AdwEntryRow::apply:
   *
   * Emitted when the apply button is pressed.
   *
   * See [property@EntryRow:show-apply-button].
   *
   * Since: 1.2
   */
  signals[SIGNAL_APPLY] =
    g_signal_new ("apply",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_APPLY],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwEntryRow::entry-activated:
   *
   * Emitted when the embedded entry is activated.
   *
   * Since: 1.2
   */
  signals[SIGNAL_ENTRY_ACTIVATED] =
    g_signal_new ("entry-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_ENTRY_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-entry-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, header);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, prefixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, suffixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, editable_area);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, text);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, empty_title);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, title);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, edit_icon);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, apply_button);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, indicator);
  gtk_widget_class_bind_template_child_private (widget_class, AdwEntryRow, buffer_signals);

  gtk_widget_class_bind_template_callback (widget_class, pressed_cb);
  gtk_widget_class_bind_template_callback (widget_class, text_state_flags_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, text_keynav_failed_cb);
  gtk_widget_class_bind_template_callback (widget_class, text_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, update_empty);
  gtk_widget_class_bind_template_callback (widget_class, text_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, apply_button_clicked_cb);

  g_type_ensure (ADW_TYPE_GIZMO);
}

static void
adw_entry_row_init (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);
  AdwAnimationTarget *target;

  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_editable_init_delegate (GTK_EDITABLE (self));

  adw_gizmo_set_measure_func (ADW_GIZMO (priv->editable_area), (AdwGizmoMeasureFunc) measure_editable_area);
  adw_gizmo_set_allocate_func (ADW_GIZMO (priv->editable_area), (AdwGizmoAllocateFunc) allocate_editable_area);
  adw_gizmo_set_focus_func (ADW_GIZMO (priv->editable_area), (AdwGizmoFocusFunc) adw_widget_focus_child);

  g_object_set_data (G_OBJECT (priv->editable_area), "row", self);

  priv->empty_progress = 0.0;

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              empty_animation_value_cb,
                                              self, NULL);

  priv->empty_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 0,
                             EMPTY_ANIMATION_DURATION, target);

  g_signal_group_connect_swapped (priv->buffer_signals, "notify::length",
                                  G_CALLBACK (on_length_changed), self);

  g_object_bind_property (GTK_TEXT (priv->text), "buffer",
                          priv->buffer_signals, "target",
                          G_BINDING_SYNC_CREATE);

  update_empty (self);
}

static void
adw_entry_row_buildable_add_child (GtkBuildable *buildable,
                                   GtkBuilder   *builder,
                                   GObject      *child,
                                   const char   *type)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (buildable);
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  if (!priv->header)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (g_strcmp0 (type, "prefix") == 0)
    adw_entry_row_add_prefix (self, GTK_WIDGET (child));
  else if (g_strcmp0 (type, "suffix") == 0)
    adw_entry_row_add_suffix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_entry_row_add_suffix (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_entry_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_entry_row_buildable_add_child;
}

static GtkEditable *
adw_entry_row_get_delegate (GtkEditable *editable)
{
  AdwEntryRow *self = ADW_ENTRY_ROW (editable);
  AdwEntryRowPrivate *priv = adw_entry_row_get_instance_private (self);

  return GTK_EDITABLE (priv->text);
}

void
adw_entry_row_editable_init (GtkEditableInterface *iface)
{
  iface->get_delegate = adw_entry_row_get_delegate;
}

/**
 * adw_entry_row_new:
 *
 * Creates a new `AdwEntryRow`.
 *
 * Returns: the newly created `AdwEntryRow`
 *
 * Since: 1.2
 */
GtkWidget *
adw_entry_row_new (void)
{
  return g_object_new (ADW_TYPE_ENTRY_ROW, NULL);
}

/**
 * adw_entry_row_add_prefix:
 * @self: an entry row
 * @widget: a widget
 *
 * Adds a prefix widget to @self.
 *
 * Since: 1.2
 */
void
adw_entry_row_add_prefix (AdwEntryRow *self,
                          GtkWidget   *widget)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adw_entry_row_get_instance_private (self);

  gtk_box_prepend (priv->prefixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->prefixes), TRUE);
}

/**
 * adw_entry_row_add_suffix:
 * @self: an entry row
 * @widget: a widget
 *
 * Adds a suffix widget to @self.
 *
 * Since: 1.2
 */
void
adw_entry_row_add_suffix (AdwEntryRow *self,
                          GtkWidget    *widget)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  priv = adw_entry_row_get_instance_private (self);

  gtk_box_append (priv->suffixes, widget);
  gtk_widget_set_visible (GTK_WIDGET (priv->suffixes), TRUE);
}

/**
 * adw_entry_row_remove:
 * @self: an entry row
 * @widget: the child to be removed
 *
 * Removes a child from @self.
 *
 * Since: 1.2
 */
void
adw_entry_row_remove (AdwEntryRow *self,
                      GtkWidget   *child)
{
  AdwEntryRowPrivate *priv;
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adw_entry_row_get_instance_private (self);

  parent = gtk_widget_get_parent (child);

  if (parent == GTK_WIDGET (priv->prefixes) || parent == GTK_WIDGET (priv->suffixes)) {
    gtk_box_remove (GTK_BOX (parent), child);
    gtk_widget_set_visible (parent, gtk_widget_get_first_child (parent) != NULL);
  }
  else {
    ADW_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
  }
}

/**
 * adw_entry_row_get_show_apply_button: (attributes org.gtk.Method.get_property=show-apply-button)
 * @self: an entry row
 *
 * Gets whether @self can show the apply button.
 *
 * Returns: whether to show the apply button
 *
 * Since: 1.2
 */
gboolean
adw_entry_row_get_show_apply_button (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), FALSE);

  priv = adw_entry_row_get_instance_private (self);

  return priv->show_apply_button;
}

/**
 * adw_entry_row_set_show_apply_button: (attributes org.gtk.Method.set_property=show-apply-button)
 * @self: an entry row
 * @show_apply_button: whether to show the apply button
 *
 * Sets whether @self can show the apply button.
 *
 * When set to `TRUE`, typing text in the entry will reveal an apply button.
 * Clicking it or pressing the <kbd>Enter</kbd> key will hide the button and
 * emit the [signal@EntryRow::apply] signal.
 *
 * This is useful if changing the entry contents can trigger an expensive
 * operation, e.g. network activity, to avoid triggering it after typing every
 * character.
 *
 * Since: 1.2
 */
void
adw_entry_row_set_show_apply_button (AdwEntryRow *self,
                                     gboolean     show_apply_button)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  show_apply_button = !!show_apply_button;

  if (priv->show_apply_button == show_apply_button)
    return;

  priv->show_apply_button = show_apply_button;

  if (!priv->show_apply_button && priv->text_changed) {
    priv->text_changed = FALSE;
    update_empty (self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_APPLY_BUTTON]);
}

/**
 * adw_entry_row_get_input_hints: (attributes org.gtk.Method.get_property=input-hints)
 * @self: an entry row
 *
 * Gets the additional input hints of @self.
 *
 * Returns: The input hints
 *
 * Since: 1.2
 */
GtkInputHints
adw_entry_row_get_input_hints (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), GTK_INPUT_HINT_NONE);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_get_input_hints (GTK_TEXT (priv->text));
}

/**
 * adw_entry_row_set_input_hints: (attributes org.gtk.Method.set_property=input-hints)
 * @self: an entry row
 * @hints: the hints
 *
 * Set additional input hints for @self.
 *
 * Input hints allow input methods to fine-tune their behavior.
 *
 * See also: [property@AdwEntryRow:input-purpose]
 *
 * Since: 1.2
 */
void
adw_entry_row_set_input_hints (AdwEntryRow   *self,
                               GtkInputHints  hints)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  if (hints == adw_entry_row_get_input_hints (self))
    return;

  gtk_text_set_input_hints (GTK_TEXT (priv->text), hints);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INPUT_HINTS]);
}

/**
 * adw_entry_row_get_input_purpose: (attributes org.gtk.Method.get_property=input-purpose)
 * @self: an entry row
 *
 * Gets the input purpose of @self.
 *
 * Returns: the input purpose
 *
 * Since: 1.2
 */
GtkInputPurpose
adw_entry_row_get_input_purpose (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), GTK_INPUT_PURPOSE_FREE_FORM);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_get_input_purpose (GTK_TEXT (priv->text));
}

/**
 * adw_entry_row_set_input_purpose: (attributes org.gtk.Method.set_property=input-purpose)
 * @self: an entry row
 * @purpose: the purpose
 *
 * Sets the input purpose of @self.
 *
 * The input purpose can be used by input methods to adjust their behavior.
 *
 * Since: 1.2
 */
void
adw_entry_row_set_input_purpose (AdwEntryRow     *self,
                                 GtkInputPurpose  purpose)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  if (purpose == adw_entry_row_get_input_purpose (self))
    return;

  gtk_text_set_input_purpose (GTK_TEXT (priv->text), purpose);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INPUT_PURPOSE]);
}

/**
 * adw_entry_row_get_attributes: (attributes org.gtk.Method.get_property=attributes)
 * @self: an entry row
 *
 * Gets Pango attributes applied to the text of the embedded entry.
 *
 * Returns: (nullable): the list of attributes
 *
 * Since: 1.2
 */
PangoAttrList *
adw_entry_row_get_attributes (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), NULL);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_get_attributes (GTK_TEXT (priv->text));
}

/**
 * adw_entry_row_set_attributes: (attributes org.gtk.Method.set_property=attributes)
 * @self: an entry row
 * @attributes: (nullable): a list of attributes
 *
 * Sets Pango attributes to apply to the text of the embedded entry.
 *
 * The [struct@Pango.Attribute]'s `start_index` and `end_index` must refer to
 * the [class@Gtk.EntryBuffer] text, i.e. without the preedit string.
 *
 * Since: 1.2
 */
void
adw_entry_row_set_attributes (AdwEntryRow   *self,
                              PangoAttrList *attributes)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  if (attributes == adw_entry_row_get_attributes (self))
    return;

  gtk_text_set_attributes (GTK_TEXT (priv->text), attributes);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ATTRIBUTES]);
}

/**
 * adw_entry_row_get_enable_emoji_completion: (attributes org.gtk.Method.get_property=enable-emoji-completion)
 * @self: an entry row
 *
 * Gets whether to suggest emoji replacements on @self.
 *
 * Returns: whether or not emoji completion is enabled
 *
 * Since: 1.2
 */
gboolean
adw_entry_row_get_enable_emoji_completion (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), FALSE);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_get_enable_emoji_completion (GTK_TEXT (priv->text));
}

/**
 * adw_entry_row_set_enable_emoji_completion: (attributes org.gtk.Method.set_property=enable-emoji-completion)
 * @self: an entry row
 * @enable_emoji_completion: Whether emoji completion should be enabled or not
 *
 * Sets whether to suggest emoji replacements on @self.
 *
 * Emoji replacement is done with :-delimited names, like `:heart:`.
 *
 * Since: 1.2
 */
void
adw_entry_row_set_enable_emoji_completion (AdwEntryRow *self,
                                           gboolean     enable_emoji_completion)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  enable_emoji_completion = !!enable_emoji_completion;

  if (enable_emoji_completion == adw_entry_row_get_enable_emoji_completion (self))
    return;

  gtk_text_set_enable_emoji_completion (GTK_TEXT (priv->text), enable_emoji_completion);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_EMOJI_COMPLETION]);
}

/**
 * adw_entry_row_get_activates_default: (attributes org.gtk.Method.get_property=activates-default)
 * @self: an entry row
 *
 * Gets whether activating the embedded entry can activate the default widget.
 *
 * Returns: whether to activate the default widget
 *
 * Since: 1.2
 */
gboolean
adw_entry_row_get_activates_default (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), FALSE);

  priv = adw_entry_row_get_instance_private (self);

  return priv->activates_default;
}

/**
 * adw_entry_row_set_activates_default: (attributes org.gtk.Method.set_property=activates-default)
 * @self: an entry row
 * @activates: whether to activate the default widget
 *
 * Sets whether activating the embedded entry can activate the default widget.
 *
 * Since: 1.2
 */
void
adw_entry_row_set_activates_default (AdwEntryRow *self,
                                     gboolean     activates)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  if (priv->activates_default == activates)
    return;

  priv->activates_default = activates;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATES_DEFAULT]);
}

void
adw_entry_row_set_indicator_icon_name (AdwEntryRow *self,
                                       const char  *icon_name)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  gtk_image_set_from_icon_name (GTK_IMAGE (priv->indicator), icon_name);
}

void
adw_entry_row_set_indicator_tooltip (AdwEntryRow *self,
                                     const char  *tooltip)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  gtk_widget_set_tooltip_text (priv->indicator, tooltip);
}

void
adw_entry_row_set_show_indicator (AdwEntryRow *self,
                                  gboolean     show_indicator)
{
  AdwEntryRowPrivate *priv;

  g_return_if_fail (ADW_IS_ENTRY_ROW (self));

  priv = adw_entry_row_get_instance_private (self);

  show_indicator = !!show_indicator;

  priv->show_indicator = show_indicator;

  update_empty (self);
}

/**
 * adw_entry_row_get_text_length:
 * @self: an entry row
 *
 * Retrieves the current length of the text in @self.
 *
 * Returns: The current number of characters in @self, or 0 if there are none.
 *
 * Since: 1.5
 */
guint
adw_entry_row_get_text_length (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), 0);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_get_text_length (GTK_TEXT (priv->text));
}

/**
 * adw_entry_row_grab_focus_without_selecting:
 * @self: an entry row
 *
 * Causes @self to have keyboard focus without selecting the text.
 * 
 * See [method@Gtk.Text.grab_focus_without_selecting] for more information.
 *
 * Returns: whether the focus is now inside @self
 *
 * Since: 1.3
 */
gboolean
adw_entry_row_grab_focus_without_selecting (AdwEntryRow *self)
{
  AdwEntryRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ENTRY_ROW (self), FALSE);

  priv = adw_entry_row_get_instance_private (self);

  return gtk_text_grab_focus_without_selecting (GTK_TEXT (priv->text));
}
