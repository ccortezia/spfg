#include <string.h>
#include "unity/unity_fixture.h"
#include "spfg/spfg.h"


TEST_GROUP(build_rt);
TEST_SETUP(build_rt) {}
TEST_TEAR_DOWN(build_rt) {}


TEST(build_rt, test_spfg_rt_init_should_not_err)
{
    spfg_runtime_t runtime;
    TEST_ASSERT_EQUAL(SPFG_ERROR_NO, spfg_rt_init(&runtime, "valid grid name"));
}

TEST_GROUP_RUNNER(build_rt) {
    RUN_TEST_CASE(build_rt, test_spfg_rt_init_should_not_err);
}
