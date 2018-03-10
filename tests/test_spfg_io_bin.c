#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(io_bin);
TEST_SETUP(io_bin) {}
TEST_TEAR_DOWN(io_bin) {}


char export_outbuf[1024 * 20];

TEST(io_bin, test_spfg_rt_export_bin_general)
{
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(&runtime, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(&runtime, dp0p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(&runtime, dp1p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, false, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(&runtime, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_BUFFER_OVERFLOW, spfg_rt_export_bin(&runtime, export_outbuf, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_rt_export_bin(&runtime, 0, sizeof(export_outbuf)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_export_bin(&runtime, export_outbuf, sizeof(export_outbuf)));

    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_rt_import_bin(&runtime, NULL, sizeof(export_outbuf)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_INVALID_VALUE, spfg_rt_import_bin(&runtime, export_outbuf, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_import_bin(&runtime, export_outbuf, sizeof(export_outbuf)));
}

TEST_GROUP_RUNNER(io_bin) {
    RUN_TEST_CASE(io_bin, test_spfg_rt_export_bin_general);
}
