#pragma once
#include "ag_expr.hpp"
#include "ag_fitness.hpp"
#include "ag_opencl.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ag {

struct PackedWeights {
    std::vector<uint32_t> words;
    std::size_t original_count{0};
};

PackedWeights pack_weights_u8x4(const std::vector<float>& weights);
std::vector<float> unpack_weights_u8x4(const PackedWeights& packed);

std::vector<OpenClVmInstruction> compile_genome_to_vm_bytecode(const Genome& genome);
std::vector<float> eval_genome_bytecode_cpu(const std::vector<OpenClVmInstruction>& bytecode,
                                            const std::vector<float>& xs);

struct VmDifferentialReport {
    std::string backend{"cpu"};
    bool opencl_available{false};
    std::size_t samples{0};
    double max_abs_error{0.0};
    double mean_abs_error{0.0};
    double kernel_ms{0.0};
    double readback_ms{0.0};
    std::string error;
};

VmDifferentialReport differential_test_bytecode_vm(const Genome& genome,
                                                   const FitnessDataset& dataset,
                                                   double tolerance,
                                                   std::size_t local_size);

std::string vm_differential_report_to_json(const VmDifferentialReport& report);

} // namespace ag
