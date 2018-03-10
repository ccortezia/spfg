#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(eval_json);
TEST_SETUP(eval_json) {}
TEST_TEAR_DOWN(eval_json) {}


TEST(eval_json, test_spfg_run_cycle_from_imported_json_should_eval)
{
    spfg_dp_id_t dp1p1_id = 4;
    spfg_dp_id_t dp0p2_id = 5;
    spfg_boolean_t output = false;
    spfg_boolean_t emitted = false;
    spfg_runtime_t runtime;

    char json_snapshot[] = "{\"id\": 1, \"name\": \"valid name\", "
        "\"fns\": [{\"id\": 1, \"name\": \"fn0p0\", \"type\": 2, \"phase\": 0, \"in_dp_ids\": [1, 3], \"out_dp_ids\": [2]}, "
                  "{\"id\": 2, \"name\": \"fn0p1\", \"type\": 2, \"phase\": 1, \"in_dp_ids\": [2, 4], \"out_dp_ids\": [5]}], "
        "\"dps\": [{\"id\": 1, \"name\": \"dp0p0\", \"type\": 3, \"value\": true, \"emitted\": false}, "
                  "{\"id\": 2, \"name\": \"dp0p1\", \"type\": 3, \"value\": true, \"emitted\": true}, "
                  "{\"id\": 3, \"name\": \"dp1p0\", \"type\": 3, \"value\": true, \"emitted\": false}, "
                  "{\"id\": 4, \"name\": \"dp1p1\", \"type\": 3, \"value\": false, \"emitted\": false}, "
                  "{\"id\": 5, \"name\": \"dp0p2\", \"type\": 3, \"value\": false, \"emitted\": false}], "
        "\"ctl\": {\"curr_fn_idx\": 1}}";

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_import_json(&runtime, json_snapshot, sizeof(json_snapshot)));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_set_bool(&runtime, dp1p1_id, true));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_run_cycle(&runtime, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_dp_get_bool(&runtime, dp0p2_id, &output, &emitted));
    TEST_ASSERT_EQUAL(true, output);
    TEST_ASSERT_EQUAL(true, emitted);
}

TEST_GROUP_RUNNER(eval_json) {
    RUN_TEST_CASE(eval_json, test_spfg_run_cycle_from_imported_json_should_eval);
}
