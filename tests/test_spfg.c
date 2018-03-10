#include "unity/unity_fixture.h"

static void run_all_tests(void)
{
    RUN_TEST_GROUP(build_rt);
    RUN_TEST_GROUP(build_fn);
    RUN_TEST_GROUP(build_dp);
    RUN_TEST_GROUP(eval_built);
    RUN_TEST_GROUP(eval_json);
    RUN_TEST_GROUP(inspect);
    RUN_TEST_GROUP(io_bin);
    RUN_TEST_GROUP(io_json);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, run_all_tests);
}
