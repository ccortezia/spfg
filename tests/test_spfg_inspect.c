#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"
#include "spfg_types.h"
#include "spfg_utils.h"


TEST_GROUP(inspect);
TEST_SETUP(inspect) {}
TEST_TEAR_DOWN(inspect) {}


TEST(inspect, test_spfg_print_runtime_sizes)
{
    printf("Type sizes (compiled x estimates)\n");
    printf("sizeof(spfg_block_name_t):  %lu / %lu\n",
           (unsigned long)sizeof(spfg_block_name_t),
           (unsigned long)ESTIMATED_SIZE_BLOCK_NAME);
    printf("sizeof(spfg_dp_value_t):    %lu / %lu\n",
           (unsigned long)sizeof(spfg_dp_value_t),
           (unsigned long) ESTIMATED_SIZE_DP_VALUE);
    printf("sizeof(spfg_dp_t):          %lu / %lu\n",
           (unsigned long)sizeof(spfg_dp_t),
           (unsigned long)ESTIMATED_SIZE_DP);
    printf("sizeof(spfg_fn_t):          %lu / %lu\n",
           (unsigned long)sizeof(spfg_fn_t),
           (unsigned long)ESTIMATED_SIZE_FN);
    printf("sizeof(spfg_gr_ctl_t):      %lu / %lu\n",
           (unsigned long)sizeof(spfg_gr_ctl_t),
           (unsigned long)ESTIMATED_SIZE_GR_CTL);
    printf("sizeof(spfg_gr_t):          %lu / %lu\n",
           (unsigned long)sizeof(spfg_gr_t),
           (unsigned long)ESTIMATED_SIZE_GR);
    printf("sizeof(spfg_fnx_t):         %lu / %lu\n",
           (unsigned long)sizeof(spfg_fnx_t),
           (unsigned long)ESTIMATED_SIZE_FNX);
    printf("sizeof(spfg_grx_t):         %lu / %lu\n",
           (unsigned long)sizeof(spfg_grx_t),
           (unsigned long)ESTIMATED_SIZE_GRX);
    printf("sizeof(spfg_rt_t):          %lu / %lu\n",
           (unsigned long)sizeof(spfg_rt_t),
           (unsigned long)sizeof(spfg_runtime_t));
    TEST_ASSERT_TRUE(sizeof(spfg_block_name_t) <= ESTIMATED_SIZE_BLOCK_NAME);
    TEST_ASSERT_TRUE(sizeof(spfg_dp_value_t) <=  ESTIMATED_SIZE_DP_VALUE);
    TEST_ASSERT_TRUE(sizeof(spfg_dp_t) <= ESTIMATED_SIZE_DP);
    TEST_ASSERT_TRUE(sizeof(spfg_fn_t) <= ESTIMATED_SIZE_FN);
    TEST_ASSERT_TRUE(sizeof(spfg_gr_ctl_t) <= ESTIMATED_SIZE_GR_CTL);
    TEST_ASSERT_TRUE(sizeof(spfg_gr_t) <= ESTIMATED_SIZE_GR);
    TEST_ASSERT_TRUE(sizeof(spfg_fnx_t) <= ESTIMATED_SIZE_FNX);
    TEST_ASSERT_TRUE(sizeof(spfg_grx_t) <= ESTIMATED_SIZE_GRX);
    TEST_ASSERT_TRUE(sizeof(spfg_rt_t) <= sizeof(spfg_runtime_t));
}


TEST(inspect, test_spfg_info_should_return_compiled_info)
{
    spfg_info_t info;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_info(NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_info(&info));
    TEST_ASSERT_EQUAL(VERSION_MAJOR, info.version_major);
    TEST_ASSERT_EQUAL(VERSION_MINOR, info.version_minor);
    TEST_ASSERT_EQUAL(VERSION_PATCH, info.version_patch);
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
    RUN_TEST_CASE(inspect, test_spfg_print_runtime_sizes);
}
