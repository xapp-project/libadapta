/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adapta.h>

#define TEST_ICON_NAME "avatar-default-symbolic"
#define TEST_STRING "Mario Rossi"
#define TEST_SIZE 128


static void
test_adap_avatar_icon_name (void)
{
  AdapAvatar *avatar = g_object_ref_sink (ADAP_AVATAR (adap_avatar_new (128, NULL, TRUE)));

  g_assert_null (adap_avatar_get_icon_name (avatar));
  adap_avatar_set_icon_name (avatar, TEST_ICON_NAME);
  g_assert_cmpstr (adap_avatar_get_icon_name (avatar), ==, TEST_ICON_NAME);

  g_assert_finalize_object (avatar);
}

static void
test_adap_avatar_text (void)
{
  AdapAvatar *avatar = g_object_ref_sink (ADAP_AVATAR (adap_avatar_new (128, NULL, TRUE)));

  g_assert_cmpstr (adap_avatar_get_text (avatar), ==, "");
  adap_avatar_set_text (avatar, TEST_STRING);
  g_assert_cmpstr (adap_avatar_get_text (avatar), ==, TEST_STRING);

  g_assert_finalize_object (avatar);
}

static void
test_adap_avatar_size (void)
{
  AdapAvatar *avatar = g_object_ref_sink (ADAP_AVATAR (adap_avatar_new (TEST_SIZE, NULL, TRUE)));

  g_assert_cmpint (adap_avatar_get_size (avatar), ==, TEST_SIZE);
  adap_avatar_set_size (avatar, TEST_SIZE / 2);
  g_assert_cmpint (adap_avatar_get_size (avatar), ==, TEST_SIZE / 2);

  g_assert_finalize_object (avatar);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/Avatar/icon_name", test_adap_avatar_icon_name);
  g_test_add_func ("/Adapta/Avatar/text", test_adap_avatar_text);
  g_test_add_func ("/Adapta/Avatar/size", test_adap_avatar_size);

  return g_test_run ();
}
