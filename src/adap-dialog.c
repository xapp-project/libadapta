/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-dialog-private.h"

#include "adw-bottom-sheet-private.h"
#include "adw-breakpoint-bin-private.h"
#include "adw-dialog-host-private.h"
#include "adw-floating-sheet-private.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

#define DEFAULT_NATURAL_SIZE 200

#define PORTRAIT_CONDITION "max-width: 450px"
#define LANDSCAPE_CONDITION "max-height: 360px"

/**
 * AdwDialogPresentationMode:
 * @ADW_DIALOG_AUTO: Switch between `ADW_DIALOG_FLOATING` and
 *   `ADW_DIALOG_BOTTOM_SHEET` depending on available size.
 * @ADW_DIALOG_FLOATING: Present dialog as a centered floating window.
 * @ADW_DIALOG_BOTTOM_SHEET: Present dialog as a bottom sheet.
 *
 * Describes the available presentation modes for [class@Dialog].
 *
 * New values may be added to this enumeration over time.
 *
 * See [property@Dialog:presentation-mode].
 *
 * Since: 1.5
 */

/**
 * AdwDialog:
 *
 * An adaptive dialog container.
 *
 * <picture>
 *   <source srcset="dialog-floating-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="dialog-floating.png" alt="dialog-floating">
 * </picture>
 * <picture>
 *   <source srcset="dialog-bottom-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="dialog-bottom.png" alt="dialog-bottom">
 * </picture>
 *
 * `AdwDialog` is similar to a window, but is shown within another window. It
 * can be used with [class@Window] and [class@ApplicationWindow], use
 * [method@Dialog.present] to show it.
 *
 * `AdwDialog` is not resizable. Use the [property@Dialog:content-width] and
 * [property@Dialog:content-height] properties to set its size, or set
 * [property@Dialog:follows-content-size] to `TRUE` to make the dialog track the
 * content's size as it changes. `AdwDialog` can never be larger than its parent
 * window.
 *
 * `AdwDialog` can be presented as a centered floating window or a bottom sheet.
 * By default it's automatic depending on the available size.
 * [property@Dialog:presentation-mode] can be used to change that.
 *
 * `AdwDialog` can be closed via [method@Dialog.close].
 *
 * When presented as a bottom sheet, `AdwDialog` can also be closed via swiping
 * it down.
 *
 * The [property@Dialog:can-close] can be used to prevent closing. In that case,
 * [signal@Dialog::close-attempt] gets emitted instead.
 *
 * Use [method@Dialog.force_close] to close the dialog even when `can-close` is set to
 * `FALSE`.
 *
 * ## Header Bar Integration
 *
 * When placed inside an `AdwDialog`, [class@HeaderBar] will display the dialog
 * title instead of window title. It will also adjust the decoration layout to
 * ensure it always has a close button and nothing else. Set
 * [property@HeaderBar:show-start-title-buttons] and
 * [property@HeaderBar:show-end-title-buttons] to `FALSE` to remove it if it's
 * unwanted.
 *
 * ## Breakpoints
 *
 * `AdwDialog` can be used with [class@Breakpoint] the same way as
 * [class@BreakpointBin]. Refer to that widget's documentation for details.
 *
 * Like `AdwBreakpointBin`, if breakpoints are used, `AdwDialog` doesn't have a
 * minimum size, and [property@Gtk.Widget:width-request] and
 * [property@Gtk.Widget:height-request] properties must be set manually.
 *
 * Since: 1.5
 */

typedef struct
{
  GtkWidget *child;
  GtkWidget *bin;

  GtkWidget *child_breakpoint_bin;

  AdwBottomSheet *bottom_sheet;
  AdwFloatingSheet *floating_sheet;
  gboolean first_map;

  guint tick_cb_id;
  int ticks;

  char *title;

  gboolean can_close;
  gboolean closing;

  int content_width;
  int content_height;
  gboolean follows_content_size;

  gboolean content_width_set;
  gboolean content_height_set;

  AdwDialogPresentationMode presentation_mode;
  AdwBreakpoint *portrait_breakpoint;
  AdwBreakpoint *landscape_breakpoint;

  GtkWidget *focus_widget;
  GtkWidget *default_widget;

  GtkWidget *last_focus;

  GFunc closing_callback;
  GFunc remove_callback;
  gpointer user_data;

  GtkWidget *window;
  gboolean force_closing;
} AdwDialogPrivate;

static void adw_dialog_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwDialog, adw_dialog, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdwDialog)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_dialog_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_TITLE,
  PROP_CAN_CLOSE,
  PROP_CONTENT_WIDTH,
  PROP_CONTENT_HEIGHT,
  PROP_FOLLOWS_CONTENT_SIZE,
  PROP_PRESENTATION_MODE,
  PROP_FOCUS_WIDGET,
  PROP_DEFAULT_WIDGET,
  PROP_CURRENT_BREAKPOINT,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CLOSE_ATTEMPT,
  SIGNAL_CLOSED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
child_breakpoint_bin_notify_current_breakpoint_cb (AdwDialog *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_BREAKPOINT]);
}

static gboolean
map_tick_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  priv->ticks++;

  /* If we're showing a bottom sheet, it has changed after the initial map,
   * and we can't animate it right away */
  if (priv->ticks == 2) {
    if (priv->bottom_sheet)
      adw_bottom_sheet_set_open (priv->bottom_sheet, TRUE);
    else if (priv->floating_sheet)
      adw_floating_sheet_set_open (priv->floating_sheet, TRUE);

    gtk_widget_grab_focus (GTK_WIDGET (self));

    priv->first_map = FALSE;
    priv->tick_cb_id = 0;
    priv->ticks = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
sheet_closing_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->closing_callback)
    priv->closing_callback (self, priv->user_data);

  g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
}

static void
sheet_closed_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->remove_callback)
    priv->remove_callback (self, priv->user_data);
}

static void
sheet_close_attempt_cb (AdwDialog *self)
{
  g_print ("a\n");
  g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
}

static void
unset_default_widget (AdwDialog *self)
{
  adw_dialog_set_default_widget (self, NULL);
}

static void
default_widget_notify_visible_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  g_assert (priv->default_widget);

  if (!gtk_widget_get_visible (priv->default_widget))
    unset_default_widget (self);
}

static void
default_widget_notify_parent_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  g_assert (priv->default_widget);

  if (!gtk_widget_get_parent (priv->default_widget))
    unset_default_widget (self);
}

static void
unset_focus_widget (AdwDialog *self)
{
  adw_dialog_set_focus (self, NULL);
}

static void
focus_widget_notify_visible_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  g_assert (priv->focus_widget);

  if (!gtk_widget_get_visible (priv->focus_widget))
    unset_focus_widget (self);
}

static void
focus_widget_notify_parent_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  g_assert (priv->focus_widget);

  if (!gtk_widget_get_parent (priv->focus_widget))
    unset_focus_widget (self);
}

static void
set_focus (AdwDialog *self,
           GtkWidget *focus)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (focus == priv->focus_widget)
    return;

  if (priv->focus_widget) {
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          unset_focus_widget, self);
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          focus_widget_notify_visible_cb, self);
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          focus_widget_notify_parent_cb, self);
    g_clear_weak_pointer (&priv->focus_widget);
  }

  priv->focus_widget = focus;

  if (priv->focus_widget) {
    g_object_add_weak_pointer (G_OBJECT (priv->focus_widget), (gpointer *) &priv->focus_widget);

    g_signal_connect_swapped (priv->focus_widget, "hide",
                              G_CALLBACK (unset_focus_widget), self);
    g_signal_connect_swapped (priv->focus_widget, "notify::visible",
                              G_CALLBACK (focus_widget_notify_visible_cb), self);
    g_signal_connect_swapped (priv->focus_widget, "notify::parent",
                              G_CALLBACK (focus_widget_notify_parent_cb), self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOCUS_WIDGET]);
}

static void
window_notify_focus_cb (AdwDialog  *self,
                        GParamSpec *pspec,
                        GtkRoot    *root)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkWidget *focus = gtk_root_get_focus (root);

  if (focus && !gtk_widget_is_ancestor (focus, GTK_WIDGET (self)))
    focus = NULL;

  if (priv->floating_sheet && focus == adw_floating_sheet_get_sheet_bin (priv->floating_sheet))
    focus = NULL;

  if (priv->bottom_sheet && focus == adw_bottom_sheet_get_sheet_bin (priv->bottom_sheet))
    focus = NULL;

  set_focus (self, focus);
}

static void
update_natural_size (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  int width, height;

  /* Follow default/content size for floating dialogs */
  if (priv->follows_content_size) {
    width = height = -1;
  } else {
    width = priv->content_width;
    height = priv->content_height;
  }

  adw_breakpoint_bin_set_natural_size (ADW_BREAKPOINT_BIN (priv->child_breakpoint_bin),
                                       width, height);

  if (priv->window)
    gtk_window_set_default_size (GTK_WINDOW (priv->window), width, height);
}

static void
set_content_size (AdwDialog *self,
                  gboolean   set_width,
                  int        width,
                  gboolean   set_height,
                  int        height)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  gboolean changed = FALSE;
  GtkRequisition min, nat;

  if (!set_width)
    width = priv->content_width_set ? priv->content_width : -1;

  if (!set_height)
    height = priv->content_height_set ? priv->content_height : -1;

  if (priv->child) {
    if (gtk_widget_get_request_mode (priv->child) == GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT) {
      gtk_widget_measure (priv->child, GTK_ORIENTATION_VERTICAL, -1,
                          &min.height, &nat.height, NULL, NULL);

      height = MAX (min.height, height < 0 ? nat.height : height);

      gtk_widget_measure (priv->child, GTK_ORIENTATION_HORIZONTAL, height,
                          &min.width, &nat.width, NULL, NULL);

      width = MAX (min.width, width < 0 ? nat.width : width);
    } else {
      gtk_widget_measure (priv->child, GTK_ORIENTATION_HORIZONTAL, -1,
                          &min.width, &nat.width, NULL, NULL);

      width = MAX (min.width, width < 0 ? nat.width : width);

      gtk_widget_measure (priv->child, GTK_ORIENTATION_VERTICAL, width,
                          &min.height, &nat.height, NULL, NULL);

      height = MAX (min.height, height < 0 ? nat.height : height);
    }
  } else {
    width = width < 0 ? DEFAULT_NATURAL_SIZE : width;
    height = height < 0 ? DEFAULT_NATURAL_SIZE : height;
  }

  g_object_freeze_notify (G_OBJECT (self));

  if (priv->content_width != width) {
    changed = TRUE;
    priv->content_width = width;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT_WIDTH]);
  }

  if (priv->content_height != height) {
    changed = TRUE;
    priv->content_height = height;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT_HEIGHT]);
  }

  if (changed && !priv->follows_content_size)
    update_natural_size (self);

  g_object_thaw_notify (G_OBJECT (self));
}

static void
update_presentation (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  AdwBreakpoint *breakpoint;
  gboolean use_bottom_sheet;
  GtkRoot *root;
  GtkWidget *focus = NULL;

  if (priv->window)
    return;

  breakpoint =
    adw_breakpoint_bin_get_current_breakpoint (ADW_BREAKPOINT_BIN (priv->bin));
  use_bottom_sheet = priv->presentation_mode == ADW_DIALOG_BOTTOM_SHEET ||
                     breakpoint != NULL;

  g_object_ref (priv->child_breakpoint_bin);

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (root) {
    focus = gtk_root_get_focus (root);

    if (focus && !gtk_widget_is_ancestor (focus, GTK_WIDGET (self)))
      focus = NULL;

    if (focus)
      g_object_add_weak_pointer (G_OBJECT (focus), (gpointer *) &focus);

    /* FIXME: the focused widget has a strong ref and it never gets cleared. The
     * real source of this leak is likely somewhere in GTK, but unsetting the
     * focus "fixes" it */
    gtk_root_set_focus (root, NULL);
  }

  if (priv->bottom_sheet) {
    adw_bottom_sheet_set_sheet (priv->bottom_sheet, NULL);
    priv->bottom_sheet = NULL;
  } else if (priv->floating_sheet) {
    adw_floating_sheet_set_child (priv->floating_sheet, NULL);
    priv->floating_sheet = NULL;
  }

  adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->bin), NULL);

  if (use_bottom_sheet) {
    priv->bottom_sheet = ADW_BOTTOM_SHEET (adw_bottom_sheet_new ());

    adw_bottom_sheet_set_min_natural_width (priv->bottom_sheet, 360);

    if (!priv->first_map)
      adw_bottom_sheet_set_open (priv->bottom_sheet, TRUE);

    adw_bottom_sheet_set_show_drag_handle (priv->bottom_sheet, FALSE);
    adw_bottom_sheet_set_sheet (priv->bottom_sheet, priv->child_breakpoint_bin);
    adw_bottom_sheet_set_can_close (priv->bottom_sheet, priv->can_close);
    adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->bin),
                                  GTK_WIDGET (priv->bottom_sheet));

    g_signal_connect_swapped (priv->bottom_sheet, "closing",
                              G_CALLBACK (sheet_closing_cb), self);
    g_signal_connect_swapped (priv->bottom_sheet, "closed",
                              G_CALLBACK (sheet_closed_cb), self);
    g_signal_connect_swapped (priv->bottom_sheet, "close-attempt",
                              G_CALLBACK (sheet_close_attempt_cb), self);

    gtk_widget_add_css_class (GTK_WIDGET (self), "bottom-sheet");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "floating");

    if (breakpoint == priv->portrait_breakpoint) {
      gtk_widget_add_css_class (GTK_WIDGET (self), "portrait");
      gtk_widget_remove_css_class (GTK_WIDGET (self), "landscape");
    } else {
      gtk_widget_add_css_class (GTK_WIDGET (self), "landscape");
      gtk_widget_remove_css_class (GTK_WIDGET (self), "portrait");
    }
  } else {
    priv->floating_sheet = ADW_FLOATING_SHEET (adw_floating_sheet_new ());

    if (!priv->first_map)
      adw_floating_sheet_set_open (priv->floating_sheet, TRUE);

    adw_floating_sheet_set_child (priv->floating_sheet, priv->child_breakpoint_bin);
    adw_floating_sheet_set_can_close (priv->floating_sheet, priv->can_close);
    adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->bin),
                                  GTK_WIDGET (priv->floating_sheet));

    g_signal_connect_swapped (priv->floating_sheet, "closing",
                              G_CALLBACK (sheet_closing_cb), self);
    g_signal_connect_swapped (priv->floating_sheet, "closed",
                              G_CALLBACK (sheet_closed_cb), self);
    g_signal_connect_swapped (priv->floating_sheet, "close-attempt",
                              G_CALLBACK (sheet_close_attempt_cb), self);

    gtk_widget_add_css_class (GTK_WIDGET (self), "floating");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "bottom-sheet");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "portrait");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "landscape");
  }

  if (focus) {
    gtk_widget_grab_focus (focus);
    g_object_remove_weak_pointer (G_OBJECT (focus), (gpointer *) &focus);
  }

  g_object_unref (priv->child_breakpoint_bin);
}

static void
update_presentation_mode (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->window)
    return;

  switch (priv->presentation_mode) {
  case ADW_DIALOG_AUTO:
    g_assert (!priv->portrait_breakpoint);
    g_assert (!priv->landscape_breakpoint);

    priv->landscape_breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse (LANDSCAPE_CONDITION));
    adw_breakpoint_bin_add_breakpoint (ADW_BREAKPOINT_BIN (priv->bin),
                                       priv->landscape_breakpoint);

    priv->portrait_breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse (PORTRAIT_CONDITION));
    adw_breakpoint_bin_add_breakpoint (ADW_BREAKPOINT_BIN (priv->bin),
                                       priv->portrait_breakpoint);
    break;
  case ADW_DIALOG_FLOATING:
  case ADW_DIALOG_BOTTOM_SHEET:
    if (priv->portrait_breakpoint) {
      adw_breakpoint_bin_remove_breakpoint (ADW_BREAKPOINT_BIN (priv->bin),
                                            priv->portrait_breakpoint);
      priv->portrait_breakpoint = NULL;

      adw_breakpoint_bin_remove_breakpoint (ADW_BREAKPOINT_BIN (priv->bin),
                                            priv->landscape_breakpoint);
      priv->landscape_breakpoint = NULL;
    }
    break;
  default:
    g_assert_not_reached ();
  }

  if (priv->bin)
    update_presentation (self);
}

static void
default_activate_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->default_widget && gtk_widget_is_sensitive (priv->default_widget) &&
      (!priv->focus_widget || !gtk_widget_get_receives_default (priv->focus_widget)))
    gtk_widget_activate (priv->default_widget);
  else if (priv->focus_widget && gtk_widget_is_sensitive (priv->focus_widget))
    gtk_widget_activate (priv->focus_widget);
}

static gboolean
window_close_request_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->force_closing || adw_dialog_close (self))
    return GDK_EVENT_PROPAGATE;

  return GDK_EVENT_STOP;
}

static gboolean
maybe_close_cb (GtkWidget *widget,
                GVariant  *args,
                AdwDialog *self)
{
  return adw_dialog_close (self);
}

static void
present_as_window (AdwDialog *self,
                   GtkWidget *parent)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkWidget *titlebar;
  GtkEventController *shortcut_controller;
  GtkShortcut *shortcut;

  if (priv->window) {
    gtk_window_present (GTK_WINDOW (priv->window));
    return;
  }

  gtk_widget_add_css_class (GTK_WIDGET (self), "floating");
  gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);

  priv->window = gtk_window_new ();
  gtk_window_set_resizable (GTK_WINDOW (priv->window), FALSE);
  gtk_widget_add_css_class (priv->window, "dialog-window");

  /* Esc to close */
  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape, 0),
                               gtk_callback_action_new ((GtkShortcutFunc) maybe_close_cb, self, NULL));

  shortcut_controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (shortcut_controller), shortcut);
  gtk_widget_add_controller (priv->window, shortcut_controller);

  if (parent) {
    GtkRoot *root = gtk_widget_get_root (parent);

    if (GTK_IS_WINDOW (root)) {
      gtk_window_set_modal (GTK_WINDOW (priv->window), TRUE);
      gtk_window_set_transient_for (GTK_WINDOW (priv->window), GTK_WINDOW (root));
    }
  }

  titlebar = adw_gizmo_new_with_role ("nothing", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                      NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_visible (titlebar, FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (priv->window), titlebar);

  gtk_widget_set_parent (priv->child_breakpoint_bin, GTK_WIDGET (self));
  gtk_window_set_child (GTK_WINDOW (priv->window), GTK_WIDGET (self));

  g_object_bind_property (self, "title",          priv->window, "title",          G_BINDING_SYNC_CREATE);
  g_object_bind_property (self, "focus-widget",   priv->window, "focus-widget",   G_BINDING_SYNC_CREATE);
  g_object_bind_property (self, "default-widget", priv->window, "default-widget", G_BINDING_SYNC_CREATE);

  g_signal_connect_swapped (priv->window, "close-request", G_CALLBACK (window_close_request_cb), self);

  gtk_window_present (GTK_WINDOW (priv->window));
}

static gboolean
activate_focus_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkRoot *root;

  if (priv->window)
    return GDK_EVENT_PROPAGATE;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WINDOW (root))
    return GDK_EVENT_PROPAGATE;

  g_signal_emit_by_name (root, "activate-focus");

  return GDK_EVENT_STOP;
}

static gboolean
activate_default_cb (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkRoot *root;

  if (priv->window)
    return GDK_EVENT_PROPAGATE;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WINDOW (root))
    return GDK_EVENT_PROPAGATE;

  g_signal_emit_by_name (root, "activate-default");

  return GDK_EVENT_STOP;
}

static void
add_tab_bindings (GtkWidgetClass   *widget_class,
                  GdkModifierType   modifiers,
                  GtkDirectionType  direction)
{
  GtkShortcut *shortcut;

  shortcut = gtk_shortcut_new_with_arguments (
                 gtk_alternative_trigger_new (gtk_keyval_trigger_new (GDK_KEY_Tab, modifiers),
                                              gtk_keyval_trigger_new (GDK_KEY_KP_Tab, modifiers)),
                 gtk_signal_action_new ("move-focus"),
                 "(i)", direction);

  gtk_widget_class_add_shortcut (widget_class, shortcut);

  g_object_unref (shortcut);
}

static void
add_arrow_bindings (GtkWidgetClass   *widget_class,
                    guint             keysym,
                    GtkDirectionType  direction)
{
  guint keypad_keysym = keysym - GDK_KEY_Left + GDK_KEY_KP_Left;

  gtk_widget_class_add_binding_signal (widget_class, keysym, 0,
                                       "move-focus", "(i)", direction);
  gtk_widget_class_add_binding_signal (widget_class, keysym, GDK_CONTROL_MASK,
                                       "move-focus", "(i)", direction);
  gtk_widget_class_add_binding_signal (widget_class, keypad_keysym, 0,
                                       "move-focus", "(i)", direction);
  gtk_widget_class_add_binding_signal (widget_class, keypad_keysym, GDK_CONTROL_MASK,
                                       "move-focus", "(i)", direction);
}

static gboolean
open_inspector_cb (AdwDialog *self,
                   GVariant  *args)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkRoot *root;
  gboolean preselect_widget, ret;

  if (priv->window)
    return GDK_EVENT_PROPAGATE;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WINDOW (root))
    return GDK_EVENT_PROPAGATE;

  preselect_widget = g_variant_get_boolean (args);

  g_signal_emit_by_name (root, "enable-debugging", preselect_widget, &ret);

  return ret;
}

static gboolean
ensure_focus (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkWidget *focus;

  if (!root)
    return FALSE;

  focus = gtk_root_get_focus (root);

  if (focus)
    return FALSE;

  /* No focusable widgets, focus something intermediate instead */
  if (priv->floating_sheet)
    return gtk_widget_grab_focus (GTK_WIDGET (priv->floating_sheet));

  if (priv->bottom_sheet)
    return gtk_widget_grab_focus (GTK_WIDGET (priv->bottom_sheet));

  if (priv->window)
    return adw_widget_grab_focus_self (GTK_WIDGET (self));

  return TRUE;
}

static void
adw_dialog_root (GtkWidget *widget)
{
  AdwDialog *self = ADW_DIALOG (widget);
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);
  GtkRoot *root;
  GtkWidget *parent;

  GTK_WIDGET_CLASS (adw_dialog_parent_class)->root (widget);

  set_content_size (self, FALSE, -1, FALSE, -1);

  root = gtk_widget_get_root (widget);

  if (!GTK_IS_WINDOW (root))
    return;

  parent = gtk_widget_get_parent (widget);

  if (parent != priv->window && !ADW_IS_DIALOG_HOST (parent)) {
    g_error ("Trying to add %s %p to %s %p. Use adw_dialog_present() to show dialogs.",
              G_OBJECT_TYPE_NAME (self), self,
              G_OBJECT_TYPE_NAME (parent), parent);
  }

  g_signal_connect_swapped (root, "notify::focus-widget",
                            G_CALLBACK (window_notify_focus_cb), self);
}

static void
adw_dialog_unroot (GtkWidget *widget)
{
  GtkRoot *root = gtk_widget_get_root (widget);

  if (GTK_IS_WINDOW (root))
    g_signal_handlers_disconnect_by_func (root, window_notify_focus_cb, widget);

  GTK_WIDGET_CLASS (adw_dialog_parent_class)->unroot (widget);
}

static void
adw_dialog_map (GtkWidget *widget)
{
  AdwDialog *self = ADW_DIALOG (widget);
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  GTK_WIDGET_CLASS (adw_dialog_parent_class)->map (widget);

  if (priv->window)
    return;

  priv->tick_cb_id = gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                                   (GtkTickCallback) map_tick_cb,
                                                   NULL, NULL);
}

static gboolean
adw_dialog_focus (GtkWidget        *widget,
                  GtkDirectionType  direction)
{
  if (adw_widget_focus_child (widget, direction))
    return TRUE;

  return ensure_focus (ADW_DIALOG (widget));
}

static gboolean
adw_dialog_grab_focus (GtkWidget *widget)
{
  AdwDialog *self = ADW_DIALOG (widget);
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->focus_widget)
    return gtk_widget_grab_focus (priv->focus_widget);

  GTK_WIDGET_GET_CLASS (widget)->move_focus (GTK_WIDGET (widget),
                                             GTK_DIR_TAB_FORWARD);

  return ensure_focus (ADW_DIALOG (widget));
}

static void
adw_dialog_dispose (GObject *object)
{
  AdwDialog *self = ADW_DIALOG (object);
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  if (priv->focus_widget) {
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          unset_focus_widget, self);
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          focus_widget_notify_visible_cb, self);
    g_signal_handlers_disconnect_by_func (priv->focus_widget,
                                          focus_widget_notify_parent_cb, self);
    g_clear_weak_pointer (&priv->focus_widget);
  }

  g_clear_weak_pointer (&priv->last_focus);

  if (priv->default_widget) {
    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          unset_default_widget, self);
    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          default_widget_notify_visible_cb, self);
    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          default_widget_notify_parent_cb, self);
    priv->default_widget = NULL;
  }

  if (priv->bin) {
    /* It's an in-window dialog */
    g_clear_pointer (&priv->bin, gtk_widget_unparent);
    priv->bottom_sheet = NULL;
    priv->floating_sheet = NULL;
    priv->child_breakpoint_bin = NULL;
    priv->child = NULL;
  } else if (priv->child_breakpoint_bin &&
             gtk_widget_get_parent (priv->child_breakpoint_bin) == GTK_WIDGET (self)) {
    /* It's an window-backed dialog */
    g_clear_pointer (&priv->child_breakpoint_bin, gtk_widget_unparent);
    priv->child = NULL;
  } else {
    /* It hasn't been presented so we have a floating child breakpoint bin */
    if (priv->child_breakpoint_bin)
      g_object_ref_sink (priv->child_breakpoint_bin);

    g_clear_object (&priv->child_breakpoint_bin);
    priv->child = NULL;
  }

  G_OBJECT_CLASS (adw_dialog_parent_class)->dispose (object);
}

static void
adw_dialog_finalize (GObject *object)
{
  AdwDialog *self = ADW_DIALOG (object);
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  g_free (priv->title);

  G_OBJECT_CLASS (adw_dialog_parent_class)->finalize (object);
}

static void
adw_dialog_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwDialog *self = ADW_DIALOG (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_dialog_get_child (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, adw_dialog_get_title (self));
    break;
  case PROP_CAN_CLOSE:
    g_value_set_boolean (value, adw_dialog_get_can_close (self));
    break;
  case PROP_CONTENT_WIDTH:
    g_value_set_int (value, adw_dialog_get_content_width (self));
    break;
  case PROP_CONTENT_HEIGHT:
    g_value_set_int (value, adw_dialog_get_content_height (self));
    break;
  case PROP_FOLLOWS_CONTENT_SIZE:
    g_value_set_boolean (value, adw_dialog_get_follows_content_size (self));
    break;
  case PROP_PRESENTATION_MODE:
    g_value_set_enum (value, adw_dialog_get_presentation_mode (self));
    break;
  case PROP_FOCUS_WIDGET:
    g_value_set_object (value, adw_dialog_get_focus (self));
    break;
  case PROP_DEFAULT_WIDGET:
    g_value_set_object (value, adw_dialog_get_default_widget (self));
    break;
  case PROP_CURRENT_BREAKPOINT:
    g_value_set_object (value, adw_dialog_get_current_breakpoint (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_dialog_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwDialog *self = ADW_DIALOG (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_dialog_set_child (self, g_value_get_object (value));
    break;
  case PROP_TITLE:
    adw_dialog_set_title (self, g_value_get_string (value));
    break;
  case PROP_CAN_CLOSE:
    adw_dialog_set_can_close (self, g_value_get_boolean (value));
    break;
  case PROP_CONTENT_WIDTH:
    adw_dialog_set_content_width (self, g_value_get_int (value));
    break;
  case PROP_CONTENT_HEIGHT:
    adw_dialog_set_content_height (self, g_value_get_int (value));
    break;
  case PROP_PRESENTATION_MODE:
    adw_dialog_set_presentation_mode (self, g_value_get_enum (value));
    break;
  case PROP_FOCUS_WIDGET:
    adw_dialog_set_focus (self, g_value_get_object (value));
    break;
  case PROP_DEFAULT_WIDGET:
    adw_dialog_set_default_widget (self, g_value_get_object (value));
    break;
  case PROP_FOLLOWS_CONTENT_SIZE:
    adw_dialog_set_follows_content_size (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_dialog_class_init (AdwDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_dialog_dispose;
  object_class->finalize = adw_dialog_finalize;
  object_class->get_property = adw_dialog_get_property;
  object_class->set_property = adw_dialog_set_property;

  widget_class->root = adw_dialog_root;
  widget_class->unroot = adw_dialog_unroot;
  widget_class->map = adw_dialog_map;
  widget_class->focus = adw_dialog_focus;
  widget_class->grab_focus = adw_dialog_grab_focus;
  widget_class->contains = adw_widget_contains_passthrough;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwDialog:child: (attributes org.gtk.Property.get=adw_dialog_get_child org.gtk.Property.set=adw_dialog_set_child)
   *
   * The child widget of the `AdwDialog`.
   *
   * Since: 1.5
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:title: (attributes org.gtk.Property.get=adw_dialog_get_title org.gtk.Property.set=adw_dialog_set_title)
   *
   * The title of the dialog.
   *
   * Since: 1.5
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:can-close: (attributes org.gtk.Property.get=adw_dialog_get_can_close org.gtk.Property.set=adw_dialog_set_can_close)
   *
   * Whether the dialog can be closed.
   *
   * If set to `FALSE`, the close button, shortcuts and
   * [method@Dialog.close] will result in [signal@Dialog::close-attempt] being
   * emitted instead, and bottom sheet close swipe will be disabled.
   * [method@Dialog.force_close] still works.
   *
   * Since: 1.5
   */
  props[PROP_CAN_CLOSE] =
    g_param_spec_boolean ("can-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:content-width: (attributes org.gtk.Property.get=adw_dialog_get_content_width org.gtk.Property.set=adw_dialog_set_content_width)
   *
   * The width of the dialog's contents.
   *
   * Set it to -1 to reset it to the content's natural width.
   *
   * See also: [property@Gtk.Window:default-width]
   *
   * Since: 1.5
   */
  props[PROP_CONTENT_WIDTH] =
    g_param_spec_int ("content-width", NULL, NULL,
                      -1, G_MAXINT, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:content-height: (attributes org.gtk.Property.get=adw_dialog_get_content_height org.gtk.Property.set=adw_dialog_set_content_height)
   *
   * The height of the dialog's contents.
   *
   * Set it to -1 to reset it to the content's natural height.
   *
   * See also: [property@Gtk.Window:default-height]
   *
   * Since: 1.5
   */
  props[PROP_CONTENT_HEIGHT] =
    g_param_spec_int ("content-height", NULL, NULL,
                      -1, G_MAXINT, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:follows-content-size: (attributes org.gtk.Property.get=adw_dialog_get_follows_content_size org.gtk.Property.set=adw_dialog_set_follows_content_size)
   *
   * Whether to size content automatically.
   *
   * If set to `TRUE`, always use the content's natural size instead of
   * [property@Dialog:content-width] and [property@Dialog:content-height]. If
   * the content resizes, the dialog will immediately resize as well.
   *
   * See also: [property@Gtk.Window:resizable]
   *
   * Since: 1.5
   */
  props[PROP_FOLLOWS_CONTENT_SIZE] =
    g_param_spec_boolean ("follows-content-size", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:presentation-mode: (attributes org.gtk.Property.get=adw_dialog_get_presentation_mode org.gtk.Property.set=adw_dialog_set_presentation_mode)
   *
   * The dialog's presentation mode.
   *
   * When set to `ADW_DIALOG_AUTO`, the dialog appears as a bottom sheet when
   * the following condition is met: `max-width: 450px or max-height: 360px`,
   * and as a floating window otherwise.
   *
   * Set it to `ADW_DIALOG_FLOATING` or `ADW_DIALOG_BOTTOM_SHEET` to always
   * present it a floating window or a bottom sheet respectively, regardless of
   * available size.
   *
   * Presentation mode does nothing for dialogs presented as a window.
   *
   * Since: 1.5
   */
  props[PROP_PRESENTATION_MODE] =
    g_param_spec_enum ("presentation-mode", NULL, NULL,
                       ADW_TYPE_DIALOG_PRESENTATION_MODE,
                       ADW_DIALOG_AUTO,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:focus-widget: (attributes org.gtk.Property.get=adw_dialog_get_focus org.gtk.Property.set=adw_dialog_set_focus)
   *
   * The focus widget.
   *
   * Since: 1.5
   */
  props[PROP_FOCUS_WIDGET] =
    g_param_spec_object ("focus-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:default-widget: (attributes org.gtk.Property.get=adw_dialog_get_default_widget org.gtk.Property.set=adw_dialog_set_default_widget)
   *
   * The default widget.
   *
   * It's activated when the user presses Enter.
   *
   * Since: 1.5
   */
  props[PROP_DEFAULT_WIDGET] =
    g_param_spec_object ("default-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwDialog:current-breakpoint: (attributes org.gtk.Property.get=adw_dialog_get_current_breakpoint)
   *
   * The current breakpoint.
   *
   * Since: 1.5
   */
  props[PROP_CURRENT_BREAKPOINT] =
    g_param_spec_object ("current-breakpoint", NULL, NULL,
                         ADW_TYPE_BREAKPOINT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwDialog::close-attempt:
   * @self: a dialog
   *
   * Emitted when the close button or shortcut is used, or
   * [method@Dialog.close] is called while [property@Dialog:can-close] is set to
   * `FALSE`.
   *
   * Since: 1.5
   */
  signals[SIGNAL_CLOSE_ATTEMPT] =
    g_signal_new ("close-attempt",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwDialogClass, close_attempt),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_ATTEMPT],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwDialog::closed:
   * @self: a dialog
   *
   * Emitted when the dialog is successfully closed.
   *
   * Since: 1.5
   */
  signals[SIGNAL_CLOSED] =
    g_signal_new ("closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwDialogClass, closed),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_install_action (widget_class, "default.activate", NULL,
                                   (GtkWidgetActionActivateFunc) default_activate_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_space, 0,
                                (GtkShortcutFunc) activate_focus_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Space, 0,
                                (GtkShortcutFunc) activate_focus_cb, NULL);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_Return, 0,
                                (GtkShortcutFunc) activate_default_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_ISO_Enter, 0,
                                (GtkShortcutFunc) activate_default_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Enter, 0,
                                (GtkShortcutFunc) activate_default_cb, NULL);

  add_arrow_bindings (widget_class, GDK_KEY_Up,    GTK_DIR_UP);
  add_arrow_bindings (widget_class, GDK_KEY_Down,  GTK_DIR_DOWN);
  add_arrow_bindings (widget_class, GDK_KEY_Left,  GTK_DIR_LEFT);
  add_arrow_bindings (widget_class, GDK_KEY_Right, GTK_DIR_RIGHT);

  add_tab_bindings (widget_class, 0,                                 GTK_DIR_TAB_FORWARD);
  add_tab_bindings (widget_class, GDK_CONTROL_MASK,                  GTK_DIR_TAB_FORWARD);
  add_tab_bindings (widget_class, GDK_SHIFT_MASK,                    GTK_DIR_TAB_BACKWARD);
  add_tab_bindings (widget_class, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_DIR_TAB_BACKWARD);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_I, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                (GtkShortcutFunc) open_inspector_cb, "b", FALSE);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_D, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                (GtkShortcutFunc) open_inspector_cb, "b", TRUE);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0,
                                (GtkShortcutFunc) adw_dialog_close, NULL);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "dialog");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_DIALOG);
}

static void
adw_dialog_init (AdwDialog *self)
{
  AdwDialogPrivate *priv = adw_dialog_get_instance_private (self);

  priv->first_map = TRUE;
  priv->title = g_strdup ("");
  priv->can_close = TRUE;
  priv->content_width = -1;
  priv->content_height = -1;
  priv->follows_content_size = FALSE;
  priv->presentation_mode = ADW_DIALOG_AUTO;

  priv->child_breakpoint_bin = adw_breakpoint_bin_new ();
  adw_breakpoint_bin_set_warning_widget (ADW_BREAKPOINT_BIN (priv->child_breakpoint_bin),
                                         GTK_WIDGET (self));
  g_object_bind_property (self, "width-request",
                          priv->child_breakpoint_bin, "width-request",
                          G_BINDING_DEFAULT);
  g_object_bind_property (self, "height-request",
                          priv->child_breakpoint_bin, "height-request",
                          G_BINDING_DEFAULT);
  g_signal_connect_swapped (priv->child_breakpoint_bin,
                            "notify::current-breakpoint",
                            G_CALLBACK (child_breakpoint_bin_notify_current_breakpoint_cb),
                            self);
}

static void
adw_dialog_buildable_add_child (GtkBuildable *buildable,
                                GtkBuilder   *builder,
                                GObject      *child,
                                const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_dialog_set_child (ADW_DIALOG (buildable), GTK_WIDGET (child));
  else if (ADW_IS_BREAKPOINT (child))
    adw_dialog_add_breakpoint (ADW_DIALOG (buildable),
                               g_object_ref (ADW_BREAKPOINT (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_dialog_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_dialog_buildable_add_child;
}

/**
 * adw_dialog_new:
 *
 * Creates a new `AdwDialog`.
 *
 * Returns: the new created `AdwDialog`
 *
 * Since: 1.5
 */
AdwDialog *
adw_dialog_new (void)
{
  return g_object_new (ADW_TYPE_DIALOG, NULL);
}

/**
 * adw_dialog_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a dialog
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.5
 */
GtkWidget *
adw_dialog_get_child (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);

  return priv->child;
}

/**
 * adw_dialog_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a dialog
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.5
 */
void
adw_dialog_set_child (AdwDialog *self,
                      GtkWidget *child)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adw_dialog_get_instance_private (self);

  if (priv->child == child)
    return;

  priv->child = child;

  adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->child_breakpoint_bin), child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * adw_dialog_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a dialog
 *
 * Gets the title of @self.
 *
 * Returns: (transfer none): the title
 *
 * Since: 1.5
 */
const char *
adw_dialog_get_title (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);

  return priv->title;
}

/**
 * adw_dialog_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a dialog
 * @title: (transfer none): the new title
 *
 * Sets the title of @self.
 *
 * Since: 1.5
 */
void
adw_dialog_set_title (AdwDialog  *self,
                      const char *title)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  if (!g_set_str (&priv->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, priv->title,
                                  -1);
}

/**
 * adw_dialog_get_can_close: (attributes org.gtk.Method.get_property=can-close)
 * @self: a dialog
 *
 * Gets whether @self can be closed.
 *
 * Returns: whether the dialog can be closed
 *
 * Since: 1.5
 */
gboolean
adw_dialog_get_can_close (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), FALSE);

  priv = adw_dialog_get_instance_private (self);

  return priv->can_close;
}

/**
 * adw_dialog_set_can_close: (attributes org.gtk.Method.set_property=can-close)
 * @self: a dialog
 * @can_close: whether to allow closing
 *
 * Sets whether @self can be closed.
 *
 * If set to `FALSE`, the close button, shortcuts and
 * [method@Dialog.close] will result in [signal@Dialog::close-attempt] being
 * emitted instead, and bottom sheet close swipe will be disabled.
 * [method@Dialog.force_close] still works.
 *
 * Since: 1.5
 */
void
adw_dialog_set_can_close (AdwDialog *self,
                          gboolean   can_close)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  can_close = !!can_close;

  if (priv->can_close == can_close)
    return;

  priv->can_close = can_close;

  if (priv->bottom_sheet)
    adw_bottom_sheet_set_can_close (priv->bottom_sheet, can_close);

  if (priv->floating_sheet)
    adw_floating_sheet_set_can_close (priv->floating_sheet, can_close);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_CLOSE]);
}

/**
 * adw_dialog_get_content_width: (attributes org.gtk.Method.get_property=content-width)
 * @self: a dialog
 *
 * Gets the width of the dialog's contents.
 *
 * Returns: the content width
 *
 * Since: 1.5
 */
int
adw_dialog_get_content_width (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), 0);

  priv = adw_dialog_get_instance_private (self);

  return priv->content_width;
}

/**
 * adw_dialog_set_content_width: (attributes org.gtk.Method.set_property=content-width)
 * @self: a dialog
 * @content_width: the content width
 *
 * Sets the width of the dialog's contents.
 *
 * Set it to -1 to reset it to the content's natural width.
 *
 * See also: [property@Gtk.Window:default-width]
 *
 * Since: 1.5
 */
void
adw_dialog_set_content_width (AdwDialog *self,
                              int        content_width)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (content_width >= -1);

  priv = adw_dialog_get_instance_private (self);

  priv->content_width_set = TRUE;
  set_content_size (self, TRUE, content_width, FALSE, -1);
}

/**
 * adw_dialog_get_content_height: (attributes org.gtk.Method.get_property=content-height)
 * @self: a dialog
 *
 * Gets the height of the dialog's contents.
 *
 * Returns: the content height
 *
 * Since: 1.5
 */
int
adw_dialog_get_content_height (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), 0);

  priv = adw_dialog_get_instance_private (self);

  return priv->content_height;
}

/**
 * adw_dialog_set_content_height: (attributes org.gtk.Method.set_property=content-height)
 * @self: a dialog
 * @content_height: the content height
 *
 * Sets the height of the dialog's contents.
 *
 * Set it to -1 to reset it to the content's natural height.
 *
 * See also: [property@Gtk.Window:default-height]
 *
 * Since: 1.5
 */
void
adw_dialog_set_content_height (AdwDialog *self,
                               int        content_height)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (content_height >= -1);

  priv = adw_dialog_get_instance_private (self);

  priv->content_height_set = TRUE;
  set_content_size (self, FALSE, -1, TRUE, content_height);
}

/**
 * adw_dialog_get_follows_content_size: (attributes org.gtk.Method.get_property=follows-content-size)
 * @self: a dialog
 *
 * Gets whether to size content of @self automatically.
 *
 * Returns: whether to size content automatically
 *
 * Since: 1.5
 */
gboolean
adw_dialog_get_follows_content_size (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), FALSE);

  priv = adw_dialog_get_instance_private (self);

  return priv->follows_content_size;
}

/**
 * adw_dialog_set_follows_content_size: (attributes org.gtk.Method.set_property=follows-content-size)
 * @self: a dialog
 * @follows_content_size: whether to size content automatically
 *
 * Sets whether to size content of @self automatically.
 *
 * If set to `TRUE`, always use the content's natural size instead of
 * [property@Dialog:content-width] and [property@Dialog:content-height]. If
 * the content resizes, the dialog will immediately resize as well.
 *
 * See also: [property@Gtk.Window:resizable]
 *
 * Since: 1.5
 */
void
adw_dialog_set_follows_content_size (AdwDialog *self,
                                     gboolean   follows_content_size)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  follows_content_size = !!follows_content_size;

  if (priv->follows_content_size == follows_content_size)
    return;

  priv->follows_content_size = follows_content_size;

  update_natural_size (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOLLOWS_CONTENT_SIZE]);
}

/**
 * adw_dialog_get_presentation_mode: (attributes org.gtk.Method.get_property=presentation-mode)
 * @self: a dialog
 *
 * Gets presentation mode for @self.
 *
 * Returns: the presentation mode
 *
 * Since: 1.5
 */
AdwDialogPresentationMode
adw_dialog_get_presentation_mode (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), ADW_DIALOG_AUTO);

  priv = adw_dialog_get_instance_private (self);

  return priv->presentation_mode;
}

/**
 * adw_dialog_set_presentation_mode: (attributes org.gtk.Method.set_property=presentation-mode)
 * @self: a dialog
 * @presentation_mode: the new presentation mode
 *
 * Sets presentation mode for @self.
 *
 * When set to `ADW_DIALOG_AUTO`, the dialog appears as a bottom sheet when the
 * following condition is met: `max-width: 450px or max-height: 360px`, and as a
 * floating window otherwise.
 *
 * Set it to `ADW_DIALOG_FLOATING` or `ADW_DIALOG_BOTTOM_SHEET` to always
 * present it a floating window or a bottom sheet respectively, regardless of
 * available size.
 *
 * Presentation mode does nothing for dialogs presented as a window.
 *
 * Since: 1.5
 */
void
adw_dialog_set_presentation_mode (AdwDialog                 *self,
                                  AdwDialogPresentationMode  presentation_mode)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (presentation_mode >= ADW_DIALOG_AUTO);
  g_return_if_fail (presentation_mode <= ADW_DIALOG_BOTTOM_SHEET);

  priv = adw_dialog_get_instance_private (self);

  if (priv->presentation_mode == presentation_mode)
    return;

  priv->presentation_mode = presentation_mode;

  update_presentation_mode (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PRESENTATION_MODE]);
}

/**
 * adw_dialog_get_focus: (attributes org.gtk.Method.get_property=focus-widget)
 * @self: a dialog
 *
 * Gets the focus widget for @self.
 *
 * Returns: (nullable) (transfer none): the focus widget
 *
 * Since: 1.5
 */
GtkWidget *
adw_dialog_get_focus (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);

  return priv->focus_widget;
}

/**
 * adw_dialog_set_focus: (attributes org.gtk.Method.set_property=focus-widget)
 * @self: a dialog
 * @focus: (nullable) (transfer none): the focus widget
 *
 * Sets the focus widget for @self.
 *
 * If @focus is not the current focus widget, and is focusable, sets it as the
 * focus widget for the dialog.
 *
 * If focus is `NULL`, unsets the focus widget for this dialog. To set the focus
 * to a particular widget in the dialog, it is usually more convenient to use
 * [method@Gtk.Widget.grab_focus] instead of this function.
 *
 * Since: 1.5
 */
void
adw_dialog_set_focus (AdwDialog *self,
                      GtkWidget *focus)
{
  AdwDialogPrivate *priv;
  GtkRoot *root;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (focus == NULL || GTK_IS_WIDGET (focus));

  priv = adw_dialog_get_instance_private (self);

  if (!gtk_widget_get_mapped (GTK_WIDGET (self)) || priv->tick_cb_id != 0) {
    set_focus (self, focus);
    return;
  }

  if (priv->focus_widget == focus)
    return;

  if (!gtk_widget_get_can_focus (priv->bin)) {
    g_set_weak_pointer (&priv->last_focus, priv->focus_widget);
    set_focus (self, focus);
    return;
  }

  root = gtk_widget_get_root (GTK_WIDGET (self));

  /* If we're mapped, we definitely have a root */
  g_assert (root != NULL);

  gtk_root_set_focus (root, focus);
}

/**
 * adw_dialog_get_default_widget: (attributes org.gtk.Method.get_property=default-widget)
 * @self: a dialog
 *
 * Gets the default widget for @self.
 *
 * Returns: (nullable) (transfer none): the default widget
 *
 * Since: 1.5
 */
GtkWidget *
adw_dialog_get_default_widget (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);

  return priv->default_widget;
}

/**
 * adw_dialog_set_default_widget: (attributes org.gtk.Method.set_property=default-widget)
 * @self: a dialog
 * @default_widget: (nullable) (transfer none): the default widget
 *
 * Sets the default widget for @self.
 *
 * It's activated when the user presses Enter.
 *
 * Since: 1.5
 */
void
adw_dialog_set_default_widget (AdwDialog *self,
                               GtkWidget *default_widget)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (default_widget == NULL || GTK_IS_WIDGET (default_widget));

  priv = adw_dialog_get_instance_private (self);

  if (priv->default_widget == default_widget)
    return;

  if (priv->default_widget) {
    if ((priv->focus_widget != priv->default_widget ||
         !gtk_widget_get_receives_default (priv->default_widget)) &&
        !gtk_widget_has_default (priv->default_widget)) {
      gtk_widget_remove_css_class (priv->default_widget, "default");
    }

    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          unset_default_widget, self);
    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          default_widget_notify_visible_cb, self);
    g_signal_handlers_disconnect_by_func (priv->default_widget,
                                          default_widget_notify_parent_cb, self);
  }

  priv->default_widget = default_widget;

  if (priv->default_widget) {
    if ((priv->focus_widget == NULL ||
         !gtk_widget_get_receives_default (priv->focus_widget)) &&
        !gtk_widget_has_default (priv->default_widget)) {
      gtk_widget_add_css_class (priv->default_widget, "default");
    }

    g_signal_connect_swapped (priv->default_widget, "hide",
                              G_CALLBACK (unset_default_widget), self);
    g_signal_connect_swapped (priv->default_widget, "notify::visible",
                              G_CALLBACK (default_widget_notify_visible_cb), self);
    g_signal_connect_swapped (priv->default_widget, "notify::parent",
                              G_CALLBACK (default_widget_notify_parent_cb), self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEFAULT_WIDGET]);
}

/**
 * adw_dialog_close:
 * @self: a dialog
 *
 * Attempts to close @self.
 *
 * If the [property@Dialog:can-close] property is set to `FALSE`, the
 * [signal@Dialog::close-attempt] signal is emitted.
 *
 * See also: [method@Dialog.force_close].
 *
 * Returns: whether @self was successfully closed
 *
 * Since: 1.5
 */
gboolean
adw_dialog_close (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), FALSE);

  priv = adw_dialog_get_instance_private (self);

  if (!priv->can_close) {
  g_print ("b\n");
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
    return FALSE;
  }

  if (priv->window) {
    GtkWidget *window = priv->window;
    priv->window = NULL;

    if (priv->closing_callback)
      priv->closing_callback (self, priv->user_data);

    g_signal_emit (self, signals[SIGNAL_CLOSED], 0);

    gtk_window_close (GTK_WINDOW (window));
  } else {
    adw_dialog_force_close (self);
  }

  return TRUE;
}

/**
 * adw_dialog_force_close:
 * @self: a dialog
 *
 * Closes @self.
 *
 * Unlike [method@Dialog.close], it succeeds even if [property@Dialog:can-close]
 * is set to `FALSE`.
 *
 * Since: 1.5
 */
void
adw_dialog_force_close (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  g_object_ref (self);

  priv->force_closing = TRUE;

  if (priv->bottom_sheet)
    adw_bottom_sheet_set_open (priv->bottom_sheet, FALSE);
  else if (priv->floating_sheet)
    adw_floating_sheet_set_open (priv->floating_sheet, FALSE);
  else if (priv->window)
    gtk_window_close (GTK_WINDOW (priv->window));

  g_object_unref (self);
}

/**
 * adw_dialog_add_breakpoint:
 * @self: a dialog
 * @breakpoint: (transfer full): the breakpoint to add
 *
 * Adds @breakpoint to @self.
 *
 * Since: 1.5
 */
void
adw_dialog_add_breakpoint (AdwDialog     *self,
                           AdwBreakpoint *breakpoint)
{
  AdwDialogPrivate *priv;
  AdwBreakpointBin *bin;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (ADW_IS_BREAKPOINT (breakpoint));

  priv = adw_dialog_get_instance_private (self);
  bin = ADW_BREAKPOINT_BIN (priv->child_breakpoint_bin);

  adw_breakpoint_bin_add_breakpoint (bin, breakpoint);
}

/**
 * adw_dialog_get_current_breakpoint: (attributes org.gtk.Method.get_property=current-breakpoint)
 * @self: a dialog
 *
 * Gets the current breakpoint.
 *
 * Returns: (nullable) (transfer none): the current breakpoint
 *
 * Since: 1.5
 */
AdwBreakpoint *
adw_dialog_get_current_breakpoint (AdwDialog *self)
{
  AdwDialogPrivate *priv;
  AdwBreakpointBin *bin;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);
  bin = ADW_BREAKPOINT_BIN (priv->child_breakpoint_bin);

  return adw_breakpoint_bin_get_current_breakpoint (bin);
}

static AdwDialogHost *
find_dialog_host (GtkWidget *widget)
{
  while (widget) {
    AdwDialogHost *proxy;

    if (ADW_IS_DIALOG_HOST (widget))
      break;

    proxy = adw_dialog_host_get_from_proxy (widget);
    if (proxy)
      return proxy;

    widget = gtk_widget_get_parent (widget);
  }

  if (ADW_IS_DIALOG_HOST (widget))
    return ADW_DIALOG_HOST (widget);

  return NULL;
}

/**
 * adw_dialog_present:
 * @self: a dialog
 * @parent: (nullable): a widget within the toplevel
 *
 * Presents @self within @parent's window.
 *
 * If @self is already shown, raises it to the top instead.
 *
 * If the window is an [class@Window] or [class@ApplicationWindow], the dialog
 * will be shown within it. Otherwise, it will be a separate window.
 *
 * Since: 1.5
 */
void
adw_dialog_present (AdwDialog *self,
                    GtkWidget *parent)
{
  AdwDialogPrivate *priv;
  AdwDialogHost *host = NULL, *current_host;

  g_return_if_fail (ADW_IS_DIALOG (self));
  g_return_if_fail (parent == NULL || GTK_IS_WIDGET (parent));

  priv = adw_dialog_get_instance_private (self);

  if (parent) {
    GtkRoot *root = gtk_widget_get_root (parent);

    host = find_dialog_host (parent);

    if (GTK_IS_WINDOW (root) && !gtk_window_get_resizable (GTK_WINDOW (root)))
      host = NULL;
  }

  if (host == NULL) {
    current_host = find_dialog_host (GTK_WIDGET (self));

    if (current_host) {
      GtkWidget *current_proxy = adw_dialog_host_get_proxy (current_host);

      if (!current_proxy)
        current_proxy = GTK_WIDGET (current_host);

      g_critical ("Cannot present %s %p as it's already presented for %s %p",
                  G_OBJECT_TYPE_NAME (self), self,
                  G_OBJECT_TYPE_NAME (current_proxy), current_proxy);
      return;
    }

    present_as_window (self, parent);
    return;
  }

  if (!priv->bin) {
    priv->bin = adw_breakpoint_bin_new ();
    adw_breakpoint_bin_set_pass_through (ADW_BREAKPOINT_BIN (priv->bin), TRUE);
    adw_breakpoint_bin_set_warnings (ADW_BREAKPOINT_BIN (priv->bin), FALSE, TRUE);

    gtk_widget_set_parent (priv->bin, GTK_WIDGET (self));

    g_signal_connect_swapped (priv->bin, "notify::current-breakpoint",
                              G_CALLBACK (update_presentation), self);

    update_presentation_mode (self);
    update_presentation (self);
  }

  current_host = find_dialog_host (GTK_WIDGET (self));

  if (current_host) {
    if (current_host != host) {
      GtkWidget *proxy = adw_dialog_host_get_proxy (host);
      GtkWidget *current_proxy = adw_dialog_host_get_proxy (current_host);

      if (!proxy)
        proxy = GTK_WIDGET (host);

      if (!current_proxy)
        current_proxy = GTK_WIDGET (current_host);

      g_critical ("Cannot present %s %p for %s %p as it's already presented for "
                  "%s %p",
                  G_OBJECT_TYPE_NAME (self), self,
                  G_OBJECT_TYPE_NAME (proxy), proxy,
                  G_OBJECT_TYPE_NAME (current_proxy), current_proxy);
    }
  }

  adw_dialog_host_present_dialog (host, self);

  if (!priv->first_map) {
    if (priv->bottom_sheet)
      adw_bottom_sheet_set_open (priv->bottom_sheet, TRUE);
    else if (priv->floating_sheet)
      adw_floating_sheet_set_open (priv->floating_sheet, TRUE);
  }

  if (current_host)
    gtk_widget_grab_focus (GTK_WIDGET (self));
}

void
adw_dialog_set_shadowed (AdwDialog *self,
                         gboolean   shadowed)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  shadowed = !!shadowed;

  if (shadowed) {
    GtkWidget *focus = priv->focus_widget;

    while (focus && !gtk_widget_get_mapped (focus))
      focus = gtk_widget_get_parent (focus);

    if (focus && gtk_widget_is_ancestor (focus, priv->child_breakpoint_bin))
      g_set_weak_pointer (&priv->last_focus, focus);
  }

  gtk_widget_set_can_focus (priv->bin, !shadowed);
  gtk_widget_set_can_target (priv->bin, !shadowed);

  if (!shadowed) {
    if (priv->last_focus)
      gtk_widget_grab_focus (priv->last_focus);

    g_clear_weak_pointer (&priv->last_focus);
  }
}

void
adw_dialog_set_callbacks (AdwDialog *self,
                          GFunc      closing_callback,
                          GFunc      remove_callback,
                          gpointer   user_data)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  priv->closing_callback = closing_callback;
  priv->remove_callback = remove_callback;
  priv->user_data = user_data;
}

gboolean
adw_dialog_get_closing (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), FALSE);

  priv = adw_dialog_get_instance_private (self);

  return priv->closing;
}

void
adw_dialog_set_closing (AdwDialog *self,
                        gboolean   closing)
{
  AdwDialogPrivate *priv;

  g_return_if_fail (ADW_IS_DIALOG (self));

  priv = adw_dialog_get_instance_private (self);

  priv->closing = closing;
}

GtkWidget *
adw_dialog_get_window (AdwDialog *self)
{
  AdwDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_DIALOG (self), NULL);

  priv = adw_dialog_get_instance_private (self);

  return priv->window;
}
