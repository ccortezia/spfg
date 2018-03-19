#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(build_fn);
TEST_SETUP(build_fn) {}
TEST_TEAR_DOWN(build_fn) {}


TEST(build_fn, test_spfg_fn_create_should_validate_input_cohesion)
{
    spfg_fn_id_t fn0;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1, dp1};
    spfg_dp_id_t out_dps[] = {dp2, dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 3, out_dps, 1, "fn0", &fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 1, out_dps, 1, "fn0", &fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 0, "fn0", &fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 2, "fn0", &fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn0", &fn0));
}

TEST(build_fn, test_spfg_fn_create_cycle_later_phase_out_to_prior_phase_in_should_err)
{
    spfg_fn_id_t fn0;
    spfg_fn_id_t fn1;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_dp_id_t dp3;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));

    {
        spfg_dp_id_t in_dps[] = {dp0, dp1};
        spfg_dp_id_t out_dps[] = {dp2};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn0", &fn0));
    }
    {
        spfg_dp_id_t in_dps[] = {dp2, dp3};
        spfg_dp_id_t out_dps[] = {dp0};
        TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_CYCLE, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn1));
    }
}

TEST(build_fn, test_spfg_fn_create_cycle_prior_phase_in_to_later_phase_out_should_err)
{
    spfg_fn_id_t fn0;
    spfg_fn_id_t fn1;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_dp_id_t dp3;
    spfg_dp_id_t dp4;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp4", &dp4));

    {
        spfg_dp_id_t in_dps[] = {dp2, dp3};
        spfg_dp_id_t out_dps[] = {dp4};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn1));
    }
    {
        spfg_dp_id_t in_dps[] = {dp0, dp4};
        spfg_dp_id_t out_dps[] = {dp2};
        TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_CYCLE, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn0", &fn0));
    }
}

TEST(build_fn, test_spfg_fn_create_cycle_same_phase_in_to_out_should_err)
{
    spfg_fn_id_t fn0;
    spfg_fn_id_t fn1;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_dp_id_t dp3;
    spfg_dp_id_t dp4;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp4", &dp4));

    {
        spfg_dp_id_t in_dps[] = {dp0, dp1};
        spfg_dp_id_t out_dps[] = {dp2};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn0", &fn0));
    }
    {
        spfg_dp_id_t in_dps[] = {dp2, dp3};
        spfg_dp_id_t out_dps[] = {dp4};
        TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_CYCLE, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn1));
    }
}

TEST(build_fn, test_spfg_fn_create_cycle_same_phase_out_to_in_should_err)
{
    spfg_fn_id_t fn0;
    spfg_fn_id_t fn1;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_dp_id_t dp3;
    spfg_dp_id_t dp4;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp4", &dp4));

    {
        spfg_dp_id_t in_dps[] = {dp0, dp1};
        spfg_dp_id_t out_dps[] = {dp2};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn0", &fn0));
    }
    {
        spfg_dp_id_t in_dps[] = {dp3, dp4};
        spfg_dp_id_t out_dps[] = {dp0};
        TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_CYCLE, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn1));
    }
}

TEST(build_fn, test_spfg_fn_remove_should_keep_data_integrity)
{
    spfg_fn_id_t fn0;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1};
    spfg_dp_id_t out_dps[] = {dp2, dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn0", &fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_ID, spfg_fn_remove(&runtime, -1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_remove(&runtime, fn0));
}

TEST_GROUP_RUNNER(build_fn) {
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_should_validate_input_cohesion);
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_cycle_later_phase_out_to_prior_phase_in_should_err);
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_cycle_prior_phase_in_to_later_phase_out_should_err);
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_cycle_same_phase_in_to_out_should_err);
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_cycle_same_phase_out_to_in_should_err);
    RUN_TEST_CASE(build_fn, test_spfg_fn_remove_should_keep_data_integrity);
}
