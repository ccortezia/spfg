#include "unity/unity_fixture.h"
#include "spfg/spfg.h"

TEST_GROUP(library);

TEST_SETUP(library) {
}

TEST_TEAR_DOWN(library) {
}

TEST(library, init_should_not_err)
{
    spfg();
    TEST_ASSERT_EQUAL_HEX8(40, 40);
}

TEST(library, finish_should_not_err)
{
    TEST_ASSERT_EQUAL_HEX8(40, 40);
}

TEST_GROUP_RUNNER(library) {
    RUN_TEST_CASE(library, init_should_not_err);
    RUN_TEST_CASE(library, finish_should_not_err);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST_GROUP(library);
    return UNITY_END();
}

