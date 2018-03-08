#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(build_dp);

TEST_SETUP(build_dp) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(build_dp) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(build_dp, test_spfg_dp_remove_should_keep_data_integrity)
{
    spfg_gr_id_t gr_id;

    spfg_fn_id_t fn0;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;

    spfg_fn_id_t fn1;
    spfg_dp_id_t dp3;
    spfg_dp_id_t dp4;
    spfg_dp_id_t dp5;

    spfg_runtime_t runtime;

    {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));
        spfg_dp_id_t in_dps[] = {dp0, dp1};
        spfg_dp_id_t out_dps[] = {dp2};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn0));
    }

    {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp3", &dp3));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp4", &dp4));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp5", &dp5));
        spfg_dp_id_t in_dps[] = {dp3, dp4};
        spfg_dp_id_t out_dps[] = {dp5};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn2", &fn1));
    }

    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_DP_ID, spfg_rt_dp_remove(&runtime, -1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_FN_INTEGRITY, spfg_rt_dp_remove(&runtime, dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_FN_INTEGRITY, spfg_rt_dp_remove(&runtime, dp2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_remove(&runtime, fn0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_remove(&runtime, dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_remove(&runtime, dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_remove(&runtime, dp2));
}

TEST_GROUP_RUNNER(build_dp) {
    RUN_TEST_CASE(build_dp, test_spfg_dp_remove_should_keep_data_integrity);
}
