#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(spfg_lifecycle_tests);

TEST_SETUP(spfg_lifecycle_tests) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(spfg_lifecycle_tests) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

// ---

TEST(spfg_lifecycle_tests, init_twice_should_err)
{
    TEST_ASSERT_EQUAL(SPFG_ERROR_ALREADY_INITIALIZED, spfg_init());
}

TEST(spfg_lifecycle_tests, finish_twice_should_err)
{
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
    TEST_ASSERT_EQUAL(SPFG_ERROR_NOT_INITIALIZED, spfg_finish());
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

// ---

TEST(spfg_lifecycle_tests, create_block_name_should_set_result)
{
    spfg_block_name_t name;
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_block_name_create("block name", &name));
    TEST_ASSERT_EQUAL_STRING("block name", name.chars);
}

TEST(spfg_lifecycle_tests, create_block_name_with_null_param_should_err)
{
    spfg_block_name_t name;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_block_name_create(NULL, &name));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_block_name_create("block name", NULL));
}

TEST(spfg_lifecycle_tests, create_block_name_should_not_overflow)
{
    spfg_block_name_t name;
    const char ascii[SPFG_BLOCK_NAME_MAX_LENGTH + 1] = "ccccccccccccccccccccJ";
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_block_name_create(ascii, &name));
    TEST_ASSERT_EQUAL(SPFG_BLOCK_NAME_MAX_LENGTH - 1, name.len);
    TEST_ASSERT_EQUAL_STRING("ccccccccccccccccccc", name.chars);
}

// ---

TEST(spfg_lifecycle_tests, create_grid_should_not_err)
{
    spfg_gr_id_t gr_id;
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid grid name"));
}

TEST(spfg_lifecycle_tests, create_grid_with_null_param_should_err)
{
    spfg_gr_id_t gr_id;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_create(NULL, ""));
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_PARAM_NULL_POINTER, spfg_gr_create(&gr_id, NULL));
}

TEST(spfg_lifecycle_tests, create_grid_with_empty_name_should_err)
{
    spfg_gr_id_t gr_id;
    TEST_ASSERT_EQUAL(SPFG_ERROR_BAD_BLOCK_NAME, spfg_gr_create(&gr_id, ""));
}

TEST(spfg_lifecycle_tests, create_grid_beyond_max_should_err)
{
    spfg_gr_id_t gr_id;

    for (int i = 0 ; i < SPFG_GR_TOTAL; i++) {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    }
    TEST_ASSERT_EQUAL(SPFG_ERROR_OUT_OF_SLOTS, spfg_gr_create(&gr_id, "valid name"));
}

// ---

TEST(spfg_lifecycle_tests, setup_grid_should_not_err)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp0p1_id;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id};
    spfg_dp_id_t out_dps[] = {dp0p0_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 1, in_dps, 1, out_dps, 1, "fn1", &fn_id));
}

TEST(spfg_lifecycle_tests, run_grid_cycle_should_not_err)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));

    spfg_dp_id_t in_dps[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps, 2, out_dps, 1, "fn1", &fn_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(1, output);
    TEST_ASSERT_EQUAL(1, emitted);
}

// ---

TEST_GROUP_RUNNER(spfg_lifecycle_tests) {
    RUN_TEST_CASE(spfg_lifecycle_tests, init_twice_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, finish_twice_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_grid_should_not_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_grid_with_empty_name_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_grid_with_null_param_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_grid_beyond_max_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_block_name_should_set_result);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_block_name_with_null_param_should_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, create_block_name_should_not_overflow);
    RUN_TEST_CASE(spfg_lifecycle_tests, setup_grid_should_not_err);
    RUN_TEST_CASE(spfg_lifecycle_tests, run_grid_cycle_should_not_err);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST_GROUP(spfg_lifecycle_tests);
    return UNITY_END();
}

