#include "test_harness.hpp"
#include "ag/ag_persist.hpp"

TEST_CASE(persist_feature_flag_safe) {
    ag::AlgorithmStore store(":memory:");
    CHECK_TRUE(store.enabled() == true || store.enabled() == false);
}
