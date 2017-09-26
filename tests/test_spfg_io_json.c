#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(io_json);

TEST_SETUP(io_json) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(io_json) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(io_json, test_spfg_gr_import_json_should_accept_valid_string)
{
    uint32_t outlen;
    char original_json[] = "{\"id\": 1, \"name\": \"gr0\", \"fns\": [{\"id\": 1, \"name\": \"fn1\", \"type\": 2, \"phase\": 0, \"in_dp_ids\": [1, 2], \"out_dp_ids\": [3]}], \"dps\": [{\"id\": 1, \"name\": \"dp0p0\", \"type\": 3, \"value\": true, \"emitted\": false}, {\"id\": 2, \"name\": \"dp1p0\", \"type\": 3, \"value\": false, \"emitted\": false}, {\"id\": 3, \"name\": \"dp0p1\", \"type\": 3, \"value\": false, \"emitted\": false}], \"ctl\": {\"curr_phase\": 0, \"curr_fn_idx\": 1}}";
    char exported_json[1024];

    // Import reference JSON string to compare with generated output.
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_import_json(original_json, sizeof(original_json), NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_export_json(1, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL_STRING(original_json, exported_json);

    // Reimport generated output JSON string to compare with reference.
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_import_json(exported_json, sizeof(exported_json), NULL));
    memset(exported_json, 0, sizeof(exported_json));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_export_json(1, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL_STRING(original_json, exported_json);
}

TEST(io_json, test_spfg_gr_export_json_should_dump_built_grid)
{
    spfg_gr_id_t gr0_id;
    spfg_gr_id_t gr1_id;
    spfg_gr_id_t gr2_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr0_id, "gr0"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr0_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr0_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr0_id, dp0p0_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr0_id, false, NULL, NULL));

    uint32_t outlen;
    char exported_json[1024];
    char expected_json[] = "{\"id\": 1, \"name\": \"gr0\", \"fns\": [{\"id\": 1, \"name\": \"fn1\", \"type\": 2, \"phase\": 0, \"in_dp_ids\": [1, 2], \"out_dp_ids\": [3]}], \"dps\": [{\"id\": 1, \"name\": \"dp0p0\", \"type\": 3, \"value\": true, \"emitted\": false}, {\"id\": 2, \"name\": \"dp1p0\", \"type\": 3, \"value\": false, \"emitted\": false}, {\"id\": 3, \"name\": \"dp0p1\", \"type\": 3, \"value\": false, \"emitted\": false}], \"ctl\": {\"curr_phase\": 0, \"curr_fn_idx\": 1}}";
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_export_json(gr0_id, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL(393, outlen);
    TEST_ASSERT_EQUAL_STRING(expected_json, exported_json);
}

TEST_GROUP_RUNNER(io_json) {
    RUN_TEST_CASE(io_json, test_spfg_gr_import_json_should_accept_valid_string);
    RUN_TEST_CASE(io_json, test_spfg_gr_export_json_should_dump_built_grid);
}
