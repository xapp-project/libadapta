/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-application-window.h"

#include "adw-breakpoint-bin-private.h"
#include "adw-dialog-host-private.h"
#include "adw-dialog-private.h"
#include "adw-gizmo-private.h"

/**
 * AdwApplicationWindow:
 *
 * A freeform application window.
 *
 * <picture>
 *   <source srcset="application-window-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="application-window.png" alt="application-window">
 * </picture>
 *
 * `AdwApplicationWindow` is a [class@Gtk.ApplicationWindow] subclass providing
 * the same features as [class@Window].
 *
 * See [class@Window] for details.
 *
 * Example of an `AdwApplicationWindow` UI definition:
 *
 * ```xml
 * <object class="AdwApplicationWindow">
 *   <property name="content">
 *     <object class="AdwToolbarView">
 *       <child type="top">
 *         <object class="AdwHeaderBar"/>
 *       </child>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * Using [property@Gtk.Application:menubar] is not supported and may result in
 * visual glitches.
 */

typedef struct
{
  GtkWidget *titlebar;
  GtkWidget *bin;
  GtkWidget *dialog_host;
} AdwApplicationWindowPrivate;

static void adw_application_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwApplicationWindow, adw_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (AdwApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_application_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_CURRENT_BREAKPOINT,
  PROP_DIALOGS,
  PROP_VISIBLE_DIALOG,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
notify_current_breakpoint_cb (AdwApplicationWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_BREAKPOINT]);
}

static void
notify_visible_dialog_cb (AdwApplicationWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

static void
adw_application_window_size_allocate (GtkWidget *widget,
                                      int        width,
                                      int        height,
                                      int        baseline)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (widget);
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (GTK_WINDOW (self)) != priv->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for AdwApplicationWindow");

  if (gtk_window_get_child (GTK_WINDOW (self)) != priv->dialog_host)
    g_error ("gtk_window_set_child() is not supported for AdwApplicationWindow");

  GTK_WIDGET_CLASS (adw_application_window_parent_class)->size_allocate (widget,
                                                                         width,
                                                                         height,
                                                                         baseline);
}

static void
adw_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_application_window_get_content (self));
    break;
  case PROP_CURRENT_BREAKPOINT:
    g_value_set_object (value, adw_application_window_get_current_breakpoint (self));
    break;
  case PROP_DIALOGS:
    g_value_take_object (value, adw_application_window_get_dialogs (self));
    break;
  case PROP_VISIBLE_DIALOG:
    g_value_set_object (value, adw_application_window_get_visible_dialog (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_application_window_set_content (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_class_init (AdwApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_application_window_get_property;
  object_class->set_property = adw_application_window_set_property;
  widget_class->size_allocate = adw_application_window_size_allocate;

  /**
   * AdwApplicationWindow:content: (attributes org.gtk.Property.get=adw_application_window_get_content org.gtk.Property.set=adw_application_window_set_content)
   *
   * The content widget.
   *
   * This property should always be used instead of [property@Gtk.Window:child].
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwApplicationWindow:current-breakpoint: (attributes org.gtk.Property.get=adw_application_window_get_current_breakpoint)
   *
   * The current breakpoint.
   *
   * Since: 1.4
   */
  props[PROP_CURRENT_BREAKPOINT] =
    g_param_spec_object ("current-breakpoint", NULL, NULL,
                         ADW_TYPE_BREAKPOINT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwApplicationWindow:dialogs: (attributes org.gtk.Property.get=adw_application_window_get_dialogs)
   *
   * The open dialogs.
   *
   * Since: 1.5
   */
  props[PROP_DIALOGS] =
    g_param_spec_object ("dialogs", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwApplicationWindow:visible-dialog: (attributes org.gtk.Property.get=adw_application_window_get_visible_dialog)
   *
   * The currently visible dialog
   *
   * Since: 1.5
   */
  props[PROP_VISIBLE_DIALOG] =
    g_param_spec_object ("visible-dialog", NULL, NULL,
                         ADW_TYPE_DIALOG,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_application_window_init (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  priv->titlebar = adw_gizmo_new_with_role ("nothing", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                            NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_visible (priv->titlebar, FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (self), priv->titlebar);

  priv->dialog_host = adw_dialog_host_new ();
  gtk_window_set_child (GTK_WINDOW (self), priv->dialog_host);
  adw_dialog_host_set_proxy (ADW_DIALOG_HOST (priv->dialog_host), GTK_WIDGET (self));

  priv->bin = adw_breakpoint_bin_new ();
  adw_breakpoint_bin_set_warning_widget (ADW_BREAKPOINT_BIN (priv->bin),
                                         GTK_WIDGET (self));
  adw_dialog_host_set_child (ADW_DIALOG_HOST (priv->dialog_host), priv->bin);

  g_signal_connect_swapped (priv->bin, "notify::current-breakpoint",
                            G_CALLBACK (notify_current_breakpoint_cb), self);
  g_signal_connect_swapped (priv->dialog_host, "notify::visible-dialog",
                            G_CALLBACK (notify_visible_dialog_cb), self);

  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), FALSE);
}

static void
adw_application_window_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (buildable), GTK_WIDGET (child));
  else if (ADW_IS_BREAKPOINT (child))
    adw_application_window_add_breakpoint (ADW_APPLICATION_WINDOW (buildable),
                                           g_object_ref (ADW_BREAKPOINT (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_application_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_application_window_buildable_add_child;
}

/**
 * adw_application_window_new:
 * @app: an application instance
 *
 * Creates a new `AdwApplicationWindow` for @app.
 *
 * Returns: the newly created `AdwApplicationWindow`
 */
GtkWidget *
adw_application_window_new (GtkApplication *app)
{
  return g_object_new (ADW_TYPE_APPLICATION_WINDOW,
                       "application", app,
                       NULL);
}

/**
 * adw_application_window_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: an application window
 * @content: (nullable): the content widget
 *
 * Sets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.set_child].
 */
void
adw_application_window_set_content (AdwApplicationWindow *self,
                                    GtkWidget            *content)
{
  AdwApplicationWindowPrivate *priv;

  g_return_if_fail (ADW_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  priv = adw_application_window_get_instance_private (self);

  if (adw_application_window_get_content (self) == content)
    return;

  adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->bin), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_application_window_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: an application window
 *
 * Gets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.get_child].
 *
 * Returns: (nullable) (transfer none): the content widget of @self
 */
GtkWidget *
adw_application_window_get_content (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  priv = adw_application_window_get_instance_private (self);

  return adw_breakpoint_bin_get_child (ADW_BREAKPOINT_BIN (priv->bin));
}

/**
 * adw_application_window_add_breakpoint:
 * @self: an application window
 * @breakpoint: (transfer full): the breakpoint to add
 *
 * Adds @breakpoint to @self.
 *
 * Since: 1.4
 */
void
adw_application_window_add_breakpoint (AdwApplicationWindow *self,
                                       AdwBreakpoint        *breakpoint)
{
  AdwApplicationWindowPrivate *priv;

  g_return_if_fail (ADW_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (ADW_IS_BREAKPOINT (breakpoint));

  priv = adw_application_window_get_instance_private (self);

  adw_breakpoint_bin_add_breakpoint (ADW_BREAKPOINT_BIN (priv->bin), breakpoint);
}

/**
 * adw_application_window_get_current_breakpoint: (attributes org.gtk.Method.get_property=current-breakpoint)
 * @self: an application window
 *
 * Gets the current breakpoint.
 *
 * Returns: (nullable) (transfer none): the current breakpoint
 *
 * Since: 1.4
 */
AdwBreakpoint *
adw_application_window_get_current_breakpoint (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  priv = adw_application_window_get_instance_private (self);

  return adw_breakpoint_bin_get_current_breakpoint (ADW_BREAKPOINT_BIN (priv->bin));
}

/**
 * adw_application_window_get_dialogs: (attributes org.gtk.Method.get_property=dialogs)
 * @self: an application window
 *
 * Returns a [iface@Gio.ListModel] that contains the open dialogs of @self.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a list model for the dialogs of @self
 *
 * Since: 1.5
 */
GListModel *
adw_application_window_get_dialogs (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  priv = adw_application_window_get_instance_private (self);

  return adw_dialog_host_get_dialogs (ADW_DIALOG_HOST (priv->dialog_host));
}

/**
 * adw_application_window_get_visible_dialog: (attributes org.gtk.Method.get_property=visible-dialog)
 * @self: an application window
 *
 * Returns the currently visible dialog in @self, if there's one.
 *
 * Returns: (transfer none) (nullable): the visible dialog
 *
 * Since: 1.5
 */
AdwDialog *
adw_application_window_get_visible_dialog (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  priv = adw_application_window_get_instance_private (self);

  return adw_dialog_host_get_visible_dialog (ADW_DIALOG_HOST (priv->dialog_host));
}
