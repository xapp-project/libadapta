/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-view-switcher-title.h"

#include "adw-squeezer.h"
#include "adw-window-title.h"

/**
 * AdwViewSwitcherTitle:
 *
 * A view switcher title.
 *
 * <picture>
 *   <source srcset="view-switcher-title-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="view-switcher-title.png" alt="view-switcher-title">
 * </picture>
 *
 * A widget letting you switch between multiple views contained by a
 * [class@ViewStack] via an [class@ViewSwitcher].
 *
 * It is designed to be used as the title widget of a [class@HeaderBar], and
 * will display the window's title when the window is too narrow to fit the view
 * switcher e.g. on mobile phones, or if there are less than two views.
 *
 * In order to center the title in narrow windows, the header bar should have
 * [property@HeaderBar:centering-policy] set to
 * `ADW_CENTERING_POLICY_STRICT`.
 *
 * `AdwViewSwitcherTitle` is intended to be used together with
 * [class@ViewSwitcherBar].
 *
 * A common use case is to bind the [property@ViewSwitcherBar:reveal] property
 * to [property@ViewSwitcherTitle:title-visible] to automatically reveal the
 * view switcher bar when the title label is displayed in place of the view
 * switcher, as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="content">
 *     <object class="AdwToolbarView">
 *       <child type="top">
 *         <object class="AdwHeaderBar">
 *           <property name="centering-policy">strict</property>
 *           <property name="title-widget">
 *             <object class="AdwViewSwitcherTitle" id="title">
 *               <property name="stack">stack</property>
 *             </object>
 *           </property>
 *         </object>
 *       </child>
 *       <property name="content">
 *         <object class="AdwViewStack" id="stack"/>
 *       </property>
 *       <child type="bottom">
 *         <object class="AdwViewSwitcherBar">
 *           <property name="stack">stack</property>
 *           <binding name="reveal">
 *             <lookup name="title-visible">title</lookup>
 *           </binding>
 *         </object>
 *       </child>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwViewSwitcherTitle` has a single CSS node with name `viewswitchertitle`.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */

enum {
  PROP_0,
  PROP_STACK,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_VIEW_SWITCHER_ENABLED,
  PROP_TITLE_VISIBLE,
  LAST_PROP,
};

struct _AdwViewSwitcherTitle
{
  GtkWidget parent_instance;

  AdwSqueezer *squeezer;
  AdwWindowTitle *title_widget;
  AdwViewSwitcher *wide_view_switcher;
  AdwViewSwitcher *narrow_view_switcher;

  gboolean view_switcher_enabled;
  gboolean is_window_narrow;

  GtkSelectionModel *pages;

  guint check_window_width_id;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwViewSwitcherTitle, adw_view_switcher_title, GTK_TYPE_WIDGET)

static void
update_view_switcher_visible (AdwViewSwitcherTitle *self)
{
  AdwSqueezerPage *switcher_page;
  int count = 0;

  if (!self->squeezer)
    return;

  if (!self->is_window_narrow && self->view_switcher_enabled && self->pages) {
    guint i, n;

    n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
    for (i = 0; i < n; i++) {
      AdwViewStackPage *page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);

      if (adw_view_stack_page_get_visible (page))
        count++;

      g_object_unref (page);
    }
  }

  switcher_page = adw_squeezer_get_page (self->squeezer, GTK_WIDGET (self->wide_view_switcher));
  adw_squeezer_page_set_enabled (switcher_page, count > 1);

  switcher_page = adw_squeezer_get_page (self->squeezer, GTK_WIDGET (self->narrow_view_switcher));
  adw_squeezer_page_set_enabled (switcher_page, count > 1);
}

static void
notify_squeezer_visible_child_cb (GObject *self)
{
  g_object_notify_by_pspec (self, props[PROP_TITLE_VISIBLE]);
}

static void
adw_view_switcher_title_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  AdwViewSwitcherTitle *self = ADW_VIEW_SWITCHER_TITLE (object);

  switch (prop_id) {
  case PROP_STACK:
    g_value_set_object (value, adw_view_switcher_title_get_stack (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, adw_view_switcher_title_get_title (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, adw_view_switcher_title_get_subtitle (self));
    break;
  case PROP_VIEW_SWITCHER_ENABLED:
    g_value_set_boolean (value, adw_view_switcher_title_get_view_switcher_enabled (self));
    break;
  case PROP_TITLE_VISIBLE:
    g_value_set_boolean (value, adw_view_switcher_title_get_title_visible (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_title_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  AdwViewSwitcherTitle *self = ADW_VIEW_SWITCHER_TITLE (object);

  switch (prop_id) {
  case PROP_STACK:
    adw_view_switcher_title_set_stack (self, g_value_get_object (value));
    break;
  case PROP_TITLE:
    adw_view_switcher_title_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    adw_view_switcher_title_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_VIEW_SWITCHER_ENABLED:
    adw_view_switcher_title_set_view_switcher_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_title_dispose (GObject *object)
{
  AdwViewSwitcherTitle *self = ADW_VIEW_SWITCHER_TITLE (object);

  if (self->pages) {
    g_signal_handlers_disconnect_by_func (self->pages, G_CALLBACK (update_view_switcher_visible), self);
    g_clear_object (&self->pages);
  }

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_VIEW_SWITCHER_TITLE);

  G_OBJECT_CLASS (adw_view_switcher_title_parent_class)->dispose (object);
}

static void
check_window_width_cb (AdwViewSwitcherTitle *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  int width = gtk_widget_get_width (GTK_WIDGET (root));

  self->is_window_narrow = width <= 360;
  update_view_switcher_visible (self);

  self->check_window_width_id = 0;
}

static void
notify_surface_width_cb (AdwViewSwitcherTitle *self)
{
  if (self->check_window_width_id)
    return;

  self->check_window_width_id =
    g_idle_add_once ((GSourceOnceFunc) check_window_width_cb, self);
}

static void
adw_view_switcher_title_realize (GtkWidget *widget)
{
  AdwViewSwitcherTitle *self = ADW_VIEW_SWITCHER_TITLE (widget);
  GdkSurface *surface;

  GTK_WIDGET_CLASS (adw_view_switcher_title_parent_class)->realize (widget);

  surface = gtk_native_get_surface (gtk_widget_get_native (widget));

  g_signal_connect_swapped (surface, "notify::width", G_CALLBACK (notify_surface_width_cb), self);

  notify_surface_width_cb (self);
}

static void
adw_view_switcher_title_unrealize (GtkWidget *widget)
{
  AdwViewSwitcherTitle *self = ADW_VIEW_SWITCHER_TITLE (widget);
  GdkSurface *surface;

  surface = gtk_native_get_surface (gtk_widget_get_native (widget));

  g_signal_handlers_disconnect_by_func (surface, notify_surface_width_cb, self);

  g_clear_handle_id (&self->check_window_width_id, g_source_remove);

  GTK_WIDGET_CLASS (adw_view_switcher_title_parent_class)->unrealize (widget);
}

static void
adw_view_switcher_title_class_init (AdwViewSwitcherTitleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_view_switcher_title_dispose;
  object_class->get_property = adw_view_switcher_title_get_property;
  object_class->set_property = adw_view_switcher_title_set_property;

  widget_class->realize = adw_view_switcher_title_realize;
  widget_class->unrealize = adw_view_switcher_title_unrealize;

  /**
   * AdwViewSwitcherTitle:stack: (attributes org.gtk.Property.get=adw_view_switcher_title_get_stack org.gtk.Property.set=adw_view_switcher_title_set_stack)
   *
   * The stack the view switcher controls.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack", NULL, NULL,
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwViewSwitcherTitle:title: (attributes org.gtk.Property.get=adw_view_switcher_title_get_title org.gtk.Property.set=adw_view_switcher_title_set_title)
   *
   * The title to display.
   *
   * The title typically identifies the current view or content item, and
   * generally does not use the application name.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwViewSwitcherTitle:subtitle: (attributes org.gtk.Property.get=adw_view_switcher_title_get_subtitle org.gtk.Property.set=adw_view_switcher_title_set_subtitle)
   *
   * The subtitle to display.
   *
   * The subtitle should give the user additional details.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwViewSwitcherTitle:view-switcher-enabled: (attributes org.gtk.Property.get=adw_view_switcher_title_get_view_switcher_enabled org.gtk.Property.set=adw_view_switcher_title_set_view_switcher_enabled)
   *
   * Whether the view switcher is enabled.
   *
   * If it is disabled, the title will be displayed instead. This allows to
   * programmatically hide the view switcher even if it fits in the available
   * space.
   *
   * This can be used e.g. to ensure the view switcher is hidden below a certain
   * window width, or any other constraint you find suitable.
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
   */
  props[PROP_VIEW_SWITCHER_ENABLED] =
    g_param_spec_boolean ("view-switcher-enabled", NULL, NULL,
                         TRUE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_DEPRECATED);

  /**
   * AdwViewSwitcherTitle:title-visible: (attributes org.gtk.Property.get=adw_view_switcher_title_get_title_visible)
   *
   * Whether the title is currently visible.
   *
   * If the title is visible, it means the view switcher is hidden an it may be
   * wanted to show an alternative switcher, e.g. a [class@ViewSwitcherBar].
   *
   * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
   */
  props[PROP_TITLE_VISIBLE] =
    g_param_spec_boolean ("title-visible", NULL, NULL,
                         TRUE,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "viewswitchertitle");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-view-switcher-title.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherTitle, squeezer);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherTitle, title_widget);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherTitle, wide_view_switcher);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherTitle, narrow_view_switcher);
  gtk_widget_class_bind_template_callback (widget_class, notify_squeezer_visible_child_cb);
}

static void
adw_view_switcher_title_init (AdwViewSwitcherTitle *self)
{
  /* This must be initialized before the template so the embedded view switcher
   * can pick up the correct default value.
   */
  self->view_switcher_enabled = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  update_view_switcher_visible (self);
}

/**
 * adw_view_switcher_title_new:
 *
 * Creates a new `AdwViewSwitcherTitle`.
 *
 * Returns: the newly created `AdwViewSwitcherTitle`
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
GtkWidget *
adw_view_switcher_title_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_TITLE, NULL);
}

/**
 * adw_view_switcher_title_get_stack: (attributes org.gtk.Method.get_property=stack)
 * @self: a view switcher title
 *
 * Gets the stack controlled by @self.
 *
 * Returns: (nullable) (transfer none): the stack
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
AdwViewStack *
adw_view_switcher_title_get_stack (AdwViewSwitcherTitle *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return adw_view_switcher_get_stack (self->wide_view_switcher);
}

/**
 * adw_view_switcher_title_set_stack: (attributes org.gtk.Method.set_property=stack)
 * @self: a view switcher title
 * @stack: (nullable): a stack
 *
 * Sets the stack controlled by @self.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
void
adw_view_switcher_title_set_stack (AdwViewSwitcherTitle *self,
                                   AdwViewStack         *stack)
{
  AdwViewStack *previous_stack;

  g_return_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  previous_stack = adw_view_switcher_get_stack (self->wide_view_switcher);

  if (previous_stack == stack)
    return;

  if (previous_stack) {
    g_signal_handlers_disconnect_by_func (self->pages, G_CALLBACK (update_view_switcher_visible), self);
    g_clear_object (&self->pages);
  }

  adw_view_switcher_set_stack (self->wide_view_switcher, stack);
  adw_view_switcher_set_stack (self->narrow_view_switcher, stack);

  if (stack) {
    self->pages = adw_view_stack_get_pages (stack);

    g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (update_view_switcher_visible), self);
  }

  update_view_switcher_visible (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * adw_view_switcher_title_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a view switcher title
 *
 * Gets the title of @self.
 *
 * Returns: the title
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
const char *
adw_view_switcher_title_get_title (AdwViewSwitcherTitle *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return adw_window_title_get_title (self->title_widget);
}

/**
 * adw_view_switcher_title_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a view switcher title
 * @title: a title
 *
 * Sets the title of @self.
 *
 * The title typically identifies the current view or content item, and
 * generally does not use the application name.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
void
adw_view_switcher_title_set_title (AdwViewSwitcherTitle *self,
                                   const char           *title)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self));

  if (g_strcmp0 (adw_window_title_get_title (self->title_widget), title) == 0)
    return;

  adw_window_title_set_title (self->title_widget, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_view_switcher_title_get_subtitle: (attributes org.gtk.Method.get_property=subtitle)
 * @self: a view switcher title
 *
 * Gets the subtitle of @self.
 *
 * Returns: the subtitle
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
const char *
adw_view_switcher_title_get_subtitle (AdwViewSwitcherTitle *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return adw_window_title_get_subtitle (self->title_widget);
}

/**
 * adw_view_switcher_title_set_subtitle: (attributes org.gtk.Method.set_property=subtitle)
 * @self: a view switcher title
 * @subtitle: a subtitle
 *
 * Sets the subtitle of @self.
 *
 * The subtitle should give the user additional details.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
void
adw_view_switcher_title_set_subtitle (AdwViewSwitcherTitle *self,
                                      const char           *subtitle)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self));

  if (g_strcmp0 (adw_window_title_get_subtitle (self->title_widget), subtitle) == 0)
    return;

  adw_window_title_set_subtitle (self->title_widget, subtitle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * adw_view_switcher_title_get_view_switcher_enabled: (attributes org.gtk.Method.get_property=view-switcher-enabled)
 * @self: a view switcher title
 *
 * Gets whether @self's view switcher is enabled.
 *
 * Returns: whether the view switcher is enabled
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
gboolean
adw_view_switcher_title_get_view_switcher_enabled (AdwViewSwitcherTitle *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self), FALSE);

  return self->view_switcher_enabled;
}

/**
 * adw_view_switcher_title_set_view_switcher_enabled: (attributes org.gtk.Method.set_property=view-switcher-enabled)
 * @self: a view switcher title
 * @enabled: whether the view switcher is enabled
 *
 * Sets whether @self's view switcher is enabled.
 *
 * If it is disabled, the title will be displayed instead. This allows to
 * programmatically hide the view switcher even if it fits in the available
 * space.
 *
 * This can be used e.g. to ensure the view switcher is hidden below a certain
 * window width, or any other constraint you find suitable.
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
void
adw_view_switcher_title_set_view_switcher_enabled (AdwViewSwitcherTitle *self,
                                                   gboolean              enabled)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self));

  enabled = !!enabled;

  if (self->view_switcher_enabled == enabled)
    return;

  self->view_switcher_enabled = enabled;
  update_view_switcher_visible (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW_SWITCHER_ENABLED]);
}

/**
 * adw_view_switcher_title_get_title_visible: (attributes org.gtk.Method.get_property=title-visible)
 * @self: a view switcher title
 *
 * Gets whether the title of @self is currently visible.
 *
 * If the title is visible, it means the view switcher is hidden an it may be
 * wanted to show an alternative switcher, e.g. a [class@ViewSwitcherBar].
 *
 * Returns: whether the title of @self is currently visible
 *
 * Deprecated: 1.4: See [the migration guide](migrating-to-breakpoints.html#replace-adwviewswitchertitle)
 */
gboolean
adw_view_switcher_title_get_title_visible (AdwViewSwitcherTitle *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_TITLE (self), FALSE);

  return adw_squeezer_get_visible_child (self->squeezer) == GTK_WIDGET (self->title_widget);
}
