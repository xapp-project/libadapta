/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-navigation-view-private.h"

#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-shadow-helper-private.h"
#include "adw-spring-animation.h"
#include "adw-swipeable.h"
#include "adw-swipe-tracker.h"
#include "adw-widget-utils-private.h"

/**
 * AdwNavigationView:
 *
 * A page-based navigation container.
 *
 * <picture>
 *   <source srcset="navigation-view-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="navigation-view.png" alt="navigation-view">
 * </picture>
 *
 * `AdwNavigationView` presents one child at a time, similar to
 * [class@Gtk.Stack].
 *
 * `AdwNavigationView` can only contain [class@NavigationPage] children.
 *
 * It maintains a navigation stack that can be controlled with
 * [method@NavigationView.push] and [method@NavigationView.pop]. The whole
 * navigation stack can also be replaced using [method@NavigationView.replace].
 *
 * `AdwNavigationView` allows to manage pages statically or dynamically.
 *
 * Static pages can be added using the [method@NavigationView.add] method. The
 * `AdwNavigationView` will keep a reference to these pages, but they aren't
 * accessible to the user until [method@NavigationView.push] is called (except
 * for the first page, which is pushed automatically). Use the
 * [method@NavigationView.remove] method to remove them. This is useful for
 * applications that have a small number of unique pages and just need
 * navigation between them.
 *
 * Dynamic pages are automatically destroyed once they are popped off the
 * navigation stack. To add a page like this, push it using the
 * [method@NavigationView.push] method without calling
 * [method@NavigationView.add] first.
 *
 * ## Tags
 *
 * Static pages, as well as any pages in the navigation stack, can be accessed
 * by their [property@NavigationPage:tag]. For example,
 * [method@NavigationView.push_by_tag] can be used to push a static page that's
 * not in the navigation stack without having to keep a reference to it manually.
 *
 * ## Header Bar Integration
 *
 * When used inside `AdwNavigationView`, [class@HeaderBar] will automatically
 * display a back button that can be used to go back to the previous page when
 * possible. The button also has a context menu, allowing to pop multiple pages
 * at once, potentially across multiple navigation views.
 *
 * Set [property@HeaderBar:show-back-button] to `FALSE` to disable this behavior
 * in rare scenarios where it's unwanted.
 *
 * `AdwHeaderBar` will also display the title of the `AdwNavigationPage` it's
 * placed into, so most applications shouldn't need to customize it at all.
 *
 * ## Shortcuts and Gestures
 *
 * `AdwNavigationView` supports the following shortcuts for going to the
 * previous page:
 *
 * - <kbd>Escape</kbd> (unless [property@NavigationView:pop-on-escape] is set to
 *   `FALSE`)
 * - <kbd>Alt</kbd>+<kbd>←</kbd>
 * - Back mouse button
 *
 * Additionally, it supports interactive gestures:
 *
 * - One-finger swipe towards the right on touchscreens
 * - Scrolling towards the right on touchpads (usually two-finger swipe)
 *
 * These gestures have transitions enabled regardless of the
 * [property@NavigationView:animate-transitions] value.
 *
 * Applications can also enable shortcuts for pushing another page onto the
 * navigation stack via connecting to the [signal@NavigationView::get-next-page]
 * signal, in that case the following shortcuts are supported:
 *
 * - <kbd>Alt</kbd>+<kbd>→</kbd>
 * - Forward mouse button
 * - Swipe/scrolling towards the left
 *
 * For right-to-left locales, the gestures and shortcuts are reversed.
 *
 * [property@NavigationPage:can-pop] can be used to disable them, along with the
 * header bar back buttons.
 *
 * ## Actions
 *
 * `AdwNavigationView` defines actions for controlling the navigation stack.
 * actions for controlling the navigation stack:
 *
 * - `navigation.push` takes a string parameter specifying the tag of the page to
 * push, and is equivalent to calling [method@NavigationView.push_by_tag].
 *
 * - `navigation.pop` doesn't take any parameters and pops the current page from
 * the navigation stack, equivalent to calling [method@NavigationView.pop].
 *
 * ## `AdwNavigationView` as `GtkBuildable`
 *
 * `AdwNavigationView` allows to add pages as children, equivalent to using the
 * [method@NavigationView.add] method.
 *
 * Example of an `AdwNavigationView` UI definition:
 *
 * ```xml
 * <object class="AdwNavigationView">
 *   <child>
 *     <object class="AdwNavigationPage">
 *       <property name="title" translatable="yes">Page 1</property>
 *       <property name="child">
 *         <object class="AdwToolbarView">
 *           <child type="top">
 *             <object class="AdwHeaderBar"/>
 *           </child>
 *           <property name="content">
 *             <object class="GtkButton">
 *               <property name="label" translatable="yes">Open Page 2</property>
 *               <property name="halign">center</property>
 *               <property name="valign">center</property>
 *               <property name="action-name">navigation.push</property>
 *               <property name="action-target">'page-2'</property>
 *               <style>
 *                 <class name="pill"/>
 *                </style>
 *             </object>
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwNavigationPage">
 *       <property name="title" translatable="yes">Page 2</property>
 *       <property name="tag">page-2</property>
 *       <property name="child">
 *         <object class="AdwToolbarView">
 *           <child type="top">
 *             <object class="AdwHeaderBar"/>
 *           </child>
 *           <property name="content">
 *             <!-- ... -->
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * <picture>
 *   <source srcset="navigation-view-example-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="navigation-view-example.png" alt="navigation-view-example">
 * </picture>
 *
 * ## CSS nodes
 *
 * `AdwNavigationView` has a single CSS node with the name `navigation-view`.
 *
 * ## Accessibility
 *
 * `AdwNavigationView` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

/**
 * AdwNavigationPage:
 *
 * A page within [class@NavigationView] or [class@NavigationSplitView].
 *
 * Each page has a child widget, a title and optionally a tag.
 *
 * The [signal@NavigationPage::showing], [signal@NavigationPage::shown],
 * [signal@NavigationPage::hiding] and [signal@NavigationPage::hidden] signals
 * can be used to track the page's visibility within its `AdwNavigationView`.
 *
 * ## Header Bar Integration
 *
 * When placed inside `AdwNavigationPage`, [class@HeaderBar] will display the
 * page title instead of window title.
 *
 * When used together with [class@NavigationView], it will also display a back
 * button that can be used to go back to the previous page. Set
 * [property@HeaderBar:show-back-button] to `FALSE` to disable that behavior if
 * it's unwanted.
 *
 * ## CSS Nodes
 *
 * `AdwNavigationPage` has a single CSS node with name
 * `navigation-view-page`.
 *
 * ## Accessibility
 *
 * `AdwNavigationPage` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

typedef struct
{
  GtkWidget *child;
  char *title;
  char *tag;
  gboolean can_pop;

  GtkWidget *last_focus;
  gboolean remove_on_pop;

  int block_signals;

  AdwNavigationView *child_view;

  int nav_split_views;
} AdwNavigationPagePrivate;

static void adw_navigation_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwNavigationPage, adw_navigation_page, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdwNavigationPage)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_navigation_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_TAG,
  PAGE_PROP_TITLE,
  PAGE_PROP_CAN_POP,
  LAST_PAGE_PROP
};

static GParamSpec *page_props[LAST_PAGE_PROP];

enum {
  PAGE_SIGNAL_SHOWING,
  PAGE_SIGNAL_SHOWN,
  PAGE_SIGNAL_HIDING,
  PAGE_SIGNAL_HIDDEN,
  LAST_PAGE_SIGNAL,
};

static guint page_signals[LAST_PAGE_SIGNAL];

struct _AdwNavigationView
{
  GtkWidget parent_instance;

  GHashTable *tag_mapping;
  GListStore *navigation_stack;

  gboolean animate_transitions;
  gboolean pop_on_escape;

  AdwAnimation *transition;
  AdwNavigationPage *showing_page;
  AdwNavigationPage *hiding_page;
  gboolean transition_pop;
  gboolean transition_cancel;
  double transition_progress;
  gboolean gesture_active;

  AdwShadowHelper *shadow_helper;
  AdwSwipeTracker *swipe_tracker;

  GtkWidget *shield;

  GListModel *navigation_stack_model;
};

static void adw_navigation_view_buildable_init (GtkBuildableIface *iface);
static void adw_navigation_view_swipeable_init (AdwSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwNavigationView, adw_navigation_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_navigation_view_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADW_TYPE_SWIPEABLE, adw_navigation_view_swipeable_init))

enum {
  PROP_0,
  PROP_VISIBLE_PAGE,
  PROP_ANIMATE_TRANSITIONS,
  PROP_POP_ON_ESCAPE,
  PROP_NAVIGATION_STACK,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PUSHED,
  SIGNAL_POPPED,
  SIGNAL_REPLACED,
  SIGNAL_GET_NEXT_PAGE,
  LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

#define ADW_TYPE_NAVIGATION_VIEW_MODEL (adw_navigation_view_model_get_type ())

G_DECLARE_FINAL_TYPE (AdwNavigationViewModel, adw_navigation_view_model, ADW, NAVIGATION_VIEW_MODEL, GObject)

struct _AdwNavigationViewModel
{
  GObject parent_instance;
  AdwNavigationView *view;
};

static GType
adw_navigation_view_model_get_item_type (GListModel *model)
{
  return ADW_TYPE_NAVIGATION_PAGE;
}

static guint
adw_navigation_view_model_get_n_items (GListModel *model)
{
  AdwNavigationViewModel *self = ADW_NAVIGATION_VIEW_MODEL (model);

  return g_list_model_get_n_items (G_LIST_MODEL (self->view->navigation_stack));
}

static gpointer
adw_navigation_view_model_get_item (GListModel *model,
                                    guint       position)
{
  AdwNavigationViewModel *self = ADW_NAVIGATION_VIEW_MODEL (model);

  return g_list_model_get_item (G_LIST_MODEL (self->view->navigation_stack),
                                position);
}

static void
adw_navigation_view_model_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_navigation_view_model_get_item_type;
  iface->get_n_items = adw_navigation_view_model_get_n_items;
  iface->get_item = adw_navigation_view_model_get_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwNavigationViewModel, adw_navigation_view_model, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_navigation_view_model_list_model_init))

static void
adw_navigation_view_model_init (AdwNavigationViewModel *self)
{
}

static void
adw_navigation_view_model_class_init (AdwNavigationViewModelClass *class)
{
}

static GListModel *
adw_navigation_view_model_new (AdwNavigationView *view)
{
  AdwNavigationViewModel *model;

  model = g_object_new (ADW_TYPE_NAVIGATION_VIEW_MODEL, NULL);
  model->view = view;

  return G_LIST_MODEL (model);
}

static gboolean
get_remove_on_pop (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  return priv->remove_on_pop;
}

static void
set_remove_on_pop (AdwNavigationPage *self,
                   gboolean           remove_on_pop)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  priv->remove_on_pop = remove_on_pop;
}

static void
adw_navigation_page_realize (GtkWidget *widget)
{
  AdwNavigationPage *self = ADW_NAVIGATION_PAGE (widget);
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  GTK_WIDGET_CLASS (adw_navigation_page_parent_class)->realize (widget);

  if (!(priv->title && *priv->title) && !priv->child_view && priv->nav_split_views == 0) {
    g_warning ("AdwNavigationPage %p is missing a title. To hide a header bar " \
               "title, consider using AdwHeaderBar:show-title instead.", self);
  }
}

static void
adw_navigation_page_dispose (GObject *object)
{
  AdwNavigationPage *self = ADW_NAVIGATION_PAGE (object);
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_clear_pointer (&priv->child, gtk_widget_unparent);

  if (priv->child_view) {
    g_object_remove_weak_pointer (G_OBJECT (priv->child_view),
                                  (gpointer *) &priv->child_view);

    priv->child_view = NULL;
  }

  G_OBJECT_CLASS (adw_navigation_page_parent_class)->dispose (object);
}

static void
adw_navigation_page_finalize (GObject *object)
{
  AdwNavigationPage *self = ADW_NAVIGATION_PAGE (object);
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_free (priv->title);
  g_free (priv->tag);

  if (priv->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (priv->last_focus),
                                  (gpointer *) &priv->last_focus);

  G_OBJECT_CLASS (adw_navigation_page_parent_class)->finalize (object);
}

static void
adw_navigation_page_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwNavigationPage *self = ADW_NAVIGATION_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, adw_navigation_page_get_child (self));
    break;
  case PAGE_PROP_TAG:
    g_value_set_string (value, adw_navigation_page_get_tag (self));
    break;
  case PAGE_PROP_TITLE:
    g_value_set_string (value, adw_navigation_page_get_title (self));
    break;
  case PAGE_PROP_CAN_POP:
    g_value_set_boolean (value, adw_navigation_page_get_can_pop (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_navigation_page_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwNavigationPage *self = ADW_NAVIGATION_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    adw_navigation_page_set_child (self, g_value_get_object (value));
    break;
  case PAGE_PROP_TAG:
    adw_navigation_page_set_tag (self, g_value_get_string (value));
    break;
  case PAGE_PROP_TITLE:
    adw_navigation_page_set_title (self, g_value_get_string (value));
    break;
  case PAGE_PROP_CAN_POP:
    adw_navigation_page_set_can_pop (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_navigation_page_class_init (AdwNavigationPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_navigation_page_dispose;
  object_class->finalize = adw_navigation_page_finalize;
  object_class->get_property = adw_navigation_page_get_property;
  object_class->set_property = adw_navigation_page_set_property;

  widget_class->realize = adw_navigation_page_realize;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwNavigationPage:child: (attributes org.gtk.Property.get=adw_navigation_page_get_child org.gtk.Property.set=adw_navigation_page_set_child)
   *
   * The child widget.
   *
   * Since: 1.4
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationPage:tag: (attributes org.gtk.Property.get=adw_navigation_page_get_tag org.gtk.Property.set=adw_navigation_page_set_tag)
   *
   * The page tag.
   *
   * The tag can be used to retrieve the page with
   * [method@NavigationView.find_page], as well as with
   * [method@NavigationView.push_by_tag], [method@NavigationView.pop_to_tag] or
   * [method@NavigationView.replace_with_tags].
   *
   * Tags must be unique within each [class@NavigationView].
   *
   * The tag also must be set to use the `navigation.push` action.
   *
   * Since: 1.4
   */
  page_props[PAGE_PROP_TAG] =
    g_param_spec_string ("tag", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationPage:title: (attributes org.gtk.Property.get=adw_navigation_page_get_title org.gtk.Property.set=adw_navigation_page_set_title)
   *
   * The page title.
   *
   * It's displayed in [class@HeaderBar] instead of the window title, and used
   * as the tooltip on the next page's back button, as well as by screen reader.
   *
   * Since: 1.4
   */
  page_props[PAGE_PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationPage:can-pop: (attributes org.gtk.Property.get=adw_navigation_page_get_can_pop org.gtk.Property.set=adw_navigation_page_set_can_pop)
   *
   * Whether the page can be popped from navigation stack.
   *
   * Set it to `FALSE` to disable shortcuts and gestures, as well as remove the
   * back button from [class@HeaderBar].
   *
   * Manually calling [method@NavigationView.pop] or using the `navigation.pop`
   * action will still work.
   *
   * See [property@HeaderBar:show-back-button] for removing only the back
   * button, but not shortcuts.
   *
   * Since: 1.4
   */
  page_props[PAGE_PROP_CAN_POP] =
    g_param_spec_boolean ("can-pop", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);

  /**
   * AdwNavigationPage::showing:
   *
   * Emitted when the page shows at the beginning of the navigation view
   * transition.
   *
   * It will always be followed by [signal@NavigationPage::shown] or
   * [signal@NavigationPage::hidden].
   *
   * Since: 1.4
   */
  page_signals[PAGE_SIGNAL_SHOWING] =
    g_signal_new ("showing",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwNavigationPageClass, showing),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_SHOWING],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwNavigationPage::shown:
   *
   * Emitted when the navigation view transition has been completed and the page
   * is fully shown.
   *
   * It will always be preceded by [signal@NavigationPage::showing] or
   * [signal@NavigationPage::hiding].
   *
   * Since: 1.4
   */
  page_signals[PAGE_SIGNAL_SHOWN] =
    g_signal_new ("shown",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwNavigationPageClass, shown),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_SHOWN],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwNavigationPage::hiding:
   *
   * Emitted when the page starts hiding at the beginning of the navigation view
   * transition.
   *
   * It will always be followed by [signal@NavigationPage::hidden] or
   * [signal@NavigationPage::shown].
   *
   * Since: 1.4
   */
  page_signals[PAGE_SIGNAL_HIDING] =
    g_signal_new ("hiding",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwNavigationPageClass, hiding),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_HIDING],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwNavigationPage::hidden:
   *
   * Emitted when the navigation view transition has been completed and the page
   * is fully hidden.
   *
   * It will always be preceded by [signal@NavigationPage::hiding] or
   * [signal@NavigationPage::showing].
   *
   * Since: 1.4
   */
  page_signals[PAGE_SIGNAL_HIDDEN] =
    g_signal_new ("hidden",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AdwNavigationPageClass, hidden),
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_HIDDEN],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "navigation-view-page");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_navigation_page_init (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  priv->title = g_strdup ("");
  priv->can_pop = TRUE;

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, priv->title,
                                  -1);
}

static void
adw_navigation_page_buildable_add_child (GtkBuildable *buildable,
                                         GtkBuilder   *builder,
                                         GObject      *child,
                                         const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_navigation_page_set_child (ADW_NAVIGATION_PAGE (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_navigation_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_navigation_page_buildable_add_child;
}

static void
switch_page (AdwNavigationView *self,
             AdwNavigationPage *prev_page,
             AdwNavigationPage *page,
             gboolean           pop,
             gboolean           animate,
             double             velocity)
{
  GtkWidget *focus = NULL;
  gboolean contains_focus = FALSE;
  GtkRoot *root;

  g_assert (page != prev_page);
  g_assert (page || prev_page);

  if (gtk_widget_in_destruction (GTK_WIDGET (self)))
    return;

  root = gtk_widget_get_root (GTK_WIDGET (self));
  if (root)
    focus = gtk_root_get_focus (root);

  if (self->transition_cancel)
    adw_animation_skip (self->transition);

  if (focus && prev_page && gtk_widget_is_ancestor (focus, GTK_WIDGET (prev_page))) {
    AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (prev_page);

    contains_focus = TRUE;

    if (priv->last_focus)
      g_object_remove_weak_pointer (G_OBJECT (priv->last_focus),
                                    (gpointer *)&priv->last_focus);
    priv->last_focus = focus;
    g_object_add_weak_pointer (G_OBJECT (priv->last_focus),
                               (gpointer *)&priv->last_focus);
  }

  if (!prev_page)
    animate = FALSE;

  if (self->hiding_page && self->hiding_page != prev_page) {
    AdwNavigationPage *hiding_page = g_steal_pointer (&self->hiding_page);

    adw_navigation_page_hidden (hiding_page);

    adw_animation_reset (self->transition);

    if (self->transition_pop && get_remove_on_pop (hiding_page))
      adw_navigation_view_remove (self, hiding_page);
    else
      gtk_widget_set_child_visible (GTK_WIDGET (hiding_page), FALSE);

    g_object_unref (hiding_page);
  }

  if (page) {
    AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (page);

    gtk_widget_set_child_visible (GTK_WIDGET (page), TRUE);

    if (page != self->showing_page)
      adw_navigation_page_showing (page);

    if (contains_focus) {
      if (priv->last_focus)
        gtk_widget_grab_focus (priv->last_focus);
      else
        gtk_widget_child_focus (GTK_WIDGET (page), GTK_DIR_TAB_FORWARD);
    }
  }

  gtk_widget_insert_before (GTK_WIDGET (self->shield), GTK_WIDGET (self), NULL);

  if (!pop && page)
    gtk_widget_insert_before (GTK_WIDGET (page), GTK_WIDGET (self), NULL);

  gtk_widget_set_child_visible (self->shield, TRUE);

  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->transition),
                                       self->transition_progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->transition),
                                     self->transition_cancel ? 0 : 1);
  adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->transition),
                                             velocity);
  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->transition), pop);

  adw_animation_reset (self->transition);

  if (prev_page && prev_page != self->hiding_page)
    adw_navigation_page_hiding (prev_page);

  g_set_object (&self->showing_page, page);
  g_set_object (&self->hiding_page, prev_page);
  self->transition_pop = pop;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  if (animate)
    adw_animation_play (self->transition);
  else
    adw_animation_skip (self->transition);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE]);
}

static void
push_to_stack (AdwNavigationView *self,
               AdwNavigationPage *page,
               gboolean           animate,
               double             velocity,
               gboolean           use_tag_for_errors)
{
  AdwNavigationPage *previous_page = adw_navigation_view_get_visible_page (self);

  if (g_list_store_find (self->navigation_stack, page, NULL)) {
    if (use_tag_for_errors) {
      g_critical ("Page with the tag '%s' is already in navigation stack\n",
                  adw_navigation_page_get_tag (page));
    } else {
      g_critical ("Page '%s' is already in navigation stack\n",
                  adw_navigation_page_get_title (page));
    }

    return;
  }

  g_list_store_append (self->navigation_stack, page);

  switch_page (self, previous_page, page, FALSE, animate, velocity);

  g_signal_emit (self, signals[SIGNAL_PUSHED], 0);

  if (self->navigation_stack_model) {
    guint length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

    g_list_model_items_changed (self->navigation_stack_model,
                                length - 1, 0, 1);
  }
}

static void
pop_from_stack (AdwNavigationView *self,
                AdwNavigationPage *page_to,
                gboolean           animate,
                double             velocity)
{
  AdwNavigationPage *old_page;
  AdwNavigationPage *new_page;
  GSList *popped = NULL, *l;
  guint length, pos, i;

  old_page = adw_navigation_view_get_visible_page (self);

  length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  g_assert (g_list_store_find (self->navigation_stack, page_to, &pos));

  for (i = pos + 1; i < length; i++) {
    AdwNavigationPage *page;

    page = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), i);
    popped = g_slist_prepend (popped, page);
  }

  g_list_store_splice (self->navigation_stack, pos + 1, length - pos - 1, NULL, 0);

  new_page = adw_navigation_view_get_visible_page (self);

  switch_page (self, old_page, new_page, TRUE, animate, velocity);

  for (l = popped; l; l = l->next) {
    AdwNavigationPage *c = l->data;

    g_signal_emit (self, signals[SIGNAL_POPPED], 0, c);

    if (c != old_page && get_remove_on_pop (c))
      adw_navigation_view_remove (self, c);
  }

  if (self->navigation_stack_model)
    g_list_model_items_changed (self->navigation_stack_model,
                                pos + 1, length - pos - 1, 0);

  g_slist_free_full (popped, g_object_unref);
}

static void
transition_cb (double             value,
               AdwNavigationView *self)
{
  self->transition_progress = value;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
transition_done_cb (AdwNavigationView *self)
{
  if (self->hiding_page) {
    AdwNavigationPage *hiding_page = g_steal_pointer (&self->hiding_page);

    if (self->transition_cancel) {
      adw_navigation_page_shown (hiding_page);

      gtk_widget_insert_before (GTK_WIDGET (hiding_page), GTK_WIDGET (self), NULL);
    } else {
      adw_navigation_page_hidden (hiding_page);

      if (self->transition_pop && get_remove_on_pop (hiding_page))
        adw_navigation_view_remove (self, hiding_page);
      else
        gtk_widget_set_child_visible (GTK_WIDGET (hiding_page), FALSE);
    }

    g_object_unref (hiding_page);
  }

  if (self->showing_page) {
    AdwNavigationPage *showing_page = g_steal_pointer (&self->showing_page);

    if (self->transition_cancel) {
      adw_navigation_page_hidden (showing_page);

      if (!self->transition_pop && get_remove_on_pop (showing_page))
        adw_navigation_view_remove (self, showing_page);
      else
        gtk_widget_set_child_visible (GTK_WIDGET (showing_page), FALSE);
    } else {
      adw_navigation_page_shown (showing_page);

      gtk_widget_insert_before (GTK_WIDGET (showing_page), GTK_WIDGET (self), NULL);
    }

    g_object_unref (showing_page);
  }

  self->transition_cancel = FALSE;
  self->transition_progress = 0;

  gtk_widget_set_child_visible (self->shield, FALSE);
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
navigation_push_cb (AdwNavigationView *self,
                    const char        *action_name,
                    GVariant          *params)
{
  AdwNavigationPage *page;
  GtkWidget *parent;
  const char *tag;

  tag = g_variant_get_string (params, NULL);
  page = adw_navigation_view_find_page (self, tag);

  if (page) {
    push_to_stack (self, page, self->animate_transitions, 0, TRUE);
    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent && gtk_widget_activate_action_variant (parent, "navigation.push", params))
    return;

  g_critical ("No page with the tag '%s' found in AdwNavigationView %p",
              tag, self);
}

static void
navigation_pop_cb (AdwNavigationView *self)
{
  GtkWidget *parent;

  if (adw_navigation_view_pop (self))
    return;

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "navigation.pop", NULL);
}

static AdwNavigationPage *
get_next_page (AdwNavigationView *self)
{
  AdwNavigationPage *page = NULL;
  GtkWidget *parent;

  g_signal_emit (self, signals[SIGNAL_GET_NEXT_PAGE], 0, &page);

  if (!page)
    return NULL;

  parent = gtk_widget_get_parent (GTK_WIDGET (page));

  if (parent && parent != GTK_WIDGET (self)) {
    g_critical ("AdwNavigationView::get-next-page result already has a parent");
    g_object_unref (page);
    return NULL;
  }

  if (!parent)
    set_remove_on_pop (page, TRUE);

  return page;
}

static gboolean
pop_shortcut_cb (AdwNavigationView *self)
{
  AdwNavigationPage *page = adw_navigation_view_get_visible_page (self);

  if (!page)
    return GDK_EVENT_PROPAGATE;

  /* Stop it so that it's not propagated to parent navigation views */
  if (!adw_navigation_page_get_can_pop (page))
    return GDK_EVENT_STOP;

  if (adw_navigation_view_pop (self))
    return GDK_EVENT_STOP;

  return GDK_EVENT_PROPAGATE;
}

static gboolean
push_shortcut_cb (AdwNavigationView *self)
{
  AdwNavigationPage *next_page = get_next_page (self);

  if (!next_page)
    return GDK_EVENT_PROPAGATE;

  adw_navigation_view_push (self, next_page);

  g_object_unref (next_page);

  return GDK_EVENT_STOP;
}

static gboolean
escape_shortcut_cb (AdwNavigationView *self)
{
  if (self->pop_on_escape)
    return pop_shortcut_cb (self);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
back_forward_shortcut_cb (AdwNavigationView *self,
                          GVariant          *args)
{
  gboolean is_pop = FALSE;

  g_variant_get (args, "b", &is_pop);

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    is_pop = !is_pop;

  if (is_pop)
    return pop_shortcut_cb (self);
  else
    return push_shortcut_cb (self);
}

static void
back_forward_button_pressed_cb (GtkGesture        *gesture,
                                int                n_press,
                                double             x,
                                double             y,
                                AdwNavigationView *self)
{
  gboolean is_pop = FALSE;
  guint button;

  button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  /* Unfortunately, there are no constants for these buttons */
  if (button == 8) {
    is_pop = TRUE;
  } else if (button == 9) {
    is_pop = FALSE;
  } else {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));
    return;
  }

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    is_pop = !is_pop;

  if (is_pop) {
    AdwNavigationPage *page = adw_navigation_view_get_visible_page (self);

    if (!page) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    /* Consume it so that it's not propagated to parent navigation views */
    if (!adw_navigation_page_get_can_pop (page)) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
      return;
    }

    if (!adw_navigation_view_get_previous_page (self, page)) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    adw_navigation_view_pop (self);
  } else {
    AdwNavigationPage *next_page = get_next_page (self);

    if (!next_page) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    adw_navigation_view_push (self, next_page);

    g_object_unref (next_page);
  }

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
add_page (AdwNavigationView *self,
          AdwNavigationPage *page,
          gboolean           auto_push)
{
  const char *tag;

  tag = adw_navigation_page_get_tag (page);

  if (tag && adw_navigation_view_find_page (self, tag)) {
    g_critical ("Duplicate page tag in AdwNavigationView: %s",
                tag);

    return;
  }

  gtk_widget_set_parent (GTK_WIDGET (page), GTK_WIDGET (self));

  if (tag)
    g_hash_table_insert (self->tag_mapping, g_strdup (tag), page);

  if (auto_push && g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack)) == 0)
    push_to_stack (self, page, FALSE, 0, FALSE);
  else
    gtk_widget_set_child_visible (GTK_WIDGET (page), FALSE);
}

static gboolean
maybe_add_page (AdwNavigationView *self,
                AdwNavigationPage *page)
{
  const char *tag;

  if (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (self))
    return TRUE;

  tag = adw_navigation_page_get_tag (page);

  if (tag && adw_navigation_view_find_page (self, tag)) {
    g_critical ("Duplicate page tag in AdwNavigationView: %s", tag);

    return FALSE;
  }

  add_page (self, page, FALSE);
  set_remove_on_pop (page, TRUE);
  return TRUE;
}

static void
remove_page (AdwNavigationView *self,
             AdwNavigationPage *page,
             gboolean           check_stack)
{
  const char *tag;

  if (page == self->hiding_page)
    adw_animation_skip (self->transition);

  /* Avoid modifying the navigation stack */
  if (check_stack && g_list_store_find (self->navigation_stack, page, NULL)) {
    set_remove_on_pop (page, TRUE);
    return;
  }

  tag = adw_navigation_page_get_tag (page);

  if (tag)
    g_hash_table_remove (self->tag_mapping, tag);

  gtk_widget_unparent (GTK_WIDGET (page));
}

static void
prepare_cb (AdwSwipeTracker        *tracker,
            AdwNavigationDirection  direction,
            AdwNavigationView      *self)
{
  AdwNavigationPage *visible_page = adw_navigation_view_get_visible_page (self);
  AdwNavigationPage *new_page;
  gboolean remove_on_pop = FALSE;

  if (!visible_page)
    return;

  if (direction == ADW_NAVIGATION_DIRECTION_BACK) {
    if (!adw_navigation_page_get_can_pop (visible_page))
      return;

    new_page = adw_navigation_view_get_previous_page (self, visible_page);

    if (!new_page)
      return;

  } else {
    new_page = get_next_page (self);

    if (!new_page || !maybe_add_page (self, new_page))
      return;

    remove_on_pop = get_remove_on_pop (new_page);
    set_remove_on_pop (new_page, FALSE);
  }

  if (self->showing_page || self->hiding_page)
    adw_animation_skip (self->transition);

  self->showing_page = new_page;
  self->hiding_page = g_object_ref (visible_page);

  self->transition_pop = (direction == ADW_NAVIGATION_DIRECTION_BACK);

  if (direction == ADW_NAVIGATION_DIRECTION_BACK) {
    g_object_ref (new_page);
  } else {
    if (remove_on_pop)
      set_remove_on_pop (new_page, TRUE);

    gtk_widget_insert_before (GTK_WIDGET (self->shield), GTK_WIDGET (self), NULL);
  }

  gtk_widget_insert_before (GTK_WIDGET (self->shield), GTK_WIDGET (self), NULL);
  gtk_widget_set_child_visible (self->shield, TRUE);

  adw_navigation_page_showing (self->showing_page);
  adw_navigation_page_hiding (self->hiding_page);

  self->gesture_active = TRUE;

  gtk_widget_set_child_visible (GTK_WIDGET (self->showing_page), TRUE);

  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->transition), 0);
  adw_animation_reset (self->transition);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  adw_swipe_tracker_set_upper_overshoot (self->swipe_tracker, TRUE);
}

static void
update_swipe_cb (AdwSwipeTracker   *tracker,
                 double             progress,
                 AdwNavigationView *self)
{
  if (!self->gesture_active)
    return;

  if (self->transition_pop)
    self->transition_progress = -progress;
  else
    self->transition_progress = progress;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
end_swipe_cb (AdwSwipeTracker   *tracker,
              double             velocity,
              double             to,
              AdwNavigationView *self)
{
  gboolean animate;

  if (!self->gesture_active)
    return;

  self->gesture_active = FALSE;

  animate = !G_APPROX_VALUE (to, self->transition_progress, DBL_EPSILON) ||
            !G_APPROX_VALUE (velocity, 0, DBL_EPSILON);

  if (ABS (to) > 0.5) {
    if (self->transition_pop)
      pop_from_stack (self, self->showing_page, animate, -velocity);
    else
      push_to_stack (self, self->showing_page, animate, velocity, FALSE);
  } else {
    self->transition_cancel = TRUE;

    if (self->transition_pop && self->hiding_page)
      gtk_widget_insert_before (GTK_WIDGET (self->hiding_page), GTK_WIDGET (self), NULL);

    adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->transition),
                                         self->transition_progress);
    adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->transition), ABS (to));

    if (self->transition_pop)
      adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->transition),
                                                 -velocity);
    else
      adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->transition),
                                                 velocity);

    adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->transition),
                                    !self->transition_pop);

    if (animate)
      adw_animation_play (self->transition);
    else
      adw_animation_skip (self->transition);
  }

  adw_swipe_tracker_set_upper_overshoot (self->swipe_tracker, FALSE);
}

static void
set_child_view (AdwNavigationPage *self,
                AdwNavigationView *view)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  if (view == priv->child_view)
    return;

  if (priv->child_view) {
    g_object_remove_weak_pointer (G_OBJECT (priv->child_view),
                                  (gpointer *) &priv->child_view);
  }

  priv->child_view = view;

  if (priv->child_view) {
    g_object_add_weak_pointer (G_OBJECT (priv->child_view),
                               (gpointer *) &priv->child_view);
  }
}

static void
adw_navigation_view_measure (GtkWidget      *widget,
                             GtkOrientation  orientation,
                             int             for_size,
                             int            *minimum,
                             int            *natural,
                             int            *minimum_baseline,
                             int            *natural_baseline)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (widget);
  AdwNavigationPage *visible_page = NULL;
  int min = 0, nat = 0, min_baseline = -1, nat_baseline = -1;
  int last_min = 0, last_nat = 0, last_min_baseline = -1, last_nat_baseline = -1;

  visible_page = adw_navigation_view_get_visible_page (self);

  if (visible_page)
    gtk_widget_measure (GTK_WIDGET (visible_page), orientation, for_size,
                        &min, &nat, &min_baseline, &nat_baseline);

  if (self->hiding_page)
    gtk_widget_measure (GTK_WIDGET (self->hiding_page),
                        orientation, for_size, &last_min, &last_nat,
                        &last_min_baseline, &last_nat_baseline);

  if (minimum)
    *minimum = MAX (min, last_min);
  if (natural)
    *natural = MAX (nat, last_nat);
  if (minimum_baseline)
    *minimum_baseline = MAX (min_baseline, last_min_baseline);
  if (natural_baseline)
    *natural_baseline = MAX (nat_baseline, last_nat_baseline);
}

static void
adw_navigation_view_size_allocate (GtkWidget *widget,
                                   int        width,
                                   int        height,
                                   int        baseline)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (widget);
  AdwNavigationPage *visible_page = NULL;
  GtkWidget *static_page = NULL, *moving_page = NULL;
  gboolean is_rtl;
  double progress;
  int offset;

  visible_page = adw_navigation_view_get_visible_page (self);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  if (!self->hiding_page || !self->showing_page) {
    if (visible_page)
      gtk_widget_allocate (GTK_WIDGET (visible_page), width, height, baseline, NULL);

    adw_shadow_helper_size_allocate (self->shadow_helper, 0, 0,
                                     baseline, 0, 0, 1,
                                     is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT);
    return;
  }

  if (self->transition_pop) {
    if (self->showing_page)
      static_page = GTK_WIDGET (self->showing_page);
    if (self->hiding_page && self->showing_page != self->hiding_page)
      moving_page = GTK_WIDGET (self->hiding_page);
  } else {
    if (self->hiding_page)
      static_page = GTK_WIDGET (self->hiding_page);
    if (self->showing_page && self->showing_page != self->hiding_page)
      moving_page = GTK_WIDGET (self->showing_page);
  }

  progress = self->transition_progress;

  if (!self->transition_pop)
    progress = 1 - progress;

  offset = (int) round (progress * width);

  if (static_page)
    gtk_widget_allocate (static_page, width, height, baseline, NULL);

  if (gtk_widget_should_layout (self->shield)) {
    GskTransform *transform = NULL;
    gboolean move_shield = !self->gesture_active &&
                           (self->transition_pop != self->transition_cancel);

    if (move_shield) {
      if (is_rtl)
        transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (-offset, 0));
      else
        transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0));
    }

    gtk_widget_allocate (self->shield, width, height, baseline, transform);
  }

  if (is_rtl) {
    if (moving_page)
      gtk_widget_allocate (moving_page, width, height, baseline,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (-offset, 0)));

    adw_shadow_helper_size_allocate (self->shadow_helper,
                                     MAX (0, offset), height,
                                     baseline, width - offset, 0, progress,
                                     GTK_PAN_DIRECTION_LEFT);
  } else {
    if (moving_page)
      gtk_widget_allocate (moving_page, width, height, baseline,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0)));

    adw_shadow_helper_size_allocate (self->shadow_helper,
                                     MAX (0, offset), height,
                                     baseline, 0, 0, progress,
                                     GTK_PAN_DIRECTION_RIGHT);
  }
}

static void
adw_navigation_view_snapshot (GtkWidget   *widget,
                              GtkSnapshot *snapshot)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (widget);
  AdwNavigationPage *visible_page = NULL;
  GtkWidget *static_page = NULL, *moving_page = NULL;
  int width, height;
  int offset;
  int clip_x, clip_width;
  double progress;

  visible_page = adw_navigation_view_get_visible_page (self);

  if (!self->hiding_page || !self->showing_page) {
    if (visible_page)
      gtk_widget_snapshot_child (widget, GTK_WIDGET (visible_page), snapshot);

    return;
  }

  if (self->transition_pop) {
    if (self->showing_page)
      static_page = GTK_WIDGET (self->showing_page);
    if (self->hiding_page && self->showing_page != self->hiding_page)
      moving_page = GTK_WIDGET (self->hiding_page);
  } else {
    if (self->hiding_page)
      static_page = GTK_WIDGET (self->hiding_page);
    if (self->showing_page && self->showing_page != self->hiding_page)
      moving_page = GTK_WIDGET (self->showing_page);
  }

  width = gtk_widget_get_width (widget);
  height = gtk_widget_get_height (widget);
  progress = self->transition_progress;

  if (!self->transition_pop)
    progress = 1 - progress;

  offset = (int) round (progress * width);

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL) {
    clip_x = width - offset;
    clip_width = offset;
  } else {
    clip_x = 0;
    clip_width = offset;
  }

  if (static_page) {
    gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT (clip_x, 0, clip_width, height));
    gtk_widget_snapshot_child (widget, static_page, snapshot);
    gtk_snapshot_pop (snapshot);
  }

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    clip_x = -offset;
  else
    clip_x = offset;

  clip_width = width;

  if (moving_page) {
    gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT (clip_x, 0, clip_width, height));
    gtk_widget_snapshot_child (widget, moving_page, snapshot);
    gtk_snapshot_pop (snapshot);
  }

  adw_shadow_helper_snapshot (self->shadow_helper, snapshot);
}

static void
adw_navigation_view_root (GtkWidget *widget)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (widget);
  GtkWidget *parent_page;

  GTK_WIDGET_CLASS (adw_navigation_view_parent_class)->root (widget);

  parent_page = adw_widget_get_ancestor (widget, ADW_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  if (parent_page)
    set_child_view (ADW_NAVIGATION_PAGE (parent_page), self);
}

static void
adw_navigation_view_unroot (GtkWidget *widget)
{
  GtkWidget *parent_page;

  parent_page = adw_widget_get_ancestor (widget, ADW_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  if (parent_page)
    set_child_view (ADW_NAVIGATION_PAGE (parent_page), NULL);

  GTK_WIDGET_CLASS (adw_navigation_view_parent_class)->unroot (widget);
}

static void
adw_navigation_view_direction_changed (GtkWidget        *widget,
                                       GtkTextDirection  previous_direction)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (widget);
  gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;

  adw_swipe_tracker_set_reversed (self->swipe_tracker, is_rtl);
}

static void
adw_navigation_view_dispose (GObject *object)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (object);
  GtkWidget *child;

  if (self->navigation_stack_model)
    g_list_model_items_changed (self->navigation_stack_model, 0,
                                g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack)), 0);

  g_clear_object (&self->shadow_helper);
  g_clear_object (&self->swipe_tracker);

  g_clear_pointer (&self->shield, gtk_widget_unparent);

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self))))
    gtk_widget_unparent (child);

  g_clear_object (&self->navigation_stack);
  g_clear_pointer (&self->tag_mapping, g_hash_table_unref);
  g_clear_object (&self->transition);

  G_OBJECT_CLASS (adw_navigation_view_parent_class)->dispose (object);
}

static void
adw_navigation_view_finalize (GObject *object)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (object);

  if (self->navigation_stack_model)
    g_object_remove_weak_pointer (G_OBJECT (self->navigation_stack_model),
                                  (gpointer *) &self->navigation_stack_model);

  G_OBJECT_CLASS (adw_navigation_view_parent_class)->finalize (object);
}

static void
adw_navigation_view_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (object);

  switch (prop_id) {
  case PROP_VISIBLE_PAGE:
    g_value_set_object (value, adw_navigation_view_get_visible_page (self));
    break;
  case PROP_ANIMATE_TRANSITIONS:
    g_value_set_boolean (value, adw_navigation_view_get_animate_transitions (self));
    break;
  case PROP_POP_ON_ESCAPE:
    g_value_set_boolean (value, adw_navigation_view_get_pop_on_escape (self));
    break;
  case PROP_NAVIGATION_STACK:
    g_value_take_object (value, adw_navigation_view_get_navigation_stack (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_navigation_view_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (object);

  switch (prop_id) {
  case PROP_ANIMATE_TRANSITIONS:
    adw_navigation_view_set_animate_transitions (self, g_value_get_boolean (value));
    break;
  case PROP_POP_ON_ESCAPE:
    adw_navigation_view_set_pop_on_escape (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
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
adw_navigation_view_class_init (AdwNavigationViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_navigation_view_dispose;
  object_class->finalize = adw_navigation_view_finalize;
  object_class->get_property = adw_navigation_view_get_property;
  object_class->set_property = adw_navigation_view_set_property;

  widget_class->measure = adw_navigation_view_measure;
  widget_class->size_allocate = adw_navigation_view_size_allocate;
  widget_class->snapshot = adw_navigation_view_snapshot;
  widget_class->root = adw_navigation_view_root;
  widget_class->unroot = adw_navigation_view_unroot;
  widget_class->direction_changed = adw_navigation_view_direction_changed;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwNavigationView:visible-page: (attributes org.gtk.Property.get=adw_navigation_view_get_visible_page)
   *
   * The currently visible page.
   *
   * Since: 1.4
   */
  props[PROP_VISIBLE_PAGE] =
    g_param_spec_object ("visible-page", NULL, NULL,
                         ADW_TYPE_NAVIGATION_PAGE,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwNavigationView:animate-transitions: (attributes org.gtk.Property.get=adw_navigation_view_get_animate_transitions org.gtk.Property.set=adw_navigation_view_set_animate_transitions)
   *
   * Whether to animate page transitions.
   *
   * Gesture-based transitions are always animated.
   *
   * Since: 1.4
   */
  props[PROP_ANIMATE_TRANSITIONS] =
    g_param_spec_boolean ("animate-transitions", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationView:pop-on-escape: (attributes org.gtk.Property.get=adw_navigation_view_get_pop_on_escape org.gtk.Property.set=adw_navigation_view_set_pop_on_escape)
   *
   * Whether pressing Escape pops the current page.
   *
   * Applications using `AdwNavigationView` to implement a browser may want to
   * disable it.
   *
   * Since: 1.4
   */
  props[PROP_POP_ON_ESCAPE] =
    g_param_spec_boolean ("pop-on-escape", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationView:navigation-stack: (attributes org.gtk.Property.get=adw_navigation_view_get_navigation_stack)
   *
   * A list model that contains the pages in navigation stack.
   *
   * The pages are sorted from root page to visible page.
   *
   * This can be used to keep an up-to-date view.
   *
   * Since: 1.4
   */
  props[PROP_NAVIGATION_STACK] =
    g_param_spec_object ("navigation-stack", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwNavigationView::pushed:
   *
   * Emitted after a page has been pushed to the navigation stack.
   *
   * See [method@NavigationView.push].
   *
   * Since: 1.4
   */
  signals[SIGNAL_PUSHED] =
    g_signal_new ("pushed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_PUSHED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwNavigationView::popped:
   * @self: a navigation view
   * @page: the popped page
   *
   * Emitted after @page has been popped from the navigation stack.
   *
   * See [method@NavigationView.pop].
   *
   * When using [method@NavigationView.pop_to_page] or
   * [method@NavigationView.pop_to_tag], this signal is emitted for each of the
   * popped pages.
   *
   * Since: 1.4
   */
  signals[SIGNAL_POPPED] =
    g_signal_new ("popped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_NAVIGATION_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_POPPED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECTv);

  /**
   * AdwNavigationView::replaced:
   *
   * Emitted after the navigation stack has been replaced.
   *
   * See [method@NavigationView.replace].
   *
   * Since: 1.4
   */
  signals[SIGNAL_REPLACED] =
    g_signal_new ("replaced",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_REPLACED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  /**
   * AdwNavigationView::get-next-page:
   *
   * Emitted when a push shortcut or a gesture is triggered.
   *
   * To support the push shortcuts and gestures, the application is expected to
   * return the page to push in the handler.
   *
   * This signal can be emitted multiple times for the gestures, for example
   * when the gesture is cancelled by the user. As such, the application must
   * not make any irreversible changes in the handler, such as removing the page
   * from a forward stack.
   *
   * Instead, it should be done in the [signal@NavigationView::pushed] handler.
   *
   * Returns: (transfer full) (nullable): the page to push
   *
   * Since: 1.4
   */
  signals[SIGNAL_GET_NEXT_PAGE] =
    g_signal_new ("get-next-page",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  object_handled_accumulator,
                  NULL,
                  adw_marshal_OBJECT__VOID,
                  ADW_TYPE_NAVIGATION_PAGE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_GET_NEXT_PAGE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_OBJECT__VOIDv);

  gtk_widget_class_install_action (widget_class, "navigation.push", "s",
                                   (GtkWidgetActionActivateFunc) navigation_push_cb);
  gtk_widget_class_install_action (widget_class, "navigation.pop", NULL,
                                   (GtkWidgetActionActivateFunc) navigation_pop_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0,
                                (GtkShortcutFunc) escape_shortcut_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Back, 0,
                                (GtkShortcutFunc) back_forward_shortcut_cb, "b", TRUE);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Forward, 0,
                                (GtkShortcutFunc) back_forward_shortcut_cb, "b", FALSE);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Left, GDK_ALT_MASK,
                                (GtkShortcutFunc) back_forward_shortcut_cb, "b", TRUE);
  gtk_widget_class_add_binding (widget_class,  GDK_KEY_Right, GDK_ALT_MASK,
                                (GtkShortcutFunc) back_forward_shortcut_cb, "b", FALSE);

  gtk_widget_class_set_css_name (widget_class, "navigation-view");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_navigation_view_init (AdwNavigationView *self)
{
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  AdwAnimationTarget *target;
  GtkGesture *gesture;

  self->animate_transitions = TRUE;
  self->pop_on_escape = TRUE;

  self->navigation_stack = g_list_store_new (ADW_TYPE_NAVIGATION_PAGE);

  self->tag_mapping = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) transition_cb,
                                              self, NULL);
  self->transition = adw_spring_animation_new (GTK_WIDGET (self), 0, 1,
                                               adw_spring_params_new (1, 1, 1000),
                                               target);
  g_signal_connect_swapped (self->transition, "done",
                            G_CALLBACK (transition_done_cb), self);

  self->shadow_helper = adw_shadow_helper_new (GTK_WIDGET (self));

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (back_forward_button_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));

  self->swipe_tracker = adw_swipe_tracker_new (ADW_SWIPEABLE (self));
  adw_swipe_tracker_set_reversed (self->swipe_tracker, is_rtl);

  g_signal_connect (self->swipe_tracker, "prepare",
                    G_CALLBACK (prepare_cb), self);
  g_signal_connect (self->swipe_tracker, "update-swipe",
                    G_CALLBACK (update_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "end-swipe",
                    G_CALLBACK (end_swipe_cb), self);

  self->shield = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_child_visible (self->shield, FALSE);
  gtk_widget_set_parent (self->shield, GTK_WIDGET (self));
}

static void
adw_navigation_view_buildable_add_child (GtkBuildable *buildable,
                                         GtkBuilder   *builder,
                                         GObject      *child,
                                         const char   *type)
{
  if (ADW_IS_NAVIGATION_PAGE (child))
    adw_navigation_view_add (ADW_NAVIGATION_VIEW (buildable),
                             ADW_NAVIGATION_PAGE (child));
  else if (GTK_IS_WIDGET (child))
    g_warning ("Cannot add an object of type %s to AdwNavigationView",
               g_type_name (G_OBJECT_TYPE (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_navigation_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_navigation_view_buildable_add_child;
}

static double
adw_navigation_view_get_distance (AdwSwipeable *swipeable)
{
  return gtk_widget_get_width (GTK_WIDGET (swipeable));
}

static double *
adw_navigation_view_get_snap_points (AdwSwipeable *swipeable,
                                     int          *n_snap_points)
{
  AdwNavigationView *self = ADW_NAVIGATION_VIEW (swipeable);
  AdwNavigationPage *visible_page;
  double *points, lower, upper;
  int n;

  visible_page = adw_navigation_view_get_visible_page (self);

  if (self->showing_page || self->hiding_page) {
    lower = self->transition_pop && self->gesture_active ? -1 : 0;
    upper = self->transition_pop || !self->gesture_active ? 0 : 1;
  } else {
    AdwNavigationPage *prev_page, *next_page;

    if (visible_page)
      prev_page = adw_navigation_view_get_previous_page (self, visible_page);
    else
      prev_page = NULL;

    next_page = get_next_page (self);

    lower = MIN (0, prev_page ? -1 : 0);
    upper = MAX (0, next_page ?  1 : 0);

    if (next_page)
      g_object_unref (next_page);
  }

  n = !G_APPROX_VALUE (lower, upper, DBL_EPSILON) ? 2 : 1;

  points = g_new0 (double, n);
  points[0] = lower;
  points[n - 1] = upper;

  if (n_snap_points)
    *n_snap_points = n;

  return points;
}

static double
adw_navigation_view_get_progress (AdwSwipeable *swipeable)
{
  return 0;
}

static double
adw_navigation_view_get_cancel_progress (AdwSwipeable *swipeable)
{
  return 0;
}

static void
adw_navigation_view_swipeable_init (AdwSwipeableInterface *iface)
{
  iface->get_distance = adw_navigation_view_get_distance;
  iface->get_snap_points = adw_navigation_view_get_snap_points;
  iface->get_progress = adw_navigation_view_get_progress;
  iface->get_cancel_progress = adw_navigation_view_get_cancel_progress;
}

/**
 * adw_navigation_page_new:
 * @child: the child widget
 * @title: the page title
 *
 * Creates a new `AdwNavigationPage`.
 *
 * Returns: the new created `AdwNavigationPage`
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_page_new (GtkWidget  *child,
                         const char *title)
{
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADW_TYPE_NAVIGATION_PAGE,
                       "child", child,
                       "title", title,
                       NULL);
}

/**
 * adw_navigation_page_new_with_tag:
 * @child: the child widget
 * @title: the page title
 * @tag: the page tag
 *
 * Creates a new `AdwNavigationPage` with provided tag.
 *
 * Returns: the new created `AdwNavigationPage`
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_page_new_with_tag (GtkWidget  *child,
                                  const char *title,
                                  const char *tag)
{
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (tag != NULL, NULL);

  return g_object_new (ADW_TYPE_NAVIGATION_PAGE,
                       "child", child,
                       "title", title,
                       "tag", tag,
                       NULL);
}

/**
 * adw_navigation_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a navigation page
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.4
 */
GtkWidget *
adw_navigation_page_get_child (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv;

  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (self), NULL);

  priv = adw_navigation_page_get_instance_private (self);

  return priv->child;
}

/**
 * adw_navigation_page_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a navigation page
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.4
 */
void
adw_navigation_page_set_child (AdwNavigationPage *self,
                               GtkWidget         *child)
{
  AdwNavigationPagePrivate *priv;

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adw_navigation_page_get_instance_private (self);

  if (priv->child == child)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (priv->child)
    gtk_widget_unparent (priv->child);

  priv->child = child;

  if (priv->child)
    gtk_widget_set_parent (priv->child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_CHILD]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_navigation_page_get_tag: (attributes org.gtk.Method.get_property=tag)
 * @self: a navigation page
 *
 * Gets the tag of @self.
 *
 * Returns: (transfer none) (nullable): the page tag
 *
 * Since: 1.4
 */
const char *
adw_navigation_page_get_tag (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv;

  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (self), NULL);

  priv = adw_navigation_page_get_instance_private (self);

  return priv->tag;
}

/**
 * adw_navigation_page_set_tag: (attributes org.gtk.Method.set_property=tag)
 * @self: a navigation page
 * @tag: (nullable): the page tag
 *
 * Sets the tag for @self.
 *
 * The tag can be used to retrieve the page with
 * [method@NavigationView.find_page], as well as with
 * [method@NavigationView.push_by_tag], [method@NavigationView.pop_to_tag] or
 * [method@NavigationView.replace_with_tags].
 *
 * Tags must be unique within each [class@NavigationView].
 *
 * The tag also must be set to use the `navigation.push` action.
 *
 *
 * Since: 1.4
 */
void
adw_navigation_page_set_tag (AdwNavigationPage *self,
                             const char        *tag)
{
  AdwNavigationPagePrivate *priv;
  GtkWidget *parent;
  AdwNavigationView *view = NULL;

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  priv = adw_navigation_page_get_instance_private (self);

  if (!g_strcmp0 (priv->tag, tag))
    return;

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (ADW_IS_NAVIGATION_VIEW (parent))
    view = ADW_NAVIGATION_VIEW (parent);

  if (tag && view && adw_navigation_view_find_page (view, tag)) {
    g_critical ("Duplicate page tag in AdwNavigationView: %s", tag);

    return;
  }

  if (priv->tag && view)
    g_hash_table_remove (view->tag_mapping, priv->tag);

  g_set_str (&priv->tag, tag);

  if (priv->tag && view)
    g_hash_table_insert (view->tag_mapping, g_strdup (priv->tag), self);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TAG]);
}

/**
 * adw_navigation_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a navigation page
 *
 * Gets the title of @self.
 *
 * Returns: (transfer none): the title of @self
 *
 * Since: 1.4
 */
const char *
adw_navigation_page_get_title (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv;

  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (self), NULL);

  priv = adw_navigation_page_get_instance_private (self);

  return priv->title;
}

/**
 * adw_navigation_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a navigation page
 * @title: the title
 *
 * Sets the title of @self.
 *
 * It's displayed in [class@HeaderBar] instead of the window title, and used as
 * the tooltip on the next page's back button, as well as by screen reader.
 *
 * Since: 1.4
 */
void
adw_navigation_page_set_title (AdwNavigationPage *self,
                               const char        *title)
{
  AdwNavigationPagePrivate *priv;

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));
  g_return_if_fail (title != NULL);

  priv = adw_navigation_page_get_instance_private (self);

  if (!g_set_str (&priv->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, priv->title,
                                  -1);
}

/**
 * adw_navigation_page_get_can_pop: (attributes org.gtk.Method.get_property=can-pop)
 * @self: a navigation page
 *
 * Gets whether @self can be popped from navigation stack.
 *
 * Returns: whether the page can be popped from navigation stack
 *
 * Since: 1.4
 */
gboolean
adw_navigation_page_get_can_pop (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv;

  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (self), FALSE);

  priv = adw_navigation_page_get_instance_private (self);

  return priv->can_pop;
}

/**
 * adw_navigation_page_set_can_pop: (attributes org.gtk.Method.set_property=can-pop)
 * @self: a navigation page
 * @can_pop: whether the page can be popped from navigation stack
 *
 * Sets whether @self can be popped from navigation stack.
 *
 * Set it to `FALSE` to disable shortcuts and gestures, as well as remove the
 * back button from [class@HeaderBar].
 *
 * Manually calling [method@NavigationView.pop] or using the `navigation.pop`
 * action will still work.
 *
 * See [property@HeaderBar:show-back-button] for removing only the back button,
 * but not shortcuts.
 *
 * Since: 1.4
 */
void
adw_navigation_page_set_can_pop (AdwNavigationPage *self,
                                 gboolean           can_pop)
{
  AdwNavigationPagePrivate *priv;

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  priv = adw_navigation_page_get_instance_private (self);

  can_pop = !!can_pop;

  if (can_pop == priv->can_pop)
    return;

  priv->can_pop = can_pop;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_CAN_POP]);
}

AdwNavigationView *
adw_navigation_page_get_child_view (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv;

  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (self), NULL);

  priv = adw_navigation_page_get_instance_private (self);

  return priv->child_view;
}

void
adw_navigation_page_showing (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_SHOWING], 0);
}

void
adw_navigation_page_shown (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_SHOWN], 0);
}

void
adw_navigation_page_hiding (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_HIDING], 0);
}

void
adw_navigation_page_hidden (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_HIDDEN], 0);
}

void
adw_navigation_page_block_signals (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  priv->block_signals++;
}

void
adw_navigation_page_unblock_signals (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  g_assert (priv->block_signals > 0);

  priv->block_signals--;
}

void
adw_navigation_page_add_child_nav_split_view (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  priv->nav_split_views++;
}

void
adw_navigation_page_remove_child_nav_split_view (AdwNavigationPage *self)
{
  AdwNavigationPagePrivate *priv = adw_navigation_page_get_instance_private (self);

  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (self));

  priv->nav_split_views--;
}

/**
 * adw_navigation_view_new:
 *
 * Creates a new `AdwNavigationView`.
 *
 * Returns: the new created `AdwNavigationView`
 *
 * Since: 1.4
 */
GtkWidget *
adw_navigation_view_new (void)
{
  return g_object_new (ADW_TYPE_NAVIGATION_VIEW, NULL);
}

/**
 * adw_navigation_view_add:
 * @self: a navigation view
 * @page: the page to add
 *
 * Permanently adds @page to @self.
 *
 * Any page that has been added will stay in @self even after being popped from
 * the navigation stack.
 *
 * Adding a page while no page is visible will automatically push it to the
 * navigation stack.
 *
 * See [method@NavigationView.remove].
 *
 * Since: 1.4
 */
void
adw_navigation_view_add (AdwNavigationView *self,
                         AdwNavigationPage *page)
{
  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (GTK_IS_WIDGET (page));

  if (get_remove_on_pop (page) &&
      gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (self) &&
      g_list_store_find (self->navigation_stack, page, NULL)) {
    set_remove_on_pop (page, FALSE);
    return;
  }

  add_page (self, page, TRUE);
}

/**
 * adw_navigation_view_remove:
 * @self: a navigation view
 * @page: the page to remove
 *
 * Removes @page from @self.
 *
 * If @page is currently in the navigation stack, it will be removed once it's
 * popped. Otherwise, it's removed immediately.
 *
 * See [method@NavigationView.add].
 *
 * Since: 1.4
 */
void
adw_navigation_view_remove (AdwNavigationView *self,
                            AdwNavigationPage *page)
{
  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (page));
  g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (self));

  remove_page (self, page, TRUE);
}

/**
 * adw_navigation_view_find_page:
 * @self: a navigation view
 * @tag: a page tag
 *
 * Finds a page in @self by its tag.
 *
 * See [property@NavigationPage:tag].
 *
 * Returns: (transfer none) (nullable): the page with the given tag
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_view_find_page (AdwNavigationView *self,
                               const char        *tag)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), NULL);
  g_return_val_if_fail (tag != NULL, NULL);

  return g_hash_table_lookup (self->tag_mapping, tag);
}

/**
 * adw_navigation_view_push:
 * @self: a navigation view
 * @page: the page to push
 *
 * Pushes @page onto the navigation stack.
 *
 * If [method@NavigationView.add] hasn't been called, the page is automatically
 * removed once it's popped.
 *
 * [signal@NavigationView::pushed] will be emitted for @page.
 *
 * See [method@NavigationView.push_by_tag].
 *
 * Since: 1.4
 */
void
adw_navigation_view_push (AdwNavigationView *self,
                          AdwNavigationPage *page)
{
  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (ADW_IS_NAVIGATION_PAGE (page));

  if (!maybe_add_page (self, page))
    return;

  push_to_stack (self, page, self->animate_transitions, 0, FALSE);
}

/**
 * adw_navigation_view_push_by_tag:
 * @self: a navigation view
 * @tag: the page tag
 *
 * Pushes the page with the tag @tag onto the navigation stack.
 *
 * If [method@NavigationView.add] hasn't been called, the page is automatically
 * removed once it's popped.
 *
 * [signal@NavigationView::pushed] will be emitted for the page.
 *
 * See [method@NavigationView.push] and [property@NavigationPage:tag].
 *
 * Since: 1.4
 */
void
adw_navigation_view_push_by_tag (AdwNavigationView *self,
                                 const char        *tag)
{
  AdwNavigationPage *page;

  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (tag != NULL);

  page = adw_navigation_view_find_page (self, tag);

  if (page == NULL) {
    g_critical ("No page with the tag '%s' found in AdwNavigationView %p",
                tag, self);
    return;
  }

  push_to_stack (self, page, self->animate_transitions, 0, TRUE);
}

/**
 * adw_navigation_view_pop:
 * @self: a navigation view
 *
 * Pops the visible page from the navigation stack.
 *
 * Does nothing if the navigation stack contains less than two pages.
 *
 * If [method@NavigationView.add] hasn't been called, the page is automatically
 * removed.
 *
 * [signal@NavigationView::popped] will be emitted for the current visible page.
 *
 * See [method@NavigationView.pop_to_page] and
 * [method@NavigationView.pop_to_tag].
 *
 * Returns: `TRUE` if a page has been popped
 *
 * Since: 1.4
 */
gboolean
adw_navigation_view_pop (AdwNavigationView *self)
{
  AdwNavigationPage *page;
  AdwNavigationPage *prev_page;

  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), FALSE);

  page = adw_navigation_view_get_visible_page (self);

  if (!page)
    return FALSE;

  prev_page = adw_navigation_view_get_previous_page (self, page);

  if (!prev_page)
    return FALSE;

  pop_from_stack (self, prev_page, self->animate_transitions, 0);

  return TRUE;
}

/**
 * adw_navigation_view_pop_to_page:
 * @self: a navigation view
 * @page: the page to pop to
 *
 * Pops pages from the navigation stack until @page is visible.
 *
 * @page must be in the navigation stack.
 *
 * If [method@NavigationView.add] hasn't been called for any of the popped pages,
 * they are automatically removed.
 *
 * [signal@NavigationView::popped] will be be emitted for each of the popped
 * pages.
 *
 * See [method@NavigationView.pop] and [method@NavigationView.pop_to_tag].
 *
 * Returns: `TRUE` if any pages have been popped
 *
 * Since: 1.4
 */
gboolean
adw_navigation_view_pop_to_page (AdwNavigationView *self,
                                 AdwNavigationPage *page)
{
  AdwNavigationPage *visible_page;

  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), FALSE);
  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (page), FALSE);

  visible_page = adw_navigation_view_get_visible_page (self);

  if (page == visible_page)
    return FALSE;

  if (!g_list_store_find (self->navigation_stack, page, NULL)) {
    g_critical ("Page '%s' is not in the navigation stack\n",
                adw_navigation_page_get_title (page));
    return FALSE;
  }

  pop_from_stack (self, page, self->animate_transitions, 0);

  return TRUE;
}

/**
 * adw_navigation_view_pop_to_tag:
 * @self: a navigation view
 * @tag: a page tag
 *
 * Pops pages from the navigation stack until page with the tag @tag is visible.
 *
 * The page must be in the navigation stack.
 *
 * If [method@NavigationView.add] hasn't been called for any of the popped pages,
 * they are automatically removed.
 *
 * [signal@NavigationView::popped] will be emitted for each of the popped pages.
 *
 * See [method@NavigationView.pop_to_page] and [property@NavigationPage:tag].
 *
 * Returns: `TRUE` if any pages have been popped
 *
 * Since: 1.4
 */
gboolean
adw_navigation_view_pop_to_tag (AdwNavigationView *self,
                                const char        *tag)
{
  AdwNavigationPage *page;

  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), FALSE);
  g_return_val_if_fail (tag != NULL, FALSE);

  page = adw_navigation_view_find_page (self, tag);

  if (page == NULL) {
    g_critical ("No page with the tag '%s' found in AdwNavigationView %p",
                tag, self);
    return FALSE;
  }

  return adw_navigation_view_pop_to_page (self, page);
}

/**
 * adw_navigation_view_replace:
 * @self: a navigation view
 * @pages: (array length=n_pages): the new navigation stack
 * @n_pages: the number of pages in @pages
 *
 * Replaces the current navigation stack with @pages.
 *
 * The last page becomes the visible page.
 *
 * Replacing the navigation stack has no animation.
 *
 * If [method@NavigationView.add] hasn't been called for any pages that are no
 * longer in the navigation stack, they are automatically removed.
 *
 * @n_pages can be 0, in that case no page will be visible after calling this
 * method. This can be useful for removing all pages from @self.
 *
 * The [signal@NavigationView::replaced] signal will be emitted.
 *
 * See [method@NavigationView.replace_with_tags].
 *
 * Since: 1.4
 */
void
adw_navigation_view_replace (AdwNavigationView  *self,
                             AdwNavigationPage **pages,
                             int                 n_pages)
{
  AdwNavigationPage *visible_page, *old_visible_page;
  GHashTable *added_pages;
  guint i, old_length;

  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (n_pages >= 0);

  visible_page = adw_navigation_view_get_visible_page (self);
  old_visible_page = visible_page;
  old_length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  added_pages = g_hash_table_new (g_direct_hash, g_direct_equal);

  for (i = 0; i < n_pages; i++) {
    if (!pages[i])
      continue;

    g_hash_table_insert (added_pages, pages[i], NULL);
  }

  for (i = 0; i < old_length; i++) {
    AdwNavigationPage *c;

    c = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack),
                               old_length - i - 1);

    if (get_remove_on_pop (c) &&
        !g_hash_table_contains (added_pages, c)) {
      if (c == visible_page) {
        adw_navigation_page_hiding (visible_page);
        adw_navigation_page_hidden (visible_page);
        visible_page = NULL;
      }

      remove_page (self, c, FALSE);
    }

    g_object_unref (c);
  }

  g_list_store_remove_all (self->navigation_stack);
  g_hash_table_remove_all (added_pages);

  for (i = 0; i < n_pages; i++) {
    if (!pages[i])
      continue;

    if (g_hash_table_contains (added_pages, pages[i])) {
      g_critical ("Page '%s' is already in navigation stack\n",
                  adw_navigation_page_get_title (pages[i]));
      continue;
    }

    if (!maybe_add_page (self, pages[i]))
      continue;

    g_hash_table_insert (added_pages, pages[i], NULL);
    g_list_store_append (self->navigation_stack, pages[i]);
  }

  if (g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack)) > 0) {
    AdwNavigationPage *new_visible_page = adw_navigation_view_get_visible_page (self);

    gtk_widget_insert_before (self->shield, GTK_WIDGET (self), NULL);
    gtk_widget_insert_before (GTK_WIDGET (new_visible_page), GTK_WIDGET (self), NULL);

    if (visible_page != new_visible_page)
      switch_page (self, visible_page, new_visible_page, TRUE, FALSE, 0);
  } else if (visible_page) {
    switch_page (self, visible_page, NULL, TRUE, FALSE, 0);
  } else if (old_visible_page) {
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE]);
  }

  g_hash_table_unref (added_pages);

  g_signal_emit (self, signals[SIGNAL_REPLACED], 0);

  if (self->navigation_stack_model) {
    guint length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

    g_list_model_items_changed (self->navigation_stack_model,
                                0, old_length, length);
  }
}

/**
 * adw_navigation_view_replace_with_tags:
 * @self: a navigation view
 * @tags: (array length=n_tags) (element-type utf8): tags of the pages in the
 *   navigation stack
 * @n_tags: the number of tags
 *
 * Replaces the current navigation stack with pages with the tags @tags.
 *
 * The last page becomes the visible page.
 *
 * Replacing the navigation stack has no animation.
 *
 * If [method@NavigationView.add] hasn't been called for any pages that are no
 * longer in the navigation stack, they are automatically removed.
 *
 * @n_tags can be 0, in that case no page will be visible after calling this
 * method. This can be useful for removing all pages from @self.
 *
 * The [signal@NavigationView::replaced] signal will be emitted.
 *
 * See [method@NavigationView.replace] and [property@NavigationPage:tag].
 *
 * Since: 1.4
 */
void
adw_navigation_view_replace_with_tags (AdwNavigationView  *self,
                                       const char * const *tags,
                                       int                 n_tags)
{
  AdwNavigationPage **pages;
  int i;

  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (n_tags >= 0);

  pages = g_new0 (AdwNavigationPage *, n_tags);

  for (i = 0; i < n_tags; i++) {
    AdwNavigationPage *page =
      adw_navigation_view_find_page (self, tags[i]);

    if (page == NULL) {
      g_critical ("No page with the tag '%s' found in AdwNavigationView %p",
                  tags[i], self);
      continue;
    }

    pages[i] = page;
  }

  adw_navigation_view_replace (self, pages, n_tags);

  g_free (pages);
}

/**
 * adw_navigation_view_get_visible_page: (attributes org.gtk.Method.get_property=visible-page)
 * @self: a navigation view
 *
 * Gets the currently visible page in @self.
 *
 * Returns: (transfer none) (nullable): the currently visible page
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_view_get_visible_page (AdwNavigationView *self)
{
  AdwNavigationPage *ret;
  guint length;

  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), NULL);

  length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  if (length == 0)
    return NULL;

  ret = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), length - 1);

  g_object_unref (ret);

  return ret;
}

/**
 * adw_navigation_view_get_previous_page:
 * @self: a navigation view
 * @page: a page in @self
 *
 * Gets the previous page for @page.
 *
 * If @page is in the navigation stack, returns the page popping @page will
 * reveal.
 *
 * If @page is the root page or is not in the navigation stack, returns `NULL`.
 *
 * Returns: (transfer none) (nullable): the previous page
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_view_get_previous_page (AdwNavigationView *self,
                                       AdwNavigationPage *page)
{
  AdwNavigationPage *ret;
  guint pos;

  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), NULL);
  g_return_val_if_fail (ADW_IS_NAVIGATION_PAGE (page), NULL);

  if (!g_list_store_find (self->navigation_stack, page, &pos))
    return NULL;

  if (pos == 0)
    return NULL;

  ret = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), pos - 1);

  g_object_unref (ret);

  return ret;
}

/**
 * adw_navigation_view_get_animate_transitions: (attributes org.gtk.Method.get_property=animate-transitions)
 * @self: a navigation view
 *
 * Gets whether @self animates page transitions.
 *
 * Returns: whether to animate page transitions
 *
 * Since: 1.4
 */
gboolean
adw_navigation_view_get_animate_transitions (AdwNavigationView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), FALSE);

  return self->animate_transitions;
}

/**
 * adw_navigation_view_set_animate_transitions: (attributes org.gtk.Method.set_property=animate-transitions)
 * @self: a navigation view
 * @animate_transitions: whether to animate page transitions
 *
 * Sets whether @self should animate page transitions.
 *
 * Gesture-based transitions are always animated.
 *
 * Since: 1.4
 */
void
adw_navigation_view_set_animate_transitions (AdwNavigationView *self,
                                             gboolean           animate_transitions)
{
  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));

  animate_transitions = !!animate_transitions;

  if (animate_transitions == self->animate_transitions)
    return;

  self->animate_transitions = animate_transitions;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ANIMATE_TRANSITIONS]);
}

/**
 * adw_navigation_view_get_pop_on_escape: (attributes org.gtk.Method.get_property=pop-on-escape)
 * @self: a navigation view
 *
 * Gets whether pressing Escape pops the current page on @self.
 *
 * Returns: whether to pop the current page
 *
 * Since: 1.4
 */
gboolean
adw_navigation_view_get_pop_on_escape (AdwNavigationView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), FALSE);

  return self->pop_on_escape;
}

/**
 * adw_navigation_view_set_pop_on_escape: (attributes org.gtk.Method.set_property=pop-on-escape)
 * @self: a navigation view
 * @pop_on_escape: whether to pop the current page when pressing Escape
 *
 * Sets whether pressing Escape pops the current page on @self.
 *
 * Applications using `AdwNavigationView` to implement a browser may want to
 * disable it.
 *
 * Since: 1.4
 */
void
adw_navigation_view_set_pop_on_escape (AdwNavigationView *self,
                                       gboolean           pop_on_escape)
{
  g_return_if_fail (ADW_IS_NAVIGATION_VIEW (self));

  pop_on_escape = !!pop_on_escape;

  if (pop_on_escape == self->pop_on_escape)
    return;

  self->pop_on_escape = pop_on_escape;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POP_ON_ESCAPE]);
}

/**
 * adw_navigation_view_get_navigation_stack: (attributes org.gtk.Method.get_property=navigation-stack)
 * @self: a navigation view
 *
 * Returns a [iface@Gio.ListModel] that contains the pages in navigation stack.
 *
 * The pages are sorted from root page to visible page.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a list model for the navigation stack
 *
 * Since: 1.4
 */
GListModel *
adw_navigation_view_get_navigation_stack (AdwNavigationView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_VIEW (self), NULL);

  if (self->navigation_stack_model)
    return g_object_ref (self->navigation_stack_model);

  self->navigation_stack_model = adw_navigation_view_model_new (self);
  g_object_add_weak_pointer (G_OBJECT (self->navigation_stack_model),
                             (gpointer *) &self->navigation_stack_model);

  return self->navigation_stack_model;
}
