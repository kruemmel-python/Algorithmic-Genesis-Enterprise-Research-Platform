#include "test_harness.hpp"
#include "ag/ag_ea.hpp"

TEST_CASE(ea_runs) {
    ag::EvolutionConfig ecfg;
    ecfg.population = 16;
    ecfg.genome_length = 12;
    ecfg.generations = 3;
    ecfg.seed = 9;
    ag::FitnessConfig fcfg;
    fcfg.samples = 24;
    ag::EvolutionEngine engine(ecfg, fcfg);
    auto report = engine.run();
    CHECK_TRUE(report.best.fitness > -1.0);
    CHECK_EQ(static_cast<int>(report.best_fitness_by_generation.size()), 3);
}
