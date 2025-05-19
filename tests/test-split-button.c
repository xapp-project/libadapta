/*
 * Copyright (C) 2021 Purism SPC
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
test_adap_split_button_icon_name (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  const char *icon_name;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::icon-name", G_CALLBACK (increment), &notified);

  g_object_get (button, "icon-name", &icon_name, NULL);
  g_assert_null (icon_name);

  adap_split_button_set_icon_name (button, "document-open-symbolic");
  g_assert_cmpint (notified, ==, 1);

  adap_split_button_set_icon_name (button, "document-open-symbolic");
  g_assert_cmpstr (adap_split_button_get_icon_name (button), ==, "document-open-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "icon-name", "edit-find-symbolic", NULL);
  g_assert_cmpstr (adap_split_button_get_icon_name (button), ==, "edit-find-symbolic");
  g_assert_cmpint (notified, ==, 2);

  adap_split_button_set_label (button, "Open");
  g_assert_null (adap_split_button_get_icon_name (button));
  g_assert_cmpint (notified, ==, 3);

  adap_split_button_set_icon_name (button, "document-open-symbolic");
  g_assert_cmpstr (adap_split_button_get_icon_name (button), ==, "document-open-symbolic");
  g_assert_cmpint (notified, ==, 4);

  adap_split_button_set_child (button, gtk_button_new ());
  g_assert_null (adap_split_button_get_icon_name (button));
  g_assert_cmpint (notified, ==, 5);

  g_assert_finalize_object (button);
}

static void
test_adap_split_button_label (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  const char *label;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::label", G_CALLBACK (increment), &notified);

  g_object_get (button, "label", &label, NULL);
  g_assert_null (label);

  adap_split_button_set_label (button, "Open");
  g_assert_cmpint (notified, ==, 1);

  adap_split_button_set_label (button, "Open");
  g_assert_cmpstr (adap_split_button_get_label (button), ==, "Open");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "label", "Find", NULL);
  g_assert_cmpstr (adap_split_button_get_label (button), ==, "Find");
  g_assert_cmpint (notified, ==, 2);

  adap_split_button_set_icon_name (button, "document-open-symbolic");
  g_assert_null (adap_split_button_get_label (button));
  g_assert_cmpint (notified, ==, 3);

  adap_split_button_set_label (button, "Open");
  g_assert_cmpstr (adap_split_button_get_label (button), ==, "Open");
  g_assert_cmpint (notified, ==, 4);

  adap_split_button_set_child (button, gtk_button_new ());
  g_assert_null (adap_split_button_get_label (button));
  g_assert_cmpint (notified, ==, 5);

  g_assert_finalize_object (button);
}

static void
test_adap_split_button_use_underline (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  gboolean use_underline;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::use-underline", G_CALLBACK (increment), &notified);

  g_object_get (button, "use-underline", &use_underline, NULL);
  g_assert_false (use_underline);

  adap_split_button_set_use_underline (button, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_use_underline (button, TRUE);
  g_assert_true (adap_split_button_get_use_underline (button));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "use-underline", FALSE, NULL);
  g_assert_false (adap_split_button_get_use_underline (button));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (button);
}

static void
test_adap_split_button_child (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  GtkWidget *child1, *child2, *child3, *child;
  int notified = 0;

  g_assert_nonnull (button);

  child1 = gtk_button_new ();
  child2 = gtk_button_new ();
  child3 = gtk_button_new ();

  g_signal_connect_swapped (button, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (button, "child", &child, NULL);
  g_assert_null (child);

  adap_split_button_set_child (button, NULL);
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_child (button, child1);
  g_assert_true (adap_split_button_get_child (button) == child1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "child", child2, NULL);
  g_assert_true (adap_split_button_get_child (button) == child2);
  g_assert_cmpint (notified, ==, 2);

  adap_split_button_set_label (button, "Open");
  /* adap_split_button_get_child() will return button's internal child, as will
   * gtk_button_get_child(). We can check that it's not same as the one we had
   * just set */
  g_assert_false (adap_split_button_get_child (button) == child2);
  g_assert_cmpint (notified, ==, 3);

  adap_split_button_set_child (button, child3);
  g_assert_true (adap_split_button_get_child (button) == child3);
  g_assert_cmpint (notified, ==, 4);

  adap_split_button_set_icon_name (button, "document-open-symbolic");
  g_assert_false (adap_split_button_get_child (button) == child3);
  g_assert_cmpint (notified, ==, 5);

  g_assert_finalize_object (button);
}

static void
test_adap_split_button_menu_model (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  GMenuModel *model;
  GMenuModel *model1 = G_MENU_MODEL (g_menu_new ());
  GMenuModel *model2 = G_MENU_MODEL (g_menu_new ());
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::menu-model", G_CALLBACK (increment), &notified);

  g_object_get (button, "menu-model", &model, NULL);
  g_assert_null (model);
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_menu_model (button, model1);
  g_assert_true (adap_split_button_get_menu_model (button) == model1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "menu-model", model2, NULL);
  g_assert_true (adap_split_button_get_menu_model (button) == model2);
  g_assert_cmpint (notified, ==, 2);

  adap_split_button_set_popover (button, GTK_POPOVER (gtk_popover_new ()));
  g_assert_null (adap_split_button_get_menu_model (button));
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (button);
  g_assert_finalize_object (model1);
  g_assert_finalize_object (model2);
}

static void
test_adap_split_button_popover (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  GtkPopover *popover, *popover1, *popover2;
  GMenuModel *model;
  int notified = 0;

  g_assert_nonnull (button);

  popover1 = GTK_POPOVER (gtk_popover_new ());
  popover2 = GTK_POPOVER (gtk_popover_new ());

  g_signal_connect_swapped (button, "notify::popover", G_CALLBACK (increment), &notified);

  g_object_get (button, "popover", &popover, NULL);
  g_assert_null (popover);
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_popover (button, popover1);
  g_assert_true (adap_split_button_get_popover (button) == popover1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "popover", popover2, NULL);
  g_assert_true (adap_split_button_get_popover (button) == popover2);
  g_assert_cmpint (notified, ==, 2);

  model = G_MENU_MODEL (g_menu_new ());
  adap_split_button_set_menu_model (button, model);
  /* When a menu model is set, we can still access popover, and what exactly
   * popover that is is an implementation detail. However, what we know for
   * sure is it's not the same one as we had just set */
  g_assert_false (adap_split_button_get_popover (button) == popover2);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (button);
  g_assert_finalize_object (model);
}

static void
test_adap_split_button_direction (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  GtkArrowType direction;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::direction", G_CALLBACK (increment), &notified);

  g_object_get (button, "direction", &direction, NULL);
  g_assert_cmpint (direction, ==, GTK_ARROW_DOWN);

  adap_split_button_set_direction (button, GTK_ARROW_DOWN);
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_direction (button, GTK_ARROW_UP);
  g_assert_cmpint (adap_split_button_get_direction (button), ==, GTK_ARROW_UP);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "direction", GTK_ARROW_DOWN, NULL);
  g_assert_cmpint (adap_split_button_get_direction (button), ==, GTK_ARROW_DOWN);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (button);
}

static void
test_adap_split_button_dropdown_tooltip (void)
{
  AdapSplitButton *button = g_object_ref_sink (ADAP_SPLIT_BUTTON (adap_split_button_new ()));
  char *tooltip;
  int notified = 0;

  g_assert_nonnull (button);

  g_signal_connect_swapped (button, "notify::dropdown-tooltip", G_CALLBACK (increment), &notified);

  g_object_get (button, "dropdown-tooltip", &tooltip, NULL);
  g_assert_cmpstr (tooltip, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adap_split_button_set_dropdown_tooltip (button, "Some tooltip");
  g_assert_cmpstr (adap_split_button_get_dropdown_tooltip (button), ==, "Some tooltip");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "dropdown-tooltip", "Some other tooltip", NULL);
  g_assert_cmpstr (adap_split_button_get_dropdown_tooltip (button), ==, "Some other tooltip");
  g_assert_cmpint (notified, ==, 2);

  g_free (tooltip);
  g_assert_finalize_object (button);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/SplitButton/icon_name", test_adap_split_button_icon_name);
  g_test_add_func ("/Adapta/SplitButton/label", test_adap_split_button_label);
  g_test_add_func ("/Adapta/SplitButton/use_underline", test_adap_split_button_use_underline);
  g_test_add_func ("/Adapta/SplitButton/child", test_adap_split_button_child);
  g_test_add_func ("/Adapta/SplitButton/menu_model", test_adap_split_button_menu_model);
  g_test_add_func ("/Adapta/SplitButton/popover", test_adap_split_button_popover);
  g_test_add_func ("/Adapta/SplitButton/direction", test_adap_split_button_direction);
  g_test_add_func ("/Adapta/SplitButton/dropdown_tooltip", test_adap_split_button_dropdown_tooltip);

  return g_test_run ();
}
