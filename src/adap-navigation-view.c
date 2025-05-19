/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adap-navigation-view-private.h"

#include "adap-gizmo-private.h"
#include "adap-marshalers.h"
#include "adap-shadow-helper-private.h"
#include "adap-spring-animation.h"
#include "adap-swipeable.h"
#include "adap-swipe-tracker.h"
#include "adap-widget-utils-private.h"

/**
 * AdapNavigationView:
 *
 * A page-based navigation container.
 *
 * <picture>
 *   <source srcset="navigation-view-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="navigation-view.png" alt="navigation-view">
 * </picture>
 *
 * `AdapNavigationView` presents one child at a time, similar to
 * [class@Gtk.Stack].
 *
 * `AdapNavigationView` can only contain [class@NavigationPage] children.
 *
 * It maintains a navigation stack that can be controlled with
 * [method@NavigationView.push] and [method@NavigationView.pop]. The whole
 * navigation stack can also be replaced using [method@NavigationView.replace].
 *
 * `AdapNavigationView` allows to manage pages statically or dynamically.
 *
 * Static pages can be added using the [method@NavigationView.add] method. The
 * `AdapNavigationView` will keep a reference to these pages, but they aren't
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
 * When used inside `AdapNavigationView`, [class@HeaderBar] will automatically
 * display a back button that can be used to go back to the previous page when
 * possible. The button also has a context menu, allowing to pop multiple pages
 * at once, potentially across multiple navigation views.
 *
 * Set [property@HeaderBar:show-back-button] to `FALSE` to disable this behavior
 * in rare scenarios where it's unwanted.
 *
 * `AdapHeaderBar` will also display the title of the `AdapNavigationPage` it's
 * placed into, so most applications shouldn't need to customize it at all.
 *
 * ## Shortcuts and Gestures
 *
 * `AdapNavigationView` supports the following shortcuts for going to the
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
 * `AdapNavigationView` defines actions for controlling the navigation stack.
 * actions for controlling the navigation stack:
 *
 * - `navigation.push` takes a string parameter specifying the tag of the page to
 * push, and is equivalent to calling [method@NavigationView.push_by_tag].
 *
 * - `navigation.pop` doesn't take any parameters and pops the current page from
 * the navigation stack, equivalent to calling [method@NavigationView.pop].
 *
 * ## `AdapNavigationView` as `GtkBuildable`
 *
 * `AdapNavigationView` allows to add pages as children, equivalent to using the
 * [method@NavigationView.add] method.
 *
 * Example of an `AdapNavigationView` UI definition:
 *
 * ```xml
 * <object class="AdapNavigationView">
 *   <child>
 *     <object class="AdapNavigationPage">
 *       <property name="title" translatable="yes">Page 1</property>
 *       <property name="child">
 *         <object class="AdapToolbarView">
 *           <child type="top">
 *             <object class="AdapHeaderBar"/>
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
 *     <object class="AdapNavigationPage">
 *       <property name="title" translatable="yes">Page 2</property>
 *       <property name="tag">page-2</property>
 *       <property name="child">
 *         <object class="AdapToolbarView">
 *           <child type="top">
 *             <object class="AdapHeaderBar"/>
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
 * `AdapNavigationView` has a single CSS node with the name `navigation-view`.
 *
 * ## Accessibility
 *
 * `AdapNavigationView` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

/**
 * AdapNavigationPage:
 *
 * A page within [class@NavigationView] or [class@NavigationSplitView].
 *
 * Each page has a child widget, a title and optionally a tag.
 *
 * The [signal@NavigationPage::showing], [signal@NavigationPage::shown],
 * [signal@NavigationPage::hiding] and [signal@NavigationPage::hidden] signals
 * can be used to track the page's visibility within its `AdapNavigationView`.
 *
 * ## Header Bar Integration
 *
 * When placed inside `AdapNavigationPage`, [class@HeaderBar] will display the
 * page title instead of window title.
 *
 * When used together with [class@NavigationView], it will also display a back
 * button that can be used to go back to the previous page. Set
 * [property@HeaderBar:show-back-button] to `FALSE` to disable that behavior if
 * it's unwanted.
 *
 * ## CSS Nodes
 *
 * `AdapNavigationPage` has a single CSS node with name
 * `navigation-view-page`.
 *
 * ## Accessibility
 *
 * `AdapNavigationPage` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
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

  AdapNavigationView *child_view;

  int nav_split_views;
} AdapNavigationPagePrivate;

static void adap_navigation_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapNavigationPage, adap_navigation_page, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdapNavigationPage)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_navigation_page_buildable_init))

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

struct _AdapNavigationView
{
  GtkWidget parent_instance;

  GHashTable *tag_mapping;
  GListStore *navigation_stack;

  gboolean animate_transitions;
  gboolean pop_on_escape;

  AdapAnimation *transition;
  AdapNavigationPage *showing_page;
  AdapNavigationPage *hiding_page;
  gboolean transition_pop;
  gboolean transition_cancel;
  double transition_progress;
  gboolean gesture_active;

  AdapShadowHelper *shadow_helper;
  AdapSwipeTracker *swipe_tracker;

  GtkWidget *shield;

  GListModel *navigation_stack_model;
};

static void adap_navigation_view_buildable_init (GtkBuildableIface *iface);
static void adap_navigation_view_swipeable_init (AdapSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapNavigationView, adap_navigation_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_navigation_view_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADAP_TYPE_SWIPEABLE, adap_navigation_view_swipeable_init))

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

#define ADAP_TYPE_NAVIGATION_VIEW_MODEL (adap_navigation_view_model_get_type ())

G_DECLARE_FINAL_TYPE (AdapNavigationViewModel, adap_navigation_view_model, ADAP, NAVIGATION_VIEW_MODEL, GObject)

struct _AdapNavigationViewModel
{
  GObject parent_instance;
  AdapNavigationView *view;
};

static GType
adap_navigation_view_model_get_item_type (GListModel *model)
{
  return ADAP_TYPE_NAVIGATION_PAGE;
}

static guint
adap_navigation_view_model_get_n_items (GListModel *model)
{
  AdapNavigationViewModel *self = ADAP_NAVIGATION_VIEW_MODEL (model);

  return g_list_model_get_n_items (G_LIST_MODEL (self->view->navigation_stack));
}

static gpointer
adap_navigation_view_model_get_item (GListModel *model,
                                    guint       position)
{
  AdapNavigationViewModel *self = ADAP_NAVIGATION_VIEW_MODEL (model);

  return g_list_model_get_item (G_LIST_MODEL (self->view->navigation_stack),
                                position);
}

static void
adap_navigation_view_model_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adap_navigation_view_model_get_item_type;
  iface->get_n_items = adap_navigation_view_model_get_n_items;
  iface->get_item = adap_navigation_view_model_get_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapNavigationViewModel, adap_navigation_view_model, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adap_navigation_view_model_list_model_init))

static void
adap_navigation_view_model_init (AdapNavigationViewModel *self)
{
}

static void
adap_navigation_view_model_class_init (AdapNavigationViewModelClass *class)
{
}

static GListModel *
adap_navigation_view_model_new (AdapNavigationView *view)
{
  AdapNavigationViewModel *model;

  model = g_object_new (ADAP_TYPE_NAVIGATION_VIEW_MODEL, NULL);
  model->view = view;

  return G_LIST_MODEL (model);
}

static gboolean
get_remove_on_pop (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  return priv->remove_on_pop;
}

static void
set_remove_on_pop (AdapNavigationPage *self,
                   gboolean           remove_on_pop)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  priv->remove_on_pop = remove_on_pop;
}

static void
adap_navigation_page_realize (GtkWidget *widget)
{
  AdapNavigationPage *self = ADAP_NAVIGATION_PAGE (widget);
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  GTK_WIDGET_CLASS (adap_navigation_page_parent_class)->realize (widget);

  if (!(priv->title && *priv->title) && !priv->child_view && priv->nav_split_views == 0) {
    g_warning ("AdapNavigationPage %p is missing a title. To hide a header bar " \
               "title, consider using AdapHeaderBar:show-title instead.", self);
  }
}

static void
adap_navigation_page_dispose (GObject *object)
{
  AdapNavigationPage *self = ADAP_NAVIGATION_PAGE (object);
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_clear_pointer (&priv->child, gtk_widget_unparent);

  if (priv->child_view) {
    g_object_remove_weak_pointer (G_OBJECT (priv->child_view),
                                  (gpointer *) &priv->child_view);

    priv->child_view = NULL;
  }

  G_OBJECT_CLASS (adap_navigation_page_parent_class)->dispose (object);
}

static void
adap_navigation_page_finalize (GObject *object)
{
  AdapNavigationPage *self = ADAP_NAVIGATION_PAGE (object);
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_free (priv->title);
  g_free (priv->tag);

  if (priv->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (priv->last_focus),
                                  (gpointer *) &priv->last_focus);

  G_OBJECT_CLASS (adap_navigation_page_parent_class)->finalize (object);
}

static void
adap_navigation_page_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdapNavigationPage *self = ADAP_NAVIGATION_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, adap_navigation_page_get_child (self));
    break;
  case PAGE_PROP_TAG:
    g_value_set_string (value, adap_navigation_page_get_tag (self));
    break;
  case PAGE_PROP_TITLE:
    g_value_set_string (value, adap_navigation_page_get_title (self));
    break;
  case PAGE_PROP_CAN_POP:
    g_value_set_boolean (value, adap_navigation_page_get_can_pop (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_navigation_page_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdapNavigationPage *self = ADAP_NAVIGATION_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    adap_navigation_page_set_child (self, g_value_get_object (value));
    break;
  case PAGE_PROP_TAG:
    adap_navigation_page_set_tag (self, g_value_get_string (value));
    break;
  case PAGE_PROP_TITLE:
    adap_navigation_page_set_title (self, g_value_get_string (value));
    break;
  case PAGE_PROP_CAN_POP:
    adap_navigation_page_set_can_pop (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_navigation_page_class_init (AdapNavigationPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_navigation_page_dispose;
  object_class->finalize = adap_navigation_page_finalize;
  object_class->get_property = adap_navigation_page_get_property;
  object_class->set_property = adap_navigation_page_set_property;

  widget_class->realize = adap_navigation_page_realize;
  widget_class->compute_expand = adap_widget_compute_expand;

  /**
   * AdapNavigationPage:child: (attributes org.gtk.Property.get=adap_navigation_page_get_child org.gtk.Property.set=adap_navigation_page_set_child)
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
   * AdapNavigationPage:tag: (attributes org.gtk.Property.get=adap_navigation_page_get_tag org.gtk.Property.set=adap_navigation_page_set_tag)
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
   * AdapNavigationPage:title: (attributes org.gtk.Property.get=adap_navigation_page_get_title org.gtk.Property.set=adap_navigation_page_set_title)
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
   * AdapNavigationPage:can-pop: (attributes org.gtk.Property.get=adap_navigation_page_get_can_pop org.gtk.Property.set=adap_navigation_page_set_can_pop)
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
   * AdapNavigationPage::showing:
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
                  G_STRUCT_OFFSET (AdapNavigationPageClass, showing),
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_SHOWING],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapNavigationPage::shown:
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
                  G_STRUCT_OFFSET (AdapNavigationPageClass, shown),
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_SHOWN],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapNavigationPage::hiding:
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
                  G_STRUCT_OFFSET (AdapNavigationPageClass, hiding),
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_HIDING],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapNavigationPage::hidden:
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
                  G_STRUCT_OFFSET (AdapNavigationPageClass, hidden),
                  NULL, NULL,
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (page_signals[PAGE_SIGNAL_HIDDEN],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "navigation-view-page");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adap_navigation_page_init (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  priv->title = g_strdup ("");
  priv->can_pop = TRUE;

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, priv->title,
                                  -1);
}

static void
adap_navigation_page_buildable_add_child (GtkBuildable *buildable,
                                         GtkBuilder   *builder,
                                         GObject      *child,
                                         const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_navigation_page_set_child (ADAP_NAVIGATION_PAGE (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_navigation_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_navigation_page_buildable_add_child;
}

static void
switch_page (AdapNavigationView *self,
             AdapNavigationPage *prev_page,
             AdapNavigationPage *page,
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
    adap_animation_skip (self->transition);

  if (focus && prev_page && gtk_widget_is_ancestor (focus, GTK_WIDGET (prev_page))) {
    AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (prev_page);

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
    AdapNavigationPage *hiding_page = g_steal_pointer (&self->hiding_page);

    adap_navigation_page_hidden (hiding_page);

    adap_animation_reset (self->transition);

    if (self->transition_pop && get_remove_on_pop (hiding_page))
      adap_navigation_view_remove (self, hiding_page);
    else
      gtk_widget_set_child_visible (GTK_WIDGET (hiding_page), FALSE);

    g_object_unref (hiding_page);
  }

  if (page) {
    AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (page);

    gtk_widget_set_child_visible (GTK_WIDGET (page), TRUE);

    if (page != self->showing_page)
      adap_navigation_page_showing (page);

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

  adap_spring_animation_set_value_from (ADAP_SPRING_ANIMATION (self->transition),
                                       self->transition_progress);
  adap_spring_animation_set_value_to (ADAP_SPRING_ANIMATION (self->transition),
                                     self->transition_cancel ? 0 : 1);
  adap_spring_animation_set_initial_velocity (ADAP_SPRING_ANIMATION (self->transition),
                                             velocity);
  adap_spring_animation_set_clamp (ADAP_SPRING_ANIMATION (self->transition), pop);

  adap_animation_reset (self->transition);

  if (prev_page && prev_page != self->hiding_page)
    adap_navigation_page_hiding (prev_page);

  g_set_object (&self->showing_page, page);
  g_set_object (&self->hiding_page, prev_page);
  self->transition_pop = pop;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  if (animate)
    adap_animation_play (self->transition);
  else
    adap_animation_skip (self->transition);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE]);
}

static void
push_to_stack (AdapNavigationView *self,
               AdapNavigationPage *page,
               gboolean           animate,
               double             velocity,
               gboolean           use_tag_for_errors)
{
  AdapNavigationPage *previous_page = adap_navigation_view_get_visible_page (self);

  if (g_list_store_find (self->navigation_stack, page, NULL)) {
    if (use_tag_for_errors) {
      g_critical ("Page with the tag '%s' is already in navigation stack\n",
                  adap_navigation_page_get_tag (page));
    } else {
      g_critical ("Page '%s' is already in navigation stack\n",
                  adap_navigation_page_get_title (page));
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
pop_from_stack (AdapNavigationView *self,
                AdapNavigationPage *page_to,
                gboolean           animate,
                double             velocity)
{
  AdapNavigationPage *old_page;
  AdapNavigationPage *new_page;
  GSList *popped = NULL, *l;
  guint length, pos, i;

  old_page = adap_navigation_view_get_visible_page (self);

  length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  g_assert (g_list_store_find (self->navigation_stack, page_to, &pos));

  for (i = pos + 1; i < length; i++) {
    AdapNavigationPage *page;

    page = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), i);
    popped = g_slist_prepend (popped, page);
  }

  g_list_store_splice (self->navigation_stack, pos + 1, length - pos - 1, NULL, 0);

  new_page = adap_navigation_view_get_visible_page (self);

  switch_page (self, old_page, new_page, TRUE, animate, velocity);

  for (l = popped; l; l = l->next) {
    AdapNavigationPage *c = l->data;

    g_signal_emit (self, signals[SIGNAL_POPPED], 0, c);

    if (c != old_page && get_remove_on_pop (c))
      adap_navigation_view_remove (self, c);
  }

  if (self->navigation_stack_model)
    g_list_model_items_changed (self->navigation_stack_model,
                                pos + 1, length - pos - 1, 0);

  g_slist_free_full (popped, g_object_unref);
}

static void
transition_cb (double             value,
               AdapNavigationView *self)
{
  self->transition_progress = value;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
transition_done_cb (AdapNavigationView *self)
{
  if (self->hiding_page) {
    AdapNavigationPage *hiding_page = g_steal_pointer (&self->hiding_page);

    if (self->transition_cancel) {
      adap_navigation_page_shown (hiding_page);

      gtk_widget_insert_before (GTK_WIDGET (hiding_page), GTK_WIDGET (self), NULL);
    } else {
      adap_navigation_page_hidden (hiding_page);

      if (self->transition_pop && get_remove_on_pop (hiding_page))
        adap_navigation_view_remove (self, hiding_page);
      else
        gtk_widget_set_child_visible (GTK_WIDGET (hiding_page), FALSE);
    }

    g_object_unref (hiding_page);
  }

  if (self->showing_page) {
    AdapNavigationPage *showing_page = g_steal_pointer (&self->showing_page);

    if (self->transition_cancel) {
      adap_navigation_page_hidden (showing_page);

      if (!self->transition_pop && get_remove_on_pop (showing_page))
        adap_navigation_view_remove (self, showing_page);
      else
        gtk_widget_set_child_visible (GTK_WIDGET (showing_page), FALSE);
    } else {
      adap_navigation_page_shown (showing_page);

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
navigation_push_cb (AdapNavigationView *self,
                    const char        *action_name,
                    GVariant          *params)
{
  AdapNavigationPage *page;
  GtkWidget *parent;
  const char *tag;

  tag = g_variant_get_string (params, NULL);
  page = adap_navigation_view_find_page (self, tag);

  if (page) {
    push_to_stack (self, page, self->animate_transitions, 0, TRUE);
    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent && gtk_widget_activate_action_variant (parent, "navigation.push", params))
    return;

  g_critical ("No page with the tag '%s' found in AdapNavigationView %p",
              tag, self);
}

static void
navigation_pop_cb (AdapNavigationView *self)
{
  GtkWidget *parent;

  if (adap_navigation_view_pop (self))
    return;

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "navigation.pop", NULL);
}

static AdapNavigationPage *
get_next_page (AdapNavigationView *self)
{
  AdapNavigationPage *page = NULL;
  GtkWidget *parent;

  g_signal_emit (self, signals[SIGNAL_GET_NEXT_PAGE], 0, &page);

  if (!page)
    return NULL;

  parent = gtk_widget_get_parent (GTK_WIDGET (page));

  if (parent && parent != GTK_WIDGET (self)) {
    g_critical ("AdapNavigationView::get-next-page result already has a parent");
    g_object_unref (page);
    return NULL;
  }

  if (!parent)
    set_remove_on_pop (page, TRUE);

  return page;
}

static gboolean
pop_shortcut_cb (AdapNavigationView *self)
{
  AdapNavigationPage *page = adap_navigation_view_get_visible_page (self);

  if (!page)
    return GDK_EVENT_PROPAGATE;

  /* Stop it so that it's not propagated to parent navigation views */
  if (!adap_navigation_page_get_can_pop (page))
    return GDK_EVENT_STOP;

  if (adap_navigation_view_pop (self))
    return GDK_EVENT_STOP;

  return GDK_EVENT_PROPAGATE;
}

static gboolean
push_shortcut_cb (AdapNavigationView *self)
{
  AdapNavigationPage *next_page = get_next_page (self);

  if (!next_page)
    return GDK_EVENT_PROPAGATE;

  adap_navigation_view_push (self, next_page);

  g_object_unref (next_page);

  return GDK_EVENT_STOP;
}

static gboolean
escape_shortcut_cb (AdapNavigationView *self)
{
  if (self->pop_on_escape)
    return pop_shortcut_cb (self);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
back_forward_shortcut_cb (AdapNavigationView *self,
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
                                AdapNavigationView *self)
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
    AdapNavigationPage *page = adap_navigation_view_get_visible_page (self);

    if (!page) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    /* Consume it so that it's not propagated to parent navigation views */
    if (!adap_navigation_page_get_can_pop (page)) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
      return;
    }

    if (!adap_navigation_view_get_previous_page (self, page)) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    adap_navigation_view_pop (self);
  } else {
    AdapNavigationPage *next_page = get_next_page (self);

    if (!next_page) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    adap_navigation_view_push (self, next_page);

    g_object_unref (next_page);
  }

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
add_page (AdapNavigationView *self,
          AdapNavigationPage *page,
          gboolean           auto_push)
{
  const char *tag;

  tag = adap_navigation_page_get_tag (page);

  if (tag && adap_navigation_view_find_page (self, tag)) {
    g_critical ("Duplicate page tag in AdapNavigationView: %s",
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
maybe_add_page (AdapNavigationView *self,
                AdapNavigationPage *page)
{
  const char *tag;

  if (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (self))
    return TRUE;

  tag = adap_navigation_page_get_tag (page);

  if (tag && adap_navigation_view_find_page (self, tag)) {
    g_critical ("Duplicate page tag in AdapNavigationView: %s", tag);

    return FALSE;
  }

  add_page (self, page, FALSE);
  set_remove_on_pop (page, TRUE);
  return TRUE;
}

static void
remove_page (AdapNavigationView *self,
             AdapNavigationPage *page,
             gboolean           check_stack)
{
  const char *tag;

  if (page == self->hiding_page)
    adap_animation_skip (self->transition);

  /* Avoid modifying the navigation stack */
  if (check_stack && g_list_store_find (self->navigation_stack, page, NULL)) {
    set_remove_on_pop (page, TRUE);
    return;
  }

  tag = adap_navigation_page_get_tag (page);

  if (tag)
    g_hash_table_remove (self->tag_mapping, tag);

  gtk_widget_unparent (GTK_WIDGET (page));
}

static void
prepare_cb (AdapSwipeTracker        *tracker,
            AdapNavigationDirection  direction,
            AdapNavigationView      *self)
{
  AdapNavigationPage *visible_page = adap_navigation_view_get_visible_page (self);
  AdapNavigationPage *new_page;
  gboolean remove_on_pop = FALSE;

  if (!visible_page)
    return;

  if (direction == ADAP_NAVIGATION_DIRECTION_BACK) {
    if (!adap_navigation_page_get_can_pop (visible_page))
      return;

    new_page = adap_navigation_view_get_previous_page (self, visible_page);

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
    adap_animation_skip (self->transition);

  self->showing_page = new_page;
  self->hiding_page = g_object_ref (visible_page);

  self->transition_pop = (direction == ADAP_NAVIGATION_DIRECTION_BACK);

  if (direction == ADAP_NAVIGATION_DIRECTION_BACK) {
    g_object_ref (new_page);
  } else {
    if (remove_on_pop)
      set_remove_on_pop (new_page, TRUE);

    gtk_widget_insert_before (GTK_WIDGET (self->shield), GTK_WIDGET (self), NULL);
  }

  gtk_widget_insert_before (GTK_WIDGET (self->shield), GTK_WIDGET (self), NULL);
  gtk_widget_set_child_visible (self->shield, TRUE);

  adap_navigation_page_showing (self->showing_page);
  adap_navigation_page_hiding (self->hiding_page);

  self->gesture_active = TRUE;

  gtk_widget_set_child_visible (GTK_WIDGET (self->showing_page), TRUE);

  adap_spring_animation_set_value_from (ADAP_SPRING_ANIMATION (self->transition), 0);
  adap_animation_reset (self->transition);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  adap_swipe_tracker_set_upper_overshoot (self->swipe_tracker, TRUE);
}

static void
update_swipe_cb (AdapSwipeTracker   *tracker,
                 double             progress,
                 AdapNavigationView *self)
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
end_swipe_cb (AdapSwipeTracker   *tracker,
              double             velocity,
              double             to,
              AdapNavigationView *self)
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

    adap_spring_animation_set_value_from (ADAP_SPRING_ANIMATION (self->transition),
                                         self->transition_progress);
    adap_spring_animation_set_value_to (ADAP_SPRING_ANIMATION (self->transition), ABS (to));

    if (self->transition_pop)
      adap_spring_animation_set_initial_velocity (ADAP_SPRING_ANIMATION (self->transition),
                                                 -velocity);
    else
      adap_spring_animation_set_initial_velocity (ADAP_SPRING_ANIMATION (self->transition),
                                                 velocity);

    adap_spring_animation_set_clamp (ADAP_SPRING_ANIMATION (self->transition),
                                    !self->transition_pop);

    if (animate)
      adap_animation_play (self->transition);
    else
      adap_animation_skip (self->transition);
  }

  adap_swipe_tracker_set_upper_overshoot (self->swipe_tracker, FALSE);
}

static void
set_child_view (AdapNavigationPage *self,
                AdapNavigationView *view)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

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
adap_navigation_view_measure (GtkWidget      *widget,
                             GtkOrientation  orientation,
                             int             for_size,
                             int            *minimum,
                             int            *natural,
                             int            *minimum_baseline,
                             int            *natural_baseline)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (widget);
  AdapNavigationPage *visible_page = NULL;
  int min = 0, nat = 0, min_baseline = -1, nat_baseline = -1;
  int last_min = 0, last_nat = 0, last_min_baseline = -1, last_nat_baseline = -1;

  visible_page = adap_navigation_view_get_visible_page (self);

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
adap_navigation_view_size_allocate (GtkWidget *widget,
                                   int        width,
                                   int        height,
                                   int        baseline)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (widget);
  AdapNavigationPage *visible_page = NULL;
  GtkWidget *static_page = NULL, *moving_page = NULL;
  gboolean is_rtl;
  double progress;
  int offset;

  visible_page = adap_navigation_view_get_visible_page (self);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  if (!self->hiding_page || !self->showing_page) {
    if (visible_page)
      gtk_widget_allocate (GTK_WIDGET (visible_page), width, height, baseline, NULL);

    adap_shadow_helper_size_allocate (self->shadow_helper, 0, 0,
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

    adap_shadow_helper_size_allocate (self->shadow_helper,
                                     MAX (0, offset), height,
                                     baseline, width - offset, 0, progress,
                                     GTK_PAN_DIRECTION_LEFT);
  } else {
    if (moving_page)
      gtk_widget_allocate (moving_page, width, height, baseline,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0)));

    adap_shadow_helper_size_allocate (self->shadow_helper,
                                     MAX (0, offset), height,
                                     baseline, 0, 0, progress,
                                     GTK_PAN_DIRECTION_RIGHT);
  }
}

static void
adap_navigation_view_snapshot (GtkWidget   *widget,
                              GtkSnapshot *snapshot)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (widget);
  AdapNavigationPage *visible_page = NULL;
  GtkWidget *static_page = NULL, *moving_page = NULL;
  int width, height;
  int offset;
  int clip_x, clip_width;
  double progress;

  visible_page = adap_navigation_view_get_visible_page (self);

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

  adap_shadow_helper_snapshot (self->shadow_helper, snapshot);
}

static void
adap_navigation_view_root (GtkWidget *widget)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (widget);
  GtkWidget *parent_page;

  GTK_WIDGET_CLASS (adap_navigation_view_parent_class)->root (widget);

  parent_page = adap_widget_get_ancestor (widget, ADAP_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  if (parent_page)
    set_child_view (ADAP_NAVIGATION_PAGE (parent_page), self);
}

static void
adap_navigation_view_unroot (GtkWidget *widget)
{
  GtkWidget *parent_page;

  parent_page = adap_widget_get_ancestor (widget, ADAP_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  if (parent_page)
    set_child_view (ADAP_NAVIGATION_PAGE (parent_page), NULL);

  GTK_WIDGET_CLASS (adap_navigation_view_parent_class)->unroot (widget);
}

static void
adap_navigation_view_direction_changed (GtkWidget        *widget,
                                       GtkTextDirection  previous_direction)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (widget);
  gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;

  adap_swipe_tracker_set_reversed (self->swipe_tracker, is_rtl);
}

static void
adap_navigation_view_dispose (GObject *object)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (object);
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

  G_OBJECT_CLASS (adap_navigation_view_parent_class)->dispose (object);
}

static void
adap_navigation_view_finalize (GObject *object)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (object);

  if (self->navigation_stack_model)
    g_object_remove_weak_pointer (G_OBJECT (self->navigation_stack_model),
                                  (gpointer *) &self->navigation_stack_model);

  G_OBJECT_CLASS (adap_navigation_view_parent_class)->finalize (object);
}

static void
adap_navigation_view_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (object);

  switch (prop_id) {
  case PROP_VISIBLE_PAGE:
    g_value_set_object (value, adap_navigation_view_get_visible_page (self));
    break;
  case PROP_ANIMATE_TRANSITIONS:
    g_value_set_boolean (value, adap_navigation_view_get_animate_transitions (self));
    break;
  case PROP_POP_ON_ESCAPE:
    g_value_set_boolean (value, adap_navigation_view_get_pop_on_escape (self));
    break;
  case PROP_NAVIGATION_STACK:
    g_value_take_object (value, adap_navigation_view_get_navigation_stack (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_navigation_view_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (object);

  switch (prop_id) {
  case PROP_ANIMATE_TRANSITIONS:
    adap_navigation_view_set_animate_transitions (self, g_value_get_boolean (value));
    break;
  case PROP_POP_ON_ESCAPE:
    adap_navigation_view_set_pop_on_escape (self, g_value_get_boolean (value));
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
adap_navigation_view_class_init (AdapNavigationViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_navigation_view_dispose;
  object_class->finalize = adap_navigation_view_finalize;
  object_class->get_property = adap_navigation_view_get_property;
  object_class->set_property = adap_navigation_view_set_property;

  widget_class->measure = adap_navigation_view_measure;
  widget_class->size_allocate = adap_navigation_view_size_allocate;
  widget_class->snapshot = adap_navigation_view_snapshot;
  widget_class->root = adap_navigation_view_root;
  widget_class->unroot = adap_navigation_view_unroot;
  widget_class->direction_changed = adap_navigation_view_direction_changed;
  widget_class->get_request_mode = adap_widget_get_request_mode;
  widget_class->compute_expand = adap_widget_compute_expand;

  /**
   * AdapNavigationView:visible-page: (attributes org.gtk.Property.get=adap_navigation_view_get_visible_page)
   *
   * The currently visible page.
   *
   * Since: 1.4
   */
  props[PROP_VISIBLE_PAGE] =
    g_param_spec_object ("visible-page", NULL, NULL,
                         ADAP_TYPE_NAVIGATION_PAGE,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdapNavigationView:animate-transitions: (attributes org.gtk.Property.get=adap_navigation_view_get_animate_transitions org.gtk.Property.set=adap_navigation_view_set_animate_transitions)
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
   * AdapNavigationView:pop-on-escape: (attributes org.gtk.Property.get=adap_navigation_view_get_pop_on_escape org.gtk.Property.set=adap_navigation_view_set_pop_on_escape)
   *
   * Whether pressing Escape pops the current page.
   *
   * Applications using `AdapNavigationView` to implement a browser may want to
   * disable it.
   *
   * Since: 1.4
   */
  props[PROP_POP_ON_ESCAPE] =
    g_param_spec_boolean ("pop-on-escape", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapNavigationView:navigation-stack: (attributes org.gtk.Property.get=adap_navigation_view_get_navigation_stack)
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
   * AdapNavigationView::pushed:
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
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_PUSHED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapNavigationView::popped:
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
                  adap_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  ADAP_TYPE_NAVIGATION_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_POPPED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__OBJECTv);

  /**
   * AdapNavigationView::replaced:
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
                  adap_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_REPLACED],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__VOIDv);

  /**
   * AdapNavigationView::get-next-page:
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
                  adap_marshal_OBJECT__VOID,
                  ADAP_TYPE_NAVIGATION_PAGE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_GET_NEXT_PAGE],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_OBJECT__VOIDv);

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
adap_navigation_view_init (AdapNavigationView *self)
{
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  AdapAnimationTarget *target;
  GtkGesture *gesture;

  self->animate_transitions = TRUE;
  self->pop_on_escape = TRUE;

  self->navigation_stack = g_list_store_new (ADAP_TYPE_NAVIGATION_PAGE);

  self->tag_mapping = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc) transition_cb,
                                              self, NULL);
  self->transition = adap_spring_animation_new (GTK_WIDGET (self), 0, 1,
                                               adap_spring_params_new (1, 1, 1000),
                                               target);
  g_signal_connect_swapped (self->transition, "done",
                            G_CALLBACK (transition_done_cb), self);

  self->shadow_helper = adap_shadow_helper_new (GTK_WIDGET (self));

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
  g_signal_connect (gesture, "pressed",
                    G_CALLBACK (back_forward_button_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));

  self->swipe_tracker = adap_swipe_tracker_new (ADAP_SWIPEABLE (self));
  adap_swipe_tracker_set_reversed (self->swipe_tracker, is_rtl);

  g_signal_connect (self->swipe_tracker, "prepare",
                    G_CALLBACK (prepare_cb), self);
  g_signal_connect (self->swipe_tracker, "update-swipe",
                    G_CALLBACK (update_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "end-swipe",
                    G_CALLBACK (end_swipe_cb), self);

  self->shield = adap_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_child_visible (self->shield, FALSE);
  gtk_widget_set_parent (self->shield, GTK_WIDGET (self));
}

static void
adap_navigation_view_buildable_add_child (GtkBuildable *buildable,
                                         GtkBuilder   *builder,
                                         GObject      *child,
                                         const char   *type)
{
  if (ADAP_IS_NAVIGATION_PAGE (child))
    adap_navigation_view_add (ADAP_NAVIGATION_VIEW (buildable),
                             ADAP_NAVIGATION_PAGE (child));
  else if (GTK_IS_WIDGET (child))
    g_warning ("Cannot add an object of type %s to AdapNavigationView",
               g_type_name (G_OBJECT_TYPE (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_navigation_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_navigation_view_buildable_add_child;
}

static double
adap_navigation_view_get_distance (AdapSwipeable *swipeable)
{
  return gtk_widget_get_width (GTK_WIDGET (swipeable));
}

static double *
adap_navigation_view_get_snap_points (AdapSwipeable *swipeable,
                                     int          *n_snap_points)
{
  AdapNavigationView *self = ADAP_NAVIGATION_VIEW (swipeable);
  AdapNavigationPage *visible_page;
  double *points, lower, upper;
  int n;

  visible_page = adap_navigation_view_get_visible_page (self);

  if (self->showing_page || self->hiding_page) {
    lower = self->transition_pop && self->gesture_active ? -1 : 0;
    upper = self->transition_pop || !self->gesture_active ? 0 : 1;
  } else {
    AdapNavigationPage *prev_page, *next_page;

    if (visible_page)
      prev_page = adap_navigation_view_get_previous_page (self, visible_page);
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
adap_navigation_view_get_progress (AdapSwipeable *swipeable)
{
  return 0;
}

static double
adap_navigation_view_get_cancel_progress (AdapSwipeable *swipeable)
{
  return 0;
}

static void
adap_navigation_view_swipeable_init (AdapSwipeableInterface *iface)
{
  iface->get_distance = adap_navigation_view_get_distance;
  iface->get_snap_points = adap_navigation_view_get_snap_points;
  iface->get_progress = adap_navigation_view_get_progress;
  iface->get_cancel_progress = adap_navigation_view_get_cancel_progress;
}

/**
 * adap_navigation_page_new:
 * @child: the child widget
 * @title: the page title
 *
 * Creates a new `AdapNavigationPage`.
 *
 * Returns: the new created `AdapNavigationPage`
 *
 * Since: 1.4
 */
AdapNavigationPage *
adap_navigation_page_new (GtkWidget  *child,
                         const char *title)
{
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADAP_TYPE_NAVIGATION_PAGE,
                       "child", child,
                       "title", title,
                       NULL);
}

/**
 * adap_navigation_page_new_with_tag:
 * @child: the child widget
 * @title: the page title
 * @tag: the page tag
 *
 * Creates a new `AdapNavigationPage` with provided tag.
 *
 * Returns: the new created `AdapNavigationPage`
 *
 * Since: 1.4
 */
AdapNavigationPage *
adap_navigation_page_new_with_tag (GtkWidget  *child,
                                  const char *title,
                                  const char *tag)
{
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (tag != NULL, NULL);

  return g_object_new (ADAP_TYPE_NAVIGATION_PAGE,
                       "child", child,
                       "title", title,
                       "tag", tag,
                       NULL);
}

/**
 * adap_navigation_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a navigation page
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.4
 */
GtkWidget *
adap_navigation_page_get_child (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (self), NULL);

  priv = adap_navigation_page_get_instance_private (self);

  return priv->child;
}

/**
 * adap_navigation_page_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a navigation page
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.4
 */
void
adap_navigation_page_set_child (AdapNavigationPage *self,
                               GtkWidget         *child)
{
  AdapNavigationPagePrivate *priv;

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adap_navigation_page_get_instance_private (self);

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
 * adap_navigation_page_get_tag: (attributes org.gtk.Method.get_property=tag)
 * @self: a navigation page
 *
 * Gets the tag of @self.
 *
 * Returns: (transfer none) (nullable): the page tag
 *
 * Since: 1.4
 */
const char *
adap_navigation_page_get_tag (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (self), NULL);

  priv = adap_navigation_page_get_instance_private (self);

  return priv->tag;
}

/**
 * adap_navigation_page_set_tag: (attributes org.gtk.Method.set_property=tag)
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
adap_navigation_page_set_tag (AdapNavigationPage *self,
                             const char        *tag)
{
  AdapNavigationPagePrivate *priv;
  GtkWidget *parent;
  AdapNavigationView *view = NULL;

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  priv = adap_navigation_page_get_instance_private (self);

  if (!g_strcmp0 (priv->tag, tag))
    return;

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (ADAP_IS_NAVIGATION_VIEW (parent))
    view = ADAP_NAVIGATION_VIEW (parent);

  if (tag && view && adap_navigation_view_find_page (view, tag)) {
    g_critical ("Duplicate page tag in AdapNavigationView: %s", tag);

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
 * adap_navigation_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a navigation page
 *
 * Gets the title of @self.
 *
 * Returns: (transfer none): the title of @self
 *
 * Since: 1.4
 */
const char *
adap_navigation_page_get_title (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (self), NULL);

  priv = adap_navigation_page_get_instance_private (self);

  return priv->title;
}

/**
 * adap_navigation_page_set_title: (attributes org.gtk.Method.set_property=title)
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
adap_navigation_page_set_title (AdapNavigationPage *self,
                               const char        *title)
{
  AdapNavigationPagePrivate *priv;

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));
  g_return_if_fail (title != NULL);

  priv = adap_navigation_page_get_instance_private (self);

  if (!g_set_str (&priv->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, priv->title,
                                  -1);
}

/**
 * adap_navigation_page_get_can_pop: (attributes org.gtk.Method.get_property=can-pop)
 * @self: a navigation page
 *
 * Gets whether @self can be popped from navigation stack.
 *
 * Returns: whether the page can be popped from navigation stack
 *
 * Since: 1.4
 */
gboolean
adap_navigation_page_get_can_pop (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (self), FALSE);

  priv = adap_navigation_page_get_instance_private (self);

  return priv->can_pop;
}

/**
 * adap_navigation_page_set_can_pop: (attributes org.gtk.Method.set_property=can-pop)
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
adap_navigation_page_set_can_pop (AdapNavigationPage *self,
                                 gboolean           can_pop)
{
  AdapNavigationPagePrivate *priv;

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  priv = adap_navigation_page_get_instance_private (self);

  can_pop = !!can_pop;

  if (can_pop == priv->can_pop)
    return;

  priv->can_pop = can_pop;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_CAN_POP]);
}

AdapNavigationView *
adap_navigation_page_get_child_view (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (self), NULL);

  priv = adap_navigation_page_get_instance_private (self);

  return priv->child_view;
}

void
adap_navigation_page_showing (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_SHOWING], 0);
}

void
adap_navigation_page_shown (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_SHOWN], 0);
}

void
adap_navigation_page_hiding (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_HIDING], 0);
}

void
adap_navigation_page_hidden (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  if (!priv->block_signals)
    g_signal_emit (self, page_signals[PAGE_SIGNAL_HIDDEN], 0);
}

void
adap_navigation_page_block_signals (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  priv->block_signals++;
}

void
adap_navigation_page_unblock_signals (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  g_assert (priv->block_signals > 0);

  priv->block_signals--;
}

void
adap_navigation_page_add_child_nav_split_view (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  priv->nav_split_views++;
}

void
adap_navigation_page_remove_child_nav_split_view (AdapNavigationPage *self)
{
  AdapNavigationPagePrivate *priv = adap_navigation_page_get_instance_private (self);

  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (self));

  priv->nav_split_views--;
}

/**
 * adap_navigation_view_new:
 *
 * Creates a new `AdapNavigationView`.
 *
 * Returns: the new created `AdapNavigationView`
 *
 * Since: 1.4
 */
GtkWidget *
adap_navigation_view_new (void)
{
  return g_object_new (ADAP_TYPE_NAVIGATION_VIEW, NULL);
}

/**
 * adap_navigation_view_add:
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
adap_navigation_view_add (AdapNavigationView *self,
                         AdapNavigationPage *page)
{
  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
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
 * adap_navigation_view_remove:
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
adap_navigation_view_remove (AdapNavigationView *self,
                            AdapNavigationPage *page)
{
  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (page));
  g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (self));

  remove_page (self, page, TRUE);
}

/**
 * adap_navigation_view_find_page:
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
AdapNavigationPage *
adap_navigation_view_find_page (AdapNavigationView *self,
                               const char        *tag)
{
  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), NULL);
  g_return_val_if_fail (tag != NULL, NULL);

  return g_hash_table_lookup (self->tag_mapping, tag);
}

/**
 * adap_navigation_view_push:
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
adap_navigation_view_push (AdapNavigationView *self,
                          AdapNavigationPage *page)
{
  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (page));

  if (!maybe_add_page (self, page))
    return;

  push_to_stack (self, page, self->animate_transitions, 0, FALSE);
}

/**
 * adap_navigation_view_push_by_tag:
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
adap_navigation_view_push_by_tag (AdapNavigationView *self,
                                 const char        *tag)
{
  AdapNavigationPage *page;

  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (tag != NULL);

  page = adap_navigation_view_find_page (self, tag);

  if (page == NULL) {
    g_critical ("No page with the tag '%s' found in AdapNavigationView %p",
                tag, self);
    return;
  }

  push_to_stack (self, page, self->animate_transitions, 0, TRUE);
}

/**
 * adap_navigation_view_pop:
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
adap_navigation_view_pop (AdapNavigationView *self)
{
  AdapNavigationPage *page;
  AdapNavigationPage *prev_page;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), FALSE);

  page = adap_navigation_view_get_visible_page (self);

  if (!page)
    return FALSE;

  prev_page = adap_navigation_view_get_previous_page (self, page);

  if (!prev_page)
    return FALSE;

  pop_from_stack (self, prev_page, self->animate_transitions, 0);

  return TRUE;
}

/**
 * adap_navigation_view_pop_to_page:
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
adap_navigation_view_pop_to_page (AdapNavigationView *self,
                                 AdapNavigationPage *page)
{
  AdapNavigationPage *visible_page;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), FALSE);
  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (page), FALSE);

  visible_page = adap_navigation_view_get_visible_page (self);

  if (page == visible_page)
    return FALSE;

  if (!g_list_store_find (self->navigation_stack, page, NULL)) {
    g_critical ("Page '%s' is not in the navigation stack\n",
                adap_navigation_page_get_title (page));
    return FALSE;
  }

  pop_from_stack (self, page, self->animate_transitions, 0);

  return TRUE;
}

/**
 * adap_navigation_view_pop_to_tag:
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
adap_navigation_view_pop_to_tag (AdapNavigationView *self,
                                const char        *tag)
{
  AdapNavigationPage *page;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), FALSE);
  g_return_val_if_fail (tag != NULL, FALSE);

  page = adap_navigation_view_find_page (self, tag);

  if (page == NULL) {
    g_critical ("No page with the tag '%s' found in AdapNavigationView %p",
                tag, self);
    return FALSE;
  }

  return adap_navigation_view_pop_to_page (self, page);
}

/**
 * adap_navigation_view_replace:
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
adap_navigation_view_replace (AdapNavigationView  *self,
                             AdapNavigationPage **pages,
                             int                 n_pages)
{
  AdapNavigationPage *visible_page, *old_visible_page;
  GHashTable *added_pages;
  guint i, old_length;

  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (n_pages >= 0);

  visible_page = adap_navigation_view_get_visible_page (self);
  old_visible_page = visible_page;
  old_length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  added_pages = g_hash_table_new (g_direct_hash, g_direct_equal);

  for (i = 0; i < n_pages; i++) {
    if (!pages[i])
      continue;

    g_hash_table_insert (added_pages, pages[i], NULL);
  }

  for (i = 0; i < old_length; i++) {
    AdapNavigationPage *c;

    c = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack),
                               old_length - i - 1);

    if (get_remove_on_pop (c) &&
        !g_hash_table_contains (added_pages, c)) {
      if (c == visible_page) {
        adap_navigation_page_hiding (visible_page);
        adap_navigation_page_hidden (visible_page);
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
                  adap_navigation_page_get_title (pages[i]));
      continue;
    }

    if (!maybe_add_page (self, pages[i]))
      continue;

    g_hash_table_insert (added_pages, pages[i], NULL);
    g_list_store_append (self->navigation_stack, pages[i]);
  }

  if (g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack)) > 0) {
    AdapNavigationPage *new_visible_page = adap_navigation_view_get_visible_page (self);

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
 * adap_navigation_view_replace_with_tags:
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
adap_navigation_view_replace_with_tags (AdapNavigationView  *self,
                                       const char * const *tags,
                                       int                 n_tags)
{
  AdapNavigationPage **pages;
  int i;

  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));
  g_return_if_fail (n_tags >= 0);

  pages = g_new0 (AdapNavigationPage *, n_tags);

  for (i = 0; i < n_tags; i++) {
    AdapNavigationPage *page =
      adap_navigation_view_find_page (self, tags[i]);

    if (page == NULL) {
      g_critical ("No page with the tag '%s' found in AdapNavigationView %p",
                  tags[i], self);
      continue;
    }

    pages[i] = page;
  }

  adap_navigation_view_replace (self, pages, n_tags);

  g_free (pages);
}

/**
 * adap_navigation_view_get_visible_page: (attributes org.gtk.Method.get_property=visible-page)
 * @self: a navigation view
 *
 * Gets the currently visible page in @self.
 *
 * Returns: (transfer none) (nullable): the currently visible page
 *
 * Since: 1.4
 */
AdapNavigationPage *
adap_navigation_view_get_visible_page (AdapNavigationView *self)
{
  AdapNavigationPage *ret;
  guint length;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), NULL);

  length = g_list_model_get_n_items (G_LIST_MODEL (self->navigation_stack));

  if (length == 0)
    return NULL;

  ret = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), length - 1);

  g_object_unref (ret);

  return ret;
}

/**
 * adap_navigation_view_get_previous_page:
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
AdapNavigationPage *
adap_navigation_view_get_previous_page (AdapNavigationView *self,
                                       AdapNavigationPage *page)
{
  AdapNavigationPage *ret;
  guint pos;

  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), NULL);
  g_return_val_if_fail (ADAP_IS_NAVIGATION_PAGE (page), NULL);

  if (!g_list_store_find (self->navigation_stack, page, &pos))
    return NULL;

  if (pos == 0)
    return NULL;

  ret = g_list_model_get_item (G_LIST_MODEL (self->navigation_stack), pos - 1);

  g_object_unref (ret);

  return ret;
}

/**
 * adap_navigation_view_get_animate_transitions: (attributes org.gtk.Method.get_property=animate-transitions)
 * @self: a navigation view
 *
 * Gets whether @self animates page transitions.
 *
 * Returns: whether to animate page transitions
 *
 * Since: 1.4
 */
gboolean
adap_navigation_view_get_animate_transitions (AdapNavigationView *self)
{
  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), FALSE);

  return self->animate_transitions;
}

/**
 * adap_navigation_view_set_animate_transitions: (attributes org.gtk.Method.set_property=animate-transitions)
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
adap_navigation_view_set_animate_transitions (AdapNavigationView *self,
                                             gboolean           animate_transitions)
{
  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));

  animate_transitions = !!animate_transitions;

  if (animate_transitions == self->animate_transitions)
    return;

  self->animate_transitions = animate_transitions;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ANIMATE_TRANSITIONS]);
}

/**
 * adap_navigation_view_get_pop_on_escape: (attributes org.gtk.Method.get_property=pop-on-escape)
 * @self: a navigation view
 *
 * Gets whether pressing Escape pops the current page on @self.
 *
 * Returns: whether to pop the current page
 *
 * Since: 1.4
 */
gboolean
adap_navigation_view_get_pop_on_escape (AdapNavigationView *self)
{
  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), FALSE);

  return self->pop_on_escape;
}

/**
 * adap_navigation_view_set_pop_on_escape: (attributes org.gtk.Method.set_property=pop-on-escape)
 * @self: a navigation view
 * @pop_on_escape: whether to pop the current page when pressing Escape
 *
 * Sets whether pressing Escape pops the current page on @self.
 *
 * Applications using `AdapNavigationView` to implement a browser may want to
 * disable it.
 *
 * Since: 1.4
 */
void
adap_navigation_view_set_pop_on_escape (AdapNavigationView *self,
                                       gboolean           pop_on_escape)
{
  g_return_if_fail (ADAP_IS_NAVIGATION_VIEW (self));

  pop_on_escape = !!pop_on_escape;

  if (pop_on_escape == self->pop_on_escape)
    return;

  self->pop_on_escape = pop_on_escape;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POP_ON_ESCAPE]);
}

/**
 * adap_navigation_view_get_navigation_stack: (attributes org.gtk.Method.get_property=navigation-stack)
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
adap_navigation_view_get_navigation_stack (AdapNavigationView *self)
{
  g_return_val_if_fail (ADAP_IS_NAVIGATION_VIEW (self), NULL);

  if (self->navigation_stack_model)
    return g_object_ref (self->navigation_stack_model);

  self->navigation_stack_model = adap_navigation_view_model_new (self);
  g_object_add_weak_pointer (G_OBJECT (self->navigation_stack_model),
                             (gpointer *) &self->navigation_stack_model);

  return self->navigation_stack_model;
}
