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
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_dp_id_t dp1p0_id;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp0p1_id, dp0p1_id};
    spfg_dp_id_t out_dps[] = {dp1p0_id, dp1p0_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 3, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 1, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 0, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_VALIDATE_FN, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 2, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn_id));
}

TEST(build_fn, test_spfg_fn_remove_should_keep_data_integrity)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_dp_id_t dp1p0_id;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp0p1_id};
    spfg_dp_id_t out_dps[] = {dp1p0_id, dp1p0_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 2, out_dps, 1, "fn1", &fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_GR_ID, spfg_fn_remove(-1, fn_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_FN_ID, spfg_fn_remove(gr_id, -1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_remove(gr_id, fn_id));
}

TEST_GROUP_RUNNER(build_fn) {
    RUN_TEST_CASE(build_fn, test_spfg_fn_create_should_validate_input_cohesion);
    RUN_TEST_CASE(build_fn, test_spfg_fn_remove_should_keep_data_integrity);
}
