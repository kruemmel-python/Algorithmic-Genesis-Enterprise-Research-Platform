#pragma once
#include "ag_algorithm.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace ag {

struct FormulaPart {
    std::string id;
    std::string label;
    std::string category;
    std::string role;
    std::string description;
    double default_weight{1.0};
    bool safety_critical{false};
};

struct GuidedExperimentManifest {
    std::string name{"guided_experiment"};
    std::string domain{"root_finding"};
    std::string profile{"balanced"};
    std::string generator{"snn"};
    std::string fitness_backend{"opencl-vm"};
    std::string archive_path{"genesis_archive.jsonl"};
    std::string export_dir{"guided_algorithms"};
    std::string report_path{"guided_report.md"};
    std::string result_json{"guided_result.json"};
    std::vector<std::string> selected_parts;
    std::size_t population{256};
    int generations{250};
    std::size_t genome_length{48};
    int samples{128};
    std::size_t save_top{32};
    std::uint64_t seed{42};
    bool vm_differential{true};
    bool require_nontrivial{true};
    int gas_limit{10000};
};

std::vector<FormulaPart> formula_part_catalog();
std::string formula_part_catalog_json();
std::string formula_part_ids_json_array(const std::vector<std::string>& ids);
GuidedExperimentManifest read_guided_experiment_manifest(const std::string& path);
void validate_guided_experiment(const GuidedExperimentManifest& manifest);
AlgorithmSynthesisConfig guided_manifest_to_synthesis_config(const GuidedExperimentManifest& manifest);
std::string guided_experiment_to_json(const GuidedExperimentManifest& manifest);
std::string guided_synthesis_result_to_json(const GuidedExperimentManifest& manifest,
                                            const AlgorithmSynthesisReport& report);
std::string guided_report_markdown(const GuidedExperimentManifest& manifest,
                                   const AlgorithmSynthesisReport& report);
std::uint64_t guided_seed(std::uint64_t base_seed, const std::vector<std::string>& parts,
                          const std::string& domain, const std::string& profile);

} // namespace ag
