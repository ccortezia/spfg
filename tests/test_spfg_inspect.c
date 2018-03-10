#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(inspect);
TEST_SETUP(inspect) {}
TEST_TEAR_DOWN(inspect) {}


TEST(inspect, test_spfg_info_should_return_compiled_info)
{
    spfg_info_t info;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_info(NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_info(&info));
    TEST_ASSERT_EQUAL(VERSION_MAJOR, info.version_major);
    TEST_ASSERT_EQUAL(VERSION_MINOR, info.version_minor);
    TEST_ASSERT_EQUAL(VERSION_PATCH, info.version_patch);
    TEST_ASSERT_EQUAL(SPFG_MAX_GRID_CNT, info.max_grid_cnt);
    TEST_ASSERT_EQUAL(SPFG_MAX_FN_IN_DPS, info.max_fn_in_dps);
    TEST_ASSERT_EQUAL(SPFG_MAX_FN_OUT_DPS, info.max_fn_out_dps);
    TEST_ASSERT_EQUAL(SPFG_MAX_GRID_FNS, info.max_grid_fns);
    TEST_ASSERT_EQUAL(SPFG_MAX_GRID_DPS, info.max_grid_dps);
    TEST_ASSERT_EQUAL(SPFG_MAX_PHASES, info.max_phases);
}

TEST(inspect, test_spfg_runtime_size_should_return_proper_size)
{
    uint32_t size;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_runtime_size(NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_runtime_size(&size));
    TEST_ASSERT_EQUAL(sizeof(spfg_runtime_t), size);
}

TEST_GROUP_RUNNER(inspect) {
    RUN_TEST_CASE(inspect, test_spfg_info_should_return_compiled_info);
    RUN_TEST_CASE(inspect, test_spfg_runtime_size_should_return_proper_size);
}
