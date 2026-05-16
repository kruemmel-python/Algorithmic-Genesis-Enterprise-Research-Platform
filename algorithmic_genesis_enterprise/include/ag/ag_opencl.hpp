#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ag {

struct OpenClProbe {
    bool available{false};
    std::string platform_name;
    std::string device_name;
    std::string error;
};

struct OpenClProfile {
    double wall_ms{0.0};
    double setup_ms{0.0};
    double kernel_ms{0.0};
    double readback_ms{0.0};
    double teardown_ms{0.0};
    std::size_t local_size{0};
    std::size_t global_neurons{0};
    std::size_t transfer_bytes{0};
    double neuron_steps_per_second{0.0};
    std::size_t kernel_launches{0};
    std::size_t differential_checks{0};
    double max_probe_error{0.0};
};

struct OpenClSnnOptions {
    std::size_t local_size{256};
    bool emit_spikes_by_step{true};
    bool profile{false};
    bool somnia_enabled{false};
    int somnia_period{0};
    int somnia_duration{0};
    float somnia_decay{0.0005f};
    bool quantized_weights{false};
    bool hardware_probe{false};
    int hardware_probe_interval{0};
};

struct OpenClSnnResult {
    bool used_opencl{false};
    std::string backend{"cpu"};
    std::string backend_model{"lif_recurrent_stdp"};
    std::string error;
    int total_spikes{0};
    double mean_voltage{0.0};
    double mean_weight{0.0};
    std::size_t synapse_count{0};
    std::vector<int> spikes_by_step;
    OpenClProfile profile;
    int somnia_cycles{0};
    bool quantized_weights{false};
    double estimated_weight_memory_saving{0.0};
    std::size_t differential_checks{0};
    double max_probe_error{0.0};
};

/// Immutable CSR topology for GPU execution.
/// outgoing_offsets/indices/weights are the canonical synapse storage:
/// edge e belongs to src when outgoing_offsets[src] <= e < outgoing_offsets[src+1].
/// incoming_* mirrors the same edge ids by destination, allowing one GPU work-item
/// per neuron to sum incoming spikes without non-portable float atomics.
struct OpenClSnnTopology {
    std::vector<uint32_t> outgoing_offsets;
    std::vector<uint32_t> outgoing_indices;
    std::vector<float> weights;
    std::vector<uint32_t> incoming_offsets;
    std::vector<uint32_t> incoming_sources;
    std::vector<uint32_t> incoming_edge_ids;
};

OpenClProbe probe_opencl();

/// Generate deterministic floats. When OpenCL is compiled and a device is available,
/// this executes kernels/random.cl equivalent code on the selected OpenCL device.
/// Otherwise it falls back to the CPU SplitMix64 generator.
std::vector<float> random_floats_opencl_or_cpu(uint64_t seed, std::size_t count, OpenClProbe* probe = nullptr);

/// Execute a recurrent LIF/STDP simulation on OpenCL when available.
/// The GPU model uses an enterprise-portable two-phase design:
/// 1. per-neuron LIF update from incoming CSR synapses using previous-step spikes;
/// 2. per-edge STDP update with no floating-point atomics.
/// This introduces a one-step synaptic latency compared with immediate CPU propagation,
/// but preserves topology, weights, leakage, stochastic drive, spike history, and STDP.
OpenClSnnResult snn_recurrent_stdp_opencl_or_cpu(uint64_t seed,
                                                 std::size_t neurons,
                                                 int steps,
                                                 float dt,
                                                 float threshold,
                                                 float leak_rate,
                                                 float reset_voltage,
                                                 float stdp_lr,
                                                 const OpenClSnnTopology& topology,
                                                 OpenClProbe* probe = nullptr,
                                                 OpenClSnnOptions options = {});

struct OpenClVmInstruction {
    uint32_t op{0};
    float value{0.0f};
    uint32_t a{0};
    uint32_t b{0};
};

struct OpenClVmResult {
    bool used_opencl{false};
    std::string backend{"cpu"};
    std::string error;
    std::vector<float> values;
    double kernel_ms{0.0};
    double readback_ms{0.0};
};

OpenClVmResult eval_bytecode_vm_opencl_or_cpu(const std::vector<OpenClVmInstruction>& bytecode,
                                              const std::vector<float>& xs,
                                              OpenClProbe* probe = nullptr,
                                              std::size_t local_size = 256);

} // namespace ag
