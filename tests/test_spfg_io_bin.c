#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(io_bin);

TEST_SETUP(io_bin) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(io_bin) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

char export_outbuf[1024 * 20];

TEST(io_bin, test_spfg_gr_export_bin_general)
{
    spfg_gr_id_t gr0_id;
    spfg_gr_id_t gr1_id;
    spfg_gr_id_t gr2_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("gr0", &gr0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr0_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr0_id, dp0p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr0_id, dp1p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr0_id, false, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr0_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_BUFFER_OVERFLOW, spfg_gr_export_bin(gr0_id, export_outbuf, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_GR_ID, spfg_gr_export_bin(-1, export_outbuf, sizeof(export_outbuf)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_export_bin(gr0_id, 0, sizeof(export_outbuf)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_export_bin(gr0_id, export_outbuf, sizeof(export_outbuf)));

    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_import_bin(NULL, sizeof(export_outbuf), &gr1_id, "gr1"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_import_bin(export_outbuf, sizeof(export_outbuf), NULL, "gr1"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_import_bin(export_outbuf, sizeof(export_outbuf), &gr1_id, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_INVALID_VALUE, spfg_gr_import_bin(export_outbuf, 1, &gr1_id, "gr1"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_import_bin(export_outbuf, sizeof(export_outbuf), &gr1_id, "gr1"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_OUT_OF_SLOTS, spfg_gr_import_bin(export_outbuf, sizeof(export_outbuf), &gr2_id, "gr2"));

    TEST_ASSERT_NOT_EQUAL(0, gr1_id);
    TEST_ASSERT_NOT_EQUAL(gr0_id, gr1_id);
}

TEST_GROUP_RUNNER(io_bin) {
    RUN_TEST_CASE(io_bin, test_spfg_gr_export_bin_general);
}
