#include "test_harness.hpp"
#include "ag/ag_genesis.hpp"
#include "ag/ag_rng.hpp"

TEST_CASE(genesis_fingerprints_and_snn_generator) {
    ag::SplitMix64 rng(7);
    ag::Genome g = ag::make_random_genome(1, 16, rng);
    ag::FitnessConfig cfg;
    ag::FingerprintSet fp = ag::compute_fingerprints(g, cfg);
    CHECK_TRUE(!fp.ast.empty());
    CHECK_TRUE(!fp.behavior.empty());
    ag::attach_fingerprints(g, fp);
    CHECK_TRUE(g.ast_fingerprint == fp.ast);
    auto pop = ag::generate_population_from_snn(8, 16, 42, 0);
    CHECK_EQ(pop.size(), static_cast<std::size_t>(8));
    CHECK_TRUE(ag::validate_genome(pop.front()));
}
