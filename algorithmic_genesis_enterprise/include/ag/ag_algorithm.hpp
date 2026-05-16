#pragma once
#include "ag_ea.hpp"
#include "ag_genesis.hpp"
#include <string>
#include <vector>

namespace ag {

struct AlgorithmSynthesisConfig {
    std::string domain{"chaotic_maps"};
    std::string algorithm_kind{"auto"};
    std::string generator{"snn"};
    std::string fitness_backend{"cpu"};
    std::string archive_path{"genesis_archive.jsonl"};
    std::string export_dir{"algorithm_discoveries"};
    std::string report_path{"algorithm_report.md"};
    std::size_t population{256};
    std::size_t genome_length{48};
    int generations{100};
    int samples{128};
    std::size_t save_top{32};
    uint64_t seed{42};
    bool vm_differential{false};
    bool require_nontrivial{true};
    int gas_limit{10000};
};

struct AlgorithmArtifact {
    std::string name;
    std::string kind;
    std::string domain;
    std::string kernel_expression;
    std::string algorithm_summary;
    std::string mathematical_contract;
    std::string pseudocode;
    std::string complexity;
    std::string validation_protocol;
    double nontriviality{0.0};
    double algorithm_score{0.0};
    Genome kernel_genome;
    std::vector<std::string> export_files;
};

struct AlgorithmSynthesisReport {
    AlgorithmArtifact artifact;
    EvolutionReport evolution;
    bool accepted{false};
    std::vector<std::string> warnings;
};

std::string choose_algorithm_kind(const std::string& domain, const std::string& requested);
double nontriviality_score(const Genome& genome);
AlgorithmSynthesisReport synthesize_algorithm(const AlgorithmSynthesisConfig& cfg);
std::string algorithm_artifact_to_json(const AlgorithmArtifact& artifact);
std::string algorithm_synthesis_report_to_json(const AlgorithmSynthesisReport& report);
std::string algorithm_discovery_report_markdown(const AlgorithmSynthesisReport& report);

} // namespace ag
