/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-password-entry-row.h"

#include "adw-entry-row-private.h"

/**
 * AdwPasswordEntryRow:
 *
 * A [class@EntryRow] tailored for entering secrets.
 *
 * <picture>
 *   <source srcset="password-entry-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="password-entry-row.png" alt="password-entry-row">
 * </picture>
 *
 * It does not show its contents in clear text, does not allow to copy it to the
 * clipboard, and shows a warning when Caps Lock is engaged. If the underlying
 * platform allows it, `AdwPasswordEntryRow` will also place the text in a
 * non-pageable memory area, to avoid it being written out to disk by the
 * operating system.
 *
 * It offer a way to reveal the contents in clear text.
 *
 * ## CSS Nodes
 *
 * `AdwPasswordEntryRow` has a single CSS node with name `row` that carries
 * `.entry` and `.password` style classes.
 *
 * Since: 1.2
 */

struct _AdwPasswordEntryRow
{
  AdwEntryRow parent_instance;

  GtkWidget *show_text_toggle;

  GdkDevice *keyboard;
};

G_DEFINE_FINAL_TYPE (AdwPasswordEntryRow, adw_password_entry_row, ADW_TYPE_ENTRY_ROW)

static void
update_caps_lock (AdwPasswordEntryRow *self)
{
  GtkEditable *delegate = gtk_editable_get_delegate (GTK_EDITABLE (self));

  adw_entry_row_set_show_indicator (ADW_ENTRY_ROW (self),
                                    !gtk_text_get_visibility (GTK_TEXT (delegate)) &&
                                    gdk_device_get_caps_lock_state (self->keyboard));
}

static void
notify_visibility_cb (AdwPasswordEntryRow *self)
{
  GtkEditable *delegate = gtk_editable_get_delegate (GTK_EDITABLE (self));

  if (gtk_text_get_visibility (GTK_TEXT (delegate))) {
    gtk_button_set_icon_name (GTK_BUTTON (self->show_text_toggle),
                              "view-conceal-symbolic");
    gtk_widget_set_tooltip_text (self->show_text_toggle, _("Hide Password"));
  } else {
    gtk_button_set_icon_name (GTK_BUTTON (self->show_text_toggle),
                              "view-reveal-symbolic");
    gtk_widget_set_tooltip_text (self->show_text_toggle, _("Show Password"));
  }

  if (self->keyboard)
    update_caps_lock (self);
}

static void
notify_has_focus_cb (AdwPasswordEntryRow *self)
{
  if (self->keyboard)
    update_caps_lock (self);
}

static void
show_text_clicked_cb (AdwPasswordEntryRow *self)
{
  GtkEditable *delegate = gtk_editable_get_delegate (GTK_EDITABLE (self));
  gboolean visible = gtk_text_get_visibility (GTK_TEXT (delegate));

  gtk_text_set_visibility (GTK_TEXT (delegate), !visible);
}

static void
adw_password_entry_row_realize (GtkWidget *widget)
{
  AdwPasswordEntryRow *self = ADW_PASSWORD_ENTRY_ROW (widget);
  GdkSeat *seat;

  GTK_WIDGET_CLASS (adw_password_entry_row_parent_class)->realize (widget);

  seat = gdk_display_get_default_seat (gtk_widget_get_display (widget));
  if (seat)
    self->keyboard = gdk_seat_get_keyboard (seat);

  if (self->keyboard) {
    g_signal_connect_swapped (self->keyboard, "notify::caps-lock-state",
                              G_CALLBACK (update_caps_lock), self);
    update_caps_lock (self);
  }
}

static void
adw_password_entry_row_dispose (GObject *object)
{
  AdwPasswordEntryRow *self = ADW_PASSWORD_ENTRY_ROW (object);

  if (self->keyboard)
    g_signal_handlers_disconnect_by_func (self->keyboard, update_caps_lock, self);

  G_OBJECT_CLASS (adw_password_entry_row_parent_class)->dispose (object);
}

static void
adw_password_entry_row_class_init (AdwPasswordEntryRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_password_entry_row_dispose;

  widget_class->realize = adw_password_entry_row_realize;
}

static void
adw_password_entry_row_init (AdwPasswordEntryRow *self)
{
  GtkEditable *delegate;
  GMenu *menu;
  GMenu *section;
  GMenuItem *item;

  self->show_text_toggle = gtk_button_new ();
  gtk_widget_set_valign (self->show_text_toggle, GTK_ALIGN_CENTER);
  gtk_widget_set_focus_on_click (self->show_text_toggle, FALSE);
  gtk_widget_add_css_class (self->show_text_toggle, "flat");
  adw_entry_row_add_suffix (ADW_ENTRY_ROW (self), self->show_text_toggle);

  delegate = gtk_editable_get_delegate (GTK_EDITABLE (self));

  g_assert (GTK_IS_TEXT (delegate));

  gtk_text_set_visibility (GTK_TEXT (delegate), FALSE);
  gtk_text_set_buffer (GTK_TEXT (delegate), gtk_password_entry_buffer_new ());
  gtk_text_set_input_purpose (GTK_TEXT (delegate), GTK_INPUT_PURPOSE_PASSWORD);

  g_signal_connect_swapped (delegate, "notify::has-focus",
                            G_CALLBACK (notify_has_focus_cb), self);
  g_signal_connect_swapped (delegate, "notify::visibility",
                            G_CALLBACK (notify_visibility_cb), self);
  g_signal_connect_swapped (self->show_text_toggle, "clicked",
                            G_CALLBACK (show_text_clicked_cb), self);

  adw_entry_row_set_indicator_icon_name (ADW_ENTRY_ROW (self), "caps-lock-symbolic");
  adw_entry_row_set_indicator_tooltip (ADW_ENTRY_ROW (self), _("Caps Lock is on"));

  gtk_widget_add_css_class (GTK_WIDGET (self), "password");

  notify_visibility_cb (self);

  menu = g_menu_new ();
  section = g_menu_new ();
  item = g_menu_item_new (_("_Show Password"), "misc.toggle-visibility");
  g_menu_item_set_attribute (item, "touch-icon", "s", "view-reveal-symbolic");
  g_menu_append_item (section, item);

  g_menu_append_section (menu, NULL, G_MENU_MODEL (section));

  gtk_text_set_extra_menu (GTK_TEXT (delegate), G_MENU_MODEL (menu));

  g_object_unref (item);
  g_object_unref (section);
  g_object_unref (menu);
}

/**
 * adw_password_entry_row_new:
 *
 * Creates a new `AdwPasswordEntryRow`.
 *
 * Returns: the newly created `AdwPasswordEntryRow`
 *
 * Since: 1.2
 */
GtkWidget *
adw_password_entry_row_new (void)
{
  return g_object_new (ADW_TYPE_PASSWORD_ENTRY_ROW, NULL);
}
