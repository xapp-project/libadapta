/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adap-fold-threshold-policy.h"

/**
 * AdapFoldThresholdPolicy:
 * @ADAP_FOLD_THRESHOLD_POLICY_MINIMUM: Folding is based on the minimum size
 * @ADAP_FOLD_THRESHOLD_POLICY_NATURAL: Folding is based on the natural size
 *
 * Determines when [class@Flap] and [class@Leaflet] will fold.
 *
 * Deprecated: 1.4: Stop using `AdapLeaflet` and `AdapFlap`
 */
