/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-window-title.h"

/**
 * AdwWindowTitle:
 *
 * A helper widget for setting a window's title and subtitle.
 *
 * <picture>
 *   <source srcset="window-title-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="window-title.png" alt="window-title">
 * </picture>
 *
 * `AdwWindowTitle` shows a title and subtitle. It's intended to be used as the
 * title child of [class@Gtk.HeaderBar] or [class@HeaderBar].
 *
 * ## CSS nodes
 *
 * `AdwWindowTitle` has a single CSS node with name `windowtitle`.
 */

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  LAST_PROP,
};

struct _AdwWindowTitle
{
  GtkWidget parent_instance;

  GtkBox *box;
  GtkLabel *title_label;
  GtkLabel *subtitle_label;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwWindowTitle, adw_window_title, GTK_TYPE_WIDGET)

static void
adw_window_title_init (AdwWindowTitle *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
adw_window_title_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwWindowTitle *self = ADW_WINDOW_TITLE (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_window_title_get_title (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, adw_window_title_get_subtitle (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_window_title_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwWindowTitle *self = ADW_WINDOW_TITLE (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_window_title_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    adw_window_title_set_subtitle (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_window_title_dispose (GObject *object)
{
  gtk_widget_dispose_template (GTK_WIDGET (object), ADW_TYPE_WINDOW_TITLE);

  G_OBJECT_CLASS (adw_window_title_parent_class)->dispose (object);
}

static void
adw_window_title_class_init (AdwWindowTitleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_window_title_get_property;
  object_class->set_property = adw_window_title_set_property;
  object_class->dispose = adw_window_title_dispose;

  /**
   * AdwWindowTitle:title: (attributes org.gtk.Property.get=adw_window_title_get_title org.gtk.Property.set=adw_window_title_set_title)
   *
   * The title to display.
   *
   * The title typically identifies the current view or content item, and
   * generally does not use the application name.
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWindowTitle:subtitle: (attributes org.gtk.Property.get=adw_window_title_get_subtitle org.gtk.Property.set=adw_window_title_set_subtitle)
   *
   * The subtitle to display.
   *
   * The subtitle should give the user additional details.
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "windowtitle");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-window-title.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwWindowTitle, box);
  gtk_widget_class_bind_template_child (widget_class, AdwWindowTitle, title_label);
  gtk_widget_class_bind_template_child (widget_class, AdwWindowTitle, subtitle_label);
}

/**
 * adw_window_title_new:
 * @title: a title
 * @subtitle: a subtitle
 *
 * Creates a new `AdwWindowTitle`.
 *
 * Returns: the newly created `AdwWindowTitle`
 */
GtkWidget *
adw_window_title_new (const char *title,
                      const char *subtitle)
{
  return g_object_new (ADW_TYPE_WINDOW_TITLE,
                       "title", title,
                       "subtitle", subtitle,
                       NULL);
}

/**
 * adw_window_title_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a window title
 *
 * Gets the title of @self.
 *
 * Returns: the title
 */
const char *
adw_window_title_get_title (AdwWindowTitle *self)
{
  g_return_val_if_fail (ADW_IS_WINDOW_TITLE (self), NULL);

  return gtk_label_get_label (self->title_label);
}

/**
 * adw_window_title_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a window title
 * @title: a title
 *
 * Sets the title of @self.
 *
 * The title typically identifies the current view or content item, and
 * generally does not use the application name.
 */
void
adw_window_title_set_title (AdwWindowTitle *self,
                            const char     *title)
{
  g_return_if_fail (ADW_IS_WINDOW_TITLE (self));

  if (g_strcmp0 (gtk_label_get_label (self->title_label), title) == 0)
    return;

  gtk_label_set_label (self->title_label, title);
  gtk_widget_set_visible (GTK_WIDGET (self->title_label),
                          title && title[0]);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_window_title_get_subtitle: (attributes org.gtk.Method.get_property=subtitle)
 * @self: a window title
 *
 * Gets the subtitle of @self.
 *
 * Returns: the subtitle
 */
const char *
adw_window_title_get_subtitle (AdwWindowTitle *self)
{
  g_return_val_if_fail (ADW_IS_WINDOW_TITLE (self), NULL);

  return gtk_label_get_label (self->subtitle_label);
}

/**
 * adw_window_title_set_subtitle: (attributes org.gtk.Method.set_property=subtitle)
 * @self: a window title
 * @subtitle: a subtitle
 *
 * Sets the subtitle of @self.
 *
 * The subtitle should give the user additional details.
 */
void
adw_window_title_set_subtitle (AdwWindowTitle *self,
                               const char     *subtitle)
{
  g_return_if_fail (ADW_IS_WINDOW_TITLE (self));

  if (g_strcmp0 (gtk_label_get_label (self->subtitle_label), subtitle) == 0)
    return;

  gtk_label_set_label (self->subtitle_label, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (self->subtitle_label),
                          subtitle && subtitle[0]);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}
