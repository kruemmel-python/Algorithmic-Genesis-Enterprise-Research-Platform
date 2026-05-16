#pragma once
#include "ag_expr.hpp"
#include "ag_fitness.hpp"
#include "ag_genesis.hpp"
#include "ag_rng.hpp"
#include "ag_substrate.hpp"
#include <string>
#include <vector>

namespace ag {

struct EvolutionConfig {
    std::size_t population{64};
    std::size_t genome_length{32};
    int generations{50};
    double mutation_rate{0.08};
    int elite_count{4};
    uint64_t seed{42};
    std::string generator{"random"};
    std::string archive_path;
    std::string export_dir;
    std::string report_path;
    std::size_t save_top{16};
    std::string fitness_backend{"cpu"};
    bool vm_differential{false};
    std::size_t vm_local_size{256};
};

struct EvolutionReport {
    Genome best;
    std::vector<double> best_fitness_by_generation;
    std::vector<double> mean_fitness_by_generation;
    std::vector<Genome> final_population;
    std::string backend{"cpu"};
    std::string generator{"random"};
    ArchiveStats archive_before;
    ArchiveStats archive_after;
    VmDifferentialReport vm_report;
};

class EvolutionEngine final {
public:
    EvolutionEngine(EvolutionConfig config, FitnessConfig fitness_config);
    EvolutionReport run();

private:
    EvolutionConfig config_;
    FitnessConfig fitness_config_;
    SplitMix64 rng_;
    FitnessDataset dataset_;
    std::vector<std::string> archive_;
    std::vector<FingerprintSet> discovery_fingerprints_;
    Genome tournament(const std::vector<Genome>& population);
};

} // namespace ag
