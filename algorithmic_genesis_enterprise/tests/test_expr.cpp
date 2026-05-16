#include "test_harness.hpp"
#include "ag/ag_expr.hpp"
#include "ag/ag_rng.hpp"

TEST_CASE(expr_eval_simple) {
    ag::Genome g;
    g.id = 1;
    g.name = "simple";
    g.genes = {
        {ag::Op::VarX, 0.0f, 0, 0},
        {ag::Op::Const, 2.0f, 0, 0},
        {ag::Op::Add, 0.0f, 0, 1}
    };
    auto r = ag::eval_genome(g, 3.0);
    CHECK_TRUE(r.ok);
    CHECK_NEAR(r.value, 5.0, 1e-9);
}

TEST_CASE(expr_random_validates) {
    ag::SplitMix64 rng(42);
    auto g = ag::make_random_genome(1, 16, rng);
    CHECK_TRUE(ag::validate_genome(g));
}
