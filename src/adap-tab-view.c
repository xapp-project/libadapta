/*
 * Copyright (C) 2020-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-tab-view-private.h"

#include "adw-bin.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-style-manager.h"
#include "adw-widget-utils-private.h"

/* FIXME replace with groups */
static GSList *tab_view_list;

#define MIN_ASPECT_RATIO 0.8
#define MAX_ASPECT_RATIO 2.7
#define DEFAULT_ICON_ALPHA_HC 0.3
#define DEFAULT_ICON_ALPHA 0.15
#define MIN_THUMBNAIL_BITMAP_WIDTH 250
#define MAX_THUMBNAIL_BITMAP_WIDTH 500
#define MIN_THUMBNAIL_BITMAP_HEIGHT 200
#define MAX_THUMBNAIL_BITMAP_HEIGHT 600

/**
 * AdwTabView:
 *
 * A dynamic tabbed container.
 *
 * `AdwTabView` is a container which shows one child at a time. While it
 * provides keyboard shortcuts for switching between pages, it does not provide
 * a visible tab switcher and relies on external widgets for that, such as
 * [class@TabBar], [class@TabOverview] and [class@TabButton].
 *
 * `AdwTabView` maintains a [class@TabPage] object for each page, which holds
 * additional per-page properties. You can obtain the `AdwTabPage` for a page
 * with [method@TabView.get_page], and as the return value for
 * [method@TabView.append] and other functions for adding children.
 *
 * `AdwTabView` only aims to be useful for dynamic tabs in multi-window
 * document-based applications, such as web browsers, file managers, text
 * editors or terminals. It does not aim to replace [class@Gtk.Notebook] for use
 * cases such as tabbed dialogs.
 *
 * As such, it does not support disabling page reordering or detaching.
 *
 * `AdwTabView` adds a number of global page switching and reordering shortcuts.
 * The [property@TabView:shortcuts] property can be used to manage them.
 *
 * See [flags@TabViewShortcuts] for the list of the available shortcuts. All of
 * the shortcuts are enabled by default.
 *
 * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts] can be
 * used to manage shortcuts in a convenient way, for example:
 *
 * ```c
 * adw_tab_view_remove_shortcuts (view, ADW_TAB_VIEW_SHORTCUT_CONTROL_HOME |
 *                                      ADW_TAB_VIEW_SHORTCUT_CONTROL_END);
 * ```
 *
 * ## CSS nodes
 *
 * `AdwTabView` has a main CSS node with the name `tabview`.
 *
 * ## Accessibility
 *
 * `AdwTabView` uses the `GTK_ACCESSIBLE_ROLE_TAB_PANEL` for the tab pages which
 * are the accessible parent objects of the child widgets.
 */

/**
 * AdwTabPage:
 *
 * An auxiliary class used by [class@TabView].
 */

/**
 * AdwTabViewShortcuts:
 * @ADW_TAB_VIEW_SHORTCUT_NONE: No shortcuts
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_TAB:
 *   <kbd>Ctrl</kbd>+<kbd>Tab</kbd> - switch to the next page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB:
 *   <kbd>Shift</kbd>+<kbd>Ctrl</kbd>+<kbd>Tab</kbd> - switch to the previous
 *   page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP:
 *   <kbd>Ctrl</kbd>+<kbd>Page Up</kbd> - switch to the previous page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN:
 *   <kbd>Ctrl</kbd>+<kbd>Page Down</kbd> - switch to the next page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_HOME:
 *   <kbd>Ctrl</kbd>+<kbd>Home</kbd> - switch to the first page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_END:
 *   <kbd>Ctrl</kbd>+<kbd>End</kbd> - switch to the last page
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Page Up</kbd> - move the selected
 *   page backward
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Page Down</kbd> - move the selected
 *   page forward
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Home</kbd> - move the selected page
 *   at the start
 * @ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>End</kbd> - move the current page at
 *   the end
 * @ADW_TAB_VIEW_SHORTCUT_ALT_DIGITS:
 *  <kbd>Alt</kbd>+<kbd>1</kbd>⋯<kbd>9</kbd> - switch to pages 1-9
 * @ADW_TAB_VIEW_SHORTCUT_ALT_ZERO:
 *  <kbd>Alt</kbd>+<kbd>0</kbd> - switch to page 10
 * @ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS: All of the shortcuts
 *
 * Describes available shortcuts in an [class@TabView].
 *
 * Shortcuts can be set with [property@TabView:shortcuts], or added/removed
 * individually with [method@TabView.add_shortcuts] and
 * [method@TabView.remove_shortcuts].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.2
 */

struct _AdwTabPage
{
  GObject parent_instance;

  GtkWidget *bin;
  GtkWidget *child;
  AdwTabPage *parent;
  gboolean selected;
  gboolean pinned;
  char *title;
  char *tooltip;
  GIcon *icon;
  gboolean loading;
  GIcon *indicator_icon;
  char *indicator_tooltip;
  gboolean indicator_activatable;
  gboolean needs_attention;
  char *keyword;
  float thumbnail_xalign;
  float thumbnail_yalign;

  GtkWidget *last_focus;
  GBinding *transfer_binding;

  GtkATContext *at_context;

  gboolean closing;
  GdkPaintable *paintable;

  gboolean live_thumbnail;
  gboolean invalidated;
  gboolean in_destruction;
};

static void adw_tab_page_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwTabPage, adw_tab_page, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, adw_tab_page_accessible_init))

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_PARENT,
  PAGE_PROP_SELECTED,
  PAGE_PROP_PINNED,
  PAGE_PROP_TITLE,
  PAGE_PROP_TOOLTIP,
  PAGE_PROP_ICON,
  PAGE_PROP_LOADING,
  PAGE_PROP_INDICATOR_ICON,
  PAGE_PROP_INDICATOR_TOOLTIP,
  PAGE_PROP_INDICATOR_ACTIVATABLE,
  PAGE_PROP_NEEDS_ATTENTION,
  PAGE_PROP_KEYWORD,
  PAGE_PROP_THUMBNAIL_XALIGN,
  PAGE_PROP_THUMBNAIL_YALIGN,
  PAGE_PROP_LIVE_THUMBNAIL,
  LAST_PAGE_PROP,
  PAGE_PROP_ACCESSIBLE_ROLE
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _AdwTabView
{
  GtkWidget parent_instance;

  GListStore *children;

  int n_pages;
  int n_pinned_pages;
  AdwTabPage *selected_page;
  GIcon *default_icon;
  GMenuModel *menu_model;
  AdwTabViewShortcuts shortcuts;

  int transfer_count;
  int overview_count;
  gulong unmap_extra_pages_cb;

  GtkSelectionModel *pages;
};

static void adw_tab_view_buildable_init (GtkBuildableIface *iface);
static void adw_tab_view_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwTabView, adw_tab_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_tab_view_buildable_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, adw_tab_view_accessible_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_N_PINNED_PAGES,
  PROP_IS_TRANSFERRING_PAGE,
  PROP_SELECTED_PAGE,
  PROP_DEFAULT_ICON,
  PROP_MENU_MODEL,
  PROP_SHORTCUTS,
  PROP_PAGES,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PAGE_ATTACHED,
  SIGNAL_PAGE_DETACHED,
  SIGNAL_PAGE_REORDERED,
  SIGNAL_CLOSE_PAGE,
  SIGNAL_SETUP_MENU,
  SIGNAL_CREATE_WINDOW,
  SIGNAL_INDICATOR_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
page_should_be_visible (AdwTabView *view,
                        AdwTabPage *page)
{
  if (!view->overview_count)
    return FALSE;

  return page->live_thumbnail || page->invalidated;
}

static void
set_page_selected (AdwTabPage *self,
                   gboolean    selected)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  selected = !!selected;

  if (self->selected == selected)
    return;

  self->selected = selected;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_SELECTED]);
}

static void
set_page_pinned (AdwTabPage *self,
                 gboolean    pinned)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  pinned = !!pinned;

  if (self->pinned == pinned)
    return;

  self->pinned = pinned;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_PINNED]);
}

static void set_page_parent (AdwTabPage *self,
                             AdwTabPage *parent);

static void
page_parent_notify_cb (AdwTabPage *self)
{
  AdwTabPage *grandparent = adw_tab_page_get_parent (self->parent);

  self->parent = NULL;

  if (grandparent)
    set_page_parent (self, grandparent);
  else
    g_object_notify_by_pspec (G_OBJECT (self), props[PAGE_PROP_PARENT]);
}

static void
set_page_parent (AdwTabPage *self,
                 AdwTabPage *parent)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));
  g_return_if_fail (parent == NULL || ADW_IS_TAB_PAGE (parent));

  if (self->parent == parent)
    return;

  if (self->parent)
    g_object_weak_unref (G_OBJECT (self->parent),
                         (GWeakNotify) page_parent_notify_cb,
                         self);

  self->parent = parent;

  if (self->parent)
    g_object_weak_ref (G_OBJECT (self->parent),
                       (GWeakNotify) page_parent_notify_cb,
                       self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PAGE_PROP_PARENT]);
}

static void
map_or_unmap_page (AdwTabPage *self)
{
  GtkWidget *parent;
  AdwTabView *view;
  gboolean should_be_visible;

  parent = gtk_widget_get_parent (self->bin);

  if (!ADW_IS_TAB_VIEW (parent))
    return;

  view = ADW_TAB_VIEW (parent);

  if (!view->overview_count || !gtk_widget_get_mapped (GTK_WIDGET (view)))
    return;

  should_be_visible = self == view->selected_page ||
                      page_should_be_visible (view, self);

  if (gtk_widget_get_child_visible (self->bin) == should_be_visible)
    return;

  gtk_widget_set_child_visible (self->bin, should_be_visible);
  gtk_widget_queue_allocate (parent);
}

static void
adw_tab_page_dispose (GObject *object)
{
  AdwTabPage *self = ADW_TAB_PAGE (object);

  self->in_destruction = TRUE;

  set_page_parent (self, NULL);

  g_clear_object (&self->at_context);

  g_clear_object (&self->bin);
  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adw_tab_page_parent_class)->dispose (object);
}

static void
adw_tab_page_finalize (GObject *object)
{
  AdwTabPage *self = (AdwTabPage *)object;

  g_clear_object (&self->child);
  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->tooltip, g_free);
  g_clear_object (&self->icon);
  g_clear_object (&self->indicator_icon);
  g_clear_pointer (&self->indicator_tooltip, g_free);
  g_clear_pointer (&self->keyword, g_free);

  if (self->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);

  G_OBJECT_CLASS (adw_tab_page_parent_class)->finalize (object);
}

static void
adw_tab_page_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwTabPage *self = ADW_TAB_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, adw_tab_page_get_child (self));
    break;

  case PAGE_PROP_PARENT:
    g_value_set_object (value, adw_tab_page_get_parent (self));
    break;

  case PAGE_PROP_SELECTED:
    g_value_set_boolean (value, adw_tab_page_get_selected (self));
    break;

  case PAGE_PROP_PINNED:
    g_value_set_boolean (value, adw_tab_page_get_pinned (self));
    break;

  case PAGE_PROP_TITLE:
    g_value_set_string (value, adw_tab_page_get_title (self));
    break;

  case PAGE_PROP_TOOLTIP:
    g_value_set_string (value, adw_tab_page_get_tooltip (self));
    break;

  case PAGE_PROP_ICON:
    g_value_set_object (value, adw_tab_page_get_icon (self));
    break;

  case PAGE_PROP_LOADING:
    g_value_set_boolean (value, adw_tab_page_get_loading (self));
    break;

  case PAGE_PROP_INDICATOR_ICON:
    g_value_set_object (value, adw_tab_page_get_indicator_icon (self));
    break;

  case PAGE_PROP_INDICATOR_TOOLTIP:
    g_value_set_string (value, adw_tab_page_get_indicator_tooltip (self));
    break;

  case PAGE_PROP_INDICATOR_ACTIVATABLE:
    g_value_set_boolean (value, adw_tab_page_get_indicator_activatable (self));
    break;

  case PAGE_PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, adw_tab_page_get_needs_attention (self));
    break;

  case PAGE_PROP_KEYWORD:
    g_value_set_string (value, adw_tab_page_get_keyword (self));
    break;

  case PAGE_PROP_THUMBNAIL_XALIGN:
    g_value_set_float (value, adw_tab_page_get_thumbnail_xalign (self));
    break;

  case PAGE_PROP_THUMBNAIL_YALIGN:
    g_value_set_float (value, adw_tab_page_get_thumbnail_yalign (self));
    break;

  case PAGE_PROP_LIVE_THUMBNAIL:
    g_value_set_boolean (value, adw_tab_page_get_live_thumbnail (self));
    break;

  case PAGE_PROP_ACCESSIBLE_ROLE:
    g_value_set_enum (value, GTK_ACCESSIBLE_ROLE_TAB_PANEL);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_page_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwTabPage *self = ADW_TAB_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->child, g_value_get_object (value));
    adw_bin_set_child (ADW_BIN (self->bin), g_value_get_object (value));
    break;

  case PAGE_PROP_PARENT:
    set_page_parent (self, g_value_get_object (value));
    break;

  case PAGE_PROP_TITLE:
    adw_tab_page_set_title (self, g_value_get_string (value));
    break;

  case PAGE_PROP_TOOLTIP:
    adw_tab_page_set_tooltip (self, g_value_get_string (value));
    break;

  case PAGE_PROP_ICON:
    adw_tab_page_set_icon (self, g_value_get_object (value));
    break;

  case PAGE_PROP_LOADING:
    adw_tab_page_set_loading (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_INDICATOR_ICON:
    adw_tab_page_set_indicator_icon (self, g_value_get_object (value));
    break;

  case PAGE_PROP_INDICATOR_TOOLTIP:
    adw_tab_page_set_indicator_tooltip (self, g_value_get_string (value));
    break;

  case PAGE_PROP_INDICATOR_ACTIVATABLE:
    adw_tab_page_set_indicator_activatable (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_NEEDS_ATTENTION:
    adw_tab_page_set_needs_attention (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_KEYWORD:
    adw_tab_page_set_keyword (self, g_value_get_string (value));
    break;

  case PAGE_PROP_THUMBNAIL_XALIGN:
    adw_tab_page_set_thumbnail_xalign (self, g_value_get_float (value));
    break;

  case PAGE_PROP_THUMBNAIL_YALIGN:
    adw_tab_page_set_thumbnail_yalign (self, g_value_get_float (value));
    break;

  case PAGE_PROP_LIVE_THUMBNAIL:
    adw_tab_page_set_live_thumbnail (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_ACCESSIBLE_ROLE:
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_page_class_init (AdwTabPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_tab_page_dispose;
  object_class->finalize = adw_tab_page_finalize;
  object_class->get_property = adw_tab_page_get_property;
  object_class->set_property = adw_tab_page_set_property;

  /**
   * AdwTabPage:child: (attributes org.gtk.Property.get=adw_tab_page_get_child)
   *
   * The child of the page.
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabPage:parent: (attributes org.gtk.Property.get=adw_tab_page_get_parent)
   *
   * The parent page of the page.
   *
   * See [method@TabView.add_page] and [method@TabView.close_page].
   */
  page_props[PAGE_PROP_PARENT] =
    g_param_spec_object ("parent", NULL, NULL,
                         ADW_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:selected: (attributes org.gtk.Property.get=adw_tab_page_get_selected)
   *
   * Whether the page is selected.
   */
  page_props[PAGE_PROP_SELECTED] =
    g_param_spec_boolean ("selected", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabPage:pinned: (attributes org.gtk.Property.get=adw_tab_page_get_pinned)
   *
   * Whether the page is pinned.
   *
   * See [method@TabView.set_page_pinned].
   */
  page_props[PAGE_PROP_PINNED] =
    g_param_spec_boolean ("pinned", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabPage:title: (attributes org.gtk.Property.get=adw_tab_page_get_title org.gtk.Property.set=adw_tab_page_set_title)
   *
   * The title of the page.
   *
   * [class@TabBar] will display it in the center of the tab unless it's pinned,
   * and will use it as a tooltip unless [property@TabPage:tooltip] is set.
   *
   * [class@TabOverview] will display it below the thumbnail unless it's pinned,
   * or inside the card otherwise, and will use it as a tooltip unless
   * [property@TabPage:tooltip] is set.
   */
  page_props[PAGE_PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:tooltip: (attributes org.gtk.Property.get=adw_tab_page_get_tooltip org.gtk.Property.set=adw_tab_page_set_tooltip)
   *
   * The tooltip of the page.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * If not set, [class@TabBar] and [class@TabOverview] will use
   * [property@TabPage:title] as a tooltip instead.
   */
  page_props[PAGE_PROP_TOOLTIP] =
    g_param_spec_string ("tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:icon: (attributes org.gtk.Property.get=adw_tab_page_get_icon org.gtk.Property.set=adw_tab_page_set_icon)
   *
   * The icon of the page.
   *
   * [class@TabBar] and [class@TabOverview] display the icon next to the title,
   * unless [property@TabPage:loading] is set to `TRUE`.
   *
   * `AdwTabBar` also won't show the icon if the page is pinned and
   * [propertyTabPage:indicator-icon] is set.
   */
  page_props[PAGE_PROP_ICON] =
    g_param_spec_object ("icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:loading: (attributes org.gtk.Property.get=adw_tab_page_get_loading org.gtk.Property.set=adw_tab_page_set_loading)
   *
   * Whether the page is loading.
   *
   * If set to `TRUE`, [class@TabBar] and [class@TabOverview] will display a
   * spinner in place of icon.
   *
   * If the page is pinned and [property@TabPage:indicator-icon] is set,
   * loading status will not be visible with `AdwTabBar`.
   */
  page_props[PAGE_PROP_LOADING] =
    g_param_spec_boolean ("loading", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:indicator-icon: (attributes org.gtk.Property.get=adw_tab_page_get_indicator_icon org.gtk.Property.set=adw_tab_page_set_indicator_icon)
   *
   * An indicator icon for the page.
   *
   * A common use case is an audio or camera indicator in a web browser.
   *
   * [class@TabBar] will show it at the beginning of the tab, alongside icon
   * representing [property@TabPage:icon] or loading spinner.
   *
   * If the page is pinned, the indicator will be shown instead of icon or
   * spinner.
   *
   * [class@TabOverview] will show it at the at the top part of the thumbnail.
   *
   * [property@TabPage:indicator-tooltip] can be used to set the tooltip on the
   * indicator icon.
   *
   * If [property@TabPage:indicator-activatable] is set to `TRUE`, the
   * indicator icon can act as a button.
   */
  page_props[PAGE_PROP_INDICATOR_ICON] =
    g_param_spec_object ("indicator-icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:indicator-tooltip: (attributes org.gtk.Property.get=adw_tab_page_get_indicator_tooltip org.gtk.Property.set=adw_tab_page_set_indicator_tooltip)
   *
   * The tooltip of the indicator icon.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * See [property@TabPage:indicator-icon].
   *
   * Since: 1.2
   */
  page_props[PAGE_PROP_INDICATOR_TOOLTIP] =
    g_param_spec_string ("indicator-tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:indicator-activatable: (attributes org.gtk.Property.get=adw_tab_page_get_indicator_activatable org.gtk.Property.set=adw_tab_page_set_indicator_activatable)
   *
   * Whether the indicator icon is activatable.
   *
   * If set to `TRUE`, [signal@TabView::indicator-activated] will be emitted
   * when the indicator icon is clicked.
   *
   * If [property@TabPage:indicator-icon] is not set, does nothing.
   */
  page_props[PAGE_PROP_INDICATOR_ACTIVATABLE] =
    g_param_spec_boolean ("indicator-activatable", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:needs-attention: (attributes org.gtk.Property.get=adw_tab_page_get_needs_attention org.gtk.Property.set=adw_tab_page_set_needs_attention)
   *
   * Whether the page needs attention.
   *
   * [class@TabBar] will display a line under the tab representing the page if
   * set to `TRUE`. If the tab is not visible, the corresponding edge of the tab
   * bar will be highlighted.
   *
   * [class@TabOverview] will display a dot in the corner of the thumbnail if set
   * to `TRUE`.
   *
   * [class@TabButton] will display a dot if any of the pages that aren't
   * selected have this property set to `TRUE`.
   */
  page_props[PAGE_PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:keyword: (attributes org.gtk.Property.get=adw_tab_page_get_keyword org.gtk.Property.set=adw_tab_page_set_keyword)
   *
   * The search keyboard of the page.
   *
   * [class@TabOverview] can search pages by their keywords in addition to their
   * titles and tooltips.
   *
   * Keywords allow to include e.g. page URLs into tab search in a web browser.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_KEYWORD] =
    g_param_spec_string ("keyword", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:thumbnail-xalign: (attributes org.gtk.Property.get=adw_tab_page_get_thumbnail_xalign org.gtk.Property.set=adw_tab_page_set_thumbnail_xalign)
   *
   * The horizontal alignment of the page thumbnail.
   *
   * If the page is so wide that [class@TabOverview] can't display it completely
   * and has to crop it, horizontal alignment will determine which part of the
   * page will be visible.
   *
   * For example, 0.5 means the center of the page will be visible, 0 means the
   * start edge will be visible and 1 means the end edge will be visible.
   *
   * The default horizontal alignment is 0.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_THUMBNAIL_XALIGN] =
    g_param_spec_float ("thumbnail-xalign", NULL, NULL,
                        0.0, 1.0,
                        0.0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:thumbnail-yalign: (attributes org.gtk.Property.get=adw_tab_page_get_thumbnail_yalign org.gtk.Property.set=adw_tab_page_set_thumbnail_yalign)
   *
   * The vertical alignment of the page thumbnail.
   *
   * If the page is so tall that [class@TabOverview] can't display it completely
   * and has to crop it, vertical alignment will determine which part of the
   * page will be visible.
   *
   * For example, 0.5 means the center of the page will be visible, 0 means the
   * top edge will be visible and 1 means the bottom edge will be visible.
   *
   * The default vertical alignment is 0.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_THUMBNAIL_YALIGN] =
    g_param_spec_float ("thumbnail-yalign", NULL, NULL,
                        0.0, 1.0,
                        0.0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabPage:live-thumbnail: (attributes org.gtk.Property.get=adw_tab_page_get_live_thumbnail org.gtk.Property.set=adw_tab_page_set_live_thumbnail)
   *
   * Whether to enable live thumbnail for this page.
   *
   * When set to `TRUE`, the page's thumbnail in [class@TabOverview] will update
   * immediately when the page is redrawn or resized.
   *
   * If it's set to `FALSE`, the thumbnail will only be live when the page is
   * selected, and otherwise it will be static and will only update when
   * [method@TabPage.invalidate_thumbnail] or
   * [method@TabView.invalidate_thumbnails] is called.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_LIVE_THUMBNAIL] =
    g_param_spec_boolean ("live-thumbnail", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);

  g_object_class_override_property (object_class, PAGE_PROP_ACCESSIBLE_ROLE, "accessible-role");
}

static void
adw_tab_page_init (AdwTabPage *self)
{
  self->title = g_strdup ("");
  self->tooltip = g_strdup ("");
  self->indicator_tooltip = g_strdup ("");
  self->thumbnail_xalign = 0;
  self->thumbnail_yalign = 0;
  self->bin = g_object_ref_sink (adw_bin_new ());
  gtk_accessible_set_accessible_parent (GTK_ACCESSIBLE (self->bin),
                                        GTK_ACCESSIBLE (self), NULL);
}

static GtkATContext *
adw_tab_page_accessible_get_at_context (GtkAccessible *accessible)
{
  AdwTabPage *self = ADW_TAB_PAGE (accessible);

  if (self->in_destruction)
    return NULL;

  if (self->at_context == NULL) {
    GtkAccessibleRole role = GTK_ACCESSIBLE_ROLE_TAB_PANEL;
    GdkDisplay *display;

    if (self->bin != NULL)
      display = gtk_widget_get_display (self->bin);
    else
      display = gdk_display_get_default ();

    self->at_context = gtk_at_context_create (role, accessible, display);

    if (self->at_context == NULL)
      return NULL;
  }

  return g_object_ref (self->at_context);
}

static gboolean
adw_tab_page_accessible_get_platform_state (GtkAccessible              *self,
                                            GtkAccessiblePlatformState  state)
{
  return FALSE;
}

static GtkAccessible *
adw_tab_page_accessible_get_accessible_parent (GtkAccessible *accessible)
{
  AdwTabPage *self = ADW_TAB_PAGE (accessible);
  GtkWidget *parent;

  if (!self->bin)
    return NULL;

  parent = gtk_widget_get_parent (self->bin);

  return GTK_ACCESSIBLE (g_object_ref (parent));
}

static GtkAccessible *
adw_tab_page_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  AdwTabPage *self = ADW_TAB_PAGE (accessible);

  if (self->bin)
    return GTK_ACCESSIBLE (g_object_ref (self->bin));

  return NULL;
}

static GtkAccessible *
adw_tab_page_accessible_get_next_accessible_sibling (GtkAccessible *accessible)
{
  AdwTabPage *self = ADW_TAB_PAGE (accessible);
  GtkWidget *view = gtk_widget_get_parent (self->bin);
  AdwTabPage *next_page;
  int pos;

  if (!ADW_TAB_VIEW (view))
    return NULL;

  pos = adw_tab_view_get_page_position (ADW_TAB_VIEW (view), self);

  if (pos >= adw_tab_view_get_n_pages (ADW_TAB_VIEW (view)) - 1)
    return NULL;

  next_page = adw_tab_view_get_nth_page (ADW_TAB_VIEW (view), pos + 1);

  return GTK_ACCESSIBLE (g_object_ref (next_page));
}

static gboolean
adw_tab_page_accessible_get_bounds (GtkAccessible *accessible,
                                    int           *x,
                                    int           *y,
                                    int           *width,
                                    int           *height)
{
  AdwTabPage *self = ADW_TAB_PAGE (accessible);

  if (self->bin)
    return gtk_accessible_get_bounds (GTK_ACCESSIBLE (self->bin), x, y, width, height);

  return FALSE;
}

static void
adw_tab_page_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_at_context = adw_tab_page_accessible_get_at_context;
  iface->get_platform_state = adw_tab_page_accessible_get_platform_state;
  iface->get_accessible_parent = adw_tab_page_accessible_get_accessible_parent;
  iface->get_first_accessible_child = adw_tab_page_accessible_get_first_accessible_child;
  iface->get_next_accessible_sibling = adw_tab_page_accessible_get_next_accessible_sibling;
  iface->get_bounds = adw_tab_page_accessible_get_bounds;
}

#define ADW_TYPE_TAB_PAINTABLE (adw_tab_paintable_get_type ())

G_DECLARE_FINAL_TYPE (AdwTabPaintable, adw_tab_paintable, ADW, TAB_PAINTABLE, GObject)

struct _AdwTabPaintable
{
  GObject parent_instance;

  GtkWidget *view;
  AdwTabPage *page;

  GdkPaintable *view_paintable;
  GdkPaintable *child_paintable;

  GdkPaintable *cached_paintable;
  double cached_aspect_ratio;

  gboolean frozen;

  double last_xalign;
  double last_yalign;
};

static void
get_background_color (AdwTabPaintable *self,
                      GdkRGBA         *rgba)
{
  GtkWidget *child = adw_tab_page_get_child (self->page);

  if (adw_widget_lookup_color (child, "window_bg_color", rgba))
    return;

  rgba->red = 1;
  rgba->green = 1;
  rgba->blue = 1;
  rgba->alpha = 1;
}

static void
get_empty_color (AdwTabPaintable *self,
                 GdkRGBA         *rgba)
{
  GtkWidget *child = adw_tab_page_get_child (self->page);

  if (adw_widget_lookup_color (child, "thumbnail_bg_color", rgba))
    return;

  rgba->red = 1;
  rgba->green = 1;
  rgba->blue = 1;
  rgba->alpha = 1;
}

static void
transform_thumbnail (GtkSnapshot *snapshot,
                     double       width,
                     double       height,
                     double       child_ratio,
                     double       xalign,
                     double       yalign,
                     double      *adjusted_width,
                     double      *adjusted_height)
{
  double snapshot_ratio = width / height;
  double new_width, new_height;

  new_width = width;
  new_height = height;

  if (child_ratio > snapshot_ratio) {
    new_width = height * child_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT ((float) (width - new_width) * xalign, 0));
  } else if (child_ratio < snapshot_ratio) {
    new_height = width / child_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT (0, (float) (height - new_height) * yalign));
  }

  if (adjusted_width)
    *adjusted_width = new_width;
  if (adjusted_height)
    *adjusted_height = new_height;
}

static double
get_unclamped_aspect_ratio (AdwTabPaintable *self)
{
  if (!self->view_paintable)
    return self->cached_aspect_ratio;

  return gdk_paintable_get_intrinsic_aspect_ratio (self->view_paintable);
}

static void
snapshot_default_icon (AdwTabPaintable *self,
                       GtkSnapshot     *snapshot,
                       double           width,
                       double           height)
{
  GdkDisplay *display;
  GtkIconTheme *icon_theme;
  GIcon *default_icon;
  GtkIconPaintable *icon;
  GdkRGBA colors[4];
  GdkRGBA bg;
  double x, y;
  double view_width, view_height;
  double view_ratio, snapshot_ratio;
  double icon_size;
  gboolean hc;

  get_empty_color (self, &bg);
  gtk_snapshot_append_color (snapshot, &bg,
                             &GRAPHENE_RECT_INIT (0, 0, width, height));

  view_width = gtk_widget_get_width (self->view);
  view_height = gtk_widget_get_height (self->view);

  view_ratio = view_width / view_height;
  snapshot_ratio = width / height;

  if (view_ratio > snapshot_ratio) {
    double new_width = height * view_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT ((float) (width - new_width) / 2, 0));

    width = new_width;
  } else if (view_ratio < snapshot_ratio) {
    double new_height = width / view_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT (0, (float) (height - new_height) / 2));

    height = new_height;
  }

  icon_size = MIN (view_width / 4, view_height / 4);

  display = gtk_widget_get_display (self->view);
  icon_theme = gtk_icon_theme_get_for_display (display);
  default_icon = adw_tab_view_get_default_icon (ADW_TAB_VIEW (self->view));
  icon = gtk_icon_theme_lookup_by_gicon (icon_theme, default_icon, icon_size,
                                         gtk_widget_get_scale_factor (self->view),
                                         gtk_widget_get_direction (self->view),
                                         GTK_ICON_LOOKUP_FORCE_SYMBOLIC);

  gtk_widget_get_color (self->view, &colors[GTK_SYMBOLIC_COLOR_FOREGROUND]);
  adw_widget_lookup_color (self->view, "error-color", &colors[GTK_SYMBOLIC_COLOR_ERROR]);
  adw_widget_lookup_color (self->view, "warning-color", &colors[GTK_SYMBOLIC_COLOR_WARNING]);
  adw_widget_lookup_color (self->view, "success-color", &colors[GTK_SYMBOLIC_COLOR_SUCCESS]);

  hc = adw_style_manager_get_high_contrast (adw_style_manager_get_for_display (display));

  gtk_snapshot_push_opacity (snapshot, hc ? DEFAULT_ICON_ALPHA_HC : DEFAULT_ICON_ALPHA);

  gtk_snapshot_scale (snapshot, width / view_width, height / view_height);

  x = (view_width - icon_size) / 2;
  y = (view_height - icon_size) / 2;
  gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (x, y));

  gtk_symbolic_paintable_snapshot_symbolic (GTK_SYMBOLIC_PAINTABLE (icon),
                                            snapshot,
                                            icon_size,
                                            icon_size,
                                            colors,
                                            4);

  gtk_snapshot_pop (snapshot);
}

static GdkTexture *
render_contents (AdwTabPaintable *self,
                 gboolean         empty)
{
  GtkSnapshot *snapshot;
  GskRenderNode *node;
  double aspect_ratio;
  int scale_factor, width, height;
  GtkNative *native;
  GskRenderer *renderer;
  graphene_rect_t bounds;
  GdkTexture *ret;

  if (self->frozen)
    return NULL;

  aspect_ratio = get_unclamped_aspect_ratio (self);

  if (G_APPROX_VALUE (aspect_ratio, 0, DBL_EPSILON))
    return NULL;

  scale_factor = gtk_widget_get_scale_factor (self->view);
  snapshot = gtk_snapshot_new ();

  if (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio < MIN_THUMBNAIL_BITMAP_HEIGHT) {
    height = MIN_THUMBNAIL_BITMAP_HEIGHT * scale_factor;
    width = ceil (height * aspect_ratio) * scale_factor;
  } else if (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio > MAX_THUMBNAIL_BITMAP_HEIGHT) {
    height = MAX_THUMBNAIL_BITMAP_HEIGHT * scale_factor;
    width = ceil (height * aspect_ratio) * scale_factor;
  } else {
    width = MAX_THUMBNAIL_BITMAP_WIDTH * scale_factor;
    height = ceil (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio) * scale_factor;
  }

  if (width < MIN_THUMBNAIL_BITMAP_WIDTH * scale_factor) {
    width = MIN_THUMBNAIL_BITMAP_WIDTH * scale_factor;
    height = ceil (MIN_THUMBNAIL_BITMAP_WIDTH / aspect_ratio) * scale_factor;
  }

  if (empty) {
    snapshot_default_icon (self, snapshot, width, height);
  } else {
    GdkPaintable *current_paintable;
    GdkRGBA background;

    get_background_color (self, &background);

    gtk_snapshot_append_color (snapshot, &background,
                               &GRAPHENE_RECT_INIT (0, 0, width, height));

    current_paintable = gdk_paintable_get_current_image (self->child_paintable);
    gdk_paintable_snapshot (current_paintable, snapshot, width, height);

    g_object_unref (current_paintable);
  }

  node = gtk_snapshot_free_to_node (snapshot);

  if (!node)
    return NULL;

  native = gtk_widget_get_native (self->view);
  renderer = gtk_native_get_renderer (native);

  graphene_rect_init (&bounds, 0, 0, width, height);
  ret = gsk_renderer_render_texture (renderer, node, &bounds);

  gsk_render_node_unref (node);

  return ret;
}

static void
invalidate_texture (AdwTabPaintable *self)
{
  GdkTexture *texture;
  double old_aspect_ratio;

  if (!self->page->bin || !gtk_widget_get_mapped (self->page->bin))
    return;

  if (self->view) {
    AdwTabView *view = ADW_TAB_VIEW (self->view);

    if (!view->overview_count) {
      adw_tab_page_invalidate_thumbnail (self->page);
      return;
    }
  }

  texture = render_contents (self, FALSE);

  if (!texture)
    return;

  g_clear_object (&self->cached_paintable);
  self->cached_paintable = GDK_PAINTABLE (texture);

  old_aspect_ratio = self->cached_aspect_ratio;
  self->cached_aspect_ratio = get_unclamped_aspect_ratio (self);

  if (G_APPROX_VALUE (old_aspect_ratio, self->cached_aspect_ratio, DBL_EPSILON))
    gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
  else
    gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
}

static void
invalidate_size_cb (AdwTabPaintable *self)
{
  if (!self->cached_paintable)
    self->cached_aspect_ratio = get_unclamped_aspect_ratio (self);

  gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
}

static void
connect_to_view (AdwTabPaintable *self)
{
  if (self->view || !gtk_widget_get_parent (self->page->bin))
    return;

  self->view = gtk_widget_get_parent (self->page->bin);
  self->view_paintable = gtk_widget_paintable_new (self->view);

  g_signal_connect_swapped (self->view_paintable, "invalidate-size",
                            G_CALLBACK (invalidate_size_cb), self);
}

static void
disconnect_from_view (AdwTabPaintable *self)
{
  g_clear_object (&self->view_paintable);
  self->view = NULL;
}

static void
child_parent_changed (AdwTabPaintable *self)
{
  disconnect_from_view (self);
  connect_to_view (self);
}

static double
adw_tab_paintable_get_intrinsic_aspect_ratio (GdkPaintable *paintable)
{
  AdwTabPaintable *self = ADW_TAB_PAINTABLE (paintable);
  double ratio;

  if (self->view_paintable)
    ratio = gdk_paintable_get_intrinsic_aspect_ratio (self->view_paintable);
  else
    ratio = self->cached_aspect_ratio;

  return CLAMP (ratio, MIN_ASPECT_RATIO, MAX_ASPECT_RATIO);
}

static GdkPaintable *
adw_tab_paintable_get_current_image (GdkPaintable *paintable)
{
  AdwTabPaintable *self = ADW_TAB_PAINTABLE (paintable);
  GtkSnapshot *snapshot = gtk_snapshot_new ();
  int width, height;

  if (!self->view)
    return NULL;

  width = gtk_widget_get_width (self->view);
  height = gtk_widget_get_height (self->view);

  gdk_paintable_snapshot (paintable, GDK_SNAPSHOT (snapshot), width, height);

  return gtk_snapshot_free_to_paintable (snapshot,
                                         &GRAPHENE_SIZE_INIT (width, height));
}

static void
adw_tab_paintable_snapshot (GdkPaintable *paintable,
                            GdkSnapshot  *snapshot,
                            double        width,
                            double        height)
{
  AdwTabPaintable *self = ADW_TAB_PAINTABLE (paintable);
  GtkWidget *child;
  double xalign, yalign;

  if (self->frozen) {
    xalign = self->last_xalign;
    yalign = self->last_yalign;
    child = NULL;
  } else {
    xalign = adw_tab_page_get_thumbnail_xalign (self->page);
    yalign = adw_tab_page_get_thumbnail_yalign (self->page);
    child = self->page->bin;

    if (gtk_widget_get_direction (child) == GTK_TEXT_DIR_RTL)
      xalign = 1 - xalign;
  }

  if (!self->cached_paintable) {
    snapshot_default_icon (self, snapshot, width, height);
    return;
  }

  transform_thumbnail (snapshot, width, height, self->cached_aspect_ratio,
                       xalign, yalign, &width, &height);

  gdk_paintable_snapshot (self->cached_paintable, snapshot, width, height);
}

static void
adw_tab_paintable_iface_init (GdkPaintableInterface *iface)
{
  iface->get_intrinsic_aspect_ratio = adw_tab_paintable_get_intrinsic_aspect_ratio;
  iface->get_current_image = adw_tab_paintable_get_current_image;
  iface->snapshot = adw_tab_paintable_snapshot;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwTabPaintable, adw_tab_paintable, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GDK_TYPE_PAINTABLE, adw_tab_paintable_iface_init))

static void
adw_tab_paintable_dispose (GObject *object)
{
  AdwTabPaintable *self = ADW_TAB_PAINTABLE (object);

  disconnect_from_view (self);

  g_clear_object (&self->child_paintable);
  g_clear_object (&self->cached_paintable);

  G_OBJECT_CLASS (adw_tab_paintable_parent_class)->dispose (object);
}

static void
adw_tab_paintable_class_init (AdwTabPaintableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_tab_paintable_dispose;
}

static void
adw_tab_paintable_init (AdwTabPaintable *self)
{
}

static GdkPaintable *
adw_tab_paintable_new (AdwTabPage *page)
{
  AdwTabPaintable *self = g_object_new (ADW_TYPE_TAB_PAINTABLE, NULL);

  self->page = page;

  connect_to_view (self);

  self->child_paintable = gtk_widget_paintable_new (page->bin);

  g_signal_connect_swapped (self->child_paintable, "invalidate-contents",
                            G_CALLBACK (invalidate_texture), self);

  g_signal_connect_object (self->page, "notify::thumbnail-xalign",
                           G_CALLBACK (gdk_paintable_invalidate_contents), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->page, "notify::thumbnail-yalign",
                           G_CALLBACK (gdk_paintable_invalidate_contents), self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (page->bin, "notify::parent",
                           G_CALLBACK (child_parent_changed), self,
                           G_CONNECT_SWAPPED);

  return GDK_PAINTABLE (self);
}

static void
adw_tab_paintable_freeze (AdwTabPaintable *self)
{
  self->last_xalign = adw_tab_page_get_thumbnail_xalign (self->page);
  self->last_yalign = adw_tab_page_get_thumbnail_yalign (self->page);

  if (!self->cached_paintable)
    self->cached_paintable = GDK_PAINTABLE (render_contents (self, TRUE));

  if (gtk_widget_get_direction (self->page->bin) == GTK_TEXT_DIR_RTL)
    self->last_xalign = 1 - self->last_xalign;

  self->frozen = TRUE;

  g_clear_object (&self->child_paintable);
}

#define ADW_TYPE_TAB_PAGES (adw_tab_pages_get_type ())

G_DECLARE_FINAL_TYPE (AdwTabPages, adw_tab_pages, ADW, TAB_PAGES, GObject)

struct _AdwTabPages
{
  GObject parent_instance;

  AdwTabView *view;
};

static GType
adw_tab_pages_get_item_type (GListModel *model)
{
  return ADW_TYPE_TAB_PAGE;
}

static guint
adw_tab_pages_get_n_items (GListModel *model)
{
  AdwTabPages *self = ADW_TAB_PAGES (model);

  if (G_UNLIKELY (!ADW_IS_TAB_VIEW (self->view)))
    return 0;

  return self->view->n_pages;
}

static gpointer
adw_tab_pages_get_item (GListModel *model,
                        guint       position)
{
  AdwTabPages *self = ADW_TAB_PAGES (model);
  AdwTabPage *page;

  if (G_UNLIKELY (!ADW_IS_TAB_VIEW (self->view)))
    return NULL;

  page = adw_tab_view_get_nth_page (self->view, position);

  if (!page)
    return NULL;

  return g_object_ref (page);
}

static void
adw_tab_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_tab_pages_get_item_type;
  iface->get_n_items = adw_tab_pages_get_n_items;
  iface->get_item = adw_tab_pages_get_item;
}

static gboolean
adw_tab_pages_is_selected (GtkSelectionModel *model,
                           guint              position)
{
  AdwTabPages *self = ADW_TAB_PAGES (model);
  AdwTabPage *page;

  if (G_UNLIKELY (!ADW_IS_TAB_VIEW (self->view)))
    return FALSE;

  page = adw_tab_view_get_nth_page (self->view, position);

  return page->selected;
}

static gboolean
adw_tab_pages_select_item (GtkSelectionModel *model,
                           guint              position,
                           gboolean           exclusive)
{
  AdwTabPages *self = ADW_TAB_PAGES (model);
  AdwTabPage *page;

  if (G_UNLIKELY (!ADW_IS_TAB_VIEW (self->view)))
    return FALSE;

  page = adw_tab_view_get_nth_page (self->view, position);

  adw_tab_view_set_selected_page (self->view, page);

  return TRUE;
}

static void
adw_tab_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = adw_tab_pages_is_selected;
  iface->select_item = adw_tab_pages_select_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwTabPages, adw_tab_pages, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_tab_pages_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_tab_pages_selection_model_init))

static void
adw_tab_pages_init (AdwTabPages *self)
{
}

static void
adw_tab_pages_dispose (GObject *object)
{
  AdwTabPages *self = ADW_TAB_PAGES (object);

  g_clear_weak_pointer (&self->view);

  G_OBJECT_CLASS (adw_tab_pages_parent_class)->dispose (object);
}

static void
adw_tab_pages_class_init (AdwTabPagesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_tab_pages_dispose;
}

static GtkSelectionModel *
adw_tab_pages_new (AdwTabView *view)
{
  AdwTabPages *pages;

  pages = g_object_new (ADW_TYPE_TAB_PAGES, NULL);
  g_set_weak_pointer (&pages->view, view);

  return GTK_SELECTION_MODEL (pages);
}

static gboolean
object_handled_accumulator (GSignalInvocationHint *ihint,
                            GValue                *return_accu,
                            const GValue          *handler_return,
                            gpointer               data)
{
  GObject *object = g_value_get_object (handler_return);

  g_value_set_object (return_accu, object);

  return !object;
}

static void
begin_transfer_for_group (AdwTabView *self)
{
  GSList *l;

  for (l = tab_view_list; l; l = l->next) {
    AdwTabView *view = l->data;

    view->transfer_count++;

    if (view->transfer_count == 1)
      g_object_notify_by_pspec (G_OBJECT (view), props[PROP_IS_TRANSFERRING_PAGE]);
  }
}

static void
end_transfer_for_group (AdwTabView *self)
{
  GSList *l;

  for (l = tab_view_list; l; l = l->next) {
    AdwTabView *view = l->data;

    view->transfer_count--;

    if (view->transfer_count == 0)
      g_object_notify_by_pspec (G_OBJECT (view), props[PROP_IS_TRANSFERRING_PAGE]);
  }
}

static void
set_n_pages (AdwTabView *self,
             int         n_pages)
{
  if (n_pages == self->n_pages)
    return;

  self->n_pages = n_pages;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
set_n_pinned_pages (AdwTabView *self,
                    int         n_pinned_pages)
{
  if (n_pinned_pages == self->n_pinned_pages)
    return;

  self->n_pinned_pages = n_pinned_pages;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PINNED_PAGES]);
}

static inline gboolean
page_belongs_to_this_view (AdwTabView *self,
                           AdwTabPage *page)
{
  if (!page)
    return FALSE;

  return gtk_widget_get_parent (page->bin) == GTK_WIDGET (self);
}

static inline gboolean
child_belongs_to_this_view (AdwTabView *self,
                            GtkWidget  *child)
{
  GtkWidget *parent;

  if (!child)
    return FALSE;

  parent = gtk_widget_get_parent (child);

  if (!parent)
    return FALSE;

  return gtk_widget_get_parent (parent) == GTK_WIDGET (self);
}

static inline gboolean
is_descendant_of (AdwTabPage *page,
                  AdwTabPage *parent)
{
  while (page && page != parent)
    page = adw_tab_page_get_parent (page);

  return page == parent;
}

static void
attach_page (AdwTabView *self,
             AdwTabPage *page,
             int         position)
{
  AdwTabPage *parent;

  g_list_store_insert (self->children, position, page);

  gtk_widget_set_child_visible (page->bin,
                                page_should_be_visible (self, page));
  gtk_widget_set_parent (page->bin, GTK_WIDGET (self));
  page->transfer_binding =
    g_object_bind_property (self, "is-transferring-page",
                            page->bin, "can-target",
                            G_BINDING_SYNC_CREATE |
                            G_BINDING_INVERT_BOOLEAN);
  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_freeze_notify (G_OBJECT (self));

  set_n_pages (self, self->n_pages + 1);

  if (adw_tab_page_get_pinned (page))
    set_n_pinned_pages (self, self->n_pinned_pages + 1);

  g_object_thaw_notify (G_OBJECT (self));

  parent = adw_tab_page_get_parent (page);

  if (parent && !page_belongs_to_this_view (self, parent))
    set_page_parent (page, NULL);

  g_signal_emit (self, signals[SIGNAL_PAGE_ATTACHED], 0, page, position);
}

static void
set_selected_page (AdwTabView *self,
                   AdwTabPage *selected_page,
                   gboolean    notify_pages)
{
  guint old_position = GTK_INVALID_LIST_POSITION;
  guint new_position = GTK_INVALID_LIST_POSITION;
  gboolean contains_focus = FALSE;

  if (self->selected_page == selected_page)
    return;

  if (self->selected_page) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
    GtkWidget *focus = root ? gtk_root_get_focus (root) : NULL;

    if (notify_pages && self->pages)
      old_position = adw_tab_view_get_page_position (self, self->selected_page);

    if (!gtk_widget_in_destruction (GTK_WIDGET (self)) &&
        focus &&
        self->selected_page &&
        self->selected_page->bin &&
        gtk_widget_is_ancestor (focus, self->selected_page->bin)) {
      contains_focus = TRUE;

      if (self->selected_page->last_focus)
        g_object_remove_weak_pointer (G_OBJECT (self->selected_page->last_focus),
                                      (gpointer *) &self->selected_page->last_focus);
      self->selected_page->last_focus = focus;
      g_object_add_weak_pointer (G_OBJECT (self->selected_page->last_focus),
                                 (gpointer *) &self->selected_page->last_focus);
    }

    if (self->selected_page->bin && selected_page) {
      gtk_widget_set_child_visible (self->selected_page->bin,
                                    page_should_be_visible (self, self->selected_page));
    }

    set_page_selected (self->selected_page, FALSE);
  }

  self->selected_page = selected_page;

  if (self->selected_page) {
    if (notify_pages && self->pages)
      new_position = adw_tab_view_get_page_position (self, self->selected_page);

    if (!gtk_widget_in_destruction (GTK_WIDGET (self))) {
      gtk_widget_set_child_visible (selected_page->bin, TRUE);

      if (contains_focus) {
        if (selected_page->last_focus)
          gtk_widget_grab_focus (selected_page->last_focus);
        else
          gtk_widget_child_focus (selected_page->bin, GTK_DIR_TAB_FORWARD);
      }

      gtk_widget_queue_allocate (GTK_WIDGET (self));
    }

    set_page_selected (self->selected_page, TRUE);
  }

  if (notify_pages && self->pages) {
    if (old_position == GTK_INVALID_LIST_POSITION && new_position == GTK_INVALID_LIST_POSITION)
      ; /* nothing to do */
    else if (old_position == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, new_position, 1);
    else if (new_position == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, old_position, 1);
    else
      gtk_selection_model_selection_changed (self->pages,
                                             MIN (old_position, new_position),
                                             MAX (old_position, new_position) -
                                             MIN (old_position, new_position) + 1);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_PAGE]);
}

static void
select_previous_page (AdwTabView *self,
                      AdwTabPage *page)
{
  int pos = adw_tab_view_get_page_position (self, page);
  AdwTabPage *parent;

  if (page != self->selected_page)
    return;

  parent = adw_tab_page_get_parent (page);

  if (parent && pos > 0) {
    AdwTabPage *prev_page = adw_tab_view_get_nth_page (self, pos - 1);

    /* This usually means we opened a few pages from the same page in a row, or
     * the previous page is the parent. Switch there. */
    if (is_descendant_of (prev_page, parent)) {
      adw_tab_view_set_selected_page (self, prev_page);

      return;
    }

    /* Pinned pages are special in that opening a page from a pinned parent
     * will place it not directly after the parent, but after the last pinned
     * page. This means that if we're closing the first non-pinned page, we need
     * to jump to the parent directly instead of the previous page which might
     * be different. */
    if (adw_tab_page_get_pinned (prev_page) &&
        adw_tab_page_get_pinned (parent)) {
      adw_tab_view_set_selected_page (self, parent);

      return;
    }
  }

  if (adw_tab_view_select_next_page (self))
    return;

  adw_tab_view_select_previous_page (self);
}

static void
detach_page (AdwTabView *self,
             AdwTabPage *page,
             gboolean    in_dispose)
{
  int pos = adw_tab_view_get_page_position (self, page);

  select_previous_page (self, page);

  g_object_ref (self);
  g_object_ref (page);
  g_object_ref (page->bin);

  if (self->n_pages == 1)
    set_selected_page (self, NULL, !in_dispose);

  g_list_store_remove (self->children, pos);

  g_object_freeze_notify (G_OBJECT (self));

  set_n_pages (self, self->n_pages - 1);

  if (adw_tab_page_get_pinned (page))
    set_n_pinned_pages (self, self->n_pinned_pages - 1);

  g_object_thaw_notify (G_OBJECT (self));

  g_clear_pointer (&page->transfer_binding, g_binding_unbind);
  gtk_widget_unparent (page->bin);

  if (!in_dispose)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_signal_emit (self, signals[SIGNAL_PAGE_DETACHED], 0, page, pos);

  if (!in_dispose && self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), pos, 1, 0);

  g_object_unref (page->bin);
  g_object_unref (page);
  g_object_unref (self);
}

static void
insert_page (AdwTabView *self,
             AdwTabPage *page,
             int         position)
{
  attach_page (self, page, position);

  g_object_freeze_notify (G_OBJECT (self));

  if (!self->selected_page)
    set_selected_page (self, page, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 0, 1);

  g_object_thaw_notify (G_OBJECT (self));
}

static AdwTabPage *
create_and_insert_page (AdwTabView *self,
                        GtkWidget  *child,
                        AdwTabPage *parent,
                        int         position,
                        gboolean    pinned)
{
  AdwTabPage *page =
    g_object_new (ADW_TYPE_TAB_PAGE,
                  "child", child,
                  "parent", parent,
                  NULL);

  set_page_pinned (page, pinned);

  insert_page (self, page, position);

  g_object_unref (page);

  return page;
}

static gboolean
close_page_cb (AdwTabView *self,
               AdwTabPage *page)
{
  adw_tab_view_close_page_finish (self, page,
                                  !adw_tab_page_get_pinned (page));

  return GDK_EVENT_STOP;
}

static gboolean
select_page_cb (GtkWidget  *widget,
                GVariant   *args,
                AdwTabView *self)
{
  AdwTabViewShortcuts mask;
  GtkDirectionType direction;
  gboolean last, success = FALSE;

  if (!adw_tab_view_get_selected_page (self))
    return GDK_EVENT_PROPAGATE;

  if (self->n_pages <= 1)
    return GDK_EVENT_PROPAGATE;

  g_variant_get (args, "(hhb)", &mask, &direction, &last);

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = adw_tab_view_select_first_page (self);
    else
      success = adw_tab_view_select_previous_page (self);

    if (!success && !last) {
      AdwTabPage *page = adw_tab_view_get_nth_page (self, self->n_pages - 1);

      adw_tab_view_set_selected_page (self, page);

      success = TRUE;
    }
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = adw_tab_view_select_last_page (self);
    else
      success = adw_tab_view_select_next_page (self);

    if (!success && !last) {
      AdwTabPage *page = adw_tab_view_get_nth_page (self, 0);

      adw_tab_view_set_selected_page (self, page);

      success = TRUE;
    }
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));

  return GDK_EVENT_STOP;
}

static inline void
add_switch_shortcut (AdwTabView          *self,
                     GtkEventController  *controller,
                     AdwTabViewShortcuts  mask,
                     guint                keysym,
                     guint                keypad_keysym,
                     GdkModifierType      modifiers,
                     GtkDirectionType     direction,
                     gboolean             last)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, modifiers),
                                         gtk_keyval_trigger_new (keypad_keysym, modifiers));
  action = gtk_callback_action_new ((GtkShortcutFunc) select_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new ("(hhb)", mask, direction, last));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static gboolean
reorder_page_cb (GtkWidget  *widget,
                 GVariant   *args,
                 AdwTabView *self)
{
  AdwTabViewShortcuts mask;
  GtkDirectionType direction;
  gboolean last, success = FALSE;
  AdwTabPage *page = adw_tab_view_get_selected_page (self);

  if (!page)
    return GDK_EVENT_PROPAGATE;

  if (self->n_pages <= 1)
    return GDK_EVENT_PROPAGATE;

  g_variant_get (args, "(hhb)", &mask, &direction, &last);

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = adw_tab_view_reorder_first (self, page);
    else
      success = adw_tab_view_reorder_backward (self, page);
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = adw_tab_view_reorder_last (self, page);
    else
      success = adw_tab_view_reorder_forward (self, page);
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));

  return GDK_EVENT_STOP;
}

static inline void
add_reorder_shortcut (AdwTabView          *self,
                      GtkEventController  *controller,
                      AdwTabViewShortcuts  mask,
                      guint                keysym,
                      guint                keypad_keysym,
                      GtkDirectionType     direction,
                      gboolean             last)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                                         gtk_keyval_trigger_new (keypad_keysym, GDK_CONTROL_MASK | GDK_SHIFT_MASK));
  action = gtk_callback_action_new ((GtkShortcutFunc) reorder_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new ("(hhb)", mask, direction, last));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static gboolean
select_nth_page_cb (GtkWidget  *widget,
                    GVariant   *args,
                    AdwTabView *self)
{
  gint8 n_page = g_variant_get_byte (args);
  AdwTabViewShortcuts mask;
  AdwTabPage *page;

  if (n_page >= self->n_pages)
    return GDK_EVENT_PROPAGATE;

  /* Pages are counted from 0, so page 9 represents Alt+0 */
  if (n_page == 9)
    mask = ADW_TAB_VIEW_SHORTCUT_ALT_ZERO;
  else
    mask = ADW_TAB_VIEW_SHORTCUT_ALT_DIGITS;

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  page = adw_tab_view_get_nth_page (self, n_page);
  if (adw_tab_view_get_selected_page (self) == page)
    return GDK_EVENT_PROPAGATE;

  adw_tab_view_set_selected_page (self, page);

  return GDK_EVENT_STOP;
}

static inline void
add_switch_nth_page_shortcut (AdwTabView         *self,
                              GtkEventController *controller,
                              guint               keysym,
                              guint               keypad_keysym,
                              int                 n_page)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, GDK_ALT_MASK),
                                         gtk_keyval_trigger_new (keypad_keysym, GDK_ALT_MASK));
  action = gtk_callback_action_new ((GtkShortcutFunc) select_nth_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new_byte (n_page));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static void
init_shortcuts (AdwTabView         *self,
                GtkEventController *controller)
{
  int i;

  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_TAB,
                       GDK_KEY_Tab, GDK_KEY_KP_Tab, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, FALSE);
  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB,
                       GDK_KEY_Tab, GDK_KEY_KP_Tab, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                       GTK_DIR_TAB_BACKWARD, FALSE);
  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP,
                       GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_BACKWARD, FALSE);
  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN,
                       GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, FALSE);
  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_HOME,
                       GDK_KEY_Home, GDK_KEY_KP_Home, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_BACKWARD, TRUE);
  add_switch_shortcut (self, controller,
                       ADW_TAB_VIEW_SHORTCUT_CONTROL_END,
                       GDK_KEY_End, GDK_KEY_KP_End, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, TRUE);

  add_reorder_shortcut (self, controller,
                        ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP,
                        GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
                        GTK_DIR_TAB_BACKWARD, FALSE);
  add_reorder_shortcut (self, controller,
                        ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN,
                        GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
                        GTK_DIR_TAB_FORWARD, FALSE);
  add_reorder_shortcut (self, controller,
                        ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME,
                        GDK_KEY_Home, GDK_KEY_KP_Home,
                        GTK_DIR_TAB_BACKWARD, TRUE);
  add_reorder_shortcut (self, controller,
                        ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END,
                        GDK_KEY_End, GDK_KEY_KP_End,
                        GTK_DIR_TAB_FORWARD,  TRUE);

  for (i = 0; i < 10; i++)
    add_switch_nth_page_shortcut (self,
                                  controller,
                                  GDK_KEY_0 + i,
                                  GDK_KEY_KP_0 + i,
                                  (i + 9) % 10); /* Alt+0 means page 10, not 0 */
}

static void
adw_tab_view_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  AdwTabView *self = ADW_TAB_VIEW (widget);
  int i;

  *minimum = 0;
  *natural = 0;

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);
    int child_min, child_nat;

    gtk_widget_measure (page->bin, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);
  }
}

static void
adw_tab_view_size_allocate (GtkWidget *widget,
                            int        width,
                            int        height,
                            int        baseline)
{
  AdwTabView *self = ADW_TAB_VIEW (widget);
  int i;

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (gtk_widget_get_child_visible (page->bin))
      gtk_widget_allocate (page->bin, width, height, baseline, NULL);
  }
}

static void
unmap_extra_pages (AdwTabView *self)
{
  int i;

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (page == self->selected_page)
      continue;

    if (!gtk_widget_get_child_visible (page->bin))
      continue;

    if (page_should_be_visible (self, page))
      continue;

    gtk_widget_set_child_visible (page->bin, FALSE);
  }

  self->unmap_extra_pages_cb = 0;
}

static void
adw_tab_view_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  AdwTabView *self = ADW_TAB_VIEW (widget);
  int i;

  if (self->selected_page)
    gtk_widget_snapshot_child (widget, self->selected_page->bin, snapshot);

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (!gtk_widget_get_child_visible (page->bin))
      continue;

    if (page->paintable) {
      if (page == self->selected_page && page->invalidated)
        gtk_widget_queue_draw (page->bin);

      /* We don't want to actually draw the child, but we do need it
       * to redraw so that it can be displayed by its paintable */
      GtkSnapshot *child_snapshot = gtk_snapshot_new ();

      gtk_widget_snapshot_child (widget, page->bin, child_snapshot);

      g_object_unref (child_snapshot);
    }

    page->invalidated = FALSE;

    if (!self->unmap_extra_pages_cb)
      self->unmap_extra_pages_cb =
        g_idle_add_once ((GSourceOnceFunc) unmap_extra_pages, self);
  }
}

static void
draw_overview_pages_after_map_cb (AdwTabView *self)
{
  int i;

  if (!self->overview_count)
    return;

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (page->live_thumbnail || page->invalidated)
      gtk_widget_set_child_visible (page->bin, TRUE);
    else if (page == self->selected_page)
      gtk_widget_queue_draw (GTK_WIDGET (page->bin));
  }

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
adw_tab_view_map (GtkWidget *widget)
{
  AdwTabView *self = ADW_TAB_VIEW (widget);

  GTK_WIDGET_CLASS (adw_tab_view_parent_class)->map (widget);

  if (self->overview_count)
    g_idle_add_once ((GSourceOnceFunc) draw_overview_pages_after_map_cb, self);
}

static void
adw_tab_view_dispose (GObject *object)
{
  AdwTabView *self = ADW_TAB_VIEW (object);

  if (self->unmap_extra_pages_cb) {
    g_source_remove (self->unmap_extra_pages_cb);
    self->unmap_extra_pages_cb = 0;
  }

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), 0, self->n_pages, 0);

  while (self->n_pages) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, 0);

    detach_page (self, page, TRUE);
  }

  g_clear_object (&self->children);

  G_OBJECT_CLASS (adw_tab_view_parent_class)->dispose (object);
}

static void
adw_tab_view_finalize (GObject *object)
{
  AdwTabView *self = (AdwTabView *) object;

  if (self->pages)
    g_object_remove_weak_pointer (G_OBJECT (self->pages),
                                  (gpointer *) &self->pages);

  g_clear_object (&self->default_icon);
  g_clear_object (&self->menu_model);

  tab_view_list = g_slist_remove (tab_view_list, self);

  G_OBJECT_CLASS (adw_tab_view_parent_class)->finalize (object);
}

static void
adw_tab_view_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwTabView *self = ADW_TAB_VIEW (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_int (value, adw_tab_view_get_n_pages (self));
    break;

  case PROP_N_PINNED_PAGES:
    g_value_set_int (value, adw_tab_view_get_n_pinned_pages (self));
    break;

  case PROP_IS_TRANSFERRING_PAGE:
    g_value_set_boolean (value, adw_tab_view_get_is_transferring_page (self));
    break;

  case PROP_SELECTED_PAGE:
    g_value_set_object (value, adw_tab_view_get_selected_page (self));
    break;

  case PROP_DEFAULT_ICON:
    g_value_set_object (value, adw_tab_view_get_default_icon (self));
    break;

  case PROP_MENU_MODEL:
    g_value_set_object (value, adw_tab_view_get_menu_model (self));
    break;

  case PROP_SHORTCUTS:
    g_value_set_flags (value, adw_tab_view_get_shortcuts (self));
    break;

  case PROP_PAGES:
    g_value_take_object (value, adw_tab_view_get_pages (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_view_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwTabView *self = ADW_TAB_VIEW (object);

  switch (prop_id) {
  case PROP_SELECTED_PAGE:
    adw_tab_view_set_selected_page (self, g_value_get_object (value));
    break;

  case PROP_DEFAULT_ICON:
    adw_tab_view_set_default_icon (self, g_value_get_object (value));
    break;

  case PROP_MENU_MODEL:
    adw_tab_view_set_menu_model (self, g_value_get_object (value));
    break;

  case PROP_SHORTCUTS:
    adw_tab_view_set_shortcuts (self, g_value_get_flags (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_view_class_init (AdwTabViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_view_dispose;
  object_class->finalize = adw_tab_view_finalize;
  object_class->get_property = adw_tab_view_get_property;
  object_class->set_property = adw_tab_view_set_property;

  widget_class->measure = adw_tab_view_measure;
  widget_class->size_allocate = adw_tab_view_size_allocate;
  widget_class->snapshot = adw_tab_view_snapshot;
  widget_class->map = adw_tab_view_map;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwTabView:n-pages: (attributes org.gtk.Property.get=adw_tab_view_get_n_pages)
   *
   * The number of pages in the tab view.
   */
  props[PROP_N_PAGES] =
    g_param_spec_int ("n-pages", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabView:n-pinned-pages: (attributes org.gtk.Property.get=adw_tab_view_get_n_pinned_pages)
   *
   * The number of pinned pages in the tab view.
   *
   * See [method@TabView.set_page_pinned].
   */
  props[PROP_N_PINNED_PAGES] =
    g_param_spec_int ("n-pinned-pages", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabView:is-transferring-page: (attributes org.gtk.Property.get=adw_tab_view_get_is_transferring_page)
   *
   * Whether a page is being transferred.
   *
   * This property will be set to `TRUE` when a drag-n-drop tab transfer starts
   * on any `AdwTabView`, and to `FALSE` after it ends.
   *
   * During the transfer, children cannot receive pointer input and a tab can
   * be safely dropped on the tab view.
   */
  props[PROP_IS_TRANSFERRING_PAGE] =
    g_param_spec_boolean ("is-transferring-page", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwTabView:selected-page: (attributes org.gtk.Property.get=adw_tab_view_get_selected_page org.gtk.Property.set=adw_tab_view_set_selected_page)
   *
   * The currently selected page.
   */
  props[PROP_SELECTED_PAGE] =
    g_param_spec_object ("selected-page", NULL, NULL,
                         ADW_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabView:default-icon: (attributes org.gtk.Property.get=adw_tab_view_get_default_icon org.gtk.Property.set=adw_tab_view_set_default_icon)
   *
   * Default page icon.
   *
   * If a page doesn't provide its own icon via [property@TabPage:icon], a
   * default icon may be used instead for contexts where having an icon is
   * necessary.
   *
   * [class@TabBar] will use default icon for pinned tabs in case the page is
   * not loading, doesn't have an icon and an indicator. Default icon is never
   * used for tabs that aren't pinned.
   *
   * [class@TabOverview] will use default icon for pages with missing
   * thumbnails.
   *
   * By default, the `adw-tab-icon-missing-symbolic` icon is used.
   */
  props[PROP_DEFAULT_ICON] =
    g_param_spec_object ("default-icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabView:menu-model: (attributes org.gtk.Property.get=adw_tab_view_get_menu_model org.gtk.Property.set=adw_tab_view_set_menu_model)
   *
   * Tab context menu model.
   *
   * When a context menu is shown for a tab, it will be constructed from the
   * provided menu model. Use the [signal@TabView::setup-menu] signal to set up
   * the menu actions for the particular tab.
   */
  props[PROP_MENU_MODEL] =
    g_param_spec_object ("menu-model", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabView:shortcuts: (attributes org.gtk.Property.get=adw_tab_view_get_shortcuts org.gtk.Property.set=adw_tab_view_set_shortcuts)
   *
   * The enabled shortcuts.
   *
   * See [flags@TabViewShortcuts] for the list of the available shortcuts. All
   * of the shortcuts are enabled by default.
   *
   * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts]
   * provide a convenient way to manage individual shortcuts.
   *
   * Since: 1.2
   */
  props[PROP_SHORTCUTS] =
    g_param_spec_flags ("shortcuts", NULL, NULL,
                        ADW_TYPE_TAB_VIEW_SHORTCUTS,
                        ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTabView:pages: (attributes org.gtk.Property.get=adw_tab_view_get_pages)
   *
   * A selection model with the tab view's pages.
   *
   * This can be used to keep an up-to-date view. The model also implements
   * [iface@Gtk.SelectionModel] and can be used to track and change the selected
   * page.
   */
  props[PROP_PAGES] =
    g_param_spec_object ("pages", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwTabView::page-attached:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position of the page, starting from 0
   *
   * Emitted when a page has been created or transferred to @self.
   *
   * A typical reason to connect to this signal would be to connect to page
   * signals for things such as updating window title.
   */
  signals[SIGNAL_PAGE_ATTACHED] =
    g_signal_new ("page-attached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  ADW_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_ATTACHED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECT_INTv);

  /**
   * AdwTabView::page-detached:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position of the removed page, starting from 0
   *
   * Emitted when a page has been removed or transferred to another view.
   *
   * A typical reason to connect to this signal would be to disconnect signal
   * handlers connected in the [signal@TabView::page-attached] handler.
   *
   * It is important not to try and destroy the page child in the handler of
   * this function as the child might merely be moved to another window; use
   * child dispose handler for that or do it in sync with your
   * [method@TabView.close_page_finish] calls.
   */
  signals[SIGNAL_PAGE_DETACHED] =
    g_signal_new ("page-detached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  ADW_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_DETACHED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECT_INTv);

  /**
   * AdwTabView::page-reordered:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position @page was moved to, starting at 0
   *
   * Emitted after @page has been reordered to @position.
   */
  signals[SIGNAL_PAGE_REORDERED] =
    g_signal_new ("page-reordered",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  ADW_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_REORDERED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECT_INTv);

  /**
   * AdwTabView::close-page:
   * @self: a tab view
   * @page: a page of @self
   *
   * Emitted after [method@TabView.close_page] has been called for @page.
   *
   * The handler is expected to call [method@TabView.close_page_finish] to
   * confirm or reject the closing.
   *
   * The default handler will immediately confirm closing for non-pinned pages,
   * or reject it for pinned pages, equivalent to the following example:
   *
   * ```c
   * static gboolean
   * close_page_cb (AdwTabView *view,
   *                AdwTabPage *page,
   *                gpointer    user_data)
   * {
   *   adw_tab_view_close_page_finish (view, page, !adw_tab_page_get_pinned (page));
   *
   *   return GDK_EVENT_STOP;
   * }
   * ```
   *
   * The [method@TabView.close_page_finish] call doesn't have to happen inside
   * the handler, so can be used to do asynchronous checks before confirming the
   * closing.
   *
   * A typical reason to connect to this signal is to show a confirmation dialog
   * for closing a tab.
   *
   * The signal handler should return `GDK_EVENT_STOP` to stop propagation or
   * `GDK_EVENT_CONTINUE` to invoke the default handler.
   *
   * Returns: whether propagation should be stopped
   */
  signals[SIGNAL_CLOSE_PAGE] =
    g_signal_new ("close-page",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_true_handled,
                  NULL,
                  adw_marshal_BOOLEAN__OBJECT,
                  G_TYPE_BOOLEAN,
                  1,
                  ADW_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_PAGE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_BOOLEAN__OBJECTv);

  /**
   * AdwTabView::setup-menu:
   * @self: a tab view
   * @page: (nullable): a page of @self
   *
   * Emitted when a context menu is opened or closed for @page.
   *
   * If the menu has been closed, @page will be set to `NULL`.
   *
   * It can be used to set up menu actions before showing the menu, for example
   * disable actions not applicable to @page.
   */
  signals[SIGNAL_SETUP_MENU] =
    g_signal_new ("setup-menu",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_SETUP_MENU],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECTv);

  /**
   * AdwTabView::create-window:
   * @self: a tab view
   *
   * Emitted when a tab should be transferred into a new window.
   *
   * This can happen after a tab has been dropped on desktop.
   *
   * The signal handler is expected to create a new window, position it as
   * needed and return its `AdwTabView` that the page will be transferred into.
   *
   * Returns: (transfer none) (nullable): the `AdwTabView` from the new window
   */
  signals[SIGNAL_CREATE_WINDOW] =
    g_signal_new ("create-window",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  object_handled_accumulator,
                  NULL,
                  adw_marshal_OBJECT__VOID,
                  ADW_TYPE_TAB_VIEW,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CREATE_WINDOW],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_OBJECT__VOIDv);

  /**
   * AdwTabView::indicator-activated:
   * @self: a tab view
   * @page: a page of @self
   *
   * Emitted after the indicator icon on @page has been activated.
   *
   * See [property@TabPage:indicator-icon] and
   * [property@TabPage:indicator-activatable].
   */
  signals[SIGNAL_INDICATOR_ACTIVATED] =
    g_signal_new ("indicator-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_INDICATOR_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECTv);

  g_signal_override_class_handler ("close-page",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (close_page_cb));

  gtk_widget_class_set_css_name (widget_class, "tabview");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_tab_view_init (AdwTabView *self)
{
  GtkEventController *controller;

  self->children = g_list_store_new (ADW_TYPE_TAB_PAGE);
  self->default_icon = G_ICON (g_themed_icon_new ("adw-tab-icon-missing-symbolic"));
  self->shortcuts = ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS;

  tab_view_list = g_slist_prepend (tab_view_list, self);

  controller = gtk_shortcut_controller_new ();
  gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (controller),
                                     GTK_SHORTCUT_SCOPE_GLOBAL);

  init_shortcuts (self, controller);

  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

static void
adw_tab_view_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  AdwTabView *self = ADW_TAB_VIEW (buildable);

  if (!type && GTK_IS_WIDGET (child))
    adw_tab_view_append (self, GTK_WIDGET (child));
  else if (!type && ADW_IS_TAB_PAGE (child))
    insert_page (self, ADW_TAB_PAGE (child), self->n_pages);
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_tab_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_tab_view_buildable_add_child;
}

static GtkAccessible *
adw_tab_view_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  AdwTabView *self = ADW_TAB_VIEW (accessible);

  if (adw_tab_view_get_n_pages (self) > 0)
    return GTK_ACCESSIBLE (g_object_ref (adw_tab_view_get_nth_page (self, 0)));

  return NULL;
}

static void
adw_tab_view_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_first_accessible_child = adw_tab_view_accessible_get_first_accessible_child;
}

/**
 * adw_tab_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a tab page
 *
 * Gets the child of @self.
 *
 * Returns: (transfer none): the child of @self
 */
GtkWidget *
adw_tab_page_get_child (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->child;
}

/**
 * adw_tab_page_get_parent: (attributes org.gtk.Method.get_property=parent)
 * @self: a tab page
 *
 * Gets the parent page of @self.
 *
 * See [method@TabView.add_page] and [method@TabView.close_page].
 *
 * Returns: (transfer none) (nullable): the parent page
 */
AdwTabPage *
adw_tab_page_get_parent (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->parent;
}

/**
 * adw_tab_page_get_selected: (attributes org.gtk.Method.get_property=selected)
 * @self: a tab page
 *
 * Gets whether @self is selected.
 *
 * Returns: whether @self is selected
 */
gboolean
adw_tab_page_get_selected (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->selected;
}

/**
 * adw_tab_page_get_pinned: (attributes org.gtk.Method.get_property=pinned)
 * @self: a tab page
 *
 * Gets whether @self is pinned.
 *
 * See [method@TabView.set_page_pinned].
 *
 * Returns: whether @self is pinned
 */
gboolean
adw_tab_page_get_pinned (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->pinned;
}

/**
 * adw_tab_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a tab page
 *
 * Gets the title of @self.
 *
 * Returns: the title of @self
 */
const char *
adw_tab_page_get_title (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->title;
}

/**
 * adw_tab_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a tab page
 * @title: the title of @self
 *
 * [class@TabBar] will display it in the center of the tab unless it's pinned,
 * and will use it as a tooltip unless [property@TabPage:tooltip] is set.
 *
 * [class@TabOverview] will display it below the thumbnail unless it's pinned,
 * or inside the card otherwise, and will use it as a tooltip unless
 * [property@TabPage:tooltip] is set.
 *
 * Sets the title of @self.
 */
void
adw_tab_page_set_title (AdwTabPage *self,
                        const char *title)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  if (!g_set_str (&self->title, title ? title : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, self->title,
                                  -1);
}

/**
 * adw_tab_page_get_tooltip: (attributes org.gtk.Method.get_property=tooltip)
 * @self: a tab page
 *
 * Gets the tooltip of @self.
 *
 * Returns: (nullable): the tooltip of @self
 */
const char *
adw_tab_page_get_tooltip (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->tooltip;
}

/**
 * adw_tab_page_set_tooltip: (attributes org.gtk.Method.set_property=tooltip)
 * @self: a tab page
 * @tooltip: the tooltip of @self
 *
 * Sets the tooltip of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * If not set, [class@TabBar] and [class@TabOverview] will use
 * [property@TabPage:title] as a tooltip instead.
 */
void
adw_tab_page_set_tooltip (AdwTabPage *self,
                          const char *tooltip)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  if (!g_set_str (&self->tooltip, tooltip ? tooltip : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TOOLTIP]);
}

/**
 * adw_tab_page_get_icon: (attributes org.gtk.Method.get_property=icon)
 * @self: a tab page
 *
 * Gets the icon of @self.
 *
 * Returns: (transfer none) (nullable): the icon of @self
 */
GIcon *
adw_tab_page_get_icon (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->icon;
}

/**
 * adw_tab_page_set_icon: (attributes org.gtk.Method.set_property=icon)
 * @self: a tab page
 * @icon: (nullable): the icon of @self
 *
 * Sets the icon of @self.
 *
 * [class@TabBar] and [class@TabOverview] display the icon next to the title,
 * unless [property@TabPage:loading] is set to `TRUE`.
 *
 * `AdwTabBar` also won't show the icon if the page is pinned and
 * [propertyTabPage:indicator-icon] is set.
 */
void
adw_tab_page_set_icon (AdwTabPage *self,
                       GIcon      *icon)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));
  g_return_if_fail (icon == NULL || G_IS_ICON (icon));

  if (!g_set_object (&self->icon, icon))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ICON]);
}

/**
 * adw_tab_page_get_loading: (attributes org.gtk.Method.get_property=loading)
 * @self: a tab page
 *
 * Gets whether @self is loading.
 *
 * Returns: whether @self is loading
 */
gboolean
adw_tab_page_get_loading (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->loading;
}

/**
 * adw_tab_page_set_loading: (attributes org.gtk.Method.set_property=loading)
 * @self: a tab page
 * @loading: whether @self is loading
 *
 * Sets whether @self is loading.
 *
 * If set to `TRUE`, [class@TabBar] and [class@TabOverview] will display a
 * spinner in place of icon.
 *
 * If the page is pinned and [property@TabPage:indicator-icon] is set, loading
 * status will not be visible with `AdwTabBar`.
 */
void
adw_tab_page_set_loading (AdwTabPage *self,
                          gboolean    loading)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  loading = !!loading;

  if (self->loading == loading)
    return;

  self->loading = loading;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_LOADING]);
}

/**
 * adw_tab_page_get_indicator_icon: (attributes org.gtk.Method.get_property=indicator-icon)
 * @self: a tab page
 *
 * Gets the indicator icon of @self.
 *
 * Returns: (transfer none) (nullable): the indicator icon of @self
 */
GIcon *
adw_tab_page_get_indicator_icon (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->indicator_icon;
}

/**
 * adw_tab_page_set_indicator_icon: (attributes org.gtk.Method.set_property=indicator-icon)
 * @self: a tab page
 * @indicator_icon: (nullable): the indicator icon of @self
 *
 * Sets the indicator icon of @self.
 *
 * A common use case is an audio or camera indicator in a web browser.
 *
 * [class@TabBar] will show it at the beginning of the tab, alongside icon
 * representing [property@TabPage:icon] or loading spinner.
 *
 * If the page is pinned, the indicator will be shown instead of icon or
 * spinner.
 *
 * [class@TabOverview] will show it at the at the top part of the thumbnail.
 *
 * [property@TabPage:indicator-tooltip] can be used to set the tooltip on the
 * indicator icon.
 *
 * If [property@TabPage:indicator-activatable] is set to `TRUE`, the
 * indicator icon can act as a button.
 */
void
adw_tab_page_set_indicator_icon (AdwTabPage *self,
                                 GIcon      *indicator_icon)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));
  g_return_if_fail (indicator_icon == NULL || G_IS_ICON (indicator_icon));

  if (!g_set_object (&self->indicator_icon, indicator_icon))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_ICON]);
}

/**
 * adw_tab_page_get_indicator_tooltip: (attributes org.gtk.Method.get_property=indicator-tooltip)
 * @self: a tab page
 *
 * Gets the tooltip of the indicator icon of @self.
 *
 * Returns: (transfer none): the indicator tooltip of @self
 *
 * Since: 1.2
 */
const char *
adw_tab_page_get_indicator_tooltip (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->indicator_tooltip;
}

/**
 * adw_tab_page_set_indicator_tooltip: (attributes org.gtk.Method.set_property=indicator-tooltip)
 * @self: a tab page
 * @tooltip: the indicator tooltip of @self
 *
 * Sets the tooltip of the indicator icon of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * See [property@TabPage:indicator-icon].
 *
 * Since: 1.2
 */
void
adw_tab_page_set_indicator_tooltip (AdwTabPage *self,
                                    const char *tooltip)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));
  g_return_if_fail (tooltip != NULL);

  if (!g_set_str (&self->indicator_tooltip, tooltip ? tooltip : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_TOOLTIP]);
}

/**
 * adw_tab_page_get_indicator_activatable: (attributes org.gtk.Method.get_property=indicator-activatable)
 * @self: a tab page
 *
 *
 * Gets whether the indicator of @self is activatable.
 *
 * Returns: whether the indicator is activatable
 */
gboolean
adw_tab_page_get_indicator_activatable (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->indicator_activatable;
}

/**
 * adw_tab_page_set_indicator_activatable: (attributes org.gtk.Method.set_property=indicator-activatable)
 * @self: a tab page
 * @activatable: whether the indicator is activatable
 *
 * Sets whether the indicator of @self is activatable.
 *
 * If set to `TRUE`, [signal@TabView::indicator-activated] will be emitted
 * when the indicator icon is clicked.
 *
 * If [property@TabPage:indicator-icon] is not set, does nothing.
 */
void
adw_tab_page_set_indicator_activatable (AdwTabPage *self,
                                        gboolean    activatable)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  activatable = !!activatable;

  if (self->indicator_activatable == activatable)
    return;

  self->indicator_activatable = activatable;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_ACTIVATABLE]);
}

/**
 * adw_tab_page_get_needs_attention: (attributes org.gtk.Method.get_property=needs-attention)
 * @self: a tab page
 *
 * Gets whether @self needs attention.
 *
 * Returns: whether @self needs attention
 */
gboolean
adw_tab_page_get_needs_attention (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->needs_attention;
}

/**
 * adw_tab_page_set_needs_attention: (attributes org.gtk.Method.set_property=needs-attention)
 * @self: a tab page
 * @needs_attention: whether @self needs attention
 *
 * Sets whether @self needs attention.
 *
 * [class@TabBar] will display a line under the tab representing the page if
 * set to `TRUE`. If the tab is not visible, the corresponding edge of the tab
 * bar will be highlighted.
 *
 * [class@TabOverview] will display a dot in the corner of the thumbnail if set
 * to `TRUE`.
 *
 * [class@TabButton] will display a dot if any of the pages that aren't
 * selected have [property@TabPage:needs-attention] set to `TRUE`.
 */
void
adw_tab_page_set_needs_attention (AdwTabPage *self,
                                  gboolean    needs_attention)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  needs_attention = !!needs_attention;

  if (self->needs_attention == needs_attention)
    return;

  self->needs_attention = needs_attention;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NEEDS_ATTENTION]);
}

/**
 * adw_tab_page_get_keyword: (attributes org.gtk.Method.get_property=keyword)
 * @self: a tab page
 *
 * Gets the search keyword of @self.
 *
 * Returns: (nullable): the search keyword of @self
 *
 * Since: 1.3
 */
const char *
adw_tab_page_get_keyword (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  return self->keyword;
}

/**
 * adw_tab_page_set_keyword: (attributes org.gtk.Method.set_property=keyword)
 * @self: a tab page
 * @keyword: the search keyword
 *
 * Sets the search keyword for @self.
 *
 * [class@TabOverview] can search pages by their keywords in addition to their
 * titles and tooltips.
 *
 * Keywords allow to include e.g. page URLs into tab search in a web browser.
 *
 * Since: 1.3
 */
void
adw_tab_page_set_keyword (AdwTabPage *self,
                          const char *keyword)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  if (!g_set_str (&self->keyword, keyword))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_KEYWORD]);
}

/**
 * adw_tab_page_get_thumbnail_xalign: (attributes org.gtk.Method.get_property=thumbnail-xalign)
 * @self: a tab page
 *
 * Gets the horizontal alignment of the thumbnail for @self.
 *
 * Returns: the horizontal alignment
 *
 * Since: 1.3
 */
float
adw_tab_page_get_thumbnail_xalign (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), 0.0f);

  return self->thumbnail_xalign;
}

/**
 * adw_tab_page_set_thumbnail_xalign: (attributes org.gtk.Method.set_property=thumbnail-xalign)
 * @self: a tab page
 * @xalign: the new value
 *
 * Sets the horizontal alignment of the thumbnail for @self.
 *
 * If the page is so wide that [class@TabOverview] can't display it completely
 * and has to crop it, horizontal alignment will determine which part of the
 * page will be visible.
 *
 * For example, 0.5 means the center of the page will be visible, 0 means the
 * start edge will be visible and 1 means the end edge will be visible.
 *
 * The default horizontal alignment is 0.
 *
 * Since: 1.3
 */
void
adw_tab_page_set_thumbnail_xalign (AdwTabPage *self,
                                   float       xalign)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  xalign = CLAMP (xalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->thumbnail_xalign, xalign, FLT_EPSILON))
    return;

  self->thumbnail_xalign = xalign;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_THUMBNAIL_XALIGN]);
}

/**
 * adw_tab_page_get_thumbnail_yalign: (attributes org.gtk.Method.get_property=thumbnail-yalign)
 * @self: a tab overview
 *
 * Gets the vertical alignment of the thumbnail for @self.
 *
 * Returns: the vertical alignment
 *
 * Since: 1.3
 */
float
adw_tab_page_get_thumbnail_yalign (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), 0.0f);

  return self->thumbnail_yalign;
}

/**
 * adw_tab_page_set_thumbnail_yalign: (attributes org.gtk.Method.set_property=thumbnail-yalign)
 * @self: a tab page
 * @yalign: the new value
 *
 * Sets the vertical alignment of the thumbnail for @self.
 *
 * If the page is so tall that [class@TabOverview] can't display it completely
 * and has to crop it, vertical alignment will determine which part of the page
 * will be visible.
 *
 * For example, 0.5 means the center of the page will be visible, 0 means the
 * top edge will be visible and 1 means the bottom edge will be visible.
 *
 * The default vertical alignment is 0.
 *
 * Since: 1.3
 */
void
adw_tab_page_set_thumbnail_yalign (AdwTabPage *self,
                                   float       yalign)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  yalign = CLAMP (yalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->thumbnail_yalign, yalign, FLT_EPSILON))
    return;

  self->thumbnail_yalign = yalign;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_THUMBNAIL_YALIGN]);
}

/**
 * adw_tab_page_get_live_thumbnail: (attributes org.gtk.Method.get_property=live-thumbnail)
 * @self: a tab overview
 *
 * Gets whether to live thumbnail is enabled @self.
 *
 * Returns: whether live thumbnail is enabled
 *
 * Since: 1.3
 */
gboolean
adw_tab_page_get_live_thumbnail (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), FALSE);

  return self->live_thumbnail;
}

/**
 * adw_tab_page_set_live_thumbnail: (attributes org.gtk.Method.set_property=live-thumbnail)
 * @self: a tab page
 * @live_thumbnail: whether to enable live thumbnail
 *
 * Sets whether to enable live thumbnail for @self.
 *
 * When set to `TRUE`, @self's thumbnail in [class@TabOverview] will update
 * immediately when @self is redrawn or resized.
 *
 * If it's set to `FALSE`, the thumbnail will only be live when the @self is
 * selected, and otherwise it will be static and will only update when
 * [method@TabPage.invalidate_thumbnail] or
 * [method@TabView.invalidate_thumbnails] is called.
 *
 * Since: 1.3
 */
void
adw_tab_page_set_live_thumbnail (AdwTabPage *self,
                                 gboolean    live_thumbnail)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  live_thumbnail = !!live_thumbnail;

  if (self->live_thumbnail == live_thumbnail)
    return;

  self->live_thumbnail = live_thumbnail;

  map_or_unmap_page (self);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_LIVE_THUMBNAIL]);
}


/**
 * adw_tab_page_invalidate_thumbnail:
 * @self: a tab page
 *
 * Invalidates thumbnail for @self.
 *
 * If an [class@TabOverview] is open, the thumbnail representing @self will be
 * immediately updated. Otherwise it will be update when opening the overview.
 *
 * Does nothing if [property@TabPage:live-thumbnail] is set to `TRUE`.
 *
 * See also [method@TabView.invalidate_thumbnails].
 *
 * Since: 1.3
 */
void
adw_tab_page_invalidate_thumbnail (AdwTabPage *self)
{
  g_return_if_fail (ADW_IS_TAB_PAGE (self));

  if (self->invalidated)
    return;

  self->invalidated = TRUE;

  map_or_unmap_page (self);
}

GdkPaintable *
adw_tab_page_get_paintable (AdwTabPage *self)
{
  g_return_val_if_fail (ADW_IS_TAB_PAGE (self), NULL);

  if (!self->paintable)
    self->paintable = adw_tab_paintable_new (self);

  return self->paintable;
}

/**
 * adw_tab_view_new:
 *
 * Creates a new `AdwTabView`.
 *
 * Returns: the newly created `AdwTabView`
 */
AdwTabView *
adw_tab_view_new (void)
{
  return g_object_new (ADW_TYPE_TAB_VIEW, NULL);
}

/**
 * adw_tab_view_get_n_pages: (attributes org.gtk.Method.get_property=n-pages)
 * @self: a tab view
 *
 * Gets the number of pages in @self.
 *
 * Returns: the number of pages in @self
 */
int
adw_tab_view_get_n_pages (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), 0);

  return self->n_pages;
}

/**
 * adw_tab_view_get_n_pinned_pages: (attributes org.gtk.Method.get_property=n-pinned-pages)
 * @self: a tab view
 *
 * Gets the number of pinned pages in @self.
 *
 * See [method@TabView.set_page_pinned].
 *
 * Returns: the number of pinned pages in @self
 */
int
adw_tab_view_get_n_pinned_pages (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), 0);

  return self->n_pinned_pages;
}

/**
 * adw_tab_view_get_is_transferring_page: (attributes org.gtk.Method.get_property=is-transferring-page)
 * @self: a tab view
 *
 * Whether a page is being transferred.
 *
 * The corresponding property will be set to `TRUE` when a drag-n-drop tab
 * transfer starts on any `AdwTabView`, and to `FALSE` after it ends.
 *
 * During the transfer, children cannot receive pointer input and a tab can
 * be safely dropped on the tab view.
 *
 * Returns: whether a page is being transferred
 */
gboolean
adw_tab_view_get_is_transferring_page (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);

  return self->transfer_count > 0;
}

/**
 * adw_tab_view_get_selected_page: (attributes org.gtk.Method.get_property=selected-page)
 * @self: a tab view
 *
 * Gets the currently selected page in @self.
 *
 * Returns: (transfer none) (nullable): the selected page
 */
AdwTabPage *
adw_tab_view_get_selected_page (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);

  return self->selected_page;
}

/**
 * adw_tab_view_set_selected_page: (attributes org.gtk.Method.set_property=selected-page)
 * @self: a tab view
 * @selected_page: a page in @self
 *
 * Sets the currently selected page in @self.
 */
void
adw_tab_view_set_selected_page (AdwTabView *self,
                                AdwTabPage *selected_page)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));

  if (self->n_pages > 0) {
    g_return_if_fail (ADW_IS_TAB_PAGE (selected_page));
    g_return_if_fail (page_belongs_to_this_view (self, selected_page));
  } else {
    g_return_if_fail (selected_page == NULL);
  }

  set_selected_page (self, selected_page, TRUE);
}

/**
 * adw_tab_view_select_previous_page:
 * @self: a tab view
 *
 * Selects the page before the currently selected page.
 *
 * If the first page was already selected, this function does nothing.
 *
 * Returns: whether the selected page was changed
 */
gboolean
adw_tab_view_select_previous_page (AdwTabView *self)
{
  AdwTabPage *page;
  int pos;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pos = adw_tab_view_get_page_position (self, self->selected_page);

  if (pos <= 0)
    return FALSE;

  page = adw_tab_view_get_nth_page (self, pos - 1);

  adw_tab_view_set_selected_page (self, page);

  return TRUE;
}

/**
 * adw_tab_view_select_next_page:
 * @self: a tab view
 *
 * Selects the page after the currently selected page.
 *
 * If the last page was already selected, this function does nothing.
 *
 * Returns: whether the selected page was changed
 */
gboolean
adw_tab_view_select_next_page (AdwTabView *self)
{
  AdwTabPage *page;
  int pos;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pos = adw_tab_view_get_page_position (self, self->selected_page);

  if (pos >= self->n_pages - 1)
    return FALSE;

  page = adw_tab_view_get_nth_page (self, pos + 1);

  adw_tab_view_set_selected_page (self, page);

  return TRUE;
}

gboolean
adw_tab_view_select_first_page (AdwTabView *self)
{
  AdwTabPage *page;
  int pos;
  gboolean pinned;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pinned = adw_tab_page_get_pinned (self->selected_page);
  pos = pinned ? 0 : self->n_pinned_pages;

  page = adw_tab_view_get_nth_page (self, pos);

  /* If we're on the first non-pinned tab, go to the first pinned tab */
  if (page == self->selected_page && !pinned)
    page = adw_tab_view_get_nth_page (self, 0);

  if (page == self->selected_page)
    return FALSE;

  adw_tab_view_set_selected_page (self, page);

  return TRUE;
}

gboolean
adw_tab_view_select_last_page (AdwTabView *self)
{
  AdwTabPage *page;
  int pos;
  gboolean pinned;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pinned = adw_tab_page_get_pinned (self->selected_page);
  pos = (pinned ? self->n_pinned_pages : self->n_pages) - 1;

  page = adw_tab_view_get_nth_page (self, pos);

  /* If we're on the last pinned tab, go to the last non-pinned tab */
  if (page == self->selected_page && pinned)
    page = adw_tab_view_get_nth_page (self, self->n_pages - 1);

  if (page == self->selected_page)
    return FALSE;

  adw_tab_view_set_selected_page (self, page);

  return TRUE;
}

/**
 * adw_tab_view_get_default_icon: (attributes org.gtk.Method.get_property=default-icon)
 * @self: a tab view
 *
 * Gets the default icon of @self.
 *
 * Returns: (transfer none): the default icon of @self.
 */
GIcon *
adw_tab_view_get_default_icon (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);

  return self->default_icon;
}

/**
 * adw_tab_view_set_default_icon: (attributes org.gtk.Method.set_property=default-icon)
 * @self: a tab view
 * @default_icon: the default icon
 *
 * Sets the default page icon for @self.
 *
 * If a page doesn't provide its own icon via [property@TabPage:icon], a default
 * icon may be used instead for contexts where having an icon is necessary.
 *
 * [class@TabBar] will use default icon for pinned tabs in case the page is not
 * loading, doesn't have an icon and an indicator. Default icon is never used
 * for tabs that aren't pinned.
 *
 * [class@TabOverview] will use default icon for pages with missing thumbnails.
 *
 * By default, the `adw-tab-icon-missing-symbolic` icon is used.
 */
void
adw_tab_view_set_default_icon (AdwTabView *self,
                               GIcon      *default_icon)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (G_IS_ICON (default_icon));
  int i;

  if (self->default_icon == default_icon)
    return;

  g_set_object (&self->default_icon, default_icon);

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (page->paintable)
      gdk_paintable_invalidate_contents (page->paintable);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEFAULT_ICON]);
}

/**
 * adw_tab_view_get_menu_model: (attributes org.gtk.Method.get_property=menu-model)
 * @self: a tab view
 *
 * Gets the tab context menu model for @self.
 *
 * Returns: (transfer none) (nullable): the tab context menu model for @self
 */
GMenuModel *
adw_tab_view_get_menu_model (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);

  return self->menu_model;
}

/**
 * adw_tab_view_set_menu_model: (attributes org.gtk.Method.set_property=menu-model)
 * @self: a tab view
 * @menu_model: (nullable): a menu model
 *
 * Sets the tab context menu model for @self.
 *
 * When a context menu is shown for a tab, it will be constructed from the
 * provided menu model. Use the [signal@TabView::setup-menu] signal to set up
 * the menu actions for the particular tab.
 */
void
adw_tab_view_set_menu_model (AdwTabView *self,
                             GMenuModel *menu_model)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (menu_model == NULL || G_IS_MENU_MODEL (menu_model));

  if (self->menu_model == menu_model)
    return;

  g_set_object (&self->menu_model, menu_model);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MENU_MODEL]);
}

/**
 * adw_tab_view_get_shortcuts: (attributes org.gtk.Method.get_property=shortcuts)
 * @self: a tab view
 *
 * Gets the enabled shortcuts for @self.
 *
 * Returns: the shortcut mask
 *
 * Since: 1.2
 */
AdwTabViewShortcuts
adw_tab_view_get_shortcuts (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), 0);

  return self->shortcuts;
}

/**
 * adw_tab_view_set_shortcuts: (attributes org.gtk.Method.set_property=shortcuts)
 * @self: a tab view
 * @shortcuts: the new shortcuts
 *
 * Sets the enabled shortcuts for @self.
 *
 * See [flags@TabViewShortcuts] for the list of the available shortcuts. All of
 * the shortcuts are enabled by default.
 *
 * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts] provide
 * a convenient way to manage individual shortcuts.
 *
 * Since: 1.2
 */
void
adw_tab_view_set_shortcuts (AdwTabView          *self,
                            AdwTabViewShortcuts  shortcuts)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  if (self->shortcuts == shortcuts)
    return;

  self->shortcuts = shortcuts;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHORTCUTS]);
}

/**
 * adw_tab_view_add_shortcuts:
 * @self: a tab view
 * @shortcuts: the shortcuts to add
 *
 * Adds @shortcuts for @self.
 *
 * See [property@TabView:shortcuts] for details.
 *
 * Since: 1.2
 */
void
adw_tab_view_add_shortcuts (AdwTabView          *self,
                            AdwTabViewShortcuts  shortcuts)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  adw_tab_view_set_shortcuts (self, self->shortcuts | shortcuts);
}

/**
 * adw_tab_view_remove_shortcuts:
 * @self: a tab view
 * @shortcuts: the shortcuts to reomve
 *
 * Removes @shortcuts from @self.
 *
 * See [property@TabView:shortcuts] for details.
 *
 * Since: 1.2
 */
void
adw_tab_view_remove_shortcuts (AdwTabView          *self,
                               AdwTabViewShortcuts  shortcuts)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= ADW_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  adw_tab_view_set_shortcuts (self, self->shortcuts & ~shortcuts);
}

/**
 * adw_tab_view_set_page_pinned:
 * @self: a tab view
 * @page: a page of @self
 * @pinned: whether @page should be pinned
 *
 * Pins or unpins @page.
 *
 * Pinned pages are guaranteed to be placed before all non-pinned pages; at any
 * given moment the first [property@TabView:n-pinned-pages] pages in @self are
 * guaranteed to be pinned.
 *
 * When a page is pinned or unpinned, it's automatically reordered: pinning a
 * page moves it after other pinned pages; unpinning a page moves it before
 * other non-pinned pages.
 *
 * Pinned pages can still be reordered between each other.
 *
 * [class@TabBar] will display pinned pages in a compact form, never showing the
 * title or close button, and only showing a single icon, selected in the
 * following order:
 *
 * 1. [property@TabPage:indicator-icon]
 * 2. A spinner if [property@TabPage:loading] is `TRUE`
 * 3. [property@TabPage:icon]
 * 4. [property@TabView:default-icon]
 *
 * [class@TabOverview] will not show a thumbnail for pinned pages, and replace
 * the close button with an unpin button. Unlike `AdwTabBar`, it will still
 * display the page's title, icon and indicator separately.
 *
 * Pinned pages cannot be closed by default, see [signal@TabView::close-page]
 * for how to override that behavior.
 *
 * Changes the value of the [property@TabPage:pinned] property.
 */
void
adw_tab_view_set_page_pinned (AdwTabView *self,
                              AdwTabPage *page,
                              gboolean    pinned)
{
  int old_pos, new_pos;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  pinned = !!pinned;

  if (adw_tab_page_get_pinned (page) == pinned)
    return;

  old_pos = adw_tab_view_get_page_position (self, page);

  g_object_ref (page);

  g_list_store_remove (self->children, old_pos);

  new_pos = self->n_pinned_pages;

  if (!pinned)
    new_pos--;

  g_list_store_insert (self->children, new_pos, page);

  g_object_unref (page);

  set_n_pinned_pages (self, new_pos + (pinned ? 1 : 0));
  set_page_pinned (page, pinned);

  if (self->pages) {
    int min = MIN (old_pos, new_pos);
    int n_changed = MAX (old_pos, new_pos) - min + 1;

    g_list_model_items_changed (G_LIST_MODEL (self->pages), min, n_changed, n_changed);
  }
}

/**
 * adw_tab_view_get_page:
 * @self: a tab view
 * @child: a child in @self
 *
 * Gets the [class@TabPage] object representing @child.
 *
 * Returns: (transfer none): the page object for @child
 */
AdwTabPage *
adw_tab_view_get_page (AdwTabView *self,
                       GtkWidget  *child)
{
  int i;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (child_belongs_to_this_view (self, child), NULL);

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    if (adw_tab_page_get_child (page) == child)
      return page;
  }

  g_assert_not_reached ();
}

/**
 * adw_tab_view_get_nth_page:
 * @self: a tab view
 * @position: the index of the page in @self, starting from 0
 *
 * Gets the [class@TabPage] representing the child at @position.
 *
 * Returns: (transfer none): the page object at @position
 */
AdwTabPage *
adw_tab_view_get_nth_page (AdwTabView *self,
                           int         position)
{
  AdwTabPage *page;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (position >= 0, NULL);
  g_return_val_if_fail (position < self->n_pages, NULL);

  page = g_list_model_get_item (G_LIST_MODEL (self->children), (guint) position);

  g_object_unref (page);

  return page;
}

/**
 * adw_tab_view_get_page_position:
 * @self: a tab view
 * @page: a page of @self
 *
 * Finds the position of @page in @self, starting from 0.
 *
 * Returns: the position of @page in @self
 */
int
adw_tab_view_get_page_position (AdwTabView *self,
                                AdwTabPage *page)
{
  int i;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), -1);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), -1);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), -1);

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *p = adw_tab_view_get_nth_page (self, i);

    if (page == p)
      return i;
  }

  g_assert_not_reached ();
}

/**
 * adw_tab_view_add_page:
 * @self: a tab view
 * @child: a widget to add
 * @parent: (nullable): a parent page for @child
 *
 * Adds @child to @self with @parent as the parent.
 *
 * This function can be used to automatically position new pages, and to select
 * the correct page when this page is closed while being selected (see
 * [method@TabView.close_page]).
 *
 * If @parent is `NULL`, this function is equivalent to [method@TabView.append].
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_add_page (AdwTabView *self,
                       GtkWidget  *child,
                       AdwTabPage *parent)
{
  int position;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (parent == NULL || ADW_IS_TAB_PAGE (parent), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  if (parent) {
    AdwTabPage *page;

    g_return_val_if_fail (page_belongs_to_this_view (self, parent), NULL);

    if (adw_tab_page_get_pinned (parent))
      position = self->n_pinned_pages - 1;
    else
      position = adw_tab_view_get_page_position (self, parent);

    do {
      position++;

      if (position >= self->n_pages)
        break;

      page = adw_tab_view_get_nth_page (self, position);
    } while (is_descendant_of (page, parent));
  } else {
    position = self->n_pages;
  }

  return create_and_insert_page (self, child, parent, position, FALSE);
}

/**
 * adw_tab_view_insert:
 * @self: a tab view
 * @child: a widget to add
 * @position: the position to add @child at, starting from 0
 *
 * Inserts a non-pinned page at @position.
 *
 * It's an error to try to insert a page before a pinned page, in that case
 * [method@TabView.insert_pinned] should be used instead.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_insert (AdwTabView *self,
                     GtkWidget  *child,
                     int         position)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);
  g_return_val_if_fail (position >= self->n_pinned_pages, NULL);
  g_return_val_if_fail (position <= self->n_pages, NULL);

  return create_and_insert_page (self, child, NULL, position, FALSE);
}

/**
 * adw_tab_view_prepend:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the first non-pinned page.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_prepend (AdwTabView *self,
                      GtkWidget  *child)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, self->n_pinned_pages, FALSE);
}

/**
 * adw_tab_view_append:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the last non-pinned page.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_append (AdwTabView *self,
                     GtkWidget  *child)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, self->n_pages, FALSE);
}

/**
 * adw_tab_view_insert_pinned:
 * @self: a tab view
 * @child: a widget to add
 * @position: the position to add @child at, starting from 0
 *
 * Inserts a pinned page at @position.
 *
 * It's an error to try to insert a pinned page after a non-pinned page, in
 * that case [method@TabView.insert] should be used instead.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_insert_pinned (AdwTabView *self,
                            GtkWidget  *child,
                            int         position)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (position >= 0, NULL);
  g_return_val_if_fail (position <= self->n_pinned_pages, NULL);

  return create_and_insert_page (self, child, NULL, position, TRUE);
}

/**
 * adw_tab_view_prepend_pinned:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the first pinned page.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_prepend_pinned (AdwTabView *self,
                             GtkWidget  *child)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, 0, TRUE);
}

/**
 * adw_tab_view_append_pinned:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the last pinned page.
 *
 * Returns: (transfer none): the page object representing @child
 */
AdwTabPage *
adw_tab_view_append_pinned (AdwTabView *self,
                            GtkWidget  *child)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, self->n_pinned_pages, TRUE);
}

/**
 * adw_tab_view_close_page:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close @page.
 *
 * Calling this function will result in the [signal@TabView::close-page] signal
 * being emitted for @page. Closing the page can then be confirmed or
 * denied via [method@TabView.close_page_finish].
 *
 * If the page is waiting for a [method@TabView.close_page_finish] call, this
 * function will do nothing.
 *
 * The default handler for [signal@TabView::close-page] will immediately confirm
 * closing the page if it's non-pinned, or reject it if it's pinned. This
 * behavior can be changed by registering your own handler for that signal.
 *
 * If @page was selected, another page will be selected instead:
 *
 * If the [property@TabPage:parent] value is `NULL`, the next page will be
 * selected when possible, or if the page was already last, the previous page
 * will be selected instead.
 *
 * If it's not `NULL`, the previous page will be selected if it's a descendant
 * (possibly indirect) of the parent. If both the previous page and the parent
 * are pinned, the parent will be selected instead.
 */
void
adw_tab_view_close_page (AdwTabView *self,
                         AdwTabPage *page)
{
  gboolean ret;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  if (page->closing)
    return;

  page->closing = TRUE;
  g_signal_emit (self, signals[SIGNAL_CLOSE_PAGE], 0, page, &ret);
}

/**
 * adw_tab_view_close_page_finish:
 * @self: a tab view
 * @page: a page of @self
 * @confirm: whether to confirm or deny closing @page
 *
 * Completes a [method@TabView.close_page] call for @page.
 *
 * If @confirm is `TRUE`, @page will be closed. If it's `FALSE`, it will be
 * reverted to its previous state and [method@TabView.close_page] can be called
 * for it again.
 *
 * This function should not be called unless a custom handler for
 * [signal@TabView::close-page] is used.
 */
void
adw_tab_view_close_page_finish (AdwTabView *self,
                                AdwTabPage *page,
                                gboolean    confirm)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));
  g_return_if_fail (page->closing);

  page->closing = FALSE;

  if (!confirm)
    return;

  if (page->paintable)
    adw_tab_paintable_freeze (ADW_TAB_PAINTABLE (page->paintable));

  detach_page (self, page, FALSE);
}

/**
 * adw_tab_view_close_other_pages:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages other than @page.
 */
void
adw_tab_view_close_other_pages (AdwTabView *self,
                                AdwTabPage *page)
{
  int i;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  for (i = self->n_pages - 1; i >= 0; i--) {
    AdwTabPage *p = adw_tab_view_get_nth_page (self, i);

    if (p == page)
      continue;

    adw_tab_view_close_page (self, p);
  }
}

/**
 * adw_tab_view_close_pages_before:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages before @page.
 */
void
adw_tab_view_close_pages_before (AdwTabView *self,
                                 AdwTabPage *page)
{
  int pos, i;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  pos = adw_tab_view_get_page_position (self, page);

  for (i = pos - 1; i >= 0; i--) {
    AdwTabPage *p = adw_tab_view_get_nth_page (self, i);

    adw_tab_view_close_page (self, p);
  }
}

/**
 * adw_tab_view_close_pages_after:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages after @page.
 */
void
adw_tab_view_close_pages_after (AdwTabView *self,
                                AdwTabPage *page)
{
  int pos, i;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  pos = adw_tab_view_get_page_position (self, page);

  for (i = self->n_pages - 1; i > pos; i--) {
    AdwTabPage *p = adw_tab_view_get_nth_page (self, i);

    adw_tab_view_close_page (self, p);
  }
}

/**
 * adw_tab_view_reorder_page:
 * @self: a tab view
 * @page: a page of @self
 * @position: the position to insert the page at, starting at 0
 *
 * Reorders @page to @position.
 *
 * It's a programmer error to try to reorder a pinned page after a non-pinned
 * one, or a non-pinned page before a pinned one.
 *
 * Returns: whether @page was moved
 */
gboolean
adw_tab_view_reorder_page (AdwTabView *self,
                           AdwTabPage *page,
                           int         position)
{
  int original_pos;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  if (adw_tab_page_get_pinned (page)) {
    g_return_val_if_fail (position >= 0, FALSE);
    g_return_val_if_fail (position < self->n_pinned_pages, FALSE);
  } else {
    g_return_val_if_fail (position >= self->n_pinned_pages, FALSE);
    g_return_val_if_fail (position < self->n_pages, FALSE);
  }

  original_pos = adw_tab_view_get_page_position (self, page);

  if (original_pos == position)
    return FALSE;

  g_object_ref (page);

  g_list_store_remove (self->children, original_pos);
  g_list_store_insert (self->children, position, page);

  g_object_unref (page);

  g_signal_emit (self, signals[SIGNAL_PAGE_REORDERED], 0, page, position);

  if (self->pages) {
    int min = MIN (original_pos, position);
    int n_changed = MAX (original_pos, position) - min + 1;

    g_list_model_items_changed (G_LIST_MODEL (self->pages), min, n_changed, n_changed);
  }

  return TRUE;
}

/**
 * adw_tab_view_reorder_backward:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to before its previous page if possible.
 *
 * Returns: whether @page was moved
 */
gboolean
adw_tab_view_reorder_backward (AdwTabView *self,
                               AdwTabPage *page)
{
  gboolean pinned;
  int pos, first;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pos = adw_tab_view_get_page_position (self, page);

  pinned = adw_tab_page_get_pinned (page);
  first = pinned ? 0 : self->n_pinned_pages;

  if (pos <= first)
    return FALSE;

  return adw_tab_view_reorder_page (self, page, pos - 1);
}

/**
 * adw_tab_view_reorder_forward:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to after its next page if possible.
 *
 * Returns: whether @page was moved
 */
gboolean
adw_tab_view_reorder_forward (AdwTabView *self,
                              AdwTabPage *page)
{
  gboolean pinned;
  int pos, last;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pos = adw_tab_view_get_page_position (self, page);

  pinned = adw_tab_page_get_pinned (page);
  last = (pinned ? self->n_pinned_pages : self->n_pages) - 1;

  if (pos >= last)
    return FALSE;

  return adw_tab_view_reorder_page (self, page, pos + 1);
}

/**
 * adw_tab_view_reorder_first:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to the first possible position.
 *
 * Returns: whether @page was moved
 */
gboolean
adw_tab_view_reorder_first (AdwTabView *self,
                            AdwTabPage *page)
{
  gboolean pinned;
  int pos;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pinned = adw_tab_page_get_pinned (page);
  pos = pinned ? 0 : self->n_pinned_pages;

  return adw_tab_view_reorder_page (self, page, pos);
}

/**
 * adw_tab_view_reorder_last:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to the last possible position.
 *
 * Returns: whether @page was moved
 */
gboolean
adw_tab_view_reorder_last (AdwTabView *self,
                           AdwTabPage *page)
{
  gboolean pinned;
  int pos;

  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pinned = adw_tab_page_get_pinned (page);
  pos = (pinned ? self->n_pinned_pages : self->n_pages) - 1;

  return adw_tab_view_reorder_page (self, page, pos);
}

void
adw_tab_view_detach_page (AdwTabView *self,
                          AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  g_object_ref (page);

  begin_transfer_for_group (self);

  detach_page (self, page, FALSE);
}

void
adw_tab_view_attach_page (AdwTabView *self,
                          AdwTabPage *page,
                          int         position)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (!page_belongs_to_this_view (self, page));
  g_return_if_fail (position >= 0);
  g_return_if_fail (position <= self->n_pages);

  attach_page (self, page, position);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 0, 1);

  adw_tab_view_set_selected_page (self, page);

  end_transfer_for_group (self);

  g_object_unref (page);
}

/**
 * adw_tab_view_transfer_page:
 * @self: a tab view
 * @page: a page of @self
 * @other_view: the tab view to transfer the page to
 * @position: the position to insert the page at, starting at 0
 *
 * Transfers @page from @self to @other_view.
 *
 * The @page object will be reused.
 *
 * It's a programmer error to try to insert a pinned page after a non-pinned
 * one, or a non-pinned page before a pinned one.
 */
void
adw_tab_view_transfer_page (AdwTabView *self,
                            AdwTabPage *page,
                            AdwTabView *other_view,
                            int         position)
{
  gboolean pinned;

  g_return_if_fail (ADW_IS_TAB_VIEW (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));
  g_return_if_fail (ADW_IS_TAB_VIEW (other_view));
  g_return_if_fail (page_belongs_to_this_view (self, page));
  g_return_if_fail (position >= 0);
  g_return_if_fail (position <= other_view->n_pages);

  pinned = adw_tab_page_get_pinned (page);

  g_return_if_fail (!pinned || position <= other_view->n_pinned_pages);
  g_return_if_fail (pinned || position >= other_view->n_pinned_pages);

  adw_tab_view_detach_page (self, page);
  adw_tab_view_attach_page (other_view, page, position);
}

/**
 * adw_tab_view_get_pages: (attributes org.gtk.Method.get_property=pages)
 * @self: a tab view
 *
 * Returns a [iface@Gio.ListModel] that contains the pages of @self.
 *
 * This can be used to keep an up-to-date view. The model also implements
 * [iface@Gtk.SelectionModel] and can be used to track and change the selected
 * page.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the pages of @self
 */
GtkSelectionModel *
adw_tab_view_get_pages (AdwTabView *self)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  self->pages = adw_tab_pages_new (self);
  g_object_add_weak_pointer (G_OBJECT (self->pages),
                             (gpointer *) &self->pages);

  return self->pages;
}

/**
 * adw_tab_view_invalidate_thumbnails:
 * @self: a tab view
 *
 * Invalidates thumbnails for all pages in @self.
 *
 * This is a convenience method, equivalent to calling
 * [method@TabPage.invalidate_thumbnail] on each page.
 *
 * Since: 1.3
 */
void
adw_tab_view_invalidate_thumbnails (AdwTabView *self)
{
  int i;
  g_return_if_fail (ADW_IS_TAB_VIEW (self));

  for (i = 0; i < self->n_pages; i++) {
    AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

    adw_tab_page_invalidate_thumbnail (page);
  }
}

AdwTabView *
adw_tab_view_create_window (AdwTabView *self)
{
  AdwTabView *new_view = NULL;

  g_signal_emit (self, signals[SIGNAL_CREATE_WINDOW], 0, &new_view);

  if (!new_view) {
    g_critical ("AdwTabView::create-window handler must not return NULL");

    return NULL;
  }

  new_view->transfer_count = self->transfer_count;

  return new_view;
}

void
adw_tab_view_open_overview (AdwTabView *self)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));

  if (self->overview_count == 0 && gtk_widget_get_mapped (GTK_WIDGET (self))) {
    int i;

    for (i = 0; i < self->n_pages; i++) {
      AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

      if (page->live_thumbnail || page->invalidated)
        gtk_widget_set_child_visible (page->bin, TRUE);
    }

    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  self->overview_count++;
}

void
adw_tab_view_close_overview (AdwTabView *self)
{
  g_return_if_fail (ADW_IS_TAB_VIEW (self));

  self->overview_count--;

  if (self->overview_count == 0) {
    int i;

    for (i = 0; i < self->n_pages; i++) {
      AdwTabPage *page = adw_tab_view_get_nth_page (self, i);

      if (page->live_thumbnail || page->invalidated)
        gtk_widget_set_child_visible (page->bin,
                                      page == self->selected_page);
    }

    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  g_assert (self->overview_count >= 0);
}

