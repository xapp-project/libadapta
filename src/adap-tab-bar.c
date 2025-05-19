/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-tab-bar-private.h"

#include "adw-bin.h"
#include "adw-tab-box-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwTabBar:
 *
 * A tab bar for [class@TabView].
 *
 * <picture>
 *   <source srcset="tab-bar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="tab-bar.png" alt="tab-bar">
 * </picture>
 *
 * The `AdwTabBar` widget is a tab bar that can be used with conjunction with
 * `AdwTabView`. It is typically used as a top bar within [class@ToolbarView].
 *
 * `AdwTabBar` can autohide and can optionally contain action widgets on both
 * sides of the tabs.
 *
 * When there's not enough space to show all the tabs, `AdwTabBar` will scroll
 * them. Pinned tabs always stay visible and aren't a part of the scrollable
 * area.
 *
 * ## CSS nodes
 *
 * `AdwTabBar` has a single CSS node with name `tabbar`.
 */

struct _AdwTabBar
{
  GtkWidget parent_instance;

  GtkRevealer *revealer;
  AdwBin *start_action_bin;
  AdwBin *end_action_bin;

  AdwTabBox *box;
  GtkScrolledWindow *scrolled_window;

  AdwTabBox *pinned_box;
  GtkScrolledWindow *pinned_scrolled_window;

  AdwTabView *view;
  gboolean autohide;

  GdkDragAction extra_drag_preferred_action;

  gboolean is_overflowing;
  gboolean resize_frozen;
};

static void adw_tab_bar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwTabBar, adw_tab_bar, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_tab_bar_buildable_init))

enum {
  PROP_0,
  PROP_VIEW,
  PROP_START_ACTION_WIDGET,
  PROP_END_ACTION_WIDGET,
  PROP_AUTOHIDE,
  PROP_TABS_REVEALED,
  PROP_EXPAND_TABS,
  PROP_INVERTED,
  PROP_IS_OVERFLOWING,
  PROP_EXTRA_DRAG_PRELOAD,
  PROP_EXTRA_DRAG_PREFERRED_ACTION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_EXTRA_DRAG_VALUE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
set_tabs_revealed (AdwTabBar *self,
                   gboolean   tabs_revealed)
{
  if (tabs_revealed == adw_tab_bar_get_tabs_revealed (self))
    return;

  gtk_revealer_set_reveal_child (self->revealer, tabs_revealed);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TABS_REVEALED]);
}

static void
update_autohide_cb (AdwTabBar *self)
{
  int n_tabs = 0, n_pinned_tabs = 0;
  gboolean is_transferring_page;

  if (!self->view) {
    set_tabs_revealed (self, FALSE);

    return;
  }

  if (!self->autohide) {
    set_tabs_revealed (self, TRUE);

    return;
  }

  n_tabs = adw_tab_view_get_n_pages (self->view);
  n_pinned_tabs = adw_tab_view_get_n_pinned_pages (self->view);
  is_transferring_page = adw_tab_view_get_is_transferring_page (self->view);

  set_tabs_revealed (self, n_tabs > 1 || n_pinned_tabs >= 1 || is_transferring_page);
}

static void
notify_selected_page_cb (AdwTabBar *self)
{
  AdwTabPage *page = adw_tab_view_get_selected_page (self->view);

  if (!page)
    return;

  if (adw_tab_page_get_pinned (page)) {
    adw_tab_box_select_page (self->pinned_box, page);
    adw_tab_box_select_page (self->box, page);
  } else {
    adw_tab_box_select_page (self->box, page);
    adw_tab_box_select_page (self->pinned_box, page);
  }
}

static void
notify_pinned_cb (AdwTabPage *page,
                  GParamSpec *pspec,
                  AdwTabBar  *self)
{
  AdwTabBox *from, *to;
  gboolean should_focus;

  if (adw_tab_page_get_pinned (page)) {
    from = self->box;
    to = self->pinned_box;
  } else {
    from = self->pinned_box;
    to = self->box;
  }

  should_focus = adw_tab_box_is_page_focused (from, page);

  adw_tab_box_detach_page (from, page);
  adw_tab_box_attach_page (to, page, adw_tab_view_get_n_pinned_pages (self->view));

  if (should_focus)
    adw_tab_box_try_focus_selected_tab (to);
}

static void
page_attached_cb (AdwTabBar  *self,
                  AdwTabPage *page,
                  int         position)
{
  g_signal_connect_object (page, "notify::pinned",
                           G_CALLBACK (notify_pinned_cb), self,
                           0);
}

static void
page_detached_cb (AdwTabBar  *self,
                  AdwTabPage *page,
                  int         position)
{
  g_signal_handlers_disconnect_by_func (page, notify_pinned_cb, self);
}

static inline gboolean
is_overflowing (GtkAdjustment *adj)
{
  double lower, upper, page_size;

  lower = gtk_adjustment_get_lower (adj);
  upper = gtk_adjustment_get_upper (adj);
  page_size = gtk_adjustment_get_page_size (adj);
  return upper - lower > page_size;
}

static void
update_is_overflowing (AdwTabBar *self)
{
  GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment (self->scrolled_window);
  GtkAdjustment *pinned_adj = gtk_scrolled_window_get_hadjustment (self->pinned_scrolled_window);
  gboolean overflowing = is_overflowing (adj) || is_overflowing (pinned_adj);

  if (overflowing == self->is_overflowing)
    return;

  overflowing |= self->resize_frozen;

  if (overflowing == self->is_overflowing)
    return;

  self->is_overflowing = overflowing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_IS_OVERFLOWING]);
}

static void
notify_resize_frozen_cb (AdwTabBar *self)
{
  gboolean frozen, pinned_frozen;

  g_object_get (self->box, "resize-frozen", &frozen, NULL);
  g_object_get (self->pinned_box, "resize-frozen", &pinned_frozen, NULL);

  self->resize_frozen = frozen || pinned_frozen;

  update_is_overflowing (self);
}

static void
stop_kinetic_scrolling_cb (GtkScrolledWindow *scrolled_window)
{
  /* HACK: Need to cancel kinetic scrolling. If only the built-in adjustment
   * animation API was public, we wouldn't have to do any of this... */
  gtk_scrolled_window_set_kinetic_scrolling (scrolled_window, FALSE);
  gtk_scrolled_window_set_kinetic_scrolling (scrolled_window, TRUE);
}

static void
set_extra_drag_preferred_action (AdwTabBar    *self,
                                 GdkDragAction preferred_action)
{
  self->extra_drag_preferred_action = preferred_action;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PREFERRED_ACTION]);
}

static gboolean
extra_drag_drop_cb (AdwTabBar    *self,
                    AdwTabPage   *page,
                    GValue       *value,
                    GdkDragAction preferred_action)
{
  gboolean ret = GDK_EVENT_PROPAGATE;

  set_extra_drag_preferred_action (self, preferred_action);
  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, page, value, &ret);
  set_extra_drag_preferred_action (self, 0);

  return ret;
}

static GdkDragAction
extra_drag_value_cb (AdwTabBar  *self,
                     AdwTabPage *page,
                     GValue     *value)
{
  GdkDragAction preferred_action;

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, page, value, &preferred_action);

  return preferred_action;
}

static GdkDragAction
extra_drag_value_notify (AdwTabBar *self,
                         GValue    *value)
{
  return GDK_ACTION_ALL;
}

static void
view_destroy_cb (AdwTabBar *self)
{
  adw_tab_bar_set_view (self, NULL);
}

static gboolean
adw_tab_bar_focus (GtkWidget        *widget,
                   GtkDirectionType  direction)
{
  AdwTabBar *self = ADW_TAB_BAR (widget);
  gboolean is_rtl;
  GtkDirectionType start, end;

  if (!adw_tab_bar_get_tabs_revealed (self))
    return GDK_EVENT_PROPAGATE;

  if (!gtk_widget_get_focus_child (widget))
    return gtk_widget_child_focus (GTK_WIDGET (self->pinned_box), direction) ||
           gtk_widget_child_focus (GTK_WIDGET (self->box), direction);

  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  start = is_rtl ? GTK_DIR_RIGHT : GTK_DIR_LEFT;
  end = is_rtl ? GTK_DIR_LEFT : GTK_DIR_RIGHT;

  if (direction == start) {
    if (adw_tab_view_select_previous_page (self->view))
      return GDK_EVENT_STOP;

    return gtk_widget_keynav_failed (widget, direction);
  }

  if (direction == end) {
    if (adw_tab_view_select_next_page (self->view))
      return GDK_EVENT_STOP;

    return gtk_widget_keynav_failed (widget, direction);
  }

  return GDK_EVENT_PROPAGATE;
}

static void
adw_tab_bar_dispose (GObject *object)
{
  AdwTabBar *self = ADW_TAB_BAR (object);

  adw_tab_bar_set_view (self, NULL);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_TAB_BAR);

  G_OBJECT_CLASS (adw_tab_bar_parent_class)->dispose (object);
}

static void
adw_tab_bar_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AdwTabBar *self = ADW_TAB_BAR (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, adw_tab_bar_get_view (self));
    break;

  case PROP_START_ACTION_WIDGET:
    g_value_set_object (value, adw_tab_bar_get_start_action_widget (self));
    break;

  case PROP_END_ACTION_WIDGET:
    g_value_set_object (value, adw_tab_bar_get_end_action_widget (self));
    break;

  case PROP_AUTOHIDE:
    g_value_set_boolean (value, adw_tab_bar_get_autohide (self));
    break;

  case PROP_TABS_REVEALED:
    g_value_set_boolean (value, adw_tab_bar_get_tabs_revealed (self));
    break;

  case PROP_EXPAND_TABS:
    g_value_set_boolean (value, adw_tab_bar_get_expand_tabs (self));
    break;

  case PROP_INVERTED:
    g_value_set_boolean (value, adw_tab_bar_get_inverted (self));
    break;

  case PROP_IS_OVERFLOWING:
    g_value_set_boolean (value, adw_tab_bar_get_is_overflowing (self));
    break;

  case PROP_EXTRA_DRAG_PREFERRED_ACTION:
    g_value_set_flags (value, adw_tab_bar_get_extra_drag_preferred_action (self));
    break;

  case PROP_EXTRA_DRAG_PRELOAD:
    g_value_set_boolean (value, adw_tab_bar_get_extra_drag_preload (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_bar_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AdwTabBar *self = ADW_TAB_BAR (object);

  switch (prop_id) {
  case PROP_VIEW:
    adw_tab_bar_set_view (self, g_value_get_object (value));
    break;

  case PROP_START_ACTION_WIDGET:
    adw_tab_bar_set_start_action_widget (self, g_value_get_object (value));
    break;

  case PROP_END_ACTION_WIDGET:
    adw_tab_bar_set_end_action_widget (self, g_value_get_object (value));
    break;

  case PROP_AUTOHIDE:
    adw_tab_bar_set_autohide (self, g_value_get_boolean (value));
    break;

  case PROP_EXPAND_TABS:
    adw_tab_bar_set_expand_tabs (self, g_value_get_boolean (value));
    break;

  case PROP_INVERTED:
    adw_tab_bar_set_inverted (self, g_value_get_boolean (value));
    break;

  case PROP_EXTRA_DRAG_PRELOAD:
    adw_tab_bar_set_extra_drag_preload (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_bar_class_init (AdwTabBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_bar_dispose;
  object_class->get_property = adw_tab_bar_get_property;
  object_class->set_property = adw_tab_bar_set_property;

  widget_class->focus = adw_tab_bar_focus;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwTabBar:view: (attributes org.gtk.Property.get=adw_tab_bar_get_view org.gtk.Property.set=adw_tab_bar_set_view)
   *
   * The tab view the tab bar controls.
   */
  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         ADW_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:start-action-widget: (attributes org.gtk.Property.get=adw_tab_bar_get_start_action_widget org.gtk.Property.set=adw_tab_bar_set_start_action_widget)
   *
   * The widget shown before the tabs.
   */
  props[PROP_START_ACTION_WIDGET] =
    g_param_spec_object ("start-action-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:end-action-widget: (attributes org.gtk.Property.get=adw_tab_bar_get_end_action_widget org.gtk.Property.set=adw_tab_bar_set_end_action_widget)
   *
   * The widget shown after the tabs.
   */
  props[PROP_END_ACTION_WIDGET] =
    g_param_spec_object ("end-action-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:autohide: (attributes org.gtk.Property.get=adw_tab_bar_get_autohide org.gtk.Property.set=adw_tab_bar_set_autohide)
   *
   * Whether the tabs automatically hide.
   *
   * If set to `TRUE`, the tab bar disappears when [property@TabBar:view] has 0
   * or 1 tab, no pinned tabs, and no tab is being transferred.
   *
   * See [property@TabBar:tabs-revealed].
   */
  props[PROP_AUTOHIDE] =
    g_param_spec_boolean ("autohide", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:tabs-revealed: (attributes org.gtk.Property.get=adw_tab_bar_get_tabs_revealed)
   *
   * Whether the tabs are currently revealed.
   *
   * See [property@TabBar:autohide].
   */
  props[PROP_TABS_REVEALED] =
    g_param_spec_boolean ("tabs-revealed", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabBar:expand-tabs: (attributes org.gtk.Property.get=adw_tab_bar_get_expand_tabs org.gtk.Property.set=adw_tab_bar_set_expand_tabs)
   *
   * Whether tabs expand to full width.
   *
   * If set to `TRUE`, the tabs will always vary width filling the whole width
   * when possible, otherwise tabs will always have the minimum possible size.
   */
  props[PROP_EXPAND_TABS] =
    g_param_spec_boolean ("expand-tabs", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:inverted: (attributes org.gtk.Property.get=adw_tab_bar_get_inverted org.gtk.Property.set=adw_tab_bar_set_inverted)
   *
   * Whether tabs use inverted layout.
   *
   * If set to `TRUE`, non-pinned tabs will have the close button at the
   * beginning and the indicator at the end rather than the opposite.
   */
  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:is-overflowing: (attributes org.gtk.Property.get=adw_tab_bar_get_is_overflowing)
   *
   * Whether the tab bar is overflowing.
   *
   * If `TRUE`, all tabs cannot be displayed at once and require scrolling.
   */
  props[PROP_IS_OVERFLOWING] =
    g_param_spec_boolean ("is-overflowing", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabBar:extra-drag-preferred-action: (attributes org.gtk.Property.get=adw_tab_bar_get_extra_drag_preferred_action)
   *
   * The unique action on the `current-drop` of the
   * [signal@TabBar::extra-drag-drop].
   *
   * This property should only be used during a [signal@TabBar::extra-drag-drop]
   * and is always a subset of what was originally passed to
   * [method@TabBar.setup_extra_drop_target].
   *
   * Since: 1.4
   */
  props[PROP_EXTRA_DRAG_PREFERRED_ACTION] =
    g_param_spec_flags ("extra-drag-preferred-action", NULL, NULL,
                       GDK_TYPE_DRAG_ACTION, 0,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabBar:extra-drag-preload: (attributes org.gtk.Property.get=adw_tab_bar_get_extra_drag_preload org.gtk.Property.set=adw_tab_bar_set_extra_drag_preload)
   *
   * Whether the drop data should be preloaded on hover.
   *
   * See [property@Gtk.DropTarget:preload].
   *
   * Since: 1.3
   */
  props[PROP_EXTRA_DRAG_PRELOAD] =
    g_param_spec_boolean ("extra-drag-preload", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwTabBar::extra-drag-drop:
   * @self: a tab bar
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when content is dropped onto a tab.
   *
   * The content must be of one of the types set up via
   * [method@TabBar.setup_extra_drop_target].
   *
   * See [signal@Gtk.DropTarget::drop].
   *
   * Returns: whether the drop was accepted for @page
   */
  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  G_TYPE_BOOLEAN,
                  2,
                  ADW_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  /**
   * AdwTabBar::extra-drag-value:
   * @self: a tab bar
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when the dropped content is preloaded.
   *
   * In order for data to be preloaded, [property@TabBar:extra-drag-preload]
   * must be set to `TRUE`.
   *
   * The content must be of one of the types set up via
   * [method@TabBar.setup_extra_drop_target].
   *
   * See [property@Gtk.DropTarget:value].
   *
   * Returns: the preferred action for the drop on @page
   *
   * Since: 1.3
   */
  signals[SIGNAL_EXTRA_DRAG_VALUE] =
    g_signal_new ("extra-drag-value",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  GDK_TYPE_DRAG_ACTION,
                  2,
                  ADW_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-tab-bar.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, revealer);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, pinned_box);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, box);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, pinned_scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, start_action_bin);
  gtk_widget_class_bind_template_child (widget_class, AdwTabBar, end_action_bin);
  gtk_widget_class_bind_template_callback (widget_class, notify_resize_frozen_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_kinetic_scrolling_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_drop_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_value_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "tabbar");

  g_signal_override_class_handler ("extra-drag-value", G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (extra_drag_value_notify));

  g_type_ensure (ADW_TYPE_TAB_BOX);
}

static void
adw_tab_bar_init (AdwTabBar *self)
{
  GtkAdjustment *adj;

  self->autohide = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  adj = gtk_scrolled_window_get_hadjustment (self->scrolled_window);
  g_signal_connect_object (adj, "changed", G_CALLBACK (update_is_overflowing),
                           self, G_CONNECT_SWAPPED);

  adj = gtk_scrolled_window_get_hadjustment (self->pinned_scrolled_window);
  g_signal_connect_object (adj, "changed", G_CALLBACK (update_is_overflowing),
                           self, G_CONNECT_SWAPPED);
}

static void
adw_tab_bar_buildable_add_child (GtkBuildable *buildable,
                                 GtkBuilder   *builder,
                                 GObject      *child,
                                 const char   *type)
{
  AdwTabBar *self = ADW_TAB_BAR (buildable);

  if (!self->revealer) {
    gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (self));

    return;
  }

  if (!type || !g_strcmp0 (type, "start"))
    adw_tab_bar_set_start_action_widget (self, GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "end"))
    adw_tab_bar_set_end_action_widget (self, GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (ADW_TAB_BAR (self), type);
}

static void
adw_tab_bar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_tab_bar_buildable_add_child;
}

gboolean
adw_tab_bar_tabs_have_visible_focus (AdwTabBar *self)
{
  GtkWidget *pinned_focus_child, *scroll_focus_child;

  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  pinned_focus_child = gtk_widget_get_focus_child (GTK_WIDGET (self->pinned_box));

  if (pinned_focus_child) {
    GtkWidget *tab = gtk_widget_get_first_child (pinned_focus_child);

    if (gtk_widget_has_visible_focus (tab))
      return TRUE;
  }

  scroll_focus_child = gtk_widget_get_focus_child (GTK_WIDGET (self->box));

  if (scroll_focus_child) {
    GtkWidget *tab = gtk_widget_get_first_child (scroll_focus_child);

    if (gtk_widget_has_visible_focus (tab))
      return TRUE;
  }

  return FALSE;
}

/**
 * adw_tab_bar_new:
 *
 * Creates a new `AdwTabBar`.
 *
 * Returns: the newly created `AdwTabBar`
 */
AdwTabBar *
adw_tab_bar_new (void)
{
  return g_object_new (ADW_TYPE_TAB_BAR, NULL);
}

/**
 * adw_tab_bar_get_view: (attributes org.gtk.Method.get_property=view)
 * @self: a tab bar
 *
 * Gets the tab view @self controls.
 *
 * Returns: (transfer none) (nullable): the view @self controls
 */
AdwTabView *
adw_tab_bar_get_view (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), NULL);

  return self->view;
}

/**
 * adw_tab_bar_set_view: (attributes org.gtk.Method.set_property=view)
 * @self: a tab bar
 * @view: (nullable): a tab view
 *
 * Sets the tab view @self controls.
 */
void
adw_tab_bar_set_view (AdwTabBar  *self,
                      AdwTabView *view)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));
  g_return_if_fail (view == NULL || ADW_IS_TAB_VIEW (view));

  if (self->view == view)
    return;

  if (self->view) {
    int i, n;

    g_signal_handlers_disconnect_by_func (self->view, update_autohide_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, notify_selected_page_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_attached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_detached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, view_destroy_cb, self);

    n = adw_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_detached_cb (self, adw_tab_view_get_nth_page (self->view, i), i);

    adw_tab_box_set_view (self->pinned_box, NULL);
    adw_tab_box_set_view (self->box, NULL);
  }

  g_set_object (&self->view, view);

  if (self->view) {
    int i, n;

    adw_tab_box_set_view (self->pinned_box, view);
    adw_tab_box_set_view (self->box, view);

    g_signal_connect_object (self->view, "notify::is-transferring-page",
                             G_CALLBACK (update_autohide_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::n-pages",
                             G_CALLBACK (update_autohide_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::n-pinned-pages",
                             G_CALLBACK (update_autohide_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::selected-page",
                             G_CALLBACK (notify_selected_page_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-attached",
                             G_CALLBACK (page_attached_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-detached",
                             G_CALLBACK (page_detached_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "destroy",
                             G_CALLBACK (view_destroy_cb), self,
                             G_CONNECT_SWAPPED);

    n = adw_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_attached_cb (self, adw_tab_view_get_nth_page (self->view, i), i);
  }

  update_autohide_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}

/**
 * adw_tab_bar_get_start_action_widget: (attributes org.gtk.Method.get_property=start-action-widget)
 * @self: a tab bar
 *
 * Gets the widget shown before the tabs.
 *
 * Returns: (transfer none) (nullable): the widget shown before the tabs
 */
GtkWidget *
adw_tab_bar_get_start_action_widget (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), NULL);

  return self->start_action_bin ? adw_bin_get_child (self->start_action_bin) : NULL;
}

/**
 * adw_tab_bar_set_start_action_widget: (attributes org.gtk.Method.set_property=start-action-widget)
 * @self: a tab bar
 * @widget: (transfer none) (nullable): the widget to show before the tabs
 *
 * Sets the widget to show before the tabs.
 */
void
adw_tab_bar_set_start_action_widget (AdwTabBar *self,
                                     GtkWidget *widget)
{
  GtkWidget *old_widget;

  g_return_if_fail (ADW_IS_TAB_BAR (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  old_widget = adw_bin_get_child (self->start_action_bin);

  if (old_widget == widget)
    return;

  adw_bin_set_child (self->start_action_bin, widget);
  gtk_widget_set_visible (GTK_WIDGET (self->start_action_bin), widget != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_START_ACTION_WIDGET]);
}

/**
 * adw_tab_bar_get_end_action_widget: (attributes org.gtk.Method.get_property=end-action-widget)
 * @self: a tab bar
 *
 * Gets the widget shown after the tabs.
 *
 * Returns: (transfer none) (nullable): the widget shown after the tabs
 */
GtkWidget *
adw_tab_bar_get_end_action_widget (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), NULL);

  return self->end_action_bin ? adw_bin_get_child (self->end_action_bin) : NULL;
}

/**
 * adw_tab_bar_set_end_action_widget: (attributes org.gtk.Method.set_property=end-action-widget)
 * @self: a tab bar
 * @widget: (transfer none) (nullable): the widget to show after the tabs
 *
 * Sets the widget to show after the tabs.
 */
void
adw_tab_bar_set_end_action_widget (AdwTabBar *self,
                                   GtkWidget *widget)
{
  GtkWidget *old_widget;

  g_return_if_fail (ADW_IS_TAB_BAR (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  old_widget = adw_bin_get_child (self->end_action_bin);

  if (old_widget == widget)
    return;

  adw_bin_set_child (self->end_action_bin, widget);
  gtk_widget_set_visible (GTK_WIDGET (self->end_action_bin), widget != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_END_ACTION_WIDGET]);
}

/**
 * adw_tab_bar_get_autohide: (attributes org.gtk.Method.get_property=autohide)
 * @self: a tab bar
 *
 * Gets whether the tabs automatically hide.
 *
 * Returns: whether the tabs automatically hide
 */
gboolean
adw_tab_bar_get_autohide (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return self->autohide;
}

/**
 * adw_tab_bar_set_autohide: (attributes org.gtk.Method.set_property=autohide)
 * @self: a tab bar
 * @autohide: whether the tabs automatically hide
 *
 * Sets whether the tabs automatically hide.
 *
 * If set to `TRUE`, the tab bar disappears when [property@TabBar:view] has 0
 * or 1 tab, no pinned tabs, and no tab is being transferred.
 *
 * See [property@TabBar:tabs-revealed].
 */
void
adw_tab_bar_set_autohide (AdwTabBar *self,
                          gboolean   autohide)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));

  autohide = !!autohide;

  if (autohide == self->autohide)
    return;

  self->autohide = autohide;

  update_autohide_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_AUTOHIDE]);
}

/**
 * adw_tab_bar_get_tabs_revealed: (attributes org.gtk.Method.get_property=tabs-revealed)
 * @self: a tab bar
 *
 * Gets whether the tabs are currently revealed.
 *
 * See [property@TabBar:autohide].
 *
 * Returns: whether the tabs are currently revealed
 */
gboolean
adw_tab_bar_get_tabs_revealed (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return gtk_revealer_get_reveal_child (self->revealer);
}

/**
 * adw_tab_bar_get_expand_tabs: (attributes org.gtk.Method.get_property=expand-tabs)
 * @self: a tab bar
 *
 * Gets whether tabs expand to full width.
 *
 * Returns: whether tabs expand to full width.
 */
gboolean
adw_tab_bar_get_expand_tabs (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return adw_tab_box_get_expand_tabs (self->box);
}

/**
 * adw_tab_bar_set_expand_tabs: (attributes org.gtk.Method.set_property=expand-tabs)
 * @self: a tab bar
 * @expand_tabs: whether to expand tabs
 *
 * Sets whether tabs expand to full width.
 *
 * If set to `TRUE`, the tabs will always vary width filling the whole width
 * when possible, otherwise tabs will always have the minimum possible size.
 */
void
adw_tab_bar_set_expand_tabs (AdwTabBar *self,
                             gboolean   expand_tabs)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));

  expand_tabs = !!expand_tabs;

  if (adw_tab_bar_get_expand_tabs (self) == expand_tabs)
    return;

  adw_tab_box_set_expand_tabs (self->box, expand_tabs);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPAND_TABS]);
}

/**
 * adw_tab_bar_get_inverted: (attributes org.gtk.Method.get_property=inverted)
 * @self: a tab bar
 *
 * Gets whether tabs use inverted layout.
 *
 * Returns: whether tabs use inverted layout
 */
gboolean
adw_tab_bar_get_inverted (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return adw_tab_box_get_inverted (self->box);
}

/**
 * adw_tab_bar_set_inverted: (attributes org.gtk.Method.set_property=inverted)
 * @self: a tab bar
 * @inverted: whether tabs use inverted layout
 *
 * Sets whether tabs tabs use inverted layout.
 *
 * If set to `TRUE`, non-pinned tabs will have the close button at the beginning
 * and the indicator at the end rather than the opposite.
 */
void
adw_tab_bar_set_inverted (AdwTabBar *self,
                          gboolean   inverted)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));

  inverted = !!inverted;

  if (adw_tab_bar_get_inverted (self) == inverted)
    return;

  adw_tab_box_set_inverted (self->box, inverted);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

/**
 * adw_tab_bar_setup_extra_drop_target:
 * @self: a tab bar
 * @actions: the supported actions
 * @types: (nullable) (transfer none) (array length=n_types):
 *   all supported `GType`s that can be dropped
 * @n_types: number of @types
 *
 * Sets the supported types for this drop target.
 *
 * Sets up an extra drop target on tabs.
 *
 * This allows to drag arbitrary content onto tabs, for example URLs in a web
 * browser.
 *
 * If a tab is hovered for a certain period of time while dragging the content,
 * it will be automatically selected.
 *
 * The [signal@TabBar::extra-drag-drop] signal can be used to handle the drop.
 */
void
adw_tab_bar_setup_extra_drop_target (AdwTabBar     *self,
                                     GdkDragAction  actions,
                                     GType         *types,
                                     gsize          n_types)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  adw_tab_box_setup_extra_drop_target (self->box, actions, types, n_types);
  adw_tab_box_setup_extra_drop_target (self->pinned_box, actions, types, n_types);
}

/**
 * adw_tab_bar_get_extra_drag_preferred_action:
 * @self: a tab bar
 *
 * Gets the current action during a drop on the extra_drop_target.
 *
 * Returns: the drag action of the current drop.
 *
 * Since: 1.4
 */
GdkDragAction
adw_tab_bar_get_extra_drag_preferred_action (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), 0);

  return self->extra_drag_preferred_action;
}

/**
 * adw_tab_bar_get_extra_drag_preload:
 * @self: a tab bar
 *
 * Gets whether drop data should be preloaded on hover.
 *
 * Returns: whether drop data should be preloaded on hover
 *
 * Since: 1.3
 */
gboolean
adw_tab_bar_get_extra_drag_preload (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return adw_tab_box_get_extra_drag_preload (self->box);
}

/**
 * adw_tab_bar_set_extra_drag_preload:
 * @self: a tab bar
 * @preload: whether to preload drop data
 *
 * Sets whether drop data should be preloaded on hover.
 *
 * See [property@Gtk.DropTarget:preload].
 *
 * Since: 1.3
 */
void
adw_tab_bar_set_extra_drag_preload (AdwTabBar *self,
                                    gboolean   preload)
{
  g_return_if_fail (ADW_IS_TAB_BAR (self));

  if (adw_tab_bar_get_extra_drag_preload (self) == preload)
    return;

  adw_tab_box_set_extra_drag_preload (self->box, preload);
  adw_tab_box_set_extra_drag_preload (self->pinned_box, preload);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PRELOAD]);
}

/**
 * adw_tab_bar_get_is_overflowing: (attributes org.gtk.Method.get_property=is-overflowing)
 * @self: a tab bar
 *
 * Gets whether @self is overflowing.
 *
 * If `TRUE`, all tabs cannot be displayed at once and require scrolling.
 *
 * Returns: whether @self is overflowing
 */
gboolean
adw_tab_bar_get_is_overflowing (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), FALSE);

  return self->is_overflowing;
}

AdwTabBox *
adw_tab_bar_get_tab_box (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), NULL);

  return self->box;
}

AdwTabBox *
adw_tab_bar_get_pinned_tab_box (AdwTabBar *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BAR (self), NULL);

  return self->pinned_box;
}
