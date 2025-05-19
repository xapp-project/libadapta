/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
add_pages (AdapTabView  *view,
           AdapTabPage **pages,
           int          n,
           int          n_pinned)
{
  int i;

  for (i = 0; i < n_pinned; i++)
    pages[i] = adap_tab_view_append_pinned (view, gtk_button_new ());

  for (i = n_pinned; i < n; i++)
    pages[i] = adap_tab_view_append (view, gtk_button_new ());
}

static void
assert_page_positions (AdapTabView  *view,
                       AdapTabPage **pages,
                       int          n,
                       int          n_pinned,
                       ...)
{
  va_list args;
  int i;

  va_start (args, n_pinned);

  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, n);
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, n_pinned);

  for (i = 0; i < n; i++) {
    int index = va_arg (args, int);

    if (index >= 0)
      g_assert_cmpint (adap_tab_view_get_page_position (view, pages[index]), ==, i);
  }

  va_end (args);
}

static gboolean
close_noop (void)
{
  return GDK_EVENT_STOP;
}

static void
check_selection_non_null (AdapTabView *view)
{
  g_assert_nonnull (adap_tab_view_get_selected_page (view));
}

static void
check_selection_null (AdapTabView *view)
{
  g_assert_null (adap_tab_view_get_selected_page (view));
}

static void
test_adap_tab_view_n_pages (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  int n_pages;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::n-pages", G_CALLBACK (increment), &notified);

  g_object_get (view, "n-pages", &n_pages, NULL);
  g_assert_cmpint (n_pages, ==, 0);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_object_get (view, "n-pages", &n_pages, NULL);
  g_assert_cmpint (n_pages, ==, 1);
  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adap_tab_view_append (view, gtk_button_new ());
  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adap_tab_view_append_pinned (view, gtk_button_new ());
  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adap_tab_view_reorder_forward (view, page);
  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adap_tab_view_close_page (view, page);
  g_assert_cmpint (adap_tab_view_get_n_pages (view), ==, 2);
  g_assert_cmpint (notified, ==, 4);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_n_pinned_pages (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  int n_pages;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::n-pinned-pages", G_CALLBACK (increment), &notified);

  g_object_get (view, "n-pinned-pages", &n_pages, NULL);
  g_assert_cmpint (n_pages, ==, 0);

  adap_tab_view_append_pinned (view, gtk_button_new ());
  g_object_get (view, "n-pinned-pages", &n_pages, NULL);
  g_assert_cmpint (n_pages, ==, 1);
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adap_tab_view_set_page_pinned (view, page, TRUE);
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adap_tab_view_reorder_backward (view, page);
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adap_tab_view_set_page_pinned (view, page, FALSE);
  g_assert_cmpint (adap_tab_view_get_n_pinned_pages (view), ==, 1);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_default_icon (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  GIcon *icon1 = g_themed_icon_new ("go-previous-symbolic");
  GIcon *icon2 = g_themed_icon_new ("go-next-symbolic");
  char *icon_str;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::default-icon", G_CALLBACK (increment), &notified);

  icon_str = g_icon_to_string (adap_tab_view_get_default_icon (view));
  g_assert_cmpstr (icon_str, ==, "adap-tab-icon-missing-symbolic");
  g_assert_cmpint (notified, ==, 0);

  adap_tab_view_set_default_icon (view, icon1);
  g_assert_true (adap_tab_view_get_default_icon (view) == icon1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "default-icon", icon2, NULL);
  g_assert_true (adap_tab_view_get_default_icon (view) == icon2);
  g_assert_cmpint (notified, ==, 2);

  g_free (icon_str);
  g_assert_finalize_object (view);
  g_assert_finalize_object (icon1);
  g_assert_finalize_object (icon2);
}

static void
test_adap_tab_view_menu_model (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  GMenuModel *model;
  GMenuModel *model1 = G_MENU_MODEL (g_menu_new ());
  GMenuModel *model2 = G_MENU_MODEL (g_menu_new ());
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::menu-model", G_CALLBACK (increment), &notified);

  g_object_get (view, "menu-model", &model, NULL);
  g_assert_null (model);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_view_set_menu_model (view, model1);
  g_assert_true (adap_tab_view_get_menu_model (view) == model1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "menu-model", model2, NULL);
  g_assert_true (adap_tab_view_get_menu_model (view) == model2);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
  g_assert_finalize_object (model1);
  g_assert_finalize_object (model2);
}

static void
test_adap_tab_view_shortcuts (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabViewShortcuts shortcuts;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::shortcuts", G_CALLBACK (increment), &notified);

  g_object_get (view, "shortcuts", &shortcuts, NULL);
  g_assert_cmpint (shortcuts, ==, ADAP_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_view_set_shortcuts (view, ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP);
  g_assert_cmpint (adap_tab_view_get_shortcuts (view), ==, ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "shortcuts", ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN, NULL);
  g_assert_cmpint (adap_tab_view_get_shortcuts (view), ==, ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN);
  g_assert_cmpint (notified, ==, 2);

  adap_tab_view_add_shortcuts (view, ADAP_TAB_VIEW_SHORTCUT_CONTROL_HOME);
  g_assert_cmpint (adap_tab_view_get_shortcuts (view), ==, ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN |
                                                          ADAP_TAB_VIEW_SHORTCUT_CONTROL_HOME);
  g_assert_cmpint (notified, ==, 3);

  adap_tab_view_remove_shortcuts (view, ADAP_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN);
  g_assert_cmpint (adap_tab_view_get_shortcuts (view), ==, ADAP_TAB_VIEW_SHORTCUT_CONTROL_HOME);
  g_assert_cmpint (notified, ==, 4);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_get_page (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  GtkWidget *child1, *child2, *child3;
  AdapTabPage *page1, *page2, *page3;

  g_assert_nonnull (view);

  child1 = gtk_button_new ();
  child2 = gtk_button_new ();
  child3 = gtk_button_new ();

  page1 = adap_tab_view_append_pinned (view, child1);
  page2 = adap_tab_view_append (view, child2);
  page3 = adap_tab_view_append (view, child3);

  g_assert_true (adap_tab_view_get_nth_page (view, 0) == page1);
  g_assert_true (adap_tab_view_get_nth_page (view, 1) == page2);
  g_assert_true (adap_tab_view_get_nth_page (view, 2) == page3);

  g_assert_true (adap_tab_view_get_page (view, child1) == page1);
  g_assert_true (adap_tab_view_get_page (view, child2) == page2);
  g_assert_true (adap_tab_view_get_page (view, child3) == page3);

  g_assert_cmpint (adap_tab_view_get_page_position (view, page1), ==, 0);
  g_assert_cmpint (adap_tab_view_get_page_position (view, page2), ==, 1);
  g_assert_cmpint (adap_tab_view_get_page_position (view, page3), ==, 2);

  g_assert_true (adap_tab_page_get_child (page1) == child1);
  g_assert_true (adap_tab_page_get_child (page2) == child2);
  g_assert_true (adap_tab_page_get_child (page3) == child3);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_select (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page1, *page2, *selected_page;
  gboolean ret;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::selected-page", G_CALLBACK (increment), &notified);

  g_object_get (view, "selected-page", &selected_page, NULL);
  g_assert_null (selected_page);

  page1 = adap_tab_view_append (view, gtk_button_new ());
  g_assert_true (adap_tab_view_get_selected_page (view) == page1);
  g_assert_true (adap_tab_page_get_selected (page1));
  g_assert_cmpint (notified, ==, 1);

  page2 = adap_tab_view_append (view, gtk_button_new ());
  g_assert_true (adap_tab_view_get_selected_page (view) == page1);
  g_assert_true (adap_tab_page_get_selected (page1));
  g_assert_false (adap_tab_page_get_selected (page2));
  g_assert_cmpint (notified, ==, 1);

  adap_tab_view_set_selected_page (view, page2);
  g_assert_true (adap_tab_view_get_selected_page (view) == page2);
  g_assert_cmpint (notified, ==, 2);

  g_object_set (view, "selected-page", page1, NULL);
  g_assert_true (adap_tab_view_get_selected_page (view) == page1);
  g_assert_cmpint (notified, ==, 3);

  ret = adap_tab_view_select_previous_page (view);
  g_assert_true (adap_tab_view_get_selected_page (view) == page1);
  g_assert_false (ret);
  g_assert_cmpint (notified, ==, 3);

  ret = adap_tab_view_select_next_page (view);
  g_assert_true (adap_tab_view_get_selected_page (view) == page2);
  g_assert_true (ret);
  g_assert_cmpint (notified, ==, 4);

  ret = adap_tab_view_select_next_page (view);
  g_assert_true (adap_tab_view_get_selected_page (view) == page2);
  g_assert_false (ret);
  g_assert_cmpint (notified, ==, 4);

  ret = adap_tab_view_select_previous_page (view);
  g_assert_true (adap_tab_view_get_selected_page (view) == page1);
  g_assert_true (ret);
  g_assert_cmpint (notified, ==, 5);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_add_basic (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[6];

  g_assert_nonnull (view);

  pages[0] = adap_tab_view_append (view, gtk_button_new ());
  assert_page_positions (view, pages, 1, 0,
                         0);

  pages[1] = adap_tab_view_prepend (view, gtk_button_new ());
  assert_page_positions (view, pages, 2, 0,
                         1, 0);

  pages[2] = adap_tab_view_insert (view, gtk_button_new (), 1);
  assert_page_positions (view, pages, 3, 0,
                         1, 2, 0);

  pages[3] = adap_tab_view_prepend_pinned (view, gtk_button_new ());
  assert_page_positions (view, pages, 4, 1,
                         3, 1, 2, 0);

  pages[4] = adap_tab_view_append_pinned (view, gtk_button_new ());
  assert_page_positions (view, pages, 5, 2,
                         3, 4, 1, 2, 0);

  pages[5] = adap_tab_view_insert_pinned (view, gtk_button_new (), 1);
  assert_page_positions (view, pages, 6, 3,
                         3, 5, 4, 1, 2, 0);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_add_auto (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[17];

  g_assert_nonnull (view);

  add_pages (view, pages, 3, 3);
  assert_page_positions (view, pages, 3, 3,
                         0, 1, 2);

  /* No parent */

  pages[3] = adap_tab_view_add_page (view, gtk_button_new (), NULL);
  g_assert_null (adap_tab_page_get_parent (pages[3]));
  assert_page_positions (view, pages, 4, 3,
                         0, 1, 2, 3);

  pages[4] = adap_tab_view_add_page (view, gtk_button_new (), NULL);
  g_assert_null (adap_tab_page_get_parent (pages[4]));
  assert_page_positions (view, pages, 5, 3,
                         0, 1, 2, 3, 4);

  pages[5] = adap_tab_view_add_page (view, gtk_button_new (), NULL);
  g_assert_null (adap_tab_page_get_parent (pages[5]));
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  /* Parent is a regular page */

  pages[6] = adap_tab_view_add_page (view, gtk_button_new (), pages[4]);
  g_assert_true (adap_tab_page_get_parent (pages[6]) == pages[4]);
  assert_page_positions (view, pages, 7, 3,
                         0, 1, 2, 3, 4, 6, 5);

  pages[7] = adap_tab_view_add_page (view, gtk_button_new (), pages[4]);
  g_assert_true (adap_tab_page_get_parent (pages[7]) == pages[4]);
  assert_page_positions (view, pages, 8, 3,
                         0, 1, 2, 3, 4, 6, 7, 5);

  pages[8] = adap_tab_view_add_page (view, gtk_button_new (), pages[6]);
  g_assert_true (adap_tab_page_get_parent (pages[8]) == pages[6]);
  assert_page_positions (view, pages, 9, 3,
                         0, 1, 2, 3, 4, 6, 8, 7, 5);

  pages[9] = adap_tab_view_add_page (view, gtk_button_new (), pages[6]);
  g_assert_true (adap_tab_page_get_parent (pages[9]) == pages[6]);
  assert_page_positions (view, pages, 10, 3,
                         0, 1, 2, 3, 4, 6, 8, 9, 7, 5);

  pages[10] = adap_tab_view_add_page (view, gtk_button_new (), pages[4]);
  g_assert_true (adap_tab_page_get_parent (pages[10]) == pages[4]);
  assert_page_positions (view, pages, 11, 3,
                         0, 1, 2, 3, 4, 6, 8, 9, 7, 10, 5);

  /* Parent is a pinned page */

  pages[11] = adap_tab_view_add_page (view, gtk_button_new (), pages[1]);
  g_assert_true (adap_tab_page_get_parent (pages[11]) == pages[1]);
  assert_page_positions (view, pages, 12, 3,
                         0, 1, 2, 11, 3, 4, 6, 8, 9, 7, 10, 5);

  pages[12] = adap_tab_view_add_page (view, gtk_button_new (), pages[11]);
  g_assert_true (adap_tab_page_get_parent (pages[12]) == pages[11]);
  assert_page_positions (view, pages, 13, 3,
                         0, 1, 2, 11, 12, 3, 4, 6, 8, 9, 7, 10, 5);

  pages[13] = adap_tab_view_add_page (view, gtk_button_new (), pages[1]);
  g_assert_true (adap_tab_page_get_parent (pages[13]) == pages[1]);
  assert_page_positions (view, pages, 14, 3,
                         0, 1, 2, 11, 12, 13, 3, 4, 6, 8, 9, 7, 10, 5);

  pages[14] = adap_tab_view_add_page (view, gtk_button_new (), pages[0]);
  g_assert_true (adap_tab_page_get_parent (pages[14]) == pages[0]);
  assert_page_positions (view, pages, 15, 3,
                         0, 1, 2, 14, 11, 12, 13, 3, 4, 6, 8, 9, 7, 10, 5);

  pages[15] = adap_tab_view_add_page (view, gtk_button_new (), pages[1]);
  g_assert_true (adap_tab_page_get_parent (pages[15]) == pages[1]);
  assert_page_positions (view, pages, 16, 3,
                         0, 1, 2, 15, 14, 11, 12, 13, 3, 4, 6, 8, 9, 7, 10, 5);

  /* Parent is the last page */

  pages[16] = adap_tab_view_add_page (view, gtk_button_new (), pages[5]);
  g_assert_true (adap_tab_page_get_parent (pages[16]) == pages[5]);
  assert_page_positions (view, pages, 17, 3,
                         0, 1, 2, 15, 14, 11, 12, 13, 3, 4, 6, 8, 9, 7, 10, 5, 16);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_reorder (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[6];
  gboolean ret;

  g_assert_nonnull (view);

  add_pages (view, pages, 6, 3);

  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_page (view, pages[1], 1);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_page (view, pages[1], 0);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 0, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_page (view, pages[1], 1);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_page (view, pages[5], 5);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_page (view, pages[5], 4);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 5, 4);

  ret = adap_tab_view_reorder_page (view, pages[5], 5);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_reorder_first_last (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[6];
  gboolean ret;

  g_assert_nonnull (view);

  add_pages (view, pages, 6, 3);

  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_first (view, pages[0]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_last (view, pages[0]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 3, 4, 5);

  ret = adap_tab_view_reorder_last (view, pages[0]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 3, 4, 5);

  ret = adap_tab_view_reorder_first (view, pages[0]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_first (view, pages[3]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_last (view, pages[3]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 4, 5, 3);

  ret = adap_tab_view_reorder_last (view, pages[3]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 4, 5, 3);

  ret = adap_tab_view_reorder_first (view, pages[3]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_reorder_forward_backward (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[6];
  gboolean ret;

  g_assert_nonnull (view);

  add_pages (view, pages, 6, 3);

  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_backward (view, pages[0]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_forward (view, pages[0]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 0, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_forward (view, pages[2]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 0, 2, 3, 4, 5);

  ret = adap_tab_view_reorder_backward (view, pages[2]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 3, 4, 5);

  ret = adap_tab_view_reorder_backward (view, pages[3]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 3, 4, 5);

  ret = adap_tab_view_reorder_forward (view, pages[3]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 4, 3, 5);

  ret = adap_tab_view_reorder_forward (view, pages[5]);
  g_assert_false (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 4, 3, 5);

  ret = adap_tab_view_reorder_backward (view, pages[5]);
  g_assert_true (ret);
  assert_page_positions (view, pages, 6, 3,
                         1, 2, 0, 4, 5, 3);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_pin (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[4];

  g_assert_nonnull (view);

  /* Test specifically pinning with only 1 page */
  pages[0] = adap_tab_view_append (view, gtk_button_new ());
  g_assert_false (adap_tab_page_get_pinned (pages[0]));
  assert_page_positions (view, pages, 1, 0,
                         0);

  adap_tab_view_set_page_pinned (view, pages[0], TRUE);
  g_assert_true (adap_tab_page_get_pinned (pages[0]));
  assert_page_positions (view, pages, 1, 1,
                         0);

  adap_tab_view_set_page_pinned (view, pages[0], FALSE);
  g_assert_false (adap_tab_page_get_pinned (pages[0]));
  assert_page_positions (view, pages, 1, 0,
                         0);

  pages[1] = adap_tab_view_append (view, gtk_button_new ());
  pages[2] = adap_tab_view_append (view, gtk_button_new ());
  pages[3] = adap_tab_view_append (view, gtk_button_new ());
  assert_page_positions (view, pages, 4, 0,
                         0, 1, 2, 3);

  adap_tab_view_set_page_pinned (view, pages[2], TRUE);
  assert_page_positions (view, pages, 4, 1,
                         2, 0, 1, 3);

  adap_tab_view_set_page_pinned (view, pages[1], TRUE);
  assert_page_positions (view, pages, 4, 2,
                         2, 1, 0, 3);

  adap_tab_view_set_page_pinned (view, pages[0], TRUE);
  assert_page_positions (view, pages, 4, 3,
                         2, 1, 0, 3);

  adap_tab_view_set_page_pinned (view, pages[1], FALSE);
  assert_page_positions (view, pages, 4, 2,
                         2, 0, 1, 3);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_close (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[3];

  g_assert_nonnull (view);

  add_pages (view, pages, 3, 0);

  adap_tab_view_set_selected_page (view, pages[1]);

  assert_page_positions (view, pages, 3, 0,
                         0, 1, 2);

  adap_tab_view_close_page (view, pages[1]);
  assert_page_positions (view, pages, 2, 0,
                         0, 2);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[2]);

  adap_tab_view_close_page (view, pages[2]);
  assert_page_positions (view, pages, 1, 0,
                         0);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[0]);

  adap_tab_view_close_page (view, pages[0]);
  assert_page_positions (view, pages, 0, 0);
  g_assert_null (adap_tab_view_get_selected_page (view));

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_close_other (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[6];

  g_assert_nonnull (view);

  add_pages (view, pages, 6, 3);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 3, 4, 5);

  adap_tab_view_close_other_pages (view, pages[4]);
  assert_page_positions (view, pages, 4, 3,
                         0, 1, 2, 4);

  adap_tab_view_close_other_pages (view, pages[2]);
  assert_page_positions (view, pages, 3, 3,
                         0, 1, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_close_before_after (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[10];

  g_assert_nonnull (view);

  add_pages (view, pages, 10, 3);
  assert_page_positions (view, pages, 10, 3,
                         0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

  adap_tab_view_close_pages_before (view, pages[3]);
  assert_page_positions (view, pages, 10, 3,
                         0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

  adap_tab_view_close_pages_before (view, pages[5]);
  assert_page_positions (view, pages, 8, 3,
                         0, 1, 2, 5, 6, 7, 8, 9);

  adap_tab_view_close_pages_after (view, pages[7]);
  assert_page_positions (view, pages, 6, 3,
                         0, 1, 2, 5, 6, 7);

  adap_tab_view_close_pages_after (view, pages[0]);
  assert_page_positions (view, pages, 3, 3,
                         0, 1, 2);

  g_assert_finalize_object (view);
}

static gboolean
close_page_position_cb (AdapTabView *view,
                        AdapTabPage *page)
{
  int position = adap_tab_view_get_page_position (view, page);

  adap_tab_view_close_page_finish (view, page, (position % 2) > 0);

  return GDK_EVENT_STOP;
}

static void
test_adap_tab_view_close_signal (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[10];
  gulong handler;

  g_assert_nonnull (view);

  /* Allow closing pages with odd positions, including pinned */
  handler = g_signal_connect (view, "close-page",
                              G_CALLBACK (close_page_position_cb), NULL);

  add_pages (view, pages, 10, 3);
  assert_page_positions (view, pages, 10, 3,
                         0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

  adap_tab_view_close_other_pages (view, pages[5]);
  assert_page_positions (view, pages, 6, 2,
                         0, 2, 4, 5, 6, 8);

  g_signal_handler_disconnect (view, handler);

  /* Defer closing */
  handler = g_signal_connect (view, "close-page", G_CALLBACK (close_noop), NULL);

  adap_tab_view_close_page (view, pages[0]);
  assert_page_positions (view, pages, 6, 2,
                         0, 2, 4, 5, 6, 8);

  adap_tab_view_close_page_finish (view, pages[0], FALSE);
  assert_page_positions (view, pages, 6, 2,
                         0, 2, 4, 5, 6, 8);

  adap_tab_view_close_page (view, pages[0]);
  assert_page_positions (view, pages, 6, 2,
                         0, 2, 4, 5, 6, 8);

  adap_tab_view_close_page_finish (view, pages[0], TRUE);
  assert_page_positions (view, pages, 5, 1,
                         2, 4, 5, 6, 8);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_close_select (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages[14];

  g_assert_nonnull (view);

  add_pages (view, pages, 9, 3);
  pages[9] = adap_tab_view_add_page (view, gtk_button_new (), pages[4]);
  pages[10] = adap_tab_view_add_page (view, gtk_button_new (), pages[4]);
  pages[11] = adap_tab_view_add_page (view, gtk_button_new (), pages[9]);
  pages[12] = adap_tab_view_add_page (view, gtk_button_new (), pages[1]);
  pages[13] = adap_tab_view_add_page (view, gtk_button_new (), pages[1]);

  assert_page_positions (view, pages, 14, 3,
                         0, 1, 2, 12, 13, 3, 4, 9, 11, 10, 5, 6, 7, 8);

  /* Nothing happens when closing unselected pages */

  adap_tab_view_set_selected_page (view, pages[0]);

  adap_tab_view_close_page (view, pages[8]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[0]);

  /* No parent */

  assert_page_positions (view, pages, 13, 3,
                         0, 1, 2, 12, 13, 3, 4, 9, 11, 10, 5, 6, 7);

  adap_tab_view_set_selected_page (view, pages[6]);

  adap_tab_view_close_page (view, pages[6]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[7]);

  adap_tab_view_close_page (view, pages[7]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[5]);

  /* Regular parent */

  assert_page_positions (view, pages, 11, 3,
                         0, 1, 2, 12, 13, 3, 4, 9, 11, 10, 5);

  adap_tab_view_set_selected_page (view, pages[10]);

  adap_tab_view_close_page (view, pages[10]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[11]);

  adap_tab_view_close_page (view, pages[11]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[9]);

  adap_tab_view_close_page (view, pages[9]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[4]);

  adap_tab_view_close_page (view, pages[4]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[5]);

  /* Pinned parent */

  assert_page_positions (view, pages, 7, 3,
                         0, 1, 2, 12, 13, 3, 5);

  adap_tab_view_set_selected_page (view, pages[13]);

  adap_tab_view_close_page (view, pages[13]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[12]);

  adap_tab_view_close_page (view, pages[12]);
  g_assert_true (adap_tab_view_get_selected_page (view) == pages[1]);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_transfer (void)
{
  AdapTabView *view1 = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabView *view2 = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *pages1[4], *pages2[4];

  g_assert_nonnull (view1);
  g_assert_nonnull (view2);

  add_pages (view1, pages1, 4, 2);
  assert_page_positions (view1, pages1, 4, 2,
                         0, 1, 2, 3);

  add_pages (view2, pages2, 4, 2);
  assert_page_positions (view2, pages2, 4, 2,
                         0, 1, 2, 3);

  adap_tab_view_transfer_page (view1, pages1[1], view2, 1);
  assert_page_positions (view1, pages1, 3, 1,
                         0, 2, 3);
  assert_page_positions (view2, pages2, 5, 3,
                         0, -1, 1, 2, 3);
  g_assert_true (adap_tab_view_get_nth_page (view2, 1) == pages1[1]);

  adap_tab_view_transfer_page (view2, pages2[3], view1, 2);
  assert_page_positions (view1, pages1, 4, 1,
                         0, 2, -1, 3);
  assert_page_positions (view2, pages2, 4, 3,
                         0, -1, 1, 2);
  g_assert_true (adap_tab_view_get_nth_page (view1, 2) == pages2[3]);

  g_assert_finalize_object (view1);
  g_assert_finalize_object (view2);
}

static void
test_adap_tab_view_pages (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  GtkSelectionModel* model;
  AdapTabPage *pages[2];

  g_assert_nonnull (view);

  model = adap_tab_view_get_pages (view);
  g_assert_nonnull (model);

  g_signal_connect_swapped (model, "items-changed", G_CALLBACK (check_selection_non_null), view);
  g_signal_connect_swapped (model, "selection-changed", G_CALLBACK (check_selection_non_null), view);

  pages[0] = adap_tab_view_add_page (view, gtk_button_new (), NULL);
  pages[1] = adap_tab_view_add_page (view, gtk_button_new (), NULL);

  adap_tab_view_close_page (view, pages[0]);

  g_signal_handlers_disconnect_by_func (model, G_CALLBACK (check_selection_non_null), view);

  g_signal_connect_swapped (model, "items-changed", G_CALLBACK (check_selection_null), view);
  g_signal_connect_swapped (model, "selection-changed", G_CALLBACK (check_selection_null), view);

  adap_tab_view_close_page (view, pages[1]);

  g_assert_finalize_object (view);
  g_assert_finalize_object (model);
}

static void
test_adap_tab_page_title (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  char *title;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_title (page, "Some title");
  g_assert_cmpstr (adap_tab_page_get_title (page), ==, "Some title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "title", "Some other title", NULL);
  g_assert_cmpstr (adap_tab_page_get_title (page), ==, "Some other title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_tooltip (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  char *tooltip;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::tooltip", G_CALLBACK (increment), &notified);

  g_object_get (page, "tooltip", &tooltip, NULL);
  g_assert_cmpstr (tooltip, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_tooltip (page, "Some tooltip");
  g_assert_cmpstr (adap_tab_page_get_tooltip (page), ==, "Some tooltip");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "tooltip", "Some other tooltip", NULL);
  g_assert_cmpstr (adap_tab_page_get_tooltip (page), ==, "Some other tooltip");
  g_assert_cmpint (notified, ==, 2);

  g_free (tooltip);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_keyword (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  char *keyword;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::keyword", G_CALLBACK (increment), &notified);

  g_object_get (page, "keyword", &keyword, NULL);
  g_assert_null (keyword);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_keyword (page, "Some keyword");
  g_assert_cmpstr (adap_tab_page_get_keyword (page), ==, "Some keyword");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "keyword", "Some other keyword", NULL);
  g_assert_cmpstr (adap_tab_page_get_keyword (page), ==, "Some other keyword");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_icon (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  GIcon *icon;
  GIcon *icon1 = g_themed_icon_new ("go-previous-symbolic");
  GIcon *icon2 = g_themed_icon_new ("go-next-symbolic");
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::icon", G_CALLBACK (increment), &notified);

  g_object_get (page, "icon", &icon, NULL);
  g_assert_null (icon);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_icon (page, icon1);
  g_assert_true (adap_tab_page_get_icon (page) == icon1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "icon", icon2, NULL);
  g_assert_true (adap_tab_page_get_icon (page) == icon2);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
  g_assert_finalize_object (icon1);
  g_assert_finalize_object (icon2);
}

static void
test_adap_tab_page_loading (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  gboolean loading;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::loading", G_CALLBACK (increment), &notified);

  g_object_get (page, "loading", &loading, NULL);
  g_assert_false (loading);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_loading (page, TRUE);
  g_object_get (page, "loading", &loading, NULL);
  g_assert_true (loading);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "loading", FALSE, NULL);
  g_assert_false (adap_tab_page_get_loading (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_indicator_icon (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  GIcon *icon;
  GIcon *icon1 = g_themed_icon_new ("go-previous-symbolic");
  GIcon *icon2 = g_themed_icon_new ("go-next-symbolic");
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::indicator-icon", G_CALLBACK (increment), &notified);

  g_object_get (page, "indicator-icon", &icon, NULL);
  g_assert_null (icon);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_indicator_icon (page, icon1);
  g_assert_true (adap_tab_page_get_indicator_icon (page) == icon1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "indicator-icon", icon2, NULL);
  g_assert_true (adap_tab_page_get_indicator_icon (page) == icon2);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
  g_assert_finalize_object (icon1);
  g_assert_finalize_object (icon2);
}

static void
test_adap_tab_page_indicator_tooltip (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  char *tooltip;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::indicator-tooltip", G_CALLBACK (increment), &notified);

  g_object_get (page, "indicator-tooltip", &tooltip, NULL);
  g_assert_cmpstr (tooltip, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_indicator_tooltip (page, "Some tooltip");
  g_assert_cmpstr (adap_tab_page_get_indicator_tooltip (page), ==, "Some tooltip");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "indicator-tooltip", "Some other tooltip", NULL);
  g_assert_cmpstr (adap_tab_page_get_indicator_tooltip (page), ==, "Some other tooltip");
  g_assert_cmpint (notified, ==, 2);

  g_free (tooltip);
  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_indicator_activatable (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  gboolean activatable;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::indicator-activatable", G_CALLBACK (increment), &notified);

  g_object_get (page, "indicator-activatable", &activatable, NULL);
  g_assert_false (activatable);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_indicator_activatable (page, TRUE);
  g_object_get (page, "indicator-activatable", &activatable, NULL);
  g_assert_true (activatable);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "indicator-activatable", FALSE, NULL);
  g_assert_false (adap_tab_page_get_indicator_activatable (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_needs_attention (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  gboolean needs_attention;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::needs-attention", G_CALLBACK (increment), &notified);

  g_object_get (page, "needs-attention", &needs_attention, NULL);
  g_assert_false (needs_attention);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_needs_attention (page, TRUE);
  g_object_get (page, "needs-attention", &needs_attention, NULL);
  g_assert_true (needs_attention);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "needs-attention", FALSE, NULL);
  g_assert_false (adap_tab_page_get_needs_attention (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_thumbnail_xalign (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  float xalign;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::thumbnail-xalign", G_CALLBACK (increment), &notified);

  g_object_get (page, "thumbnail_xalign", &xalign, NULL);
  g_assert_true (G_APPROX_VALUE (xalign, 0, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_thumbnail_xalign (page, 1);
  g_object_get (page, "thumbnail-xalign", &xalign, NULL);
  g_assert_true (G_APPROX_VALUE (xalign, 1, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "thumbnail-xalign", 0.5, NULL);
  g_assert_true (G_APPROX_VALUE (adap_tab_page_get_thumbnail_xalign (page), 0.5, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_thumbnail_yalign (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  float yalign;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::thumbnail-yalign", G_CALLBACK (increment), &notified);

  g_object_get (page, "thumbnail_yalign", &yalign, NULL);
  g_assert_true (G_APPROX_VALUE (yalign, 0, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_thumbnail_yalign (page, 1);
  g_object_get (page, "thumbnail-yalign", &yalign, NULL);
  g_assert_true (G_APPROX_VALUE (yalign, 1, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "thumbnail-yalign", 0.5, NULL);
  g_assert_true (G_APPROX_VALUE (adap_tab_page_get_thumbnail_yalign (page), 0.5, FLT_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_page_live_thumbnail (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  AdapTabPage *page;
  gboolean live_thumbnail;
  int notified = 0;

  g_assert_nonnull (view);

  page = adap_tab_view_append (view, gtk_button_new ());
  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::live-thumbnail", G_CALLBACK (increment), &notified);

  g_object_get (page, "live-thumbnail", &live_thumbnail, NULL);
  g_assert_false (live_thumbnail);
  g_assert_cmpint (notified, ==, 0);

  adap_tab_page_set_live_thumbnail (page, TRUE);
  g_object_get (page, "live-thumbnail", &live_thumbnail, NULL);
  g_assert_true (live_thumbnail);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "live-thumbnail", FALSE, NULL);
  g_assert_false (adap_tab_page_get_live_thumbnail (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_tab_view_pages_to_list_view_setup (GtkSignalListItemFactory *factory,
                                            GtkListItem              *list_item,
                                            gpointer                  unused)
{
  gtk_list_item_set_child (list_item, gtk_label_new (NULL));
}


static void
test_adap_tab_view_pages_to_list_view_bind (GtkSignalListItemFactory *factory,
                                           GtkListItem              *list_item,
                                           gpointer                  unused)
{
  AdapTabPage *item = gtk_list_item_get_item (list_item);
  GtkWidget *row = gtk_list_item_get_child (list_item);
  GBinding *binding;

  g_assert (GTK_IS_LABEL (item));
  g_assert (GTK_IS_LABEL (row));

  binding = g_object_bind_property (item, "label", row, "label", G_BINDING_SYNC_CREATE);
  g_object_set_data (G_OBJECT (list_item), "BINDING", binding);
}

static void
test_adap_tab_view_pages_to_list_view_unbind (GtkSignalListItemFactory *factory,
                                             GtkListItem              *list_item,
                                             gpointer                  unused)
{
  GBinding *binding = g_object_get_data (G_OBJECT (list_item), "BINDING");
  g_binding_unbind (binding);
}

static void
test_adap_tab_view_pages_to_list_view (void)
{
  AdapTabView *view = g_object_ref_sink (ADAP_TAB_VIEW (adap_tab_view_new ()));
  GtkListView *list_view = g_object_ref_sink (GTK_LIST_VIEW (gtk_list_view_new (NULL, NULL)));
  GtkSelectionModel *pages;
  GtkListItemFactory *factory;
  GtkLabel *label;

  g_assert_nonnull (view);
  g_assert_nonnull (list_view);

  pages = adap_tab_view_get_pages (view);
  g_assert_nonnull (pages);
  g_assert_true (GTK_IS_SELECTION_MODEL (pages));

  factory = gtk_signal_list_item_factory_new ();
  g_signal_connect (factory, "setup", G_CALLBACK (test_adap_tab_view_pages_to_list_view_setup), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (test_adap_tab_view_pages_to_list_view_bind), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (test_adap_tab_view_pages_to_list_view_unbind), NULL);

  gtk_list_view_set_factory (list_view, GTK_LIST_ITEM_FACTORY (factory));
  gtk_list_view_set_model (list_view, pages);

  label = GTK_LABEL (gtk_label_new ("test label"));
  adap_tab_view_append (view, GTK_WIDGET (label));

  g_clear_object (&factory);

  g_assert_finalize_object (list_view);
  g_assert_finalize_object (view);
  g_assert_finalize_object (pages);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/TabView/n_pages", test_adap_tab_view_n_pages);
  g_test_add_func ("/Adapta/TabView/n_pinned_pages", test_adap_tab_view_n_pinned_pages);
  g_test_add_func ("/Adapta/TabView/default_icon", test_adap_tab_view_default_icon);
  g_test_add_func ("/Adapta/TabView/menu_model", test_adap_tab_view_menu_model);
  g_test_add_func ("/Adapta/TabView/shortcuts", test_adap_tab_view_shortcuts);
  g_test_add_func ("/Adapta/TabView/get_page", test_adap_tab_view_get_page);
  g_test_add_func ("/Adapta/TabView/select", test_adap_tab_view_select);
  g_test_add_func ("/Adapta/TabView/add_basic", test_adap_tab_view_add_basic);
  g_test_add_func ("/Adapta/TabView/add_auto", test_adap_tab_view_add_auto);
  g_test_add_func ("/Adapta/TabView/reorder", test_adap_tab_view_reorder);
  g_test_add_func ("/Adapta/TabView/reorder_first_last", test_adap_tab_view_reorder_first_last);
  g_test_add_func ("/Adapta/TabView/reorder_forward_backward", test_adap_tab_view_reorder_forward_backward);
  g_test_add_func ("/Adapta/TabView/pin", test_adap_tab_view_pin);
  g_test_add_func ("/Adapta/TabView/close", test_adap_tab_view_close);
  g_test_add_func ("/Adapta/TabView/close_other", test_adap_tab_view_close_other);
  g_test_add_func ("/Adapta/TabView/close_before_after", test_adap_tab_view_close_before_after);
  g_test_add_func ("/Adapta/TabView/close_signal", test_adap_tab_view_close_signal);
  g_test_add_func ("/Adapta/TabView/close_select", test_adap_tab_view_close_select);
  g_test_add_func ("/Adapta/TabView/transfer", test_adap_tab_view_transfer);
  g_test_add_func ("/Adapta/TabView/pages", test_adap_tab_view_pages);
  g_test_add_func ("/Adapta/TabView/pages_to_list_view", test_adap_tab_view_pages_to_list_view);
  g_test_add_func ("/Adapta/TabPage/title", test_adap_tab_page_title);
  g_test_add_func ("/Adapta/TabPage/tooltip", test_adap_tab_page_tooltip);
  g_test_add_func ("/Adapta/TabPage/keyword", test_adap_tab_page_keyword);
  g_test_add_func ("/Adapta/TabPage/icon", test_adap_tab_page_icon);
  g_test_add_func ("/Adapta/TabPage/loading", test_adap_tab_page_loading);
  g_test_add_func ("/Adapta/TabPage/indicator_icon", test_adap_tab_page_indicator_icon);
  g_test_add_func ("/Adapta/TabPage/indicator_tooltip", test_adap_tab_page_indicator_tooltip);
  g_test_add_func ("/Adapta/TabPage/indicator_activatable", test_adap_tab_page_indicator_activatable);
  g_test_add_func ("/Adapta/TabPage/needs_attention", test_adap_tab_page_needs_attention);
  g_test_add_func ("/Adapta/TabPage/thumbnail_xalign", test_adap_tab_page_thumbnail_xalign);
  g_test_add_func ("/Adapta/TabPage/thumbnail_yalign", test_adap_tab_page_thumbnail_yalign);
  g_test_add_func ("/Adapta/TabPage/live_thumbnail", test_adap_tab_page_live_thumbnail);

  return g_test_run ();
}
