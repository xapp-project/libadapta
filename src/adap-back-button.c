/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-back-button-private.h"

#include "adw-bin.h"
#include "adw-navigation-view-private.h"
#include "adw-widget-utils-private.h"

typedef struct {
  AdwBackButton *self;
  AdwNavigationView *view;
  AdwNavigationPage *page;
} NavigationViewData;

struct _AdwBackButton {
  AdwBin parent_instance;

  GSList *navigation_views;

  AdwNavigationPage *page;

  GtkWidget *navigation_menu;
  GPtrArray *navigation_history;
  guint clear_menu_id;
};

G_DEFINE_FINAL_TYPE (AdwBackButton, adw_back_button, ADW_TYPE_BIN)

typedef gboolean (*TraverseFunc) (AdwNavigationView *view,
                                  AdwNavigationPage *page,
                                  gboolean           is_child_view,
                                  gpointer           user_data);

static gboolean
traverse_view (AdwNavigationView *view,
               gboolean           skip_first,
               gboolean           is_in_child_view,
               TraverseFunc       callback,
               gpointer           user_data)
{
  AdwNavigationPage *page = adw_navigation_view_get_visible_page (view);
  gboolean first_page = TRUE;

  /* Skip the current page unless it's a child view */
  if (page && skip_first) {
    page = adw_navigation_view_get_previous_page (view, page);
    first_page = FALSE;
  }

  while (page) {
    AdwNavigationView *child_view;

    if (callback (view, page, is_in_child_view, user_data))
      return TRUE;

    if (first_page) {
      child_view = NULL;
      first_page = FALSE;
    } else {
      child_view = adw_navigation_page_get_child_view (page);
    }

    if (child_view && traverse_view (child_view, FALSE, TRUE, callback, user_data))
      return TRUE;

    if (!adw_navigation_page_get_can_pop (page))
      return TRUE;

    page = adw_navigation_view_get_previous_page (view, page);
  }

  return FALSE;
}

static void
update_page (AdwBackButton *self)
{
  AdwNavigationPage *prev_page = NULL;
  GSList *l;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    prev_page = adw_navigation_view_get_previous_page (data->view, data->page);

    if (!adw_navigation_page_get_can_pop (data->page)) {
      prev_page = NULL;
      break;
    }

    if (prev_page)
      break;
  }

  if (prev_page == self->page)
    return;

  self->page = prev_page;

  gtk_widget_set_visible (GTK_WIDGET (self), !!prev_page);
}

static gboolean
traverse_gather_history (AdwNavigationView *view,
                         AdwNavigationPage *page,
                         gboolean           is_child_view,
                         gpointer           user_data)
{
  AdwNavigationView *child_view;
  GPtrArray *pages = user_data;

  child_view = adw_navigation_page_get_child_view (page);
  if (!child_view)
    g_ptr_array_add (pages, page);

  return FALSE;
}

static GPtrArray *
gather_navigation_history (AdwBackButton *self)
{
  GPtrArray *pages = g_ptr_array_new ();
  GSList *l;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    if (traverse_view (data->view, TRUE, FALSE, traverse_gather_history, pages))
      break;
  }

  return pages;
}

typedef struct {
  AdwBackButton *self;
  AdwNavigationPage *target_page;
  gboolean last_view;

  NavigationViewData outer_view;

  GSList *pop_before;
  GSList *pop_after;
} PopData;

static gboolean
traverse_find_target (AdwNavigationView *view,
                      AdwNavigationPage *page,
                      gboolean           is_child_view,
                      gpointer           user_data)
{
  PopData *data = user_data;

  if (page == data->target_page) {
    data->last_view = TRUE;
    return TRUE;
  }

  return FALSE;
}

static gboolean
traverse_pop_pages (AdwNavigationView *view,
                    AdwNavigationPage *page,
                    gboolean           is_child_view,
                    gpointer           user_data)
{
  PopData *data = user_data;
  GSList **list;
  NavigationViewData *nav_data = NULL;

  if (data->last_view && !is_child_view) {
    data->outer_view.view = view;
    data->outer_view.page = page;
  }

  if (data->last_view)
    list = &data->pop_before;
  else
    list = &data->pop_after;

  if (*list)
    nav_data = (*list)->data;

  if (!nav_data || nav_data->view != view) {
    nav_data = g_new0 (NavigationViewData, 1);
    nav_data->view = view;

    *list = g_slist_prepend (*list, nav_data);
  }

  nav_data->page = page;

  if (page == data->target_page)
    return TRUE;

  return FALSE;
}

static void
pop_pages_hidden (AdwNavigationPage *page,
                  GSList            *pop_after)
{
  GSList *l;

  g_signal_handlers_disconnect_by_func (page, pop_pages_hidden, pop_after);

  for (l = pop_after; l; l = l->next) {
    NavigationViewData *data = l->data;

    adw_navigation_view_pop_to_page (data->view, data->page);

    g_object_unref (data->view);
    g_object_unref (data->page);
  }

  g_slist_free_full (pop_after, g_free);
  g_object_unref (page);
}

static void
pop_to_page_cb (AdwBackButton *self,
                const char    *action_name,
                GVariant      *param)
{
  int index = g_variant_get_int32 (param);
  AdwNavigationPage *target_page = g_ptr_array_index (self->navigation_history, index);
  GSList *l;
  PopData pop_data;

  /* The page has been unparented while the menu was opened */
  if (!ADW_IS_NAVIGATION_VIEW (gtk_widget_get_parent (GTK_WIDGET (target_page))))
    return;

  pop_data.self = self;
  pop_data.target_page = target_page;
  pop_data.pop_before = NULL;
  pop_data.pop_after = NULL;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    pop_data.last_view = FALSE;

    if (traverse_view (data->view, FALSE, FALSE, traverse_find_target, &pop_data) &&
        !pop_data.last_view) {
      break;
    }

    if (traverse_view (data->view, FALSE, FALSE, traverse_pop_pages, &pop_data))
      break;
  }

  g_assert (pop_data.outer_view.view);
  g_assert (pop_data.outer_view.page);

  for (l = pop_data.pop_before; l; l = l->next) {
    NavigationViewData *data = l->data;

    adw_navigation_view_pop_to_page (data->view, data->page);
  }

  for (l = pop_data.pop_after; l; l = l->next) {
    NavigationViewData *data = l->data;

    g_object_ref (data->view);
    g_object_ref (data->page);
  }

  g_object_ref (pop_data.outer_view.page);

  g_signal_connect (pop_data.outer_view.page, "shown",
                    G_CALLBACK (pop_pages_hidden), pop_data.pop_after);
  adw_navigation_view_pop_to_page (pop_data.outer_view.view, pop_data.outer_view.page);

  g_slist_free_full (pop_data.pop_before, g_free);
}

static void
clear_menu (AdwBackButton *self)
{
  g_clear_pointer (&self->navigation_menu, gtk_widget_unparent);

  if (self->navigation_history) {
    g_ptr_array_free (self->navigation_history, TRUE);
    self->navigation_history = NULL;
  }

  self->clear_menu_id = 0;
}

static void
navigation_menu_closed_cb (AdwBackButton *self)
{
  GtkWidget *button = adw_bin_get_child (ADW_BIN (self));

  gtk_widget_unset_state_flags (button, GTK_STATE_FLAG_CHECKED);

  self->clear_menu_id = g_idle_add_once ((GSourceOnceFunc) clear_menu, self);
}

static void
create_navigation_menu (AdwBackButton *self)
{
  GtkWidget *popover;
  GPtrArray *history;
  GMenu *menu = g_menu_new ();
  int i;

  g_clear_handle_id (&self->clear_menu_id, g_source_remove);
  clear_menu (self);

  history = gather_navigation_history (self);

  for (i = 0; i < history->len; i++) {
    AdwNavigationPage *page = g_ptr_array_index (history, i);
    const char *title = adw_navigation_page_get_title (page);
    GMenuItem *item = g_menu_item_new ((title && *title) ? title : _("Back"), NULL);

    g_menu_item_set_action_and_target (item, "menu.pop-to-page", "i", i);

    g_menu_append_item (menu, item);
  }

  popover = gtk_popover_menu_new_from_model (G_MENU_MODEL (menu));
  gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
  gtk_widget_set_halign (popover, GTK_ALIGN_START);
  gtk_widget_set_parent (popover, GTK_WIDGET (self));
  g_signal_connect_swapped (popover, "closed",
                            G_CALLBACK (navigation_menu_closed_cb), self);

  self->navigation_menu = popover;
  self->navigation_history = history;

  g_object_unref (menu);
}

static void
open_navigation_menu (AdwBackButton *self)
{
  GtkWidget *button = adw_bin_get_child (ADW_BIN (self));

  create_navigation_menu (self);

  gtk_popover_popup (GTK_POPOVER (self->navigation_menu));

  gtk_widget_set_state_flags (button, GTK_STATE_FLAG_CHECKED, FALSE);
}

static void
long_pressed_cb (GtkGesture    *gesture,
                 double         x,
                 double         y,
                 AdwBackButton *self)
{
  if (!gtk_widget_contains (GTK_WIDGET (self), x, y)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  open_navigation_menu (self);

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
right_click_pressed_cb (GtkGesture    *gesture,
                        int            n_click,
                        double         x,
                        double         y,
                        AdwBackButton *self)
{
  if (!gtk_widget_contains (GTK_WIDGET (self), x, y)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  open_navigation_menu (self);

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static AdwNavigationPage *
get_inner_page (AdwNavigationPage *page)
{
  AdwNavigationView *child_view = adw_navigation_page_get_child_view (page);
  AdwNavigationPage *visible_page;

  if (!child_view)
    return page;

  visible_page = adw_navigation_view_get_visible_page (child_view);

  if (!visible_page)
    return page;

  return get_inner_page (visible_page);
}

static gboolean
query_tooltip (AdwBackButton *self,
               int            x,
               int            y,
               gboolean       keyboard_tooltip,
               GtkTooltip    *tooltip)
{
  AdwNavigationPage *page;
  const char *title;

  if (!self->page)
    return FALSE;

  page = get_inner_page (self->page);
  title = adw_navigation_page_get_title (page);

  gtk_tooltip_set_text (tooltip, (title && *title) ? title : _("Back"));

  return TRUE;
}

static void
adw_back_button_root (GtkWidget *widget)
{
  AdwBackButton *self = ADW_BACK_BUTTON (widget);
  GtkWidget *page;

  GTK_WIDGET_CLASS (adw_back_button_parent_class)->root (widget);

  page = adw_widget_get_ancestor (widget, ADW_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  while (page) {
    GtkWidget *view = gtk_widget_get_parent (page);

    if (ADW_IS_NAVIGATION_VIEW (view)) {
      NavigationViewData *data = g_new0 (NavigationViewData, 1);

      data->self = self;
      data->view = ADW_NAVIGATION_VIEW (view);
      data->page = ADW_NAVIGATION_PAGE (page);

      g_signal_connect_swapped (data->view, "replaced",
                                G_CALLBACK (update_page), self);
      g_signal_connect_swapped (data->page, "showing",
                                G_CALLBACK (update_page), self);
      g_signal_connect_swapped (data->page, "notify::can-pop",
                                G_CALLBACK (update_page), self);

      self->navigation_views = g_slist_prepend (self->navigation_views, data);
    }

    page = adw_widget_get_ancestor (view, ADW_TYPE_NAVIGATION_PAGE, TRUE, TRUE);
  }

  self->navigation_views = g_slist_reverse (self->navigation_views);

  update_page (self);
}

static void
adw_back_button_unroot (GtkWidget *widget)
{
  AdwBackButton *self = ADW_BACK_BUTTON (widget);
  GSList *l;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    g_signal_handlers_disconnect_by_func (data->view, update_page, self);
    g_signal_handlers_disconnect_by_func (data->page, update_page, self);

    g_free (data);
  }

  g_clear_pointer (&self->navigation_views, g_slist_free);

  update_page (self);

  GTK_WIDGET_CLASS (adw_back_button_parent_class)->unroot (widget);
}

static void
adw_back_button_dispose (GObject *object)
{
  AdwBackButton *self = ADW_BACK_BUTTON (object);

  g_clear_handle_id (&self->clear_menu_id, g_source_remove);

  clear_menu (self);

  G_OBJECT_CLASS (adw_back_button_parent_class)->dispose (object);
}

static void
adw_back_button_class_init (AdwBackButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_back_button_dispose;

  widget_class->root = adw_back_button_root;
  widget_class->unroot = adw_back_button_unroot;

  gtk_widget_class_install_action (widget_class, "menu.popup", NULL,
                                   (GtkWidgetActionActivateFunc) open_navigation_menu);
  gtk_widget_class_install_action (widget_class, "menu.pop-to-page", "i",
                                   (GtkWidgetActionActivateFunc) pop_to_page_cb);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_F10, GDK_SHIFT_MASK, "menu.popup", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Menu, 0, "menu.popup", NULL);
}

static void
adw_back_button_init (AdwBackButton *self)
{
  GtkWidget *button;
  GtkGesture *gesture;

  gtk_widget_set_visible (GTK_WIDGET (self), FALSE);

  button = gtk_button_new_from_icon_name ("go-previous-symbolic");
  gtk_actionable_set_action_name (GTK_ACTIONABLE (button), "navigation.pop");
  gtk_widget_add_css_class (GTK_WIDGET (button), "back");
  gtk_widget_set_has_tooltip (GTK_WIDGET (button), TRUE);
  gtk_accessible_update_property (GTK_ACCESSIBLE (button),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, _("Back"),
                                  -1);
  g_signal_connect_swapped (button, "query-tooltip",
                            G_CALLBACK (query_tooltip), self);
  adw_bin_set_child (ADW_BIN (self), button);

  gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), GDK_BUTTON_SECONDARY);
  g_signal_connect (gesture, "pressed", G_CALLBACK (right_click_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));

  gesture = gtk_gesture_long_press_new ();
  g_signal_connect (gesture, "pressed", G_CALLBACK (long_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (gesture));
}

GtkWidget *
adw_back_button_new (void)
{
  return g_object_new (ADW_TYPE_BACK_BUTTON, NULL);
}
