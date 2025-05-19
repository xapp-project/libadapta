/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-window.h"

#include "adap-breakpoint-bin-private.h"
#include "adap-dialog-host-private.h"
#include "adap-dialog-private.h"
#include "adap-gizmo-private.h"

/**
 * AdapWindow:
 *
 * A freeform window.
 *
 * <picture>
 *   <source srcset="window-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="window.png" alt="window">
 * </picture>
 *
 * The `AdapWindow` widget is a subclass of [class@Gtk.Window] which has no
 * titlebar area. Instead, [class@ToolbarView] can be used together with
 * [class@HeaderBar] or [class@Gtk.HeaderBar] as follows:
 *
 * ```xml
 * <object class="AdapWindow">
 *   <property name="content">
 *     <object class="AdapToolbarView">
 *       <child type="top">
 *         <object class="AdapHeaderBar"/>
 *       </child>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * Using [property@Gtk.Window:titlebar] or [property@Gtk.Window:child]
 * is not supported and will result in a crash. Use [property@Window:content]
 * instead.
 *
 * ## Dialogs
 *
 * `AdapWindow` can contain [class@Dialog]. Use [method@Dialog.present] with the
 * window or a widget within a window to show a dialog.
 *
 * ## Breakpoints
 *
 * `AdapWindow` can be used with [class@Breakpoint] the same way as
 * [class@BreakpointBin]. Refer to that widget's documentation for details.
 *
 * Example:
 *
 * ```xml
 * <object class="AdapWindow">
 *   <property name="width-request">360</property>
 *   <property name="height-request">200</property>
 *   <property name="content">
 *     <object class="AdapToolbarView">
 *       <child type="top">
 *         <object class="AdapHeaderBar"/>
 *       </child>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *       <child type="bottom">
 *         <object class="GtkActionBar" id="bottom_bar">
 *           <property name="revealed">True</property>
 *           <property name="visible">False</property>
 *         </object>
 *       </child>
 *     </object>
 *   </property>
 *   <child>
 *     <object class="AdapBreakpoint">
 *       <condition>max-width: 500px</condition>
 *       <setter object="bottom_bar" property="visible">True</setter>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * Like `AdapBreakpointBin`, if breakpoints are used, `AdapWindow` doesn't have a
 * minimum size, and [property@Gtk.Widget:width-request] and
 * [property@Gtk.Widget:height-request] properties must be set manually.
 */

typedef struct
{
  GtkWidget *titlebar;
  GtkWidget *bin;
  GtkWidget *dialog_host;
} AdapWindowPrivate;

static void adap_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapWindow, adap_window, GTK_TYPE_WINDOW,
                         G_ADD_PRIVATE (AdapWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_window_buildable_init))

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
notify_current_breakpoint_cb (AdapWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_BREAKPOINT]);
}

static void
notify_visible_dialog_cb (AdapWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

static void
adap_window_size_allocate (GtkWidget *widget,
                          int        width,
                          int        height,
                          int        baseline)
{
  AdapWindow *self = ADAP_WINDOW (widget);
  AdapWindowPrivate *priv = adap_window_get_instance_private (self);

  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (GTK_WINDOW (self)) != priv->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for AdapWindow");

  if (gtk_window_get_child (GTK_WINDOW (self)) != priv->dialog_host)
    g_error ("gtk_window_set_child() is not supported for AdapWindow");

  GTK_WIDGET_CLASS (adap_window_parent_class)->size_allocate (widget,
                                                             width,
                                                             height,
                                                             baseline);
}

static void
adap_window_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdapWindow *self = ADAP_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adap_window_get_content (self));
    break;
  case PROP_CURRENT_BREAKPOINT:
    g_value_set_object (value, adap_window_get_current_breakpoint (self));
    break;
  case PROP_DIALOGS:
    g_value_take_object (value, adap_window_get_dialogs (self));
    break;
  case PROP_VISIBLE_DIALOG:
    g_value_set_object (value, adap_window_get_visible_dialog (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_window_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdapWindow *self = ADAP_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adap_window_set_content (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_window_class_init (AdapWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_window_get_property;
  object_class->set_property = adap_window_set_property;
  widget_class->size_allocate = adap_window_size_allocate;

  /**
   * AdapWindow:content: (attributes org.gtk.Property.get=adap_window_get_content org.gtk.Property.set=adap_window_set_content)
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
   * AdapWindow:current-breakpoint: (attributes org.gtk.Property.get=adap_window_get_current_breakpoint)
   *
   * The current breakpoint.
   *
   * Since: 1.4
   */
  props[PROP_CURRENT_BREAKPOINT] =
    g_param_spec_object ("current-breakpoint", NULL, NULL,
                         ADAP_TYPE_BREAKPOINT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdapWindow:dialogs: (attributes org.gtk.Property.get=adap_window_get_dialogs)
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
   * AdapWindow:visible-dialog: (attributes org.gtk.Property.get=adap_window_get_visible_dialog)
   *
   * The currently visible dialog
   *
   * Since: 1.5
   */
  props[PROP_VISIBLE_DIALOG] =
    g_param_spec_object ("visible-dialog", NULL, NULL,
                         ADAP_TYPE_DIALOG,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adap_window_init (AdapWindow *self)
{
  AdapWindowPrivate *priv = adap_window_get_instance_private (self);

  priv->titlebar = adap_gizmo_new_with_role ("nothing", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                            NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_visible (priv->titlebar, FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (self), priv->titlebar);

  priv->dialog_host = adap_dialog_host_new ();
  gtk_window_set_child (GTK_WINDOW (self), priv->dialog_host);
  adap_dialog_host_set_proxy (ADAP_DIALOG_HOST (priv->dialog_host), GTK_WIDGET (self));

  priv->bin = adap_breakpoint_bin_new ();
  adap_breakpoint_bin_set_warning_widget (ADAP_BREAKPOINT_BIN (priv->bin),
                                         GTK_WIDGET (self));
  adap_dialog_host_set_child (ADAP_DIALOG_HOST (priv->dialog_host), priv->bin);

  g_signal_connect_swapped (priv->bin, "notify::current-breakpoint",
                            G_CALLBACK (notify_current_breakpoint_cb), self);
  g_signal_connect_swapped (priv->dialog_host, "notify::visible-dialog",
                            G_CALLBACK (notify_visible_dialog_cb), self);
}

static void
adap_window_buildable_add_child (GtkBuildable *buildable,
                                GtkBuilder   *builder,
                                GObject      *child,
                                const char   *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adap_window_set_content (ADAP_WINDOW (buildable), GTK_WIDGET (child));
  else if (ADAP_IS_BREAKPOINT (child))
    adap_window_add_breakpoint (ADAP_WINDOW (buildable),
                               g_object_ref (ADAP_BREAKPOINT (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_window_buildable_add_child;
}

/**
 * adap_window_new:
 *
 * Creates a new `AdapWindow`.
 *
 * Returns: the newly created `AdapWindow`
 */
GtkWidget *
adap_window_new (void)
{
  return g_object_new (ADAP_TYPE_WINDOW, NULL);
}

/**
 * adap_window_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: a window
 * @content: (nullable): the content widget
 *
 * Sets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.set_child].
 */
void
adap_window_set_content (AdapWindow *self,
                        GtkWidget *content)
{
  AdapWindowPrivate *priv;

  g_return_if_fail (ADAP_IS_WINDOW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  priv = adap_window_get_instance_private (self);

  if (adap_window_get_content (self) == content)
    return;

  adap_breakpoint_bin_set_child (ADAP_BREAKPOINT_BIN (priv->bin), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adap_window_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: a window
 *
 * Gets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.get_child].
 *
 * Returns: (nullable) (transfer none): the content widget of @self
 */
GtkWidget *
adap_window_get_content (AdapWindow *self)
{
  AdapWindowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_WINDOW (self), NULL);

  priv = adap_window_get_instance_private (self);

  return adap_breakpoint_bin_get_child (ADAP_BREAKPOINT_BIN (priv->bin));
}

/**
 * adap_window_add_breakpoint:
 * @self: a window
 * @breakpoint: (transfer full): the breakpoint to add
 *
 * Adds @breakpoint to @self.
 *
 * Since: 1.4
 */
void
adap_window_add_breakpoint (AdapWindow     *self,
                           AdapBreakpoint *breakpoint)
{
  AdapWindowPrivate *priv;

  g_return_if_fail (ADAP_IS_WINDOW (self));
  g_return_if_fail (ADAP_IS_BREAKPOINT (breakpoint));

  priv = adap_window_get_instance_private (self);

  adap_breakpoint_bin_add_breakpoint (ADAP_BREAKPOINT_BIN (priv->bin), breakpoint);
}

/**
 * adap_window_get_current_breakpoint: (attributes org.gtk.Method.get_property=current-breakpoint)
 * @self: a window
 *
 * Gets the current breakpoint.
 *
 * Returns: (nullable) (transfer none): the current breakpoint
 *
 * Since: 1.4
 */
AdapBreakpoint *
adap_window_get_current_breakpoint (AdapWindow *self)
{
  AdapWindowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_WINDOW (self), NULL);

  priv = adap_window_get_instance_private (self);

  return adap_breakpoint_bin_get_current_breakpoint (ADAP_BREAKPOINT_BIN (priv->bin));
}

/**
 * adap_window_get_dialogs: (attributes org.gtk.Method.get_property=dialogs)
 * @self: a window
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
adap_window_get_dialogs (AdapWindow *self)
{
  AdapWindowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_WINDOW (self), NULL);

  priv = adap_window_get_instance_private (self);

  return adap_dialog_host_get_dialogs (ADAP_DIALOG_HOST (priv->dialog_host));
}

/**
 * adap_window_get_visible_dialog: (attributes org.gtk.Method.get_property=visible-dialog)
 * @self: a window
 *
 * Returns the currently visible dialog in @self, if there's one.
 *
 * Returns: (transfer none) (nullable): the visible dialog
 *
 * Since: 1.5
 */
AdapDialog *
adap_window_get_visible_dialog (AdapWindow *self)
{
  AdapWindowPrivate *priv;

  g_return_val_if_fail (ADAP_IS_WINDOW (self), NULL);

  priv = adap_window_get_instance_private (self);

  return adap_dialog_host_get_visible_dialog (ADAP_DIALOG_HOST (priv->dialog_host));
}
