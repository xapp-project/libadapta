/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-version.h"

/**
 * adw_get_major_version:
 *
 * Returns the major version number of the Adwaita library.
 *
 * For example, in libadwaita version 1.2.3 this is 1.
 *
 * This function is in the library, so it represents the libadwaita library your
 * code is running against. Contrast with the [const@MAJOR_VERSION] constant,
 * which represents the major version of the libadwaita headers you have
 * included when compiling your code.
 *
 * Returns: the major version number of the Adwaita library
 */
guint
adw_get_major_version (void)
{
  return ADW_MAJOR_VERSION;
}

/**
 * adw_get_minor_version:
 *
 * Returns the minor version number of the Adwaita library.
 *
 * For example, in libadwaita version 1.2.3 this is 2.
 *
 * This function is in the library, so it represents the libadwaita library your
 * code is running against. Contrast with the [const@MAJOR_VERSION] constant,
 * which represents the minor version of the libadwaita headers you have
 * included when compiling your code.
 *
 * Returns: the minor version number of the Adwaita library
 */
guint
adw_get_minor_version (void)
{
  return ADW_MINOR_VERSION;
}

/**
 * adw_get_micro_version:
 *
 * Returns the micro version number of the Adwaita library.
 *
 * For example, in libadwaita version 1.2.3 this is 3.
 *
 * This function is in the library, so it represents the libadwaita library your
 * code is running against. Contrast with the [const@MAJOR_VERSION] constant,
 * which represents the micro version of the libadwaita headers you have
 * included when compiling your code.
 *
 * Returns: the micro version number of the Adwaita library
 */
guint
adw_get_micro_version (void)
{
  return ADW_MICRO_VERSION;
}
