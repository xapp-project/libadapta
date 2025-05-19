/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adap_combo_row_set_for_enum (void)
{
  AdapComboRow *row = g_object_ref_sink (ADAP_COMBO_ROW (adap_combo_row_new ()));
  GtkExpression *expr = NULL;
  GListModel *model;
  AdapEnumListItem *item;

  g_assert_nonnull (row);
  g_assert_null (adap_combo_row_get_model (row));

  expr = gtk_property_expression_new (ADAP_TYPE_ENUM_LIST_ITEM, NULL, "nick");
  adap_combo_row_set_expression (row, expr);
  gtk_expression_unref (expr);

  model = G_LIST_MODEL (adap_enum_list_model_new (GTK_TYPE_ORIENTATION));
  adap_combo_row_set_model (row, model);
  g_object_unref (model);

  model = adap_combo_row_get_model (row);
  g_assert_true (G_IS_LIST_MODEL (model));

  g_assert_cmpuint (g_list_model_get_n_items (model), ==, 2);

  item = g_list_model_get_item (model, 0);
  g_assert_true (ADAP_IS_ENUM_LIST_ITEM (item));
  g_assert_cmpstr (adap_enum_list_item_get_nick (item), ==, "horizontal");

  item = g_list_model_get_item (model, 1);
  g_assert_true (ADAP_IS_ENUM_LIST_ITEM (item));
  g_assert_cmpstr (adap_enum_list_item_get_nick (item), ==, "vertical");

  g_assert_finalize_object (row);
}

static void
test_adap_combo_row_selected (void)
{
  AdapComboRow *row = g_object_ref_sink (ADAP_COMBO_ROW (adap_combo_row_new ()));
  GListModel *model;
  int selected = 0, notified = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "notify::selected", G_CALLBACK (increment), &notified);

  g_object_get (row, "selected", &selected, NULL);
  g_assert_cmpint (selected, ==, -1);

  adap_combo_row_set_selected (row, -1);
  g_assert_cmpint (notified, ==, 0);

  model = G_LIST_MODEL (adap_enum_list_model_new (GTK_TYPE_SELECTION_MODE));

  adap_combo_row_set_model (row, model);

  g_assert_cmpint (adap_combo_row_get_selected (row), ==, 0);
  g_assert_cmpint (notified, ==, 1);

  adap_combo_row_set_selected (row, 3);
  g_assert_cmpint (adap_combo_row_get_selected (row), ==, 3);
  g_assert_cmpint (notified, ==, 2);

  g_object_set (row, "selected", 1, NULL);
  g_assert_cmpint (adap_combo_row_get_selected (row), ==, 1);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (row);
  g_assert_finalize_object (model);
}

static void
test_adap_combo_row_use_subtitle (void)
{
  AdapComboRow *row = g_object_ref_sink (ADAP_COMBO_ROW (adap_combo_row_new ()));
  gboolean use_subtitle = FALSE;
  int notified = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "notify::use-subtitle", G_CALLBACK (increment), &notified);

  g_assert_false (adap_combo_row_get_use_subtitle (row));

  adap_combo_row_set_use_subtitle (row, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adap_combo_row_set_use_subtitle (row, TRUE);
  g_assert_true (adap_combo_row_get_use_subtitle (row));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (row, "use-subtitle", FALSE, NULL);
  g_object_get (row, "use-subtitle", &use_subtitle, NULL);
  g_assert_false (use_subtitle);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func("/Adapta/ComboRow/set_for_enum", test_adap_combo_row_set_for_enum);
  g_test_add_func("/Adapta/ComboRow/selected", test_adap_combo_row_selected);
  g_test_add_func("/Adapta/ComboRow/use_subtitle", test_adap_combo_row_use_subtitle);

  return g_test_run();
}
