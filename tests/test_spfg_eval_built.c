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
    spfg_fn_id_t fn0;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_boolean_t output;
    spfg_boolean_t emitted;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1};
    spfg_dp_id_t out_dps[] = {dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn0));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp2, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp0, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp1, false));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp2, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp0, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp1, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp2, &output, &emitted));
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);
}

typedef struct test_cb_ctl {
    int counter;
    int stop_at;
} test_loop_ctl_t;

spfg_err_t cycle_callback_stop_ctl(spfg_fn_id_t fn_id, spfg_phase_t phase, void *cdata) {
    test_loop_ctl_t *ctl = (test_loop_ctl_t *)cdata;
    if (ctl->stop_at >= 0 && ctl->counter == ctl->stop_at) {
        return SPFG_LOOP_CONTROL_STOP;
    }
    ctl->counter += 1;
    return SPFG_ERROR_NO;
}

TEST(eval_built, test_spfg_run_cycle_control_should_eval)
{
    spfg_fn_id_t fn0;
    spfg_fn_id_t fn1;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_dp_id_t dp3;
    spfg_dp_id_t dp4;
    spfg_boolean_t output;
    spfg_boolean_t emitted;
    test_loop_ctl_t loop_ctl_1 = {0, -1};
    test_loop_ctl_t loop_ctl_2 = {0, 1};
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp4", &dp4));

    spfg_dp_id_t in_dps_fn0p0[] = {dp0, dp1};
    spfg_dp_id_t out_dps_fn0p0[] = {dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p0, 2, out_dps_fn0p0, 1, "fn0", &fn0));

    spfg_dp_id_t in_dps_fn0p1[] = {dp2, dp3};
    spfg_dp_id_t out_dps_fn0p1[] = {dp4};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p1, 2, out_dps_fn0p1, 1, "fn1", &fn1));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, cycle_callback_stop_ctl, &loop_ctl_1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp4, &output, &emitted));
    TEST_ASSERT_EQUAL(2, loop_ctl_1.counter);
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp0, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp1, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, cycle_callback_stop_ctl, &loop_ctl_2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp2, &output, &emitted));
    TEST_ASSERT_EQUAL(1, loop_ctl_2.counter);
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp4, &output, &emitted));
    TEST_ASSERT_EQUAL(false, output);
    TEST_ASSERT_EQUAL(false, emitted);
}

TEST_GROUP_RUNNER(eval_built) {
    RUN_TEST_CASE(eval_built, test_spfg_run_cycle_loose_should_eval);
    RUN_TEST_CASE(eval_built, test_spfg_run_cycle_control_should_eval);
}
