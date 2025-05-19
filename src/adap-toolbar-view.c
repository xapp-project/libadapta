/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-toolbar-view.h"

#include "adw-widget-utils-private.h"

/**
 * AdwToolbarView:
 *
 * A widget containing a page, as well as top and/or bottom bars.
 *
 * <picture>
 *   <source srcset="toolbar-view-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view.png" alt="toolbar-view">
 * </picture>
 *
 * `AdwToolbarView` has a single content widget and one or multiple top and
 * bottom bars, shown at the top and bottom sides respectively.
 *
 * Example of an `AdwToolbarView` UI definition:
 * ```xml
 * <object class="AdwToolbarView">
 *   <child type="top">
 *     <object class="AdwHeaderBar"/>
 *   </child>
 *   <property name="content">
 *     <object class="AdwPreferencesPage">
 *       <!-- ... -->
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * The following kinds of top and bottom bars are supported:
 *
 * - [class@HeaderBar]
 * - [class@TabBar]
 * - [class@ViewSwitcherBar]
 * - [class@Gtk.ActionBar]
 * - [class@Gtk.HeaderBar]
 * - [class@Gtk.PopoverMenuBar]
 * - [class@Gtk.SearchBar]
 * - Any [class@Gtk.Box] or a similar widget with the
 *   [`.toolbar`](style-classes.html#toolbars) style class
 *
 * By default, top and bottom bars are flat and scrolling content has a subtle
 * undershoot shadow, same as when using the
 * [`.undershoot-top`](style-classes.html#undershoot-indicators) and
 * [`.undershoot-bottom`](style-classes.html#undershoot-indicators) style
 * classes. This works well in most cases, e.g. with [class@StatusPage] or
 * [class@PreferencesPage], where the background at the top and bottom parts of
 * the page is uniform. Additionally, windows with sidebars should always use
 * this style.
 *
 * [property@ToolbarView:top-bar-style] and
 * [property@ToolbarView:bottom-bar-style] properties can be used add an opaque
 * background and a persistent shadow to top and bottom bars, this can be useful
 * for content such as [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
 * where some elements are adjacent to the top/bottom bars, or [class@TabView],
 * where each page can have a different background.
 *
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-flat-1-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-flat-1.png" alt="toolbar-view-flat-1">
 * </picture>
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-flat-2-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-flat-2.png" alt="toolbar-view-flat-2">
 * </picture>
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-raised-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-raised.png" alt="toolbar-view-raised">
 * </picture>
 *
 * `AdwToolbarView` ensures the top and bottom bars have consistent backdrop
 * styles and vertical spacing. For comparison:
 *
 * <picture style="min-width: 40%; display: inline-block;">
 *   <source srcset="toolbar-view-spacing-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-spacing.png" alt="toolbar-view-spacing">
 * </picture>
 * <picture style="min-width: 40%; display: inline-block;">
 *   <source srcset="toolbar-view-spacing-box-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-spacing-box.png" alt="toolbar-view-spacing-box">
 * </picture>
 *
 * Any top and bottom bars can also be dragged to move the window, equivalent
 * to putting them into a [class@Gtk.WindowHandle].
 *
 * Content is typically place between top and bottom bars, but can also extend
 * behind them. This is controlled with the
 * [property@ToolbarView:extend-content-to-top-edge] and
 * [property@ToolbarView:extend-content-to-bottom-edge] properties.
 *
 * Top and bottom bars can be hidden and revealed with an animation using the
 * [property@ToolbarView:reveal-top-bars] and
 * [property@ToolbarView:reveal-bottom-bars] properties.
 *
 * ## `AdwToolbarView` as `GtkBuildable`
 *
 * The `AdwToolbarView` implementation of the [iface@Gtk.Buildable] interface
 * supports adding a top bar by specifying “top” as the “type” attribute of a
 * `<child>` element, or adding a bottom bar by specifying “bottom”.
 *
 * ## Accessibility
 *
 * `AdwToolbarView` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

/**
 * AdwToolbarStyle:
 * @ADW_TOOLBAR_FLAT: No background, shadow only for scrolled content
 * @ADW_TOOLBAR_RAISED: Opaque background with a persistent shadow
 * @ADW_TOOLBAR_RAISED_BORDER: Opaque background with a persistent border
 *
 * Describes the possible top or bottom bar styles in an [class@ToolbarView]
 * widget.
 *
 * `ADW_TOOLBAR_FLAT` is suitable for simple content, such as
 * [class@StatusPage] or [class@PreferencesPage], where the background at the
 * top and bottom parts of the page is uniform. Additionally, windows with
 * sidebars should always use this style.
 *
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-flat-1-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-flat-1.png" alt="toolbar-view-flat-1">
 * </picture>
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-flat-2-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-flat-2.png" alt="toolbar-view-flat-2">
 * </picture>
 *
 * `ADW_TOOLBAR_RAISED` style is suitable for content such as
 * [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
 * where some elements are directly adjacent to the top/bottom bars, or
 * [class@TabView], where each page can have a different background.
 *
 * `ADW_TOOLBAR_RAISED_BORDER` style is similar to `ADW_TOOLBAR_RAISED`, but
 * with the shadow replaced with a more subtle border. It's intended to be used
 * in applications like image viewers, where a shadow over the content might be
 * undesired.
 *
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-raised-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-raised.png" alt="toolbar-view-raised">
 * </picture>
 * <picture style="min-width: 33%; display: inline-block;">
 *   <source srcset="toolbar-view-raised-border-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toolbar-view-raised-border.png" alt="toolbar-view-raised-border">
 * </picture>
 *
 * See [property@ToolbarView:top-bar-style] and
 * [property@ToolbarView:bottom-bar-style].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.4
 */

struct _AdwToolbarView
{
  GtkWidget parent_instance;

  GtkWidget *content;

  GtkWidget *top_bar;
  GtkWidget *top_box;

  GtkWidget *bottom_bar;
  GtkWidget *bottom_box;

  AdwToolbarStyle top_bar_style;
  AdwToolbarStyle bottom_bar_style;

  gboolean extend_content_to_top_edge;
  gboolean extend_content_to_bottom_edge;

  int top_bar_height;
  int bottom_bar_height;
};

static void adw_toolbar_view_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwToolbarView, adw_toolbar_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_toolbar_view_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_TOP_BAR_STYLE,
  PROP_BOTTOM_BAR_STYLE,
  PROP_REVEAL_TOP_BARS,
  PROP_REVEAL_BOTTOM_BARS,
  PROP_EXTEND_CONTENT_TO_TOP_EDGE,
  PROP_EXTEND_CONTENT_TO_BOTTOM_EDGE,
  PROP_TOP_BAR_HEIGHT,
  PROP_BOTTOM_BAR_HEIGHT,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
update_undershoots (AdwToolbarView *self)
{
  if (self->top_bar_style == ADW_TOOLBAR_FLAT &&
      !self->extend_content_to_top_edge &&
      gtk_widget_get_height (self->top_bar) > 0) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "undershoot-top");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "undershoot-top");
  }

  if (self->bottom_bar_style == ADW_TOOLBAR_FLAT &&
      !self->extend_content_to_bottom_edge &&
      gtk_widget_get_height (self->bottom_bar) > 0) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "undershoot-bottom");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "undershoot-bottom");
  }
}

static void
update_collapse_style (GtkWidget *box)
{
  int n_visible = 0;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (box);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    if (gtk_widget_get_visible (child))
      n_visible++;

    if (n_visible > 1)
      break;
   }

  if (n_visible > 1)
    gtk_widget_add_css_class (box, "collapse-spacing");
  else
    gtk_widget_remove_css_class (box, "collapse-spacing");
}

static GtkSizeRequestMode
adw_toolbar_view_get_request_mode (GtkWidget *widget)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (widget);

  if (self->content)
    return gtk_widget_get_request_mode (self->content);

  return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
adw_toolbar_view_measure (GtkWidget      *widget,
                          GtkOrientation  orientation,
                          int             for_size,
                          int            *minimum,
                          int            *natural,
                          int            *minimum_baseline,
                          int            *natural_baseline)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (widget);
  int top_min, bottom_min, content_min = 0;
  int top_nat, bottom_nat, content_nat = 0;

  gtk_widget_measure (self->top_bar, orientation, -1,
                      &top_min, &top_nat, NULL, NULL);

  gtk_widget_measure (self->bottom_bar, orientation, -1,
                      &bottom_min, &bottom_nat, NULL, NULL);

  if (self->content)
    gtk_widget_measure (self->content, orientation, for_size,
                        &content_min, &content_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (minimum)
      *minimum = MAX (content_min, MAX (top_min, bottom_min));
    if (natural)
      *natural = MAX (content_nat, MAX (top_nat, bottom_nat));
  } else {
    int min = content_min;
    int nat = content_nat;

    if (self->extend_content_to_top_edge && self->extend_content_to_bottom_edge) {
      min = MAX (min, top_min + bottom_min);
      nat = MAX (nat, top_nat + bottom_nat);
    } else if (self->extend_content_to_top_edge) {
      min = MAX (min, top_min) + bottom_min;
      nat = MAX (nat, top_nat) + bottom_nat;
    } else if (self->extend_content_to_bottom_edge) {
      min = MAX (min, bottom_min) + top_min;
      nat = MAX (nat, bottom_nat) + top_nat;
    } else {
      min += top_min + bottom_min;
      nat += top_nat + bottom_nat;
    }

    if (minimum)
      *minimum = min;
    if (natural)
      *natural = nat;
  }

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_toolbar_view_size_allocate (GtkWidget *widget,
                                int        width,
                                int        height,
                                int        baseline)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (widget);
  int top_min, top_nat, bottom_min, bottom_nat, content_min = 0;
  int top_height, bottom_height;
  int content_height, content_offset;

  gtk_widget_measure (self->top_bar, GTK_ORIENTATION_VERTICAL, -1,
                      &top_min, &top_nat, NULL, NULL);
  gtk_widget_measure (self->bottom_bar, GTK_ORIENTATION_VERTICAL, -1,
                      &bottom_min, &bottom_nat, NULL, NULL);

  if (self->content)
    gtk_widget_measure (self->content, GTK_ORIENTATION_VERTICAL, -1,
                        &content_min, NULL, NULL, NULL);

  if (self->extend_content_to_top_edge)
    content_min -= top_min;

  if (self->extend_content_to_bottom_edge)
    content_min -= bottom_min;

  content_min = MAX (content_min, 0);

  top_height = CLAMP (height - content_min - bottom_min, top_min, top_nat);
  bottom_height = CLAMP (height - content_min - top_height, bottom_min, bottom_nat);

  content_height = height;
  content_offset = 0;

  if (!self->extend_content_to_top_edge) {
    content_height -= top_height;
    content_offset = top_height;
  }

  if (!self->extend_content_to_bottom_edge)
    content_height -= bottom_height;

  if (self->top_bar_height != top_height) {
    self->top_bar_height = top_height;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TOP_BAR_HEIGHT]);
  }

  if (self->bottom_bar_height != bottom_height) {
    self->bottom_bar_height = bottom_height;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BOTTOM_BAR_HEIGHT]);
  }

  gtk_widget_allocate (self->top_bar, width, top_height, -1, NULL);
  gtk_widget_allocate (self->bottom_bar, width, bottom_height, -1,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, height - bottom_height)));

  if (self->content)
    gtk_widget_allocate (self->content, width, content_height, -1,
                         gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, content_offset)));

  update_undershoots (self);
}

static void
adw_toolbar_view_dispose (GObject *object)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (object);

  g_clear_pointer (&self->content, gtk_widget_unparent);
  g_clear_pointer (&self->top_bar, gtk_widget_unparent);
  g_clear_pointer (&self->bottom_bar, gtk_widget_unparent);

  self->top_box = NULL;
  self->bottom_box = NULL;

  G_OBJECT_CLASS (adw_toolbar_view_parent_class)->dispose (object);
}

static void
adw_toolbar_view_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_toolbar_view_get_content (self));
    break;
  case PROP_TOP_BAR_STYLE:
    g_value_set_enum (value, adw_toolbar_view_get_top_bar_style (self));
    break;
  case PROP_BOTTOM_BAR_STYLE:
    g_value_set_enum (value, adw_toolbar_view_get_bottom_bar_style (self));
    break;
  case PROP_REVEAL_TOP_BARS:
    g_value_set_boolean (value, adw_toolbar_view_get_reveal_top_bars (self));
    break;
  case PROP_REVEAL_BOTTOM_BARS:
    g_value_set_boolean (value, adw_toolbar_view_get_reveal_bottom_bars (self));
    break;
  case PROP_EXTEND_CONTENT_TO_TOP_EDGE:
    g_value_set_boolean (value, adw_toolbar_view_get_extend_content_to_top_edge (self));
    break;
  case PROP_EXTEND_CONTENT_TO_BOTTOM_EDGE:
    g_value_set_boolean (value, adw_toolbar_view_get_extend_content_to_bottom_edge (self));
    break;
  case PROP_TOP_BAR_HEIGHT:
    g_value_set_int (value, adw_toolbar_view_get_top_bar_height (self));
    break;
  case PROP_BOTTOM_BAR_HEIGHT:
    g_value_set_int (value, adw_toolbar_view_get_bottom_bar_height (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toolbar_view_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_toolbar_view_set_content (self, g_value_get_object (value));
    break;
  case PROP_TOP_BAR_STYLE:
    adw_toolbar_view_set_top_bar_style (self, g_value_get_enum (value));
    break;
  case PROP_BOTTOM_BAR_STYLE:
    adw_toolbar_view_set_bottom_bar_style (self, g_value_get_enum (value));
    break;
  case PROP_REVEAL_TOP_BARS:
    adw_toolbar_view_set_reveal_top_bars (self, g_value_get_boolean (value));
    break;
  case PROP_REVEAL_BOTTOM_BARS:
    adw_toolbar_view_set_reveal_bottom_bars (self, g_value_get_boolean (value));
    break;
  case PROP_EXTEND_CONTENT_TO_TOP_EDGE:
    adw_toolbar_view_set_extend_content_to_top_edge (self, g_value_get_boolean (value));
    break;
  case PROP_EXTEND_CONTENT_TO_BOTTOM_EDGE:
    adw_toolbar_view_set_extend_content_to_bottom_edge (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toolbar_view_class_init (AdwToolbarViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_toolbar_view_dispose;
  object_class->get_property = adw_toolbar_view_get_property;
  object_class->set_property = adw_toolbar_view_set_property;

  widget_class->get_request_mode = adw_toolbar_view_get_request_mode;
  widget_class->measure = adw_toolbar_view_measure;
  widget_class->size_allocate = adw_toolbar_view_size_allocate;
  widget_class->focus = adw_widget_focus_child;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwToolbarView:content: (attributes org.gtk.Property.get=adw_toolbar_view_get_content org.gtk.Property.set=adw_toolbar_view_set_content)
   *
   * The content widget.
   *
   * Since: 1.4
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:top-bar-style: (attributes org.gtk.Property.get=adw_toolbar_view_get_top_bar_style org.gtk.Property.set=adw_toolbar_view_set_top_bar_style)
   *
   * Appearance of the top bars.
   *
   * If set to `ADW_TOOLBAR_FLAT`, top bars are flat and scrolling content has a
   * subtle undershoot shadow when touching them, same as the
   * [`.undershoot-top`](style-classes.html#undershoot-indicators)
   * style class. This works well for simple content, e.g. [class@StatusPage] or
   * [class@PreferencesPage], where the background at the top of the page is
   * uniform. Additionally, windows with sidebars should always use this style.
   *
   * Undershoot shadow is only present if a top bar is actually present and
   * visible. It is also never present if
   * [property@ToolbarView:extend-content-to-top-edge] is set to `TRUE`.
   *
   * If set to `ADW_TOOLBAR_RAISED`, top bars have an opaque background and a
   * persistent shadow, this is suitable for content such as
   * [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
   * where some elements are directly adjacent to the top bars, or
   * [class@TabView], where each page can have a different background.
   *
   * `ADW_TOOLBAR_RAISED_BORDER` is similar to `ADW_TOOLBAR_RAISED`, but the
   * shadow is replaced with a more subtle border. This can be useful for
   * applications like image viewers.
   *
   * See also [property@ToolbarView:bottom-bar-style].
   *
   * Since: 1.4
   */
  props[PROP_TOP_BAR_STYLE] =
    g_param_spec_enum ("top-bar-style", NULL, NULL,
                       ADW_TYPE_TOOLBAR_STYLE,
                       ADW_TOOLBAR_FLAT,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:bottom-bar-style: (attributes org.gtk.Property.get=adw_toolbar_view_get_bottom_bar_style org.gtk.Property.set=adw_toolbar_view_set_bottom_bar_style)
   *
   * Appearance of the bottom bars.
   *
   * If set to `ADW_TOOLBAR_FLAT`, bottom bars are flat and scrolling content
   * has a subtle undershoot shadow when touching them, same as the
   * [`.undershoot-bottom`](style-classes.html#undershoot-indicators)
   * style class. This works well for simple content, e.g. [class@StatusPage] or
   * [class@PreferencesPage], where the background at the bottom of the page is
   * uniform. Additionally, windows with sidebars should always use this style.
   *
   * Undershoot shadow is only present if a bottom bar is actually present and
   * visible. It is also never present if
   * [property@ToolbarView:extend-content-to-bottom-edge] is set to `TRUE`.
   *
   * If set to `ADW_TOOLBAR_RAISED`, bottom bars have an opaque background and a
   * persistent shadow, this is suitable for content such as
   * [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
   * where some elements are directly adjacent to the bottom bars, or
   * [class@TabView], where each page can have a different background.
   *
   * `ADW_TOOLBAR_RAISED_BORDER` is similar to `ADW_TOOLBAR_RAISED`, but the
   * shadow is replaced with a more subtle border. This can be useful for
   * applications like image viewers.
   *
   * See also [property@ToolbarView:top-bar-style].
   *
   * Since: 1.4
   */
  props[PROP_BOTTOM_BAR_STYLE] =
    g_param_spec_enum ("bottom-bar-style", NULL, NULL,
                       ADW_TYPE_TOOLBAR_STYLE,
                       ADW_TOOLBAR_FLAT,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:reveal-top-bars: (attributes org.gtk.Property.get=adw_toolbar_view_get_reveal_top_bars org.gtk.Property.set=adw_toolbar_view_get_reveal_top_bars)
   *
   * Whether top bars are revealed.
   *
   * The transition will be animated.
   *
   * This can be used in combination with
   * [property@ToolbarView:extend-content-to-top-edge] to show and hide toolbars
   * in fullscreen.
   *
   * See [property@ToolbarView:reveal-bottom-bars].
   *
   * Since: 1.4
   */
  props[PROP_REVEAL_TOP_BARS] =
    g_param_spec_boolean ("reveal-top-bars", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:reveal-bottom-bars: (attributes org.gtk.Property.get=adw_toolbar_view_get_reveal_bottom_bars org.gtk.Property.set=adw_toolbar_view_get_reveal_bottom_bars)
   *
   * Whether bottom bars are visible.
   *
   * The transition will be animated.
   *
   * This can be used in combination with
   * [property@ToolbarView:extend-content-to-bottom-edge] to show and hide
   * toolbars in fullscreen.
   *
   * See [property@ToolbarView:reveal-top-bars].
   *
   * Since: 1.4
   */
  props[PROP_REVEAL_BOTTOM_BARS] =
    g_param_spec_boolean ("reveal-bottom-bars", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:extend-content-to-top-edge: (attributes org.gtk.Property.get=adw_toolbar_view_get_extend_content_to_top_edge org.gtk.Property.set=adw_toolbar_view_set_extend_content_to_top_edge)
   *
   * Whether the content widget can extend behind top bars.
   *
   * This can be used in combination with [property@ToolbarView:reveal-top-bars]
   * to show and hide toolbars in fullscreen.
   *
   * See [property@ToolbarView:extend-content-to-bottom-edge].
   *
   * Since: 1.4
   */
  props[PROP_EXTEND_CONTENT_TO_TOP_EDGE] =
    g_param_spec_boolean ("extend-content-to-top-edge", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:extend-content-to-bottom-edge: (attributes org.gtk.Property.get=adw_toolbar_view_get_extend_content_to_bottom_edge org.gtk.Property.set=adw_toolbar_view_set_extend_content_to_bottom_edge)
   *
   * Whether the content widget can extend behind bottom bars.
   *
   * This can be used in combination with
   * [property@ToolbarView:reveal-bottom-bars] to show and hide toolbars in
   * fullscreen.
   *
   * See [property@ToolbarView:extend-content-to-top-edge].
   *
   * Since: 1.4
   */
  props[PROP_EXTEND_CONTENT_TO_BOTTOM_EDGE] =
    g_param_spec_boolean ("extend-content-to-bottom-edge", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToolbarView:top-bar-height: (attributes org.gtk.Property.get=adw_toolbar_view_get_top_bar_height)
   *
   * The current top bar height.
   *
   * Top bar height does change depending [property@ToolbarView:reveal-top-bars],
   * including during the transition.
   *
   * See [property@ToolbarView:bottom-bar-height].
   *
   * Since: 1.4
   */
  props[PROP_TOP_BAR_HEIGHT] =
    g_param_spec_int ("top-bar-height", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwToolbarView:bottom-bar-height: (attributes org.gtk.Property.get=adw_toolbar_view_get_bottom_bar_height)
   *
   * The current bottom bar height.
   *
   * Bottom bar height does change depending on
   * [property@ToolbarView:reveal-bottom-bars], including during the transition.
   *
   * See [property@ToolbarView:top-bar-height].
   *
   * Since: 1.4
   */
  props[PROP_BOTTOM_BAR_HEIGHT] =
    g_param_spec_int ("bottom-bar-height", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "toolbarview");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_toolbar_view_init (AdwToolbarView *self)
{
  GtkWidget *top_handle, *bottom_handle;

  self->top_bar_style = ADW_TOOLBAR_FLAT;
  self->bottom_bar_style = ADW_TOOLBAR_FLAT;

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  self->top_bar = gtk_revealer_new ();
  gtk_widget_set_overflow (self->top_bar, GTK_OVERFLOW_VISIBLE);
  gtk_revealer_set_reveal_child (GTK_REVEALER (self->top_bar), TRUE);
  gtk_widget_set_vexpand (self->top_bar, FALSE);
  gtk_widget_add_css_class (self->top_bar, "top-bar");
  gtk_widget_set_parent (self->top_bar, GTK_WIDGET (self));

  top_handle = gtk_window_handle_new ();
  gtk_revealer_set_child (GTK_REVEALER (self->top_bar), top_handle);

  self->top_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_handle_set_child (GTK_WINDOW_HANDLE (top_handle), self->top_box);

  self->bottom_bar = gtk_revealer_new ();
  gtk_widget_set_overflow (self->bottom_bar, GTK_OVERFLOW_VISIBLE);
  gtk_revealer_set_reveal_child (GTK_REVEALER (self->bottom_bar), TRUE);
  gtk_revealer_set_transition_type (GTK_REVEALER (self->bottom_bar),
                                    GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
  gtk_widget_set_vexpand (self->bottom_bar, FALSE);
  gtk_widget_add_css_class (self->bottom_bar, "bottom-bar");
  gtk_widget_set_parent (self->bottom_bar, GTK_WIDGET (self));

  bottom_handle = gtk_window_handle_new ();
  gtk_revealer_set_child (GTK_REVEALER (self->bottom_bar), bottom_handle);

  self->bottom_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_handle_set_child (GTK_WINDOW_HANDLE (bottom_handle), self->bottom_box);
}

static void
adw_toolbar_view_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  AdwToolbarView *self = ADW_TOOLBAR_VIEW (buildable);

  if (g_strcmp0 (type, "top") == 0)
    adw_toolbar_view_add_top_bar (self, GTK_WIDGET (child));
  else if (g_strcmp0 (type, "bottom") == 0)
    adw_toolbar_view_add_bottom_bar (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_toolbar_view_set_content (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_toolbar_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_toolbar_view_buildable_add_child;
}

/**
 * adw_toolbar_view_new:
 *
 * Creates a new `AdwToolbarView`.
 *
 * Returns: the newly created `AdwToolbarView`
 *
 * Since: 1.4
 */
GtkWidget *
adw_toolbar_view_new (void)
{
  return g_object_new (ADW_TYPE_TOOLBAR_VIEW, NULL);
}

/**
 * adw_toolbar_view_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: a toolbar view
 *
 * Gets the content widget for @self.
 *
 * Returns: (transfer none) (nullable): the content widget
 *
 * Since: 1.4
 */
GtkWidget *
adw_toolbar_view_get_content (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), NULL);

  return self->content;
}

/**
 * adw_toolbar_view_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: a toolbar view
 * @content: (nullable): the content widget
 *
 * Sets the content widget for @self.
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_content (AdwToolbarView *self,
                              GtkWidget      *content)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  if (content == self->content)
    return;

  if (self->content)
    gtk_widget_unparent (self->content);

  self->content = content;

  if (self->content)
    gtk_widget_insert_before (self->content, GTK_WIDGET (self), self->top_bar);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_toolbar_view_add_top_bar:
 * @self: a toolbar view
 * @widget: a widget
 *
 * Adds a top bar to @self.
 *
 * Since: 1.4
 */
void
adw_toolbar_view_add_top_bar (AdwToolbarView *self,
                              GtkWidget      *widget)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  gtk_box_append (GTK_BOX (self->top_box), widget);
  update_collapse_style (self->top_box);

  g_signal_connect_swapped (widget, "notify::visible",
                            G_CALLBACK (update_collapse_style),
                            self->top_box);
}

/**
 * adw_toolbar_view_add_bottom_bar:
 * @self: a toolbar view
 * @widget: a widget
 *
 * Adds a bottom bar to @self.
 *
 * Since: 1.4
 */
void
adw_toolbar_view_add_bottom_bar (AdwToolbarView *self,
                                 GtkWidget      *widget)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  gtk_box_append (GTK_BOX (self->bottom_box), widget);
  update_collapse_style (self->bottom_box);

  g_signal_connect_swapped (widget, "notify::visible",
                            G_CALLBACK (update_collapse_style),
                            self->bottom_box);
}

/**
 * adw_toolbar_view_remove:
 * @self: a toolbar view
 * @widget: the child to be removed
 *
 * Removes a child from @self.
 *
 * Since: 1.4
 */
void
adw_toolbar_view_remove (AdwToolbarView *self,
                         GtkWidget      *widget)
{
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  parent = gtk_widget_get_parent (widget);

  if (parent == self->top_box || parent == self->bottom_box) {
    g_signal_handlers_disconnect_by_func (widget,
                                          update_collapse_style,
                                          parent);

    gtk_box_remove (GTK_BOX (parent), widget);

    update_collapse_style (parent);

    return;
  }

  if (widget == self->content) {
    adw_toolbar_view_set_content (self, NULL);
    return;
  }

  ADW_CRITICAL_CANNOT_REMOVE_CHILD (self, widget);
}

/**
 * adw_toolbar_view_get_top_bar_style: (attributes org.gtk.Method.get_property=top-bar-style)
 * @self: a toolbar view
 *
 * Gets appearance of the top bars for @self.
 *
 * Returns: top bar style
 *
 * Since: 1.4
 */
AdwToolbarStyle
adw_toolbar_view_get_top_bar_style (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), ADW_TOOLBAR_FLAT);

  return self->top_bar_style;
}

/**
 * adw_toolbar_view_set_top_bar_style: (attributes org.gtk.Method.set_property=top-bar-style)
 * @self: a toolbar view
 * @style: top bar style
 *
 * Sets appearance of the top bars for @self.
 *
 * If set to `ADW_TOOLBAR_FLAT`, top bars are flat and scrolling content has a
 * subtle undershoot shadow when touching them, same as the
 * [`.undershoot-top`](style-classes.html#undershoot-indicators)
 * style class. This works well for simple content, e.g. [class@StatusPage] or
 * [class@PreferencesPage], where the background at the top of the page is
 * uniform. Additionally, windows with sidebars should always use this style.
 *
 * Undershoot shadow is only present if a top bar is actually present and
 * visible. It is also never present if
 * [property@ToolbarView:extend-content-to-top-edge] is set to `TRUE`.
 *
 * If set to `ADW_TOOLBAR_RAISED`, top bars have an opaque background and a
 * persistent shadow, this is suitable for content such as
 * [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
 * where some elements are directly adjacent to the top bars, or
 * [class@TabView], where each page can have a different background.
 *
 * `ADW_TOOLBAR_RAISED_BORDER` is similar to `ADW_TOOLBAR_RAISED`, but the
 * shadow is replaced with a more subtle border. This can be useful for
 * applications like image viewers.
 *
 * See also [method@ToolbarView.set_bottom_bar_style].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_top_bar_style (AdwToolbarView  *self,
                                    AdwToolbarStyle  style)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (style <= ADW_TOOLBAR_RAISED_BORDER);

  if (self->top_bar_style == style)
    return;

  self->top_bar_style = style;

  switch (style) {
    case ADW_TOOLBAR_FLAT:
      gtk_widget_remove_css_class (self->top_bar, "raised");
      gtk_widget_remove_css_class (self->top_bar, "border");
      break;
    case ADW_TOOLBAR_RAISED:
      gtk_widget_add_css_class (self->top_bar, "raised");
      gtk_widget_remove_css_class (self->top_bar, "border");
      break;
    case ADW_TOOLBAR_RAISED_BORDER:
      gtk_widget_add_css_class (self->top_bar, "raised");
      gtk_widget_add_css_class (self->top_bar, "border");
      break;
    default:
      g_assert_not_reached ();
  }

  update_undershoots (self);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TOP_BAR_STYLE]);
}

/**
 * adw_toolbar_view_get_bottom_bar_style: (attributes org.gtk.Method.get_property=bottom-bar-style)
 * @self: a toolbar view
 *
 * Gets appearance of the botom bars for @self.
 *
 * Returns: bottom bar style
 *
 * Since: 1.4
 */
AdwToolbarStyle
adw_toolbar_view_get_bottom_bar_style (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), ADW_TOOLBAR_FLAT);

  return self->bottom_bar_style;
}

/**
 * adw_toolbar_view_set_bottom_bar_style: (attributes org.gtk.Method.set_property=bottom-bar-style)
 * @self: a toolbar view
 * @style: bottom bar style
 *
 * Sets appearance of the bottom bars for @self.
 *
 * If set to `ADW_TOOLBAR_FLAT`, bottom bars are flat and scrolling content has
 * a subtle undershoot shadow when touching them, same as the
 * [`.undershoot-bottom`](style-classes.html#undershoot-indicators)
 * style class. This works well for simple content, e.g. [class@StatusPage] or
 * [class@PreferencesPage], where the background at the bottom of the page is
 * uniform. Additionally, windows with sidebars should always use this style.
 *
 * Undershoot shadow is only present if a bottom bar is actually present and
 * visible. It is also never present if
 * [property@ToolbarView:extend-content-to-bottom-edge] is set to `TRUE`.
 *
 * If set to `ADW_TOOLBAR_RAISED`, bottom bars have an opaque background and a
 * persistent shadow, this is suitable for content such as
 * [utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html),
 * where some elements are directly adjacent to the bottom bars, or
 * [class@TabView], where each page can have a different background.
 *
 * `ADW_TOOLBAR_RAISED_BORDER` is similar to `ADW_TOOLBAR_RAISED`, but the
 * shadow is replaced with a more subtle border. This can be useful for
 * applications like image viewers.
 *
 * See also [method@ToolbarView.set_top_bar_style].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_bottom_bar_style (AdwToolbarView  *self,
                                       AdwToolbarStyle  style)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));
  g_return_if_fail (style <= ADW_TOOLBAR_RAISED_BORDER);

  if (self->bottom_bar_style == style)
    return;

  self->bottom_bar_style = style;

  switch (style) {
    case ADW_TOOLBAR_FLAT:
      gtk_widget_remove_css_class (self->bottom_bar, "raised");
      gtk_widget_remove_css_class (self->bottom_bar, "border");
      break;
    case ADW_TOOLBAR_RAISED:
      gtk_widget_add_css_class (self->bottom_bar, "raised");
      gtk_widget_remove_css_class (self->bottom_bar, "border");
      break;
    case ADW_TOOLBAR_RAISED_BORDER:
      gtk_widget_add_css_class (self->bottom_bar, "raised");
      gtk_widget_add_css_class (self->bottom_bar, "border");
      break;
    default:
      g_assert_not_reached ();
  }

  update_undershoots (self);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BOTTOM_BAR_STYLE]);
}

/**
 * adw_toolbar_view_get_reveal_top_bars: (attributes org.gtk.Method.get_property=reveal-top-bars)
 * @self: a toolbar view
 *
 * Gets whether top bars are revealed for @self.
 *
 * Returns: whether top bars are revealed
 *
 * Since: 1.4
 */
gboolean
adw_toolbar_view_get_reveal_top_bars (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), FALSE);

  return gtk_revealer_get_reveal_child (GTK_REVEALER (self->top_bar));
}

/**
 * adw_toolbar_view_set_reveal_top_bars: (attributes org.gtk.Method.set_property=reveal-top-bars)
 * @self: a toolbar view
 * @reveal: whether to reveal top bars
 *
 * Sets whether top bars are revealed for @self.
 *
 * The transition will be animated.
 *
 * This can be used in combination with
 * [property@ToolbarView:extend-content-to-top-edge] to show and hide toolbars
 * in fullscreen.
 *
 * See [method@ToolbarView.set_reveal_bottom_bars].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_reveal_top_bars (AdwToolbarView *self,
                                      gboolean        reveal)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));

  reveal = !!reveal;

  if (reveal == adw_toolbar_view_get_reveal_top_bars (self))
    return;

  gtk_revealer_set_reveal_child (GTK_REVEALER (self->top_bar), reveal);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_TOP_BARS]);
}

/**
 * adw_toolbar_view_get_reveal_bottom_bars: (attributes org.gtk.Method.get_property=reveal-bottom-bars)
 * @self: a toolbar view
 *
 * Gets whether bottom bars are revealed for @self.
 *
 * Returns: whether bottom bars are revealed
 *
 * Since: 1.4
 */
gboolean
adw_toolbar_view_get_reveal_bottom_bars (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), FALSE);

  return gtk_revealer_get_reveal_child (GTK_REVEALER (self->bottom_bar));
}

/**
 * adw_toolbar_view_set_reveal_bottom_bars: (attributes org.gtk.Method.set_property=reveal-bottom-bars)
 * @self: a toolbar view
 * @reveal: whether to reveal bottom bars
 *
 * Sets whether bottom bars are revealed for @self.
 *
 * The transition will be animated.
 *
 * This can be used in combination with
 * [property@ToolbarView:extend-content-to-bottom-edge] to show and hide
 * toolbars in fullscreen.
 *
 * See [method@ToolbarView.set_reveal_top_bars].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_reveal_bottom_bars (AdwToolbarView *self,
                                         gboolean        reveal)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));

  reveal = !!reveal;

  if (reveal == adw_toolbar_view_get_reveal_bottom_bars (self))
    return;

  gtk_revealer_set_reveal_child (GTK_REVEALER (self->bottom_bar), reveal);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_BOTTOM_BARS]);
}

/**
 * adw_toolbar_view_get_extend_content_to_top_edge: (attributes org.gtk.Method.get_property=extend-content-to-top-edge)
 * @self: a toolbar view
 *
 * Gets whether the content widget can extend behind top bars.
 *
 * Returns: whether content extends behind top bars
 *
 * Since: 1.4
 */
gboolean
adw_toolbar_view_get_extend_content_to_top_edge (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), FALSE);

  return self->extend_content_to_top_edge;
}

/**
 * adw_toolbar_view_set_extend_content_to_top_edge: (attributes org.gtk.Method.set_property=extend-content-to-top-edge)
 * @self: a toolbar view
 * @extend: whether content extends behind top bars
 *
 * Sets whether the content widget can extend behind top bars.
 *
 * This can be used in combination with [property@ToolbarView:reveal-top-bars]
 * to show and hide toolbars in fullscreen.
 *
 * See [method@ToolbarView.set_extend_content_to_bottom_edge].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_extend_content_to_top_edge (AdwToolbarView *self,
                                                 gboolean        extend)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));

  extend = !!extend;

  if (extend == self->extend_content_to_top_edge)
    return;

  self->extend_content_to_top_edge = extend;

  update_undershoots (self);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTEND_CONTENT_TO_TOP_EDGE]);
}

/**
 * adw_toolbar_view_get_extend_content_to_bottom_edge: (attributes org.gtk.Method.get_property=extend-content-to-bottom-edge)
 * @self: a toolbar view
 *
 * Gets whether the content widget can extend behind bottom bars.
 *
 * Returns: whether content extends behind bottom bars
 *
 * Since: 1.4
 */
gboolean
adw_toolbar_view_get_extend_content_to_bottom_edge (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), FALSE);

  return self->extend_content_to_bottom_edge;
}

/**
 * adw_toolbar_view_set_extend_content_to_bottom_edge: (attributes org.gtk.Method.set_property=extend-content-to-bottom-edge)
 * @self: a toolbar view
 * @extend: whether content extends behind bottom bars
 *
 * Sets whether the content widget can extend behind bottom bars.
 *
 * This can be used in combination with [property@ToolbarView:reveal-bottom-bars]
 * to show and hide toolbars in fullscreen.
 *
 * See [method@ToolbarView.set_extend_content_to_top_edge].
 *
 * Since: 1.4
 */
void
adw_toolbar_view_set_extend_content_to_bottom_edge (AdwToolbarView *self,
                                                    gboolean        extend)
{
  g_return_if_fail (ADW_IS_TOOLBAR_VIEW (self));

  extend = !!extend;

  if (extend == self->extend_content_to_bottom_edge)
    return;

  self->extend_content_to_bottom_edge = extend;

  update_undershoots (self);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTEND_CONTENT_TO_BOTTOM_EDGE]);
}

/**
 * adw_toolbar_view_get_top_bar_height: (attributes org.gtk.Method.get_property=top-bar-height)
 * @self: a toolbar view
 *
 * Gets the current top bar height for @self.
 *
 * Top bar height does change depending on
 * [property@ToolbarView:reveal-top-bars], including during the transition.
 *
 * See [method@ToolbarView.get_bottom_bar_height].
 *
 * Returns: the current top bar height
 *
 * Since: 1.4
 */
int
adw_toolbar_view_get_top_bar_height (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), 0);

  return self->top_bar_height;
}

/**
 * adw_toolbar_view_get_bottom_bar_height: (attributes org.gtk.Method.get_property=bottom-bar-height)
 * @self: a toolbar view
 *
 * Gets the current bottom bar height for @self.
 *
 * Bottom bar height does change depending on
 * [property@ToolbarView:reveal-bottom-bars], including during the transition.
 *
 * See [method@ToolbarView.get_top_bar_height].
 *
 * Returns: the current bottom bar height
 *
 * Since: 1.4
 */
int
adw_toolbar_view_get_bottom_bar_height (AdwToolbarView *self)
{
  g_return_val_if_fail (ADW_IS_TOOLBAR_VIEW (self), 0);

  return self->bottom_bar_height;
}
