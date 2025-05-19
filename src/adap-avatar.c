/*
 * Copyright (C) 2020 Purism SPC
 * Copyright (C) 2020 Felipe Borges
 *
 * Authors:
 * Felipe Borges <felipeborges@gnome.org>
 * Julian Sparber <julian@sparber.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "config.h"
#include <math.h>

#include "adw-avatar.h"
#include "adw-gizmo-private.h"

#define NUMBER_OF_COLORS 14

/**
 * AdwAvatar:
 *
 * A widget displaying an image, with a generated fallback.
 *
 * <picture>
 *   <source srcset="avatar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="avatar.png" alt="avatar">
 * </picture>
 *
 * `AdwAvatar` is a widget that shows a round avatar.
 *
 * `AdwAvatar` generates an avatar with the initials of  the
 * [property@Avatar:text] on top of a colored background.
 *
 * The color is picked based on the hash of the [property@Avatar:text].
 *
 * If [property@Avatar:show-initials] is set to `FALSE`,
 * [property@Avatar:icon-name] or `avatar-default-symbolic` is shown instead of
 * the initials.
 *
 * Use [property@Avatar:custom-image] to set a custom image.
 *
 * ## CSS nodes
 *
 * `AdwAvatar` has a single CSS node with name `avatar`.
 */

struct _AdwAvatar
{
  GtkWidget parent_instance;

  GtkWidget *gizmo;
  GtkLabel *label;
  GtkImage *icon;
  GtkImage *custom_image;
  GdkPaintable *custom_image_source;

  char *icon_name;
  char *text;
  gboolean show_initials;
  guint color_class;
  int size;
};

G_DEFINE_FINAL_TYPE (AdwAvatar, adw_avatar, GTK_TYPE_WIDGET);

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TEXT,
  PROP_SHOW_INITIALS,
  PROP_CUSTOM_IMAGE,
  PROP_SIZE,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

static char *
extract_initials_from_text (const char *text)
{
  GString *initials;
  char *p = g_utf8_strup (text, -1);
  char *normalized = g_utf8_normalize (g_strstrip (p), -1, G_NORMALIZE_DEFAULT_COMPOSE);
  gunichar unichar;
  char *q = NULL;

  g_clear_pointer (&p, g_free);

  if (normalized == NULL)
    return NULL;

  initials = g_string_new ("");

  unichar = g_utf8_get_char (normalized);
  g_string_append_unichar (initials, unichar);

  q = g_utf8_strrchr (normalized, -1, ' ');
  if (q != NULL) {
    unichar = g_utf8_get_char (g_utf8_next_char (q));

    if (unichar != 0)
      g_string_append_unichar (initials, unichar);
  }

  g_free (normalized);

  return g_string_free_and_steal (initials);
}

static void
update_visibility (AdwAvatar *self)
{
  gboolean has_custom_image = gtk_image_get_paintable (self->custom_image) != NULL;
  gboolean has_initials = self->show_initials && self->text && strlen (self->text);

  gtk_widget_set_visible (GTK_WIDGET (self->label), !has_custom_image && has_initials);
  gtk_widget_set_visible (GTK_WIDGET (self->icon), !has_custom_image && !has_initials);
  gtk_widget_set_visible (GTK_WIDGET (self->custom_image), has_custom_image);
}

static void
set_class_color (AdwAvatar *self)
{
  char *old_class, *new_class;

  old_class = g_strdup_printf ("color%d", self->color_class);
  gtk_widget_remove_css_class (self->gizmo, old_class);

  if (self->text == NULL || strlen (self->text) == 0) {
    /* Use a random color if we don't have a text */
    GRand *rand = g_rand_new ();

    self->color_class = g_rand_int_range (rand, 1, NUMBER_OF_COLORS);

    g_rand_free (rand);
  } else {
    self->color_class = (g_str_hash (self->text) % NUMBER_OF_COLORS) + 1;
  }

  new_class = g_strdup_printf ("color%d", self->color_class);
  gtk_widget_add_css_class (self->gizmo, new_class);

  g_free (old_class);
  g_free (new_class);
}

static void
update_initials (AdwAvatar *self)
{
  char *initials;

  if (gtk_image_get_paintable (self->custom_image) != NULL ||
      !self->show_initials ||
      self->text == NULL ||
      strlen (self->text) == 0)
    return;

  initials = extract_initials_from_text (self->text);

  gtk_label_set_label (self->label, initials);

  g_free (initials);
}

static void
update_icon (AdwAvatar *self)
{
  if (self->icon_name)
    gtk_image_set_from_icon_name (self->icon, self->icon_name);
  else
    gtk_image_set_from_icon_name (self->icon, "avatar-default-symbolic");
}

static void
update_font_size (AdwAvatar *self)
{
  int width, height;
  double padding;
  double sqr_size;
  double max_size;
  double new_font_size;
  PangoAttrList *attributes;

  if (gtk_image_get_paintable (self->custom_image) != NULL ||
      !self->show_initials ||
      self->text == NULL ||
      strlen (self->text) == 0)
    return;

  /* Reset font size first to avoid rounding errors */
  attributes = pango_attr_list_new ();
  gtk_label_set_attributes (self->label, attributes);

  pango_layout_get_pixel_size (gtk_label_get_layout (self->label), &width, &height);

  /* This is the size of the biggest square fitting inside the circle */
  sqr_size = (double) self->size / 1.4142;
  /* The padding has to be a function of the overall size.
   * The 0.4 is how steep the linear function grows and the -5 is just
   * an adjustment for smaller sizes which doesn't have a big impact on bigger sizes.
   * Make also sure we don't have a negative padding */
  padding = MAX (self->size * 0.4 - 5, 0);
  max_size = sqr_size - padding;
  new_font_size = (double) height * (max_size / (double) width);

  pango_attr_list_change (attributes, pango_attr_size_new_absolute (CLAMP (new_font_size, 0, max_size) * PANGO_SCALE));
  gtk_label_set_attributes (self->label, attributes);

  pango_attr_list_unref (attributes);
}

static void
update_custom_image_snapshot (AdwAvatar *self)
{
  GtkSnapshot *snapshot = NULL;
  GdkPaintable *square_image;
  int width, height;
  float scaled_width, scaled_height;
  float size;

  if (!self->custom_image_source)
    return;

  width = gdk_paintable_get_intrinsic_width (self->custom_image_source);
  height = gdk_paintable_get_intrinsic_height (self->custom_image_source);

  if (height == width && !GDK_IS_TEXTURE (self->custom_image_source)) {
    gtk_image_set_from_paintable (self->custom_image, self->custom_image_source);
    return;
  }

  size = self->size * gtk_widget_get_scale_factor (GTK_WIDGET (self));

  if (width > height) {
    scaled_height = size;
    scaled_width = (float) width * scaled_height / (float) height;
  } else if (width < height) {
    scaled_width = size;
    scaled_height = (float) height * scaled_width / (float) width;
  } else {
    scaled_width = scaled_height = size;
  }

  snapshot = gtk_snapshot_new ();
  gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT ((size - scaled_width) / 2.f, (size - scaled_height) / 2.f));

  if (GDK_IS_TEXTURE (self->custom_image_source)) {
    GskScalingFilter filter;

    if (scaled_width > width || scaled_height > height)
      filter = GSK_SCALING_FILTER_NEAREST;
    else
      filter = GSK_SCALING_FILTER_TRILINEAR;

    gtk_snapshot_append_scaled_texture (snapshot,
                                        GDK_TEXTURE (self->custom_image_source),
                                        filter,
                                        &GRAPHENE_RECT_INIT (0, 0, scaled_width, scaled_height));

  } else {
    gdk_paintable_snapshot (self->custom_image_source, snapshot, scaled_width, scaled_height);
  }

  square_image = gtk_snapshot_free_to_paintable (snapshot, &GRAPHENE_SIZE_INIT (size, size));
  gtk_image_set_from_paintable (self->custom_image, square_image);

  g_object_unref (square_image);
}

static void
adw_avatar_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwAvatar *self = ADW_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_avatar_get_icon_name (self));
    break;

  case PROP_TEXT:
    g_value_set_string (value, adw_avatar_get_text (self));
    break;

  case PROP_SHOW_INITIALS:
    g_value_set_boolean (value, adw_avatar_get_show_initials (self));
    break;

  case PROP_CUSTOM_IMAGE:
    g_value_set_object (value, adw_avatar_get_custom_image (self));
    break;

  case PROP_SIZE:
    g_value_set_int (value, adw_avatar_get_size (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_avatar_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwAvatar *self = ADW_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    adw_avatar_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_TEXT:
    adw_avatar_set_text (self, g_value_get_string (value));
    break;

  case PROP_SHOW_INITIALS:
    adw_avatar_set_show_initials (self, g_value_get_boolean (value));
    break;

  case PROP_CUSTOM_IMAGE:
    adw_avatar_set_custom_image (self, g_value_get_object (value));
    break;

  case PROP_SIZE:
    adw_avatar_set_size (self, g_value_get_int (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_avatar_dispose (GObject *object)
{
  AdwAvatar *self = ADW_AVATAR (object);

  g_clear_pointer (&self->gizmo, gtk_widget_unparent);

  self->label = NULL;
  self->icon = NULL;
  self->custom_image = NULL;

  G_OBJECT_CLASS (adw_avatar_parent_class)->dispose (object);
}

static void
adw_avatar_finalize (GObject *object)
{
  AdwAvatar *self = ADW_AVATAR (object);

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->text, g_free);
  g_clear_object (&self->custom_image_source);

  G_OBJECT_CLASS (adw_avatar_parent_class)->finalize (object);
}

static void
adw_avatar_class_init (AdwAvatarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_avatar_dispose;
  object_class->finalize = adw_avatar_finalize;
  object_class->set_property = adw_avatar_set_property;
  object_class->get_property = adw_avatar_get_property;

  /**
   * AdwAvatar:icon-name: (attributes org.gtk.Property.get=adw_avatar_get_icon_name org.gtk.Property.set=adw_avatar_set_icon_name)
   *
   * The name of an icon to use as a fallback.
   *
   * If no name is set, `avatar-default-symbolic` will be used.
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:text: (attributes org.gtk.Property.get=adw_avatar_get_text org.gtk.Property.set=adw_avatar_set_text)
   *
   * Sets the text used to generate the fallback initials and color.
   *
   * It's only used to generate the color if [property@Avatar:show-initials] is
   * `FALSE`.
   */
  props[PROP_TEXT] =
    g_param_spec_string ("text", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:show-initials: (attributes org.gtk.Property.get=adw_avatar_get_show_initials org.gtk.Property.set=adw_avatar_set_show_initials)
   *
   * Whether initials are used instead of an icon on the fallback avatar.
   *
   * See [property@Avatar:icon-name] for how to change the fallback icon.
   */
  props[PROP_SHOW_INITIALS] =
    g_param_spec_boolean ("show-initials", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:custom-image: (attributes org.gtk.Property.get=adw_avatar_get_custom_image org.gtk.Property.set=adw_avatar_set_custom_image)
   *
   * A custom image paintable.
   *
   * Custom image is displayed instead of initials or icon.
   */
  props[PROP_CUSTOM_IMAGE] =
    g_param_spec_object ("custom-image", NULL, NULL,
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:size: (attributes org.gtk.Property.get=adw_avatar_get_size org.gtk.Property.set=adw_avatar_set_size)
   *
   * The size of the avatar.
   */
  props[PROP_SIZE] =
    g_param_spec_int ("size", NULL, NULL,
                      -1, INT_MAX, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
adw_avatar_init (AdwAvatar *self)
{
  self->gizmo = adw_gizmo_new ("avatar", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_overflow (self->gizmo, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_halign (self->gizmo, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (self->gizmo, GTK_ALIGN_CENTER);
  gtk_widget_set_layout_manager (self->gizmo, gtk_bin_layout_new ());
  gtk_widget_set_parent (self->gizmo, GTK_WIDGET (self));

  self->label = GTK_LABEL (gtk_label_new (NULL));
  gtk_widget_set_parent (GTK_WIDGET (self->label), self->gizmo);

  self->icon = GTK_IMAGE (gtk_image_new ());
  gtk_widget_set_parent (GTK_WIDGET (self->icon), self->gizmo);

  self->custom_image = GTK_IMAGE (gtk_image_new ());
  gtk_widget_set_parent (GTK_WIDGET (self->custom_image), self->gizmo);

  self->text = g_strdup ("");
  self->size = -1;

  set_class_color (self);
  update_initials (self);
  update_font_size (self);
  update_icon (self);
  update_visibility (self);

  g_signal_connect (self, "notify::root", G_CALLBACK (update_font_size), NULL);
  g_signal_connect (self, "notify::scale-factor", G_CALLBACK (update_custom_image_snapshot), NULL);
}

/**
 * adw_avatar_new:
 * @size: The size of the avatar
 * @text: (nullable): the text used to get the initials and color
 * @show_initials: whether to use initials instead of an icon as fallback
 *
 * Creates a new `AdwAvatar`.
 *
 * Returns: the newly created `AdwAvatar`
 */
GtkWidget *
adw_avatar_new (int         size,
                const char *text,
                gboolean    show_initials)
{
  return g_object_new (ADW_TYPE_AVATAR,
                       "size", size,
                       "text", text,
                       "show-initials", show_initials,
                       NULL);
}

/**
 * adw_avatar_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: an avatar
 *
 * Gets the name of an icon to use as a fallback.
 *
 * Returns: (nullable): the icon name
 */
const char *
adw_avatar_get_icon_name (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);

  return self->icon_name;
}

/**
 * adw_avatar_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: an avatar
 * @icon_name: (nullable): the icon name
 *
 * Sets the name of an icon to use as a fallback.
 *
 * If no name is set, `avatar-default-symbolic` will be used.
 */
void
adw_avatar_set_icon_name (AdwAvatar  *self,
                          const char *icon_name)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (!g_set_str (&self->icon_name, icon_name))
    return;

  update_icon (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adw_avatar_get_text: (attributes org.gtk.Method.get_property=text)
 * @self: an avatar
 *
 * Gets the text used to generate the fallback initials and color.
 *
 * Returns: (nullable): the text used to generate the fallback initials and
 *   color
 */
const char *
adw_avatar_get_text (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);

  return self->text;
}

/**
 * adw_avatar_set_text: (attributes org.gtk.Method.set_property=text)
 * @self: an avatar
 * @text: (nullable): the text used to get the initials and color
 *
 * Sets the text used to generate the fallback initials and color.
 *
 * It's only used to generate the color if [property@Avatar:show-initials] is
 * `FALSE`.
 */
void
adw_avatar_set_text (AdwAvatar  *self,
                     const char *text)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (!g_set_str (&self->text, text ? text : ""))
    return;

  set_class_color (self);

  update_initials (self);
  update_font_size (self);
  update_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TEXT]);
}

/**
 * adw_avatar_get_show_initials: (attributes org.gtk.Method.get_property=show-initials)
 * @self: an avatar
 *
 * Gets whether initials are used instead of an icon on the fallback avatar.
 *
 * Returns: whether initials are used instead of an icon as fallback
 */
gboolean
adw_avatar_get_show_initials (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), FALSE);

  return self->show_initials;
}

/**
 * adw_avatar_set_show_initials: (attributes org.gtk.Method.set_property=show-initials)
 * @self: an avatar
 * @show_initials: whether to use initials instead of an icon as fallback
 *
 * Sets whether to use initials instead of an icon on the fallback avatar.
 *
 * See [property@Avatar:icon-name] for how to change the fallback icon.
 */
void
adw_avatar_set_show_initials (AdwAvatar *self,
                              gboolean   show_initials)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (self->show_initials == show_initials)
    return;

  self->show_initials = show_initials;

  update_initials (self);
  update_font_size (self);
  update_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_INITIALS]);
}

/**
 * adw_avatar_get_custom_image: (attributes org.gtk.Method.get_property=custom-image)
 * @self: an avatar
 *
 * Gets the custom image paintable.
 *
 * Returns: (nullable) (transfer none): the custom image
 */
GdkPaintable *
adw_avatar_get_custom_image (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);

  return self->custom_image_source;
}

/**
 * adw_avatar_set_custom_image: (attributes org.gtk.Method.set_property=custom-image)
 * @self: an avatar
 * @custom_image: (nullable) (transfer none): a custom image
 *
 * Sets the custom image paintable.
 *
 * Custom image is displayed instead of initials or icon.
 */
void
adw_avatar_set_custom_image (AdwAvatar    *self,
                             GdkPaintable *custom_image)
{
  g_return_if_fail (ADW_IS_AVATAR (self));
  g_return_if_fail (custom_image == NULL || GDK_IS_PAINTABLE (custom_image));

  if (self->custom_image_source == custom_image)
    return;

  if (self->custom_image_source) {
    g_signal_handlers_disconnect_by_func (self->custom_image_source,
                                          update_custom_image_snapshot, self);
  }

  g_set_object (&self->custom_image_source, custom_image);

  if (custom_image) {
    int height = gdk_paintable_get_intrinsic_height (custom_image);
    int width = gdk_paintable_get_intrinsic_width (custom_image);

    update_custom_image_snapshot (self);

    if (height != width && !GDK_IS_TEXTURE (custom_image)) {
      g_signal_connect_swapped (custom_image, "invalidate-contents",
                                G_CALLBACK (update_custom_image_snapshot),
                                self);
    }

    gtk_widget_add_css_class (self->gizmo, "image");
  } else {
    gtk_image_set_from_paintable (self->custom_image, NULL);
    gtk_widget_remove_css_class (self->gizmo, "image");
  }

  update_initials (self);
  update_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CUSTOM_IMAGE]);
}

/**
 * adw_avatar_get_size: (attributes org.gtk.Method.get_property=size)
 * @self: an avatar
 *
 * Gets the size of the avatar.
 *
 * Returns: the size of the avatar
 */
int
adw_avatar_get_size (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), 0);

  return self->size;
}

/**
 * adw_avatar_set_size: (attributes org.gtk.Method.set_property=size)
 * @self: an avatar
 * @size: The size of the avatar
 *
 * Sets the size of the avatar.
 */
void
adw_avatar_set_size (AdwAvatar *self,
                     int        size)
{
  g_return_if_fail (ADW_IS_AVATAR (self));
  g_return_if_fail (size >= -1);

  if (self->size == size)
    return;

  self->size = size;

  gtk_widget_set_size_request (self->gizmo, size, size);
  gtk_image_set_pixel_size (self->icon, size / 2);

  if (size < 25)
    gtk_widget_add_css_class (self->gizmo, "contrasted");
  else
    gtk_widget_remove_css_class (self->gizmo, "contrasted");

  update_font_size (self);
  update_custom_image_snapshot (self);

  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIZE]);
}

/**
 * adw_avatar_draw_to_texture:
 * @self: an avatar
 * @scale_factor: The scale factor
 *
 * Renders @self into a [class@Gdk.Texture] at @scale_factor.
 *
 * This can be used to export the fallback avatar.
 *
 * Returns: (transfer full): the texture
 */
GdkTexture *
adw_avatar_draw_to_texture (AdwAvatar *self,
                            int        scale_factor)
{
  GdkTexture *result;
  GskRenderNode *node;
  GtkSnapshot *snapshot;
  GtkNative *native;
  GskRenderer *renderer;
  int size;
  graphene_matrix_t transform;

  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);
  g_return_val_if_fail (scale_factor > 0, NULL);

  size = self->size * scale_factor;

  g_assert (gtk_widget_compute_transform (GTK_WIDGET (self),
                                          self->gizmo,
                                          &transform));

  snapshot = gtk_snapshot_new ();
  gtk_snapshot_scale (snapshot, scale_factor, scale_factor);
  gtk_snapshot_transform_matrix (snapshot, &transform);
  GTK_WIDGET_GET_CLASS (self)->snapshot (GTK_WIDGET (self), snapshot);

  node = gtk_snapshot_free_to_node (snapshot);

  native = gtk_widget_get_native (GTK_WIDGET (self));
  renderer = gtk_native_get_renderer (native);

  result = gsk_renderer_render_texture (renderer, node, &GRAPHENE_RECT_INIT (0, 0, size, size));

  gsk_render_node_unref (node);

  return result;
}
