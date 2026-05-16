#pragma once
#include "ag_expr.hpp"
#include "ag_fitness.hpp"
#include "ag_rng.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ag {

struct FingerprintSet {
    std::string ast;
    std::string behavior;
    std::string derivative;
    std::string stability;
    std::string complexity;
};

struct ArchiveEntry {
    Genome genome;
    std::string expression;
    FingerprintSet fingerprints;
    std::string domain;
    std::string backend;
    std::string seed;
    std::string created_at;
};

struct ArchiveStats {
    std::size_t entries{0};
    std::size_t unique_ast{0};
    std::size_t unique_behavior{0};
};

FingerprintSet compute_fingerprints(const Genome& genome, const FitnessConfig& cfg);
void attach_fingerprints(Genome& genome, const FingerprintSet& fp);
double novelty_against_archive(const FingerprintSet& fp, const std::vector<FingerprintSet>& archive);

class DiscoveryArchive final {
public:
    explicit DiscoveryArchive(std::string path);
    bool load();
    bool append(const ArchiveEntry& entry);
    std::vector<FingerprintSet> fingerprints() const;
    ArchiveStats stats() const;
    const std::vector<ArchiveEntry>& entries() const { return entries_; }
private:
    std::string path_;
    std::vector<ArchiveEntry> entries_;
};

struct CodeExportConfig {
    std::string output_dir{"exports"};
    std::string module_name{"ag_candidate"};
    std::string domain{"default"};
};

bool export_candidate_code(const Genome& genome, const CodeExportConfig& cfg, std::string* error = nullptr);
std::string discovery_report_markdown(const Genome& best,
                                      const std::vector<Genome>& population,
                                      const ArchiveStats& before,
                                      const ArchiveStats& after,
                                      const FitnessConfig& cfg,
                                      const std::string& backend,
                                      const std::string& generator);

std::vector<Genome> generate_population_from_snn(std::size_t population,
                                                 std::size_t genome_length,
                                                 uint64_t seed,
                                                 int birth_generation,
                                                 std::size_t virtual_neurons = 256,
                                                 int virtual_steps = 64);

} // namespace ag
