/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-sheet-controls-private.h"

#include "adw-widget-utils-private.h"

struct _AdwSheetControls
{
  GtkWidget parent_instance;

  GtkPackType side;
  char *decoration_layout;

  gboolean empty;
  gboolean prefers_start;
};

enum {
  PROP_0,
  PROP_SIDE,
  PROP_DECORATION_LAYOUT,
  PROP_EMPTY,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwSheetControls, adw_sheet_controls, GTK_TYPE_WIDGET)

static gboolean
get_prefers_start (AdwSheetControls *self)
{
  GtkWidget *widget = GTK_WIDGET (self);
  char *layout_desc;
  gboolean ret;

  if (self->decoration_layout) {
    layout_desc = g_strdup (self->decoration_layout);
  } else {
    g_object_get (gtk_widget_get_settings (widget),
                  "gtk-decoration-layout", &layout_desc,
                  NULL);
  }

  ret = adw_decoration_layout_prefers_start (layout_desc);

  g_free (layout_desc);

  return ret;
}

static void
set_empty (AdwSheetControls *self,
           gboolean          empty)
{
  if (empty == self->empty)
    return;

  self->empty = empty;

  if (empty)
    gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "empty");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EMPTY]);
}

static void
clear_controls (AdwSheetControls *self)
{
  GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (self));

  while (child) {
    GtkWidget *next = gtk_widget_get_next_sibling (child);

    gtk_widget_unparent (child);

    child = next;
  }
}

static void
update_window_buttons (AdwSheetControls *self,
                       gboolean          force_update)
{
  GtkWidget *widget = GTK_WIDGET (self);
  gboolean prefers_start;
  GtkWidget *button = NULL;
  GtkWidget *image = NULL;

  prefers_start = get_prefers_start (self);

  if (prefers_start == self->prefers_start && !force_update)
    return;

  clear_controls (self);

  if (prefers_start != (self->side == GTK_PACK_START)) {
    set_empty (self, TRUE);
    return;
  }

  button = gtk_button_new ();
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  /* The icon is not relevant for accessibility purposes */
  image = g_object_new (GTK_TYPE_IMAGE,
                        "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                        "icon-name", "window-close-symbolic",
                        "use-fallback", TRUE,
                        NULL);
  gtk_widget_add_css_class (button, "close");
  gtk_button_set_child (GTK_BUTTON (button), image);
  gtk_widget_set_can_focus (button, FALSE);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (button),
                                  "sheet.close");
  gtk_accessible_update_property (GTK_ACCESSIBLE (button),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, _("Close"),
                                  GTK_ACCESSIBLE_PROPERTY_DESCRIPTION,
                                    _("Close"),
                                  -1);

  gtk_widget_set_parent (button, widget);
  set_empty (self, FALSE);
}

static void
adw_sheet_controls_root (GtkWidget *widget)
{
  GtkSettings *settings;

  GTK_WIDGET_CLASS (adw_sheet_controls_parent_class)->root (widget);

  settings = gtk_widget_get_settings (widget);
  g_signal_connect_swapped (settings, "notify::gtk-decoration-layout",
                            G_CALLBACK (update_window_buttons), widget);

  update_window_buttons (ADW_SHEET_CONTROLS (widget), FALSE);
}

static void
adw_sheet_controls_unroot (GtkWidget *widget)
{
  GtkSettings *settings;

  settings = gtk_widget_get_settings (widget);

  g_signal_handlers_disconnect_by_func (settings, update_window_buttons, widget);

  GTK_WIDGET_CLASS (adw_sheet_controls_parent_class)->unroot (widget);
}

static void
adw_sheet_controls_dispose (GObject *object)
{
  AdwSheetControls *self = ADW_SHEET_CONTROLS (object);

  clear_controls (self);

  G_OBJECT_CLASS (adw_sheet_controls_parent_class)->dispose (object);
}

static void
adw_sheet_controls_finalize (GObject *object)
{
  AdwSheetControls *self = ADW_SHEET_CONTROLS (object);

  g_free (self->decoration_layout);

  G_OBJECT_CLASS (adw_sheet_controls_parent_class)->finalize (object);
}

static void
adw_sheet_controls_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwSheetControls *self = ADW_SHEET_CONTROLS (object);

  switch (prop_id) {
  case PROP_SIDE:
    g_value_set_enum (value, adw_sheet_controls_get_side (self));
    break;
  case PROP_DECORATION_LAYOUT:
    g_value_set_string (value, adw_sheet_controls_get_decoration_layout (self));
    break;
  case PROP_EMPTY:
    g_value_set_boolean (value, adw_sheet_controls_get_empty (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sheet_controls_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwSheetControls *self = ADW_SHEET_CONTROLS (object);

  switch (prop_id) {
  case PROP_SIDE:
    adw_sheet_controls_set_side (self, g_value_get_enum (value));
    break;
  case PROP_DECORATION_LAYOUT:
    adw_sheet_controls_set_decoration_layout (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sheet_controls_class_init (AdwSheetControlsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_sheet_controls_dispose;
  object_class->finalize = adw_sheet_controls_finalize;
  object_class->get_property = adw_sheet_controls_get_property;
  object_class->set_property = adw_sheet_controls_set_property;

  widget_class->root = adw_sheet_controls_root;
  widget_class->unroot = adw_sheet_controls_unroot;

  props[PROP_SIDE] =
      g_param_spec_enum ("side", NULL, NULL,
                         GTK_TYPE_PACK_TYPE,
                         GTK_PACK_START,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DECORATION_LAYOUT] =
      g_param_spec_string ("decoration-layout", NULL, NULL,
                           NULL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_EMPTY] =
    g_param_spec_boolean ("empty", NULL, NULL,
                          TRUE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "windowcontrols");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_sheet_controls_init (AdwSheetControls *self)
{
  self->decoration_layout = NULL;
  self->side = GTK_PACK_START;
  self->empty = TRUE;

  gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  gtk_widget_add_css_class (GTK_WIDGET (self), "start");

  gtk_widget_set_can_focus (GTK_WIDGET (self), FALSE);

  update_window_buttons (self, TRUE);
}

GtkWidget *
adw_sheet_controls_new (GtkPackType side)
{
  return g_object_new (ADW_TYPE_SHEET_CONTROLS,
                       "side", side,
                       NULL);
}

GtkPackType
adw_sheet_controls_get_side (AdwSheetControls *self)
{
  g_return_val_if_fail (ADW_IS_SHEET_CONTROLS (self), GTK_PACK_START);

  return self->side;
}

void
adw_sheet_controls_set_side (AdwSheetControls *self,
                             GtkPackType       side)
{
  g_return_if_fail (ADW_IS_SHEET_CONTROLS (self));

  if (self->side == side)
    return;

  self->side = side;

  switch (side) {
  case GTK_PACK_START:
    gtk_widget_add_css_class (GTK_WIDGET (self), "start");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "end");
    break;
  case GTK_PACK_END:
    gtk_widget_add_css_class (GTK_WIDGET (self), "end");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "start");
    break;
  default:
    g_assert_not_reached ();
    break;
  }

  update_window_buttons (self, TRUE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDE]);
}

const char *
adw_sheet_controls_get_decoration_layout (AdwSheetControls *self)
{
  g_return_val_if_fail (ADW_IS_SHEET_CONTROLS (self), NULL);

  return self->decoration_layout;
}

void
adw_sheet_controls_set_decoration_layout (AdwSheetControls *self,
                                          const char       *layout)
{
  g_return_if_fail (ADW_IS_SHEET_CONTROLS (self));

  if (!g_set_str (&self->decoration_layout, layout))
    return;

  update_window_buttons (self, TRUE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DECORATION_LAYOUT]);
}

gboolean
adw_sheet_controls_get_empty (AdwSheetControls *self)
{
  g_return_val_if_fail (ADW_IS_SHEET_CONTROLS (self), FALSE);

  return self->empty;
}
