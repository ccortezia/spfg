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

    spfg_fn_id_t fn0_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_dp_id_t dp1p0_id;

    spfg_fn_id_t fn1_id;
    spfg_dp_id_t dp0p2_id;
    spfg_dp_id_t dp0p3_id;
    spfg_dp_id_t dp1p1_id;

    {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
        spfg_dp_id_t in_dps[] = {dp0p0_id, dp0p1_id};
        spfg_dp_id_t out_dps[] = {dp1p0_id};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn0_id));
    }

    {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p2", &dp0p2_id));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p3", &dp0p3_id));
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p1", &dp1p1_id));
        spfg_dp_id_t in_dps[] = {dp0p2_id, dp0p3_id};
        spfg_dp_id_t out_dps[] = {dp1p1_id};
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn2", &fn1_id));
    }

    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_GR_ID, spfg_dp_remove(-1, dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_DP_ID, spfg_dp_remove(gr_id, -1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_FN_INTEGRITY, spfg_dp_remove(gr_id, dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_FN_INTEGRITY, spfg_dp_remove(gr_id, dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_remove(gr_id, fn0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_remove(gr_id, dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_remove(gr_id, dp0p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_remove(gr_id, dp1p0_id));
}

TEST_GROUP_RUNNER(build_dp) {
    RUN_TEST_CASE(build_dp, test_spfg_dp_remove_should_keep_data_integrity);
}
