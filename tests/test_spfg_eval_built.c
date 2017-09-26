#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(eval_built);

TEST_SETUP(eval_built) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(eval_built) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(eval_built, test_spfg_run_cycle_loose_should_eval)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, false));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);
}

typedef struct test_cb_ctl {
    int counter;
    int stop_at;
} test_loop_ctl_t;

spfg_err_t cycle_callback_stop_ctl(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id, spfg_phase_t phase, void *cdata) {
    test_loop_ctl_t *ctl = (test_loop_ctl_t *)cdata;
    if (ctl->stop_at >= 0 && ctl->counter == ctl->stop_at) {
        return SPFG_LOOP_CONTROL_STOP;
    }
    ctl->counter += 1;
    return SPFG_ERROR_NO;
}

TEST(eval_built, test_spfg_run_cycle_control_should_eval)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn0p0_id;
    spfg_fn_id_t fn0p1_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_dp_id_t dp1p1_id;
    spfg_dp_id_t dp0p2_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;
    test_loop_ctl_t loop_ctl_1 = {0, -1};
    test_loop_ctl_t loop_ctl_2 = {0, 1};

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p1", &dp1p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p2", &dp0p2_id));

    spfg_dp_id_t in_dps_fn0p0[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps_fn0p0[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p0, 2, out_dps_fn0p0, 1, "fn0p0", &fn0p0_id));

    spfg_dp_id_t in_dps_fn0p1[] = {dp0p1_id, dp1p1_id};
    spfg_dp_id_t out_dps_fn0p1[] = {dp0p2_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p1, 2, out_dps_fn0p1, 1, "fn0p1", &fn0p1_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, cycle_callback_stop_ctl, &loop_ctl_1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p2_id, &output, &emitted));
    TEST_ASSERT_EQUAL(2, loop_ctl_1.counter);
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, cycle_callback_stop_ctl, &loop_ctl_2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(1, loop_ctl_2.counter);
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p2_id, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);
}

TEST_GROUP_RUNNER(eval_built) {
    RUN_TEST_CASE(eval_built, test_spfg_run_cycle_loose_should_eval);
    RUN_TEST_CASE(eval_built, test_spfg_run_cycle_control_should_eval);
}
