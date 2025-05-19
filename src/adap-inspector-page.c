/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-inspector-page-private.h"

#include <adwaita.h>
#include "adw-settings-private.h"

struct _AdwInspectorPage
{
  AdwBin parent_instance;

  AdwSettings *settings;

  AdwSwitchRow *support_color_schemes_row;
  AdwComboRow *color_scheme_row;
  AdwSwitchRow *high_contrast_row;

  GObject *object;
};

G_DEFINE_FINAL_TYPE (AdwInspectorPage, adw_inspector_page, ADW_TYPE_BIN)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_OBJECT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
color_scheme_changed_cb (AdwInspectorPage *self)
{
  AdwEnumListItem *item = adw_combo_row_get_selected_item (self->color_scheme_row);
  AdwSystemColorScheme color_scheme = adw_enum_list_item_get_value (item);

  adw_settings_override_color_scheme (self->settings, color_scheme);
}

static void
support_color_schemes_changed_cb (AdwInspectorPage *self)
{
  gboolean supports = adw_switch_row_get_active (self->support_color_schemes_row);

  adw_settings_override_system_supports_color_schemes (self->settings, supports);

  if (supports)
    color_scheme_changed_cb (self);
}

static void
high_contrast_changed_cb (AdwInspectorPage *self)
{
  gboolean hc = adw_switch_row_get_active (self->high_contrast_row);

  adw_settings_override_high_contrast (self->settings, hc);
}

static char *
get_system_color_scheme_name (AdwEnumListItem *item,
                              gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_SYSTEM_COLOR_SCHEME_DEFAULT:
    return g_strdup (_("No Preference"));
  case ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK:
    return g_strdup (_("Prefer Dark"));
  case ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT:
    return g_strdup (_("Prefer Light"));
  default:
    return NULL;
  }
  return "";
}

static void
adw_inspector_page_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  switch (prop_id) {
  case PROP_TITLE:
    /* Translators: The name of the library, not the stylesheet */
    g_value_set_string (value, _("Adwaita"));
    break;
  case PROP_OBJECT:
    g_value_set_object (value, self->object);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inspector_page_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  switch (prop_id) {
  case PROP_OBJECT:
    g_set_object (&self->object, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inspector_page_dispose (GObject *object)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  if (self->settings) {
    adw_settings_end_override (self->settings);
    self->settings = NULL;
  }

  g_clear_object (&self->object);

  G_OBJECT_CLASS (adw_inspector_page_parent_class)->dispose (object);
}

static void
adw_inspector_page_class_init (AdwInspectorPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_inspector_page_get_property;
  object_class->set_property = adw_inspector_page_set_property;
  object_class->dispose = adw_inspector_page_dispose;

  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "Libadwaita",
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_OBJECT] =
    g_param_spec_object ("object", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-inspector-page.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, support_color_schemes_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, color_scheme_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, high_contrast_row);

  gtk_widget_class_bind_template_callback (widget_class, get_system_color_scheme_name);
  gtk_widget_class_bind_template_callback (widget_class, support_color_schemes_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, color_scheme_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, high_contrast_changed_cb);
}

static void
adw_inspector_page_init (AdwInspectorPage *self)
{
  AdwSystemColorScheme color_scheme;
  gboolean supports, hc;

  self->settings = adw_settings_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));

  adw_settings_start_override (self->settings);

  color_scheme = adw_settings_get_color_scheme (self->settings);
  adw_combo_row_set_selected (self->color_scheme_row, color_scheme);

  supports = adw_settings_get_system_supports_color_schemes (self->settings);
  adw_switch_row_set_active (self->support_color_schemes_row, supports);

  hc = adw_settings_get_high_contrast (self->settings);
  adw_switch_row_set_active (self->high_contrast_row, hc);
}
