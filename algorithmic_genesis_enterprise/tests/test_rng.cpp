#include "test_harness.hpp"
#include "ag/ag_rng.hpp"

TEST_CASE(rng_reproducible) {
    ag::SplitMix64 a(123), b(123);
    for (int i = 0; i < 100; ++i) CHECK_EQ(a.next_u64(), b.next_u64());
}

TEST_CASE(rng_uniform_range) {
    ag::SplitMix64 r(1);
    for (int i = 0; i < 1000; ++i) {
        double x = r.uniform01();
        CHECK_TRUE(x >= 0.0 && x < 1.0);
    }
}
