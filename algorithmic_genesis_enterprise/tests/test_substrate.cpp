#include "test_harness.hpp"
#include "ag/ag_substrate.hpp"
#include "ag/ag_rng.hpp"
#include <cmath>

using namespace ag;

TEST_CASE(substrate_packed_weights_roundtrip) {
    std::vector<float> w{0.0f, 0.5f, 1.0f, 1.5f, 2.0f};
    PackedWeights p = pack_weights_u8x4(w);
    auto u = unpack_weights_u8x4(p);
    CHECK_EQ(u.size(), w.size());
    for (std::size_t i = 0; i < w.size(); ++i) {
        CHECK_TRUE(std::abs(w[i] - u[i]) < 0.01f);
    }
    CHECK_EQ(p.words.size(), static_cast<std::size_t>(2));
}

TEST_CASE(substrate_bytecode_vm_cpu_matches_eval) {
    SplitMix64 rng(123);
    Genome g = make_random_genome(7, 32, rng);
    auto code = compile_genome_to_vm_bytecode(g);
    std::vector<float> xs{-1.0f, -0.25f, 0.0f, 0.5f, 1.0f};
    auto ys = eval_genome_bytecode_cpu(code, xs);
    CHECK_EQ(ys.size(), xs.size());
    for (std::size_t i = 0; i < xs.size(); ++i) {
        EvalResult e = eval_genome(g, xs[i]);
        CHECK_TRUE(e.ok);
        CHECK_TRUE(std::abs(static_cast<double>(ys[i]) - e.value) < 1e-3);
    }
}
