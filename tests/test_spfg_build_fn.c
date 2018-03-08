#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(build_fn);

TEST_SETUP(build_fn) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(build_fn) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(build_fn, test_spfg_fn_create_should_validate_input_cohesion)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1, dp1};
    spfg_dp_id_t out_dps[] = {dp2, dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 3, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 1, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 0, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 2, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn_id));
}

TEST(build_fn, test_spfg_fn_remove_should_keep_data_integrity)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1};
    spfg_dp_id_t out_dps[] = {dp2, dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_ID, spfg_rt_fn_remove(&runtime, -1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_remove(&runtime, fn_id));
}

TEST_GROUP_RUNNER(build_fn) {
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_should_validate_input_cohesion);
    RUN_TEST_CASE(build_fn, test_spfg_fn_remove_should_keep_data_integrity);
}
