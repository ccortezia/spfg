#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

#include "spfg_types.h"

TEST_GROUP(build_gr);

TEST_SETUP(build_gr) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(build_gr) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(build_gr, test_spfg_rt_init_should_not_err)
{
    spfg_runtime_t runtime;
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "valid grid name"));
}

TEST(build_gr, test_spfg_gr_create_beyond_max_should_err)
{
    spfg_gr_id_t gr_id;

    for (int i = 0 ; i < SPFG_MAX_GRID_CNT; i++) {
        TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("valid name", &gr_id));
    }
    TEST_ASSERT_EQUAL(SPFG_ERROR_OUT_OF_SLOTS, spfg_gr_create("valid name", &gr_id));
}

TEST(build_gr, test_spfg_gr_remove_should_free_inner_slot)
{
    spfg_gr_id_t gr_id;
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("valid grid name", &gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_INVALID_GR_ID, spfg_gr_remove(-1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_remove(gr_id));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("another grid name", &gr_id));
}

TEST(build_gr, test_spfg_gr_get_ids_should_reflect_inner_state)
{
    spfg_gr_id_t gr_id_1;
    spfg_gr_id_t gr_id_2;
    spfg_gr_cnt_t count = 0;
    spfg_gr_id_t out_ids[100];

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("gr1", &gr_id_1));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_get_ids(out_ids, 100, &count));
    TEST_ASSERT_EQUAL(1, count);
    TEST_ASSERT_EQUAL(out_ids[0], gr_id_1);

    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_create("gr2", &gr_id_2));
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_gr_get_ids(out_ids, 100, &count));
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL(gr_id_1, out_ids[0]);
    TEST_ASSERT_EQUAL(gr_id_2, out_ids[1]);
}

TEST_GROUP_RUNNER(build_gr) {
    RUN_TEST_CASE(build_gr, test_spfg_rt_init_should_not_err);
    RUN_TEST_CASE(build_gr, test_spfg_gr_create_beyond_max_should_err);
    RUN_TEST_CASE(build_gr, test_spfg_gr_remove_should_free_inner_slot);
    RUN_TEST_CASE(build_gr, test_spfg_gr_get_ids_should_reflect_inner_state);
}
