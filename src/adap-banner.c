/*
 * Copyright (C) 2022 Jamie Murphy <hello@itsjamie.dev>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-banner.h"

#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

#define HORZ_SPACING 6
#define VERT_SPACING 9
#define HORZ_PADDING 6
#define VERT_PADDING 6
#define LABEL_MAX_WIDTH 500
#define BUTTON_HORZ_MIN_WIDTH 84
#define BUTTON_VERT_MIN_WIDTH 160

/**
 * AdwBanner:
 *
 * A bar with contextual information.
 *
 * <picture>
 *   <source srcset="banner-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="banner.png" alt="banner">
 * </picture>
 *
 * Banners are hidden by default, use [property@Banner:revealed] to show them.
 *
 * Banners have a title, set with [property@Banner:title]. Titles can be marked
 * up with Pango markup, use [property@Banner:use-markup] to enable it.
 *
 * The title will be shown centered or left-aligned depending on available
 * space.
 *
 * Banners can optionally have a button with text on it, set through
 * [property@Banner:button-label]. The button can be used with a `GAction`,
 * or with the [signal@Banner::button-clicked] signal.
 *
 * ## CSS nodes
 *
 * `AdwBanner` has a main CSS node with the name `banner`.
 *
 * Since: 1.3
 */

struct _AdwBanner
{
  GtkWidget parent_instance;

  AdwGizmo *gizmo;
  GtkLabel *title;
  GtkRevealer *revealer;
  GtkButton *button;
};

static void adw_banner_actionable_init (GtkActionableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwBanner, adw_banner, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACTIONABLE, adw_banner_actionable_init))

enum {
  PROP_0,
  PROP_TITLE,
  PROP_BUTTON_LABEL,
  PROP_REVEALED,
  PROP_USE_MARKUP,

  /* Actionable properties */
  PROP_ACTION_NAME,
  PROP_ACTION_TARGET,
  LAST_PROP = PROP_ACTION_NAME,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_BUTTON_CLICKED,
  SIGNAL_LAST_SIGNAL
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
button_clicked (AdwBanner *self)
{
  g_assert (ADW_IS_BANNER (self));

  g_signal_emit (self, signals[SIGNAL_BUTTON_CLICKED], 0);
}

static void
adw_banner_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwBanner *self = ADW_BANNER (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_banner_get_title (self));
    break;
  case PROP_BUTTON_LABEL:
    g_value_set_string (value, adw_banner_get_button_label (self));
    break;
  case PROP_REVEALED:
    g_value_set_boolean (value, adw_banner_get_revealed (self));
    break;
  case PROP_USE_MARKUP:
    g_value_set_boolean (value, adw_banner_get_use_markup (self));
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
adw_banner_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwBanner *self = ADW_BANNER (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_banner_set_title (self, g_value_get_string (value));
    break;
  case PROP_BUTTON_LABEL:
    adw_banner_set_button_label (self, g_value_get_string (value));
    break;
  case PROP_REVEALED:
    adw_banner_set_revealed (self, g_value_get_boolean (value));
    break;
  case PROP_USE_MARKUP:
    adw_banner_set_use_markup (self, g_value_get_boolean (value));
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
adw_banner_dispose (GObject *object)
{
  AdwBanner *self = ADW_BANNER (object);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_BANNER);

  G_OBJECT_CLASS (adw_banner_parent_class)->dispose (object);
}

static void
measure_content (GtkWidget       *widget,
                 GtkOrientation   orientation,
                 int              for_size,
                 int             *minimum,
                 int             *natural,
                 int             *minimum_baseline,
                 int             *natural_baseline)
{
  AdwBanner *self = ADW_BANNER (gtk_widget_get_ancestor (widget, ADW_TYPE_BANNER));
  gboolean button_shown = gtk_widget_is_visible (GTK_WIDGET (self->button));
  int label_min, label_nat;
  int button_min, button_nat;
  int min = 0, nat = 0;

  gtk_widget_measure (GTK_WIDGET (self->title), orientation, for_size,
                      &label_min, &label_nat, NULL, NULL);
  gtk_widget_measure (GTK_WIDGET (self->button), orientation, for_size,
                      &button_min, &button_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_VERTICAL) {
    int label_width_nat, label_min_padded, label_nat_padded;

    gtk_widget_measure (GTK_WIDGET (self->title),
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &label_width_nat, NULL, NULL);
    gtk_widget_measure (GTK_WIDGET (self->title),
                        orientation,
                        for_size >= 0 ? for_size - HORZ_PADDING * 2 : -1,
                        &label_min_padded, &label_nat_padded, NULL, NULL);

    if (button_shown) {
      if (for_size >= 0) {
        int button_width_nat;

        gtk_widget_measure (GTK_WIDGET (self->button), GTK_ORIENTATION_HORIZONTAL, -1,
                            NULL, &button_width_nat, NULL, NULL);

        button_width_nat = MAX (button_width_nat, BUTTON_HORZ_MIN_WIDTH);

        /* Does button besides label + spacing + padding on the left fit? */
        if (HORZ_PADDING + label_width_nat + HORZ_SPACING + button_width_nat > for_size) {
          /* Button below label, with spacing and padding above and below */
          min = VERT_PADDING * 2 + label_min_padded + VERT_SPACING + button_min;
          nat = VERT_PADDING * 2 + label_nat_padded + VERT_SPACING + button_nat;
        } else {
          /* Button besides label, no padding */
          min = MAX (label_min, button_min);
          nat = MAX (label_nat, button_nat);
        }
      } else {
        /* Unlimited width, button besides label, no padding */
        min = MAX (label_min, button_min);
        nat = MAX (label_nat, button_nat);
      }
    } else {
      /* Does the whole label fit? */
      if (for_size >= 0 && label_width_nat > for_size) {
        /* It doesn't even with padding. No padding on either side */
        min = label_min;
        nat = label_nat;
      } else if (for_size >= 0 && label_width_nat > for_size - HORZ_PADDING * 2) {
        /* It fits without padding, but doesn't with it.
         * Use padded size since we want to prefer two lines */
        min = label_min_padded;
        nat = label_nat_padded;
      } else {
        /* It fits with padding. Add padding on all sides */
        min = VERT_PADDING * 2 + label_min_padded;
        nat = VERT_PADDING * 2 + label_nat_padded;
      }
    }
  } else {
    if (button_shown) {
      /* Button + label, with spacing and padding on the left only */
      min = HORZ_PADDING * 2 + MAX (button_min, BUTTON_VERT_MIN_WIDTH);
      nat = MAX (HORZ_PADDING + label_nat + HORZ_SPACING + button_nat, min);
    } else {
      /* Only label, padding on the left and right */
      min = HORZ_PADDING * 2 + label_min;
      nat = HORZ_PADDING * 2 + label_nat;
    }
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
allocate_content (GtkWidget *widget,
                  int        width,
                  int        height,
                  int        baseline)
{
  AdwBanner *self = ADW_BANNER (gtk_widget_get_ancestor (widget, ADW_TYPE_BANNER));
  gboolean button_shown = gtk_widget_is_visible (GTK_WIDGET (self->button));
  int button_width, button_height;
  int button_x = 0, button_y = 0;
  int label_width, label_height;
  int label_x, label_y;
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  gtk_widget_measure (GTK_WIDGET (self->title),
                      GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &label_width, NULL, NULL);
  gtk_widget_measure (GTK_WIDGET (self->button),
                      GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &button_width, NULL, NULL);
  gtk_widget_measure (GTK_WIDGET (self->title),
                      GTK_ORIENTATION_VERTICAL, width,
                      NULL, &label_height, NULL, NULL);
  gtk_widget_measure (GTK_WIDGET (self->button),
                      GTK_ORIENTATION_VERTICAL, width,
                      &button_height, NULL, NULL, NULL);

  if (button_shown || label_width > width)
    label_width = MIN (label_width, width);
  else
    label_width = MIN (label_width, width - HORZ_PADDING * 2);

  label_x = (width / 2) - (label_width / 2);
  label_y = (height / 2) - (label_height / 2);

  if (button_shown) {
    if (HORZ_PADDING + label_width + HORZ_SPACING + MAX (button_width, BUTTON_HORZ_MIN_WIDTH) > width) {
      /* Non-centered title + button don't fit. Now we need padding on the
       * sides too, so measure the label for that width instead */
      label_width = MIN (label_width, width - HORZ_PADDING * 2);
      gtk_widget_measure (GTK_WIDGET (self->title),
                          GTK_ORIENTATION_VERTICAL, width - HORZ_PADDING * 2,
                          NULL, &label_height, NULL, NULL);

      button_width = CLAMP (button_width, BUTTON_VERT_MIN_WIDTH, width);
      label_x = (width - label_width) / 2;
      label_y = VERT_PADDING;
      button_x = (width / 2) - (button_width / 2);
      button_y = height - button_height - VERT_PADDING;
    } else {
      button_width = MAX (button_width, BUTTON_HORZ_MIN_WIDTH);

      /* Does centered title fit? */
      if (label_width + (button_width + HORZ_SPACING) * 2 > width)
        label_x = is_rtl ? (width - label_width - HORZ_PADDING) : HORZ_PADDING;

      button_x = is_rtl ? 0 : width - button_width;
      button_y = (height / 2) - (button_height / 2);
    }
  }

  gtk_widget_allocate (GTK_WIDGET (self->title),
                       label_width, label_height, -1,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (label_x, label_y)));

  gtk_widget_allocate (GTK_WIDGET (self->button),
                       button_width, button_height, -1,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (button_x, button_y)));
}

static GtkSizeRequestMode
get_content_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
adw_banner_class_init (AdwBannerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_banner_get_property;
  object_class->set_property = adw_banner_set_property;
  object_class->dispose = adw_banner_dispose;

  /**
   * AdwBanner:title: (attributes org.gtk.Property.get=adw_banner_get_title org.gtk.Property.set=adw_banner_set_title)
   *
   * The title for this banner.
   *
   * See also: [property@Banner:use-markup].
   *
   * Since: 1.3
   */
  props[PROP_TITLE] =
      g_param_spec_string ("title", NULL, NULL,
                           "",
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBanner:button-label: (attributes org.gtk.Property.get=adw_banner_get_button_label org.gtk.Property.set=adw_banner_set_button_label)
   *
   * The label to show on the button.
   *
   * If set to `""` or `NULL`, the button won't be shown.
   *
   * The button can be used with a `GAction`, or with the
   * [signal@Banner::button-clicked] signal.
   *
   * Since: 1.3
   */
  props[PROP_BUTTON_LABEL] =
      g_param_spec_string ("button-label", NULL, NULL,
                           "",
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBanner:use-markup: (attributes org.gtk.Property.get=adw_banner_get_use_markup org.gtk.Property.set=adw_banner_set_use_markup)
   *
   * Whether to use Pango markup for the banner title.
   *
   * See also [func@Pango.parse_markup].
   *
   * Since: 1.3
   */
  props[PROP_USE_MARKUP] =
    g_param_spec_boolean ("use-markup", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBanner:revealed: (attributes org.gtk.Property.get=adw_banner_get_revealed org.gtk.Property.set=adw_banner_set_revealed)
   *
   * Whether the banner is currently revealed.
   *
   * Since: 1.3
   */
  props[PROP_REVEALED] =
      g_param_spec_boolean ("revealed", NULL, NULL,
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBanner::button-clicked:
   * 
   * This signal is emitted after the action button has been clicked.
   *
   * It can be used as an alternative to setting an action.
   * 
   * Since: 1.3
   */
  signals[SIGNAL_BUTTON_CLICKED] = 
    g_signal_new ("button-clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_BUTTON_CLICKED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  g_object_class_install_properties (object_class, LAST_PROP, props);
  g_object_class_override_property (object_class, PROP_ACTION_NAME, "action-name");
  g_object_class_override_property (object_class, PROP_ACTION_TARGET, "action-target");

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-banner.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwBanner, gizmo);
  gtk_widget_class_bind_template_child (widget_class, AdwBanner, title);
  gtk_widget_class_bind_template_child (widget_class, AdwBanner, revealer);
  gtk_widget_class_bind_template_child (widget_class, AdwBanner, button);

  gtk_widget_class_bind_template_callback (widget_class, button_clicked);

  gtk_widget_class_set_css_name (widget_class, "banner");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);

  g_type_ensure (ADW_TYPE_GIZMO);
}

static void
adw_banner_init (AdwBanner *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_widget_set_layout_manager (GTK_WIDGET (self->gizmo), gtk_custom_layout_new (get_content_request_mode,
                                                                                  measure_content,
                                                                                  allocate_content));

  adw_gizmo_set_focus_func (self->gizmo, (AdwGizmoFocusFunc) adw_widget_focus_child);
  adw_gizmo_set_grab_focus_func (self->gizmo, (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);
}

static const char *
adw_banner_get_action_name (GtkActionable *actionable)
{
  AdwBanner *self = ADW_BANNER (actionable);

  return gtk_actionable_get_action_name (GTK_ACTIONABLE (self->button));
}

static void
adw_banner_set_action_name (GtkActionable *actionable,
                            const char    *action_name)
{
  AdwBanner *self = ADW_BANNER (actionable);

  gtk_actionable_set_action_name (GTK_ACTIONABLE (self->button), action_name);
}

static GVariant *
adw_banner_get_action_target_value (GtkActionable *actionable)
{
  AdwBanner *self = ADW_BANNER (actionable);

  return gtk_actionable_get_action_target_value (GTK_ACTIONABLE (self->button));
}

static void
adw_banner_set_action_target_value (GtkActionable *actionable,
                                    GVariant      *action_target)
{
  AdwBanner *self = ADW_BANNER (actionable);

  gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self->button), action_target);
}

static void
adw_banner_actionable_init (GtkActionableInterface *iface)
{
  iface->get_action_name = adw_banner_get_action_name;
  iface->set_action_name = adw_banner_set_action_name;
  iface->get_action_target_value = adw_banner_get_action_target_value;
  iface->set_action_target_value = adw_banner_set_action_target_value;
}

/**
 * adw_banner_new:
 * @title: the banner title
 *
 * Creates a new `AdwBanner`.
 *
 * Returns: the newly created `AdwBanner`
 *
 * Since: 1.3
 */
GtkWidget *
adw_banner_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADW_TYPE_BANNER,
                       "title", title,
                       NULL);
}

/**
 * adw_banner_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a banner
 *
 * Gets the title for @self.
 *
 * Returns: the title for @self
 *
 * Since: 1.3
 */
const char *
adw_banner_get_title (AdwBanner *self)
{
  g_return_val_if_fail (ADW_IS_BANNER (self), NULL);

  return gtk_label_get_label (self->title);
}

/**
 * adw_banner_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a banner
 * @title: the title
 *
 * Sets the title for this banner.
 *
 * See also: [property@Banner:use-markup].
 *
 * Since: 1.3
 */
void
adw_banner_set_title (AdwBanner  *self,
                      const char *title)
{
  g_return_if_fail (ADW_IS_BANNER (self));
  g_return_if_fail (title != NULL);

  if (g_strcmp0 (gtk_label_get_label (self->title), title) == 0)
    return;

  gtk_label_set_label (self->title, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_banner_get_button_label: (attributes org.gtk.Method.get_property=button-label)
 * @self: a banner
 *
 * Gets the button label for @self.
 *
 * Returns: (nullable): the button label for @self
 *
 * Since: 1.3
 */
const char *
adw_banner_get_button_label (AdwBanner *self)
{
  g_return_val_if_fail (ADW_IS_BANNER (self), NULL);

  return gtk_button_get_label (self->button);
}

/**
 * adw_banner_set_button_label: (attributes org.gtk.Method.set_property=button-label)
 * @self: a banner
 * @label: (nullable): the label
 *
 * Sets the button label for @self.
 *
 * If set to `""` or `NULL`, the button won't be shown.
 *
 * The button can be used with a `GAction`, or with the
 * [signal@Banner::button-clicked] signal.
 *
 * Since: 1.3
 */
void
adw_banner_set_button_label (AdwBanner  *self,
                             const char *label)
{
  g_return_if_fail (ADW_IS_BANNER (self));

  if (g_strcmp0 (gtk_button_get_label (self->button), label) == 0)
    return;

  gtk_widget_set_visible (GTK_WIDGET (self->button), label && label[0]);

  gtk_button_set_label (self->button, label);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BUTTON_LABEL]);
}

/**
 * adw_banner_get_use_markup: (attributes org.gtk.Method.get_property=use-markup)
 * @self: a banner
 *
 * Gets whether to use Pango markup for the banner title.
 *
 * Returns: whether to use markup
 *
 * Since: 1.3
 */
gboolean
adw_banner_get_use_markup (AdwBanner *self)
{
  g_return_val_if_fail (ADW_IS_BANNER (self), FALSE);

  return gtk_label_get_use_markup (self->title);
}

/**
 * adw_banner_set_use_markup: (attributes org.gtk.Method.set_property=use-markup)
 * @self: a banner
 * @use_markup: whether to use markup
 *
 * Sets whether to use Pango markup for the banner title.
 *
 * See also [func@Pango.parse_markup].
 *
 * Since: 1.3
 */
void
adw_banner_set_use_markup (AdwBanner *self,
                           gboolean   use_markup)
{
  g_return_if_fail (ADW_IS_BANNER (self));

  use_markup = !!use_markup;

  if (gtk_label_get_use_markup (self->title) == use_markup)
    return;

  gtk_label_set_use_markup (self->title, use_markup);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_MARKUP]);
}

/**
 * adw_banner_get_revealed: (attributes org.gtk.Method.get_property=revealed)
 * @self: a banner
 *
 * Gets if a banner is revealed
 *
 * Returns: Whether a banner is revealed
 *
 * Since: 1.3
 */
gboolean
adw_banner_get_revealed (AdwBanner *self)
{
  g_return_val_if_fail (ADW_IS_BANNER (self), FALSE);

  return gtk_revealer_get_reveal_child (GTK_REVEALER (self->revealer));
}

/**
 * adw_banner_set_revealed: (attributes org.gtk.Method.set_property=revealed)
 * @self: a banner
 * @revealed: whether a banner should be revealed
 *
 * Sets whether a banner should be revealed
 *
 * Since: 1.3
 */
void
adw_banner_set_revealed (AdwBanner *self,
                         gboolean   revealed)
{
  g_return_if_fail (ADW_IS_BANNER (self));

  revealed = !!revealed;

  if (gtk_revealer_get_reveal_child (GTK_REVEALER (self->revealer)) == revealed)
    return;

  gtk_revealer_set_reveal_child (GTK_REVEALER (self->revealer), revealed);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEALED]);
}
