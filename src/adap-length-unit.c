/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-length-unit.h"

/**
 * AdwLengthUnit:
 * @ADW_LENGTH_UNIT_PX: pixels
 * @ADW_LENGTH_UNIT_PT: points, changes with text scale factor
 * @ADW_LENGTH_UNIT_SP: scale independent pixels, changes with text scale factor
 *
 * Describes length units.
 *
 * | Unit | Regular Text | Large Text |
 * | ---- | ------------ | ---------- |
 * | 1px  | 1px          | 1px        |
 * | 1pt  | 1.333333px   | 1.666667px |
 * | 1sp  | 1px          | 1.25px     |
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.4
 */

static double
get_dpi (GtkSettings *settings)
{
  int xft_dpi;

  g_object_get (settings, "gtk-xft-dpi", &xft_dpi, NULL);

  if (xft_dpi <= 0)
    xft_dpi = 96 * PANGO_SCALE;

  return xft_dpi / PANGO_SCALE;
}

/**
 * adw_length_unit_to_px:
 * @unit: a length unit
 * @value: a value in @unit
 * @settings: (nullable): settings to use, or `NULL` for default settings
 *
 * Converts @value from @unit to pixels.
 *
 * Returns: the length in pixels
 *
 * Since: 1.4
 */
double
adw_length_unit_to_px (AdwLengthUnit  unit,
                       double         value,
                       GtkSettings   *settings)
{
  g_return_val_if_fail (unit >= ADW_LENGTH_UNIT_PX, 0.0);
  g_return_val_if_fail (unit <= ADW_LENGTH_UNIT_SP, 0.0);
  g_return_val_if_fail (settings == NULL || GTK_IS_SETTINGS (settings), 0.0);

  if (!settings)
    settings = gtk_settings_get_default ();

  if (!settings)
    return 0;

  switch (unit) {
  case ADW_LENGTH_UNIT_PX:
    return value;
  case ADW_LENGTH_UNIT_PT:
    return value * get_dpi (settings) / 72.0;
  case ADW_LENGTH_UNIT_SP:
    return value * get_dpi (settings) / 96.0;
  default:
    g_assert_not_reached ();
  }
}

/**
 * adw_length_unit_from_px:
 * @unit: a length unit
 * @value: a value in pixels
 * @settings: (nullable): settings to use, or `NULL` for default settings
 *
 * Converts @value from pixels to @unit.
 *
 * Returns: the length in @unit
 *
 * Since: 1.4
 */
double
adw_length_unit_from_px (AdwLengthUnit  unit,
                         double         value,
                         GtkSettings   *settings)
{
  g_return_val_if_fail (unit >= ADW_LENGTH_UNIT_PX, 0.0);
  g_return_val_if_fail (unit <= ADW_LENGTH_UNIT_SP, 0.0);
  g_return_val_if_fail (settings == NULL || GTK_IS_SETTINGS (settings), 0.0);

  if (!settings)
    settings = gtk_settings_get_default ();

  if (!settings)
    return 0;

  switch (unit) {
  case ADW_LENGTH_UNIT_PX:
    return value;
  case ADW_LENGTH_UNIT_PT:
    return value / get_dpi (settings) * 72.0;
  case ADW_LENGTH_UNIT_SP:
    return value / get_dpi (settings) * 96.0;
  default:
    g_assert_not_reached ();
  }
}
