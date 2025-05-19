/*
 * Copyright (C) 2023 Purism SPC
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
check_navigation_stack (AdapNavigationView *view,
                        int                n_pages,
                        ...)
{
  GListModel *stack = adap_navigation_view_get_navigation_stack (view);
  va_list args;
  guint i;

  va_start (args, n_pages);

  g_assert_cmpint (g_list_model_get_n_items (stack), ==, n_pages);

  for (i = 0; i < n_pages; i++) {
    AdapNavigationPage *page = g_list_model_get_item (stack, i);
    const char *tag = va_arg (args, const char *);

    g_assert_cmpstr (adap_navigation_page_get_tag (page), ==, tag);

    g_object_unref (page);
  }

  va_end (args);

  g_assert_finalize_object (stack);
}

static void
test_adap_navigation_view_add_remove (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2 again", "page-2"));
  int notified = 0, pushed = 0, popped = 0;

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (increment), &pushed);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (increment), &popped);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (increment), &notified);

  g_assert_null (adap_navigation_view_get_visible_page (view));
  check_navigation_stack (view, 0);
  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_view_add (view, page_1);
  adap_navigation_view_add (view, page_2);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adap_navigation_view_add (view, page_3);
  g_test_assert_expected_messages ();

  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);

  adap_navigation_view_remove (view, page_1);
  adap_navigation_view_remove (view, page_2);

  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));

  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");

  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 1);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

static void
test_adap_navigation_view_push_pop (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdapNavigationPage *page_4 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));
  AdapNavigationPage *page_5 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 4 again", "page-4"));
  int notified = 0, pushed = 0, popped = 0;

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (increment), &pushed);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (increment), &popped);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (increment), &notified);

  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 0);

  /* Will be autoremoved after pop */
  adap_navigation_view_push (view, page_1);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  /* Explicitly added page - will persist after pop */
  adap_navigation_view_add (view, page_2);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adap_navigation_view_push (view, page_2);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adap_navigation_view_push (view, page_2);
  g_test_assert_expected_messages ();
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adap_navigation_view_add (view, page_3);
  adap_navigation_view_push (view, page_3);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  /* Removing while in navigation stack - no effect until it's popped */
  adap_navigation_view_remove (view, page_3);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adap_navigation_view_push (view, page_4);
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adap_navigation_view_push (view, page_5);
  g_test_assert_expected_messages ();
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adap_navigation_view_pop (view));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_null (adap_navigation_view_find_page (view, "page-4"));
  g_assert_cmpint (popped, ==, 1);
  g_assert_cmpint (notified, ==, 5);

  g_assert_true (adap_navigation_view_pop (view));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_null (adap_navigation_view_find_page (view, "page-3"));
  g_assert_cmpint (popped, ==, 2);
  g_assert_cmpint (notified, ==, 6);

  g_assert_true (adap_navigation_view_pop (view));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 7);

  /* Last page - not allowed to pop */
  g_assert_false (adap_navigation_view_pop (view));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 7);

  g_assert_cmpint (pushed, ==, 4);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
  g_assert_finalize_object (page_5);
}

static void
test_adap_navigation_view_push_pop_by_tag (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdapNavigationPage *page_4 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));
  int notified = 0, pushed = 0, popped = 0;

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (increment), &pushed);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (increment), &popped);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (increment), &notified);

  adap_navigation_view_add (view, page_1);
  adap_navigation_view_add (view, page_2);
  adap_navigation_view_add (view, page_3);
  adap_navigation_view_add (view, page_4);

  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*with the tag*");
  adap_navigation_view_push_by_tag (view, "page-0");
  g_test_assert_expected_messages ();
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adap_navigation_view_push_by_tag (view, "page-1");
  g_test_assert_expected_messages ();
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adap_navigation_view_push_by_tag (view, "page-2");
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adap_navigation_view_push_by_tag (view, "page-3");
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adap_navigation_view_push_by_tag (view, "page-4");
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (notified, ==, 4);

  g_assert_cmpint (popped, ==, 0);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*with the tag*");
  g_assert_false (adap_navigation_view_pop_to_tag (view, "page-5"));
  g_test_assert_expected_messages ();
  g_assert_cmpint (notified, ==, 4);

  g_assert_false (adap_navigation_view_pop_to_tag (view, "page-4"));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adap_navigation_view_pop_to_tag (view, "page-2"));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (popped, ==, 2);
  g_assert_cmpint (notified, ==, 5);

  g_assert_true (adap_navigation_view_pop_to_tag (view, "page-1"));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 6);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
}

static void
test_adap_navigation_view_pop_to_page (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdapNavigationPage *page_4 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));
  AdapNavigationPage *page_5 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 5", "page-5"));
  int notified = 0, popped = 0;

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  g_signal_connect_swapped (view, "popped", G_CALLBACK (increment), &popped);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (increment), &notified);

  adap_navigation_view_add (view, page_1);
  adap_navigation_view_add (view, page_3);
  adap_navigation_view_push (view, page_2);
  adap_navigation_view_push (view, page_3);
  adap_navigation_view_push (view, page_4);

  g_assert_true (adap_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (notified, ==, 4);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*not in the navigation stack*");
  g_assert_false (adap_navigation_view_pop_to_page (view, page_5));
  g_test_assert_expected_messages ();
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_false (adap_navigation_view_pop_to_page (view, page_4));
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adap_navigation_view_pop_to_page (view, page_3));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (popped, ==, 1);
  g_assert_cmpint (notified, ==, 5);
  g_assert_null (adap_navigation_view_find_page (view, "page-4"));

  g_assert_true (adap_navigation_view_pop_to_page (view, page_1));
  g_assert_true (adap_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 6);
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));
  g_assert_true (adap_navigation_view_find_page (view, "page-3") == page_3);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
  g_assert_finalize_object (page_5);
}

static void
test_adap_navigation_view_replace (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  int notified = 0, pushed = 0, popped = 0, replaced = 0;

  g_signal_connect_swapped (view, "pushed", G_CALLBACK (increment), &pushed);
  g_signal_connect_swapped (view, "popped", G_CALLBACK (increment), &popped);
  g_signal_connect_swapped (view, "replaced", G_CALLBACK (increment), &replaced);
  g_signal_connect_swapped (view, "notify::visible-page", G_CALLBACK (increment), &notified);

  check_navigation_stack (view, 0);

  adap_navigation_view_replace (view, NULL, 0);

  check_navigation_stack (view, 0);
  g_assert_cmpint (replaced, ==, 1);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_view_replace (view, &page_1, 1);

  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));
  g_assert_cmpint (replaced, ==, 2);
  g_assert_cmpint (notified, ==, 1);

  adap_navigation_view_replace (view, (AdapNavigationPage *[2]) {
    page_2,
    page_1,
  }, 2);

  check_navigation_stack (view, 2, "page-2", "page-1");
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 3);
  g_assert_cmpint (notified, ==, 1);

  adap_navigation_view_replace (view, (AdapNavigationPage *[2]) {
    page_1,
    page_2,
  }, 2);

  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 4);
  g_assert_cmpint (notified, ==, 2);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adap_navigation_view_replace (view, (AdapNavigationPage *[2]) {
    page_1,
    page_1,
  }, 2);
  g_test_assert_expected_messages ();
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));
  g_assert_cmpint (replaced, ==, 5);
  g_assert_cmpint (notified, ==, 3);

  adap_navigation_view_replace (view, (AdapNavigationPage *[2]) {
    page_1,
    page_2,
  }, 2);
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 6);
  g_assert_cmpint (notified, ==, 4);

  adap_navigation_view_add (view, page_2);
  adap_navigation_view_add (view, page_3);

  adap_navigation_view_replace_with_tags (view, (const char *[2]) {
    "page-2",
    "page-3",
  }, 2);
  g_assert_null (adap_navigation_view_find_page (view, "page-1"));
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_true (adap_navigation_view_find_page (view, "page-3") == page_3);
  g_assert_cmpint (replaced, ==, 7);
  g_assert_cmpint (notified, ==, 5);

  adap_navigation_view_remove (view, page_3);

  adap_navigation_view_replace_with_tags (view, NULL, 0);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_null (adap_navigation_view_find_page (view, "page-3"));
  g_assert_cmpint (replaced, ==, 8);
  g_assert_cmpint (notified, ==, 6);

  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

static void
test_adap_navigation_view_previous_page (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdapNavigationPage *page_4 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));

  adap_navigation_view_add (view, page_1);
  adap_navigation_view_add (view, page_2);
  adap_navigation_view_add (view, page_4);

  adap_navigation_view_push (view, page_2);
  adap_navigation_view_push (view, page_3);

  g_assert_null (adap_navigation_view_get_previous_page (view, page_1));
  g_assert_true (adap_navigation_view_get_previous_page (view, page_2) == page_1);
  g_assert_true (adap_navigation_view_get_previous_page (view, page_3) == page_2);
  g_assert_null (adap_navigation_view_get_previous_page (view, page_4));

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
}

static void
test_adap_navigation_view_find_page (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page_1 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  g_assert_null (adap_navigation_view_find_page (view, "page-1"));
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));

  adap_navigation_view_add (view, page_1);
  adap_navigation_view_add (view, page_2);
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);

  g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adap_navigation_page_set_tag (page_1, "page-2");
  g_test_assert_expected_messages ();
  g_assert_true (adap_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);

  adap_navigation_page_set_tag (page_1, "page-3");
  g_assert_null (adap_navigation_view_find_page (view, "page-1"));
  g_assert_true (adap_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_true (adap_navigation_view_find_page (view, "page-3") == page_1);

  adap_navigation_view_replace (view, NULL, 0);
  adap_navigation_view_remove (view, page_1);
  adap_navigation_view_remove (view, page_2);
  g_assert_null (adap_navigation_view_find_page (view, "page-1"));
  g_assert_null (adap_navigation_view_find_page (view, "page-2"));
  g_assert_null (adap_navigation_view_find_page (view, "page-3"));

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
}

static void
test_adap_navigation_view_animate_transitions (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  gboolean animate_transitions;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::animate-transitions", G_CALLBACK (increment), &notified);

  g_object_get (view, "animate-transitions", &animate_transitions, NULL);
  g_assert_true (animate_transitions);

  adap_navigation_view_set_animate_transitions (view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_view_set_animate_transitions (view, FALSE);
  g_assert_false (adap_navigation_view_get_animate_transitions (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "animate-transitions", TRUE, NULL);
  g_assert_true (adap_navigation_view_get_animate_transitions (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_navigation_view_pop_on_escape (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  gboolean pop_on_escape;
  int notified = 0;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::pop-on-escape", G_CALLBACK (increment), &notified);

  g_object_get (view, "pop-on-escape", &pop_on_escape, NULL);
  g_assert_true (pop_on_escape);

  adap_navigation_view_set_pop_on_escape (view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_view_set_pop_on_escape (view, FALSE);
  g_assert_false (adap_navigation_view_get_pop_on_escape (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "pop-on-escape", TRUE, NULL);
  g_assert_true (adap_navigation_view_get_pop_on_escape (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adap_navigation_page_child (void)
{
  GtkWidget *button = g_object_ref_sink (gtk_button_new ());
  AdapNavigationPage *page = g_object_ref_sink (adap_navigation_page_new (button, "Title"));
  GtkWidget *widget;
  int notified = 0;

  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (page, "child", &widget, NULL);
  g_assert_true (widget == button);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_page_set_child (page, NULL);
  g_assert_null (adap_navigation_page_get_child (page));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "child", button, NULL);
  g_assert_true (adap_navigation_page_get_child (page) == button);
  g_assert_cmpint (notified, ==, 2);

  g_object_unref (widget);
  g_assert_finalize_object (page);
  g_assert_finalize_object (button);
}

static void
test_adap_navigation_page_title (void)
{
  AdapNavigationPage *page = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  char *title;
  int notified = 0;

  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_page_set_title (page, "Some title");
  g_assert_cmpstr (adap_navigation_page_get_title (page), ==, "Some title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "title", "Some other title", NULL);
  g_assert_cmpstr (adap_navigation_page_get_title (page), ==, "Some other title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (page);
}

static void
test_adap_navigation_page_tag (void)
{
  AdapNavigationPage *page = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  char *tag;
  int notified = 0;

  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::tag", G_CALLBACK (increment), &notified);

  g_object_get (page, "tag", &tag, NULL);
  g_assert_null (tag);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_page_set_tag (page, "tag");
  g_assert_cmpstr (adap_navigation_page_get_tag (page), ==, "tag");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "tag", "other-tag", NULL);
  g_assert_cmpstr (adap_navigation_page_get_tag (page), ==, "other-tag");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (page);
}

static void
test_adap_navigation_page_can_pop (void)
{
  AdapNavigationPage *page = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  gboolean can_pop;
  int notified = 0;

  g_assert_nonnull (page);

  g_signal_connect_swapped (page, "notify::can-pop", G_CALLBACK (increment), &notified);

  g_object_get (page, "can-pop", &can_pop, NULL);
  g_assert_true (can_pop);

  adap_navigation_page_set_can_pop (page, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adap_navigation_page_set_can_pop (page, FALSE);
  g_assert_false (adap_navigation_page_get_can_pop (page));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "can-pop", TRUE, NULL);
  g_assert_true (adap_navigation_page_get_can_pop (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (page);
}

static void
test_adap_navigation_page_signals (void)
{
  AdapNavigationView *view = g_object_ref_sink (ADAP_NAVIGATION_VIEW (adap_navigation_view_new ()));
  AdapNavigationPage *page = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  AdapNavigationPage *page_2 = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  AdapNavigationPage *page_3 = g_object_ref_sink (adap_navigation_page_new (gtk_button_new (), "Title"));
  int showing = 0, shown = 0, hiding = 0, hidden = 0;

  g_signal_connect_swapped (page, "showing", G_CALLBACK (increment), &showing);
  g_signal_connect_swapped (page, "shown", G_CALLBACK (increment), &shown);
  g_signal_connect_swapped (page, "hiding", G_CALLBACK (increment),&hiding);
  g_signal_connect_swapped (page, "hidden", G_CALLBACK (increment), &hidden);

  g_assert_cmpint (showing, ==, 0);
  g_assert_cmpint (shown, ==, 0);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  adap_navigation_view_add (view, page);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  adap_navigation_view_push (view, page_2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  adap_navigation_view_pop (view);
  g_assert_cmpint (showing, ==, 2);
  g_assert_cmpint (shown, ==, 2);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  adap_navigation_view_replace (view, &page_2, 1);
  g_assert_cmpint (showing, ==, 2);
  g_assert_cmpint (shown, ==, 2);
  g_assert_cmpint (hiding, ==, 2);
  g_assert_cmpint (hidden, ==, 2);

  adap_navigation_view_push (view, page);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 2);
  g_assert_cmpint (hidden, ==, 2);

  adap_navigation_view_pop (view);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  adap_navigation_view_replace (view, (AdapNavigationPage *[3]) {
    page_2,
    page,
    page_3,
  }, 3);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  adap_navigation_view_pop_to_page (view, page_2);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/NavigationView/add_remove", test_adap_navigation_view_add_remove);
  g_test_add_func ("/Adapta/NavigationView/push_pop", test_adap_navigation_view_push_pop);
  g_test_add_func ("/Adapta/NavigationView/push_pop_by_tag", test_adap_navigation_view_push_pop_by_tag);
  g_test_add_func ("/Adapta/NavigationView/pop_to_page", test_adap_navigation_view_pop_to_page);
  g_test_add_func ("/Adapta/NavigationView/replace", test_adap_navigation_view_replace);
  g_test_add_func ("/Adapta/NavigationView/previous_page", test_adap_navigation_view_previous_page);
  g_test_add_func ("/Adapta/NavigationView/find_page", test_adap_navigation_view_find_page);
  g_test_add_func ("/Adapta/NavigationView/animate_transitions", test_adap_navigation_view_animate_transitions);
  g_test_add_func ("/Adapta/NavigationView/pop_on_escape", test_adap_navigation_view_pop_on_escape);
  g_test_add_func ("/Adapta/NavigationPage/child", test_adap_navigation_page_child);
  g_test_add_func ("/Adapta/NavigationPage/title", test_adap_navigation_page_title);
  g_test_add_func ("/Adapta/NavigationPage/tag", test_adap_navigation_page_tag);
  g_test_add_func ("/Adapta/NavigationPage/can-pop", test_adap_navigation_page_can_pop);
  g_test_add_func ("/Adapta/NavigationPage/signals", test_adap_navigation_page_signals);

  return g_test_run ();
}
