#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(io_json);
TEST_SETUP(io_json) {}
TEST_TEAR_DOWN(io_json) {}


TEST(io_json, test_spfg_rt_import_json_should_accept_valid_string)
{
    uint32_t outlen;
    char original_json[] = "{\"id\": 1, \"name\": \"gr0\", \"fns\": [{\"id\": 1, \"name\": \"fn1\", \"type\": 2, \"phase\": 0, \"in_dp_ids\": [1, 2], \"out_dp_ids\": [3]}], \"dps\": [{\"id\": 1, \"name\": \"dp0p0\", \"type\": 3, \"value\": true, \"emitted\": false}, {\"id\": 2, \"name\": \"dp1p0\", \"type\": 3, \"value\": false, \"emitted\": false}, {\"id\": 3, \"name\": \"dp0p1\", \"type\": 3, \"value\": false, \"emitted\": false}], \"ctl\": {\"curr_fn_idx\": 1}}";
    char exported_json[1024];
    spfg_runtime_t runtime;

    // Import reference JSON string to compare with generated output.
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_import_json(&runtime, original_json, sizeof(original_json)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_export_json(&runtime, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL_STRING(original_json, exported_json);

    // Reimport generated output JSON string to compare with reference.
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_import_json(&runtime, exported_json, sizeof(exported_json)));
    memset(exported_json, 0, sizeof(exported_json));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_export_json(&runtime, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL_STRING(original_json, exported_json);
}

TEST(io_json, test_spfg_rt_export_json_should_dump_built_grid)
{
    spfg_fn_id_t fn0;
    spfg_dp_id_t dp0;
    spfg_dp_id_t dp1;
    spfg_dp_id_t dp2;
    spfg_runtime_t runtime;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "runtime"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp0", &dp0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp1", &dp1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_create(&runtime, SPFG_DP_BOOL, "dp2", &dp2));

    spfg_dp_id_t in_dps[] = {dp0, dp1};
    spfg_dp_id_t out_dps[] = {dp2};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_fn_create(&runtime, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn0", &fn0));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_reset_cycle(&runtime));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp0, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, false, NULL, NULL));

    uint32_t outlen;
    char exported_json[1024];
    char expected_json[] = "{\"id\": 1, \"name\": \"runtime\", \"fns\": [{\"id\": 1, \"name\": \"fn0\", \"type\": 2, \"phase\": 0, \"in_dp_ids\": [1, 2], \"out_dp_ids\": [3]}], \"dps\": [{\"id\": 1, \"name\": \"dp0\", \"type\": 3, \"value\": true, \"emitted\": false}, {\"id\": 2, \"name\": \"dp1\", \"type\": 3, \"value\": false, \"emitted\": false}, {\"id\": 3, \"name\": \"dp2\", \"type\": 3, \"value\": false, \"emitted\": false}], \"ctl\": {\"curr_fn_idx\": 1}}";
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_export_json(&runtime, exported_json, sizeof(exported_json), &outlen));
    TEST_ASSERT_EQUAL(374, outlen);
    TEST_ASSERT_EQUAL_STRING(expected_json, exported_json);
}

TEST_GROUP_RUNNER(io_json) {
    RUN_TEST_CASE(io_json, test_spfg_rt_import_json_should_accept_valid_string);
    RUN_TEST_CASE(io_json, test_spfg_rt_export_json_should_dump_built_grid);
}
