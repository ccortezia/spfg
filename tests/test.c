#include <string.h>
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
    const char ascii[SPFG_BLOCK_NAME_MAX_LENGTH + 1] = "ccccccccccccccccccc";
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_block_name_create(ascii, &name));
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

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 0));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, NULL, NULL));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(1, output);
    TEST_ASSERT_EQUAL(1, emitted);
}


typedef struct test_cb_ctl {
    int counter;
    int stop_at;
} test_loop_ctl_t;

spfg_err_t cycle_callback_stop_ctl(spfg_gr_id_t gr_id, spfg_fn_id_t fn_id, spfg_phase_t phase, void *cdata) {
    test_loop_ctl_t *ctl = (test_loop_ctl_t *)cdata;
    printf("counter: %d\n", ctl->counter);
    if (ctl->stop_at >= 0 && ctl->counter == ctl->stop_at) {
        return SPFG_LOOP_CONTROL_STOP;
    }
    ctl->counter += 1;
    return SPFG_ERROR_NO;
}

TEST(spfg_lifecycle_tests, run_grid_cycle_should_stop_on_callback)
{
    spfg_gr_id_t gr_id;
    spfg_fn_id_t fn0p0_id;
    spfg_fn_id_t fn0p1_id;
    spfg_dp_id_t dp0p0_id;
    spfg_dp_id_t dp1p0_id;
    spfg_dp_id_t dp0p1_id;
    spfg_dp_id_t dp1p1_id;
    spfg_dp_id_t dp0p2_id;
    spfg_boolean_t output;
    spfg_boolean_t emitted;
    test_loop_ctl_t loop_ctl_1 = {0, -1};
    test_loop_ctl_t loop_ctl_2 = {0, 1};

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create(&gr_id, "valid name"));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p0", &dp0p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p1", &dp0p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p0", &dp1p0_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp1p1", &dp1p1_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_create(gr_id, SPFG_DP_BOOL, "dp0p2", &dp0p2_id));

    spfg_dp_id_t in_dps_fn0p0[] = {dp0p0_id, dp1p0_id};
    spfg_dp_id_t out_dps_fn0p0[] = {dp0p1_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p0, 2, out_dps_fn0p0, 1, "fn0p0", &fn0p0_id));

    spfg_dp_id_t in_dps_fn0p1[] = {dp0p1_id, dp1p1_id};
    spfg_dp_id_t out_dps_fn0p1[] = {dp0p2_id};
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_fn_create(gr_id, SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, in_dps_fn0p1, 2, out_dps_fn0p1, 1, "fn0p1", &fn0p1_id));

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, cycle_callback_stop_ctl, &loop_ctl_1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p2_id, &output, &emitted));
    TEST_ASSERT_EQUAL(2, loop_ctl_1.counter);
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_reset_cycle(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp0p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_set_bool(gr_id, dp1p0_id, 1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_run_cycle(gr_id, 0, cycle_callback_stop_ctl, &loop_ctl_2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p1_id, &output, &emitted));
    TEST_ASSERT_EQUAL(1, loop_ctl_2.counter);
    TEST_ASSERT_EQUAL(1, output);
    TEST_ASSERT_EQUAL(1, emitted);
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_dp_get_bool(gr_id, dp0p2_id, &output, &emitted));
    TEST_ASSERT_EQUAL(0, output);
    TEST_ASSERT_EQUAL(0, emitted);
}

// ---

TEST_GROUP(spfg_export_tests);

TEST_SETUP(spfg_export_tests) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(spfg_export_tests) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}


char export_outbuf[sizeof(spfg_grxp_t)];

TEST(spfg_export_tests, run_grid_export_should_not_err)
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

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_export_schema(gr_id, export_outbuf, sizeof(export_outbuf)));
    TEST_ASSERT_EQUAL_STRING("fn1", ((spfg_grxp_t *)export_outbuf)->data.fns[0].name.chars);
    TEST_ASSERT_EQUAL_STRING("", ((spfg_grxp_t *)export_outbuf)->data.fns[1].name.chars);
    TEST_ASSERT_EQUAL_STRING("dp0p0", ((spfg_grxp_t *)export_outbuf)->data.dps[0].name.chars);
    TEST_ASSERT_EQUAL_STRING("dp0p1", ((spfg_grxp_t *)export_outbuf)->data.dps[1].name.chars);
    TEST_ASSERT_EQUAL_STRING("", ((spfg_grxp_t *)export_outbuf)->data.dps[2].name.chars);
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
    RUN_TEST_CASE(spfg_lifecycle_tests, run_grid_cycle_should_stop_on_callback);
    RUN_TEST_CASE(spfg_export_tests, run_grid_export_should_not_err);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST_GROUP(spfg_lifecycle_tests);
    return UNITY_END();
}

