#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(lifecycle);

TEST_SETUP(lifecycle) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_TEAR_DOWN(lifecycle) {
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
}

TEST(lifecycle, init_twice_should_err)
{
    TEST_ASSERT_EQUAL(SPFG_ERROR_ALREADY_INITIALIZED, spfg_init());
}

TEST(lifecycle, finish_twice_should_err)
{
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_finish());
    TEST_ASSERT_EQUAL(SPFG_ERROR_NOT_INITIALIZED, spfg_finish());
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_init());
}

TEST_GROUP_RUNNER(lifecycle) {
    RUN_TEST_CASE(lifecycle, init_twice_should_err);
    RUN_TEST_CASE(lifecycle, finish_twice_should_err);
}
