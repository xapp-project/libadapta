/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adapta.h>

static inline void
check_to_string (AdapBreakpointCondition *condition,
                 const char             *expected)
{
  char *str = adap_breakpoint_condition_to_string (condition);

  g_assert_cmpstr (str, ==, expected);

  g_free (str);
  adap_breakpoint_condition_free (condition);
}

static inline void
check_parse (const char *input,
             const char *expected)
{
  AdapBreakpointCondition *condition;

  if (!expected)
    g_test_expect_message (ADAP_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Unable to parse condition*");

  condition = adap_breakpoint_condition_parse (input);

  if (expected) {
    g_assert_nonnull (condition);

    check_to_string (condition, expected);
  } else {
    if (condition != NULL) {
      char *str = adap_breakpoint_condition_to_string (condition);

      g_test_message ("'%s' is invalid, but was parsed as '%s'", input, str);
      g_free (str);
    }

    g_test_assert_expected_messages ();

    g_assert_null (condition);
  }
}

static void
test_adap_breakpoint_condition_to_string (void)
{
  AdapBreakpointCondition *condition_1, *condition_2, *condition_3;

  check_to_string (adap_breakpoint_condition_new_length (ADAP_BREAKPOINT_CONDITION_MAX_WIDTH,
                                                        400,
                                                        ADAP_LENGTH_UNIT_PX),
                   "max-width: 400px");

  check_to_string (adap_breakpoint_condition_new_length (ADAP_BREAKPOINT_CONDITION_MIN_HEIGHT,
                                                        200,
                                                        ADAP_LENGTH_UNIT_PT),
                   "min-height: 200pt");

  check_to_string (adap_breakpoint_condition_new_ratio (ADAP_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       4, 3),
                   "min-aspect-ratio: 4/3");

  check_to_string (adap_breakpoint_condition_new_ratio (ADAP_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       2, 1),
                   "min-aspect-ratio: 2");
  check_to_string (adap_breakpoint_condition_new_ratio (ADAP_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       0, 2),
                   "min-aspect-ratio: 0");

  condition_1 = adap_breakpoint_condition_new_length (ADAP_BREAKPOINT_CONDITION_MAX_WIDTH,
                                                     400,
                                                     ADAP_LENGTH_UNIT_PX);
  condition_2 = adap_breakpoint_condition_new_ratio (ADAP_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                    4, 3);
  condition_3 = adap_breakpoint_condition_new_ratio (ADAP_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO,
                                                    2, 1);

  check_to_string (adap_breakpoint_condition_new_and (adap_breakpoint_condition_copy (condition_1),
                                                     adap_breakpoint_condition_copy (condition_2)),
                   "max-width: 400px and min-aspect-ratio: 4/3");

  check_to_string (adap_breakpoint_condition_new_or (adap_breakpoint_condition_copy (condition_1),
                                                    adap_breakpoint_condition_copy (condition_2)),
                   "max-width: 400px or min-aspect-ratio: 4/3");

  check_to_string (adap_breakpoint_condition_new_and (adap_breakpoint_condition_copy (condition_1),
                                                     adap_breakpoint_condition_new_and (adap_breakpoint_condition_copy (condition_2),
                                                                                       adap_breakpoint_condition_copy (condition_3))),
                   "max-width: 400px and min-aspect-ratio: 4/3 and max-aspect-ratio: 2");

  check_to_string (adap_breakpoint_condition_new_and (adap_breakpoint_condition_copy (condition_1),
                                                     adap_breakpoint_condition_new_or (adap_breakpoint_condition_copy (condition_2),
                                                                                       adap_breakpoint_condition_copy (condition_3))),
                   "max-width: 400px and (min-aspect-ratio: 4/3 or max-aspect-ratio: 2)");

  check_to_string (adap_breakpoint_condition_new_or (adap_breakpoint_condition_new_and (adap_breakpoint_condition_copy (condition_1),
                                                                                      adap_breakpoint_condition_copy (condition_2)),
                                                    adap_breakpoint_condition_copy (condition_3)),
                   "(max-width: 400px and min-aspect-ratio: 4/3) or max-aspect-ratio: 2");

  adap_breakpoint_condition_free (condition_1);
  adap_breakpoint_condition_free (condition_2);
  adap_breakpoint_condition_free (condition_3);
}

static void
test_adap_breakpoint_condition_parse (void)
{
  check_parse ("", NULL);
  check_parse ("()", NULL);
  check_parse ("foo", NULL);

  /* Length */
  check_parse ("max-width: 400px",   "max-width: 400px");
  check_parse ("max-width: 400",     "max-width: 400px");
  check_parse ("max-width: 400pt",   "max-width: 400pt");
  check_parse ("max-width:400pt",    "max-width: 400pt");
  check_parse ("max-width: 400.0px", "max-width: 400px");
  check_parse ("max-width: 400.5px", "max-width: 400.5px");
  check_parse ("      max-width        :        400     pt       ", "max-width: 400pt");

  check_parse ("max-width:",        NULL);
  check_parse ("max-width: px",     NULL);
  check_parse ("max-length: 400px", NULL);
  check_parse ("max-width 400px",   NULL);
  check_parse ("max-width: -1px",   NULL);
  check_parse ("max-width: 400p",   NULL);
  check_parse ("max-width: 400px;", NULL);

  /* Ratio */
  check_parse ("max-aspect-ratio: 4/3", "max-aspect-ratio: 4/3");
  check_parse ("max-aspect-ratio: 2",   "max-aspect-ratio: 2");
  check_parse ("max-aspect-ratio: 2/1", "max-aspect-ratio: 2");
  check_parse ("max-aspect-ratio: 0/3", "max-aspect-ratio: 0");
  check_parse ("max-aspect-ratio:4/3",  "max-aspect-ratio: 4/3");
  check_parse ("       max-aspect-ratio   :         4/3       ", "max-aspect-ratio: 4/3");

  check_parse ("max-aspect-ratio:",       NULL);
  check_parse ("max-aspect-ratio: 4/3px", NULL);
  check_parse ("max-aspect-ratio: 4px",   NULL);
  check_parse ("max-aspect-ratio: -4",    NULL);
  check_parse ("max-aspect-ratio: -4/3",  NULL);
  check_parse ("max-aspect-ratio: 4/0",   NULL);
  check_parse ("max-aspect-ratio: 4/3;",  NULL);

  /* Single + parentheses */
  check_parse ("(max-width: 100px)",     "max-width: 100px");
  check_parse ("(((max-width: 100px)))", "max-width: 100px");
  check_parse ("   (   max-width   :   100px   )   ", "max-width: 100px");

  check_parse ("(max-width: 100px",   NULL);
  check_parse ("(max-width: 100px(",  NULL);
  check_parse ("(max-width): 100px",  NULL);
  check_parse ("(max-width: 100px))", NULL);

  /* Multi */
  check_parse ("max-width: 100px and max-height: 200px", "max-width: 100px and max-height: 200px");
  check_parse ("max-width: 100px or max-height: 200px",  "max-width: 100px or max-height: 200px");
  check_parse ("   max-width   :   100px   or   max-height   :   200px   ", "max-width: 100px or max-height: 200px");

  check_parse ("(max-width: 100px) and max-height: 200px",   "max-width: 100px and max-height: 200px");
  check_parse ("max-width: 100px and (max-height: 200px)",   "max-width: 100px and max-height: 200px");
  check_parse ("(max-width: 100px) and (max-height: 200px)", "max-width: 100px and max-height: 200px");
  check_parse ("(max-width: 100px and max-height: 200px)",   "max-width: 100px and max-height: 200px");

  check_parse ("(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2",
               "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2");
  check_parse ("max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)",
               "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)");

  check_parse ("(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2",
               "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2");
  check_parse ("max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)",
               "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)");

  check_parse ("max-width: 100px and max-height: 200px and max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2",
               "max-width: 100px and max-height: 200px and max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2");
  check_parse ("max-width: 100px or max-height: 200px or max-aspect-ratio: 3/2 or min-aspect-ratio: 1/2",
               "max-width: 100px or max-height: 200px or max-aspect-ratio: 3/2 or min-aspect-ratio: 1/2");

  check_parse ("max-width: 100px and max-height: 200px or max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2",
               "((max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2) and min-aspect-ratio: 1/2");

  check_parse ("max-width: 100pxor max-height: 200px", NULL);
  check_parse ("max-width: 100px ormax-height: 200px", NULL);
  check_parse ("max-width: 100px max-height: 200px",   NULL);
  check_parse ("max-width: 100px or max-height",       NULL);
  check_parse ("max-width: 100px o",                   NULL);
  check_parse ("max-width: 100px or ()",               NULL);
  check_parse ("() or max-height: 200px",              NULL);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adap_init ();

  g_test_add_func ("/Adapta/BreakpointCondition/to_string", test_adap_breakpoint_condition_to_string);
  g_test_add_func ("/Adapta/BreakpointCondition/parse", test_adap_breakpoint_condition_parse);

  return g_test_run ();
}
