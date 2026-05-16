#pragma once
#include "ag_rng.hpp"
#include "ag_opencl.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace ag {

struct SnnConfig {
    std::size_t neurons{128};
    std::size_t connections_per_neuron{8};
    int steps{100};
    float dt{1.0f};
    float threshold{1.0f};
    float leak_rate{0.04f};
    float reset_voltage{0.0f};
    float stdp_lr{0.006f};
    uint64_t seed{42};

    /// "cpu" keeps the recurrent STDP implementation.
    /// "opencl" forces the GPU LIF kernel and throws only via JSON fallback errors.
    /// "auto" uses OpenCL when available and falls back to CPU otherwise.
    std::string backend{"cpu"};
    std::string semantic{"immediate"};
    bool emit_spikes_by_step{true};
    bool profile{false};
    std::size_t opencl_local_size{256};

    // Somnia Shadow-Phase: periodic low-drive consolidation with weight decay.
    bool somnia_enabled{false};
    int somnia_period{0};
    int somnia_duration{0};
    float somnia_decay{0.0005f};

    // CCQ-inspired quantization. Runtime keeps public metrics; GPU path may
    // quantize in-kernel, CPU path uses equivalent 8-bit codebook projection.
    bool quantized_weights{false};

    // Mycelial growth field with zero-sum arena. The edge count never exceeds
    // max_synapses; new growth replaces weakest synapses when arena is full.
    bool mycelial_growth{false};
    std::size_t max_synapses{0};
    double growth_rate{0.0};

    // Continuous Differential Hardware Probing.
    bool hardware_probe{false};
    int hardware_probe_interval{0};
};

struct SnnState {
    std::vector<float> voltage;
    std::vector<float> threshold;
    std::vector<float> leak_rate;
    std::vector<float> last_spike_time;
    std::vector<uint32_t> conn_offsets;
    std::vector<uint32_t> conn_indices;
    std::vector<float> weights;
    std::vector<uint8_t> spikes;
};

struct SnnReport {
    int total_spikes{0};
    double mean_voltage{0.0};
    double mean_weight{0.0};
    std::size_t synapse_count{0};
    std::string backend_model{"lif_recurrent_stdp"};
    std::vector<int> spikes_by_step;
    std::string backend{"cpu"};
    OpenClProbe opencl;
    std::string backend_error;
    OpenClProfile profile;
    int somnia_cycles{0};
    bool quantized_weights{false};
    double estimated_weight_memory_saving{0.0};
    std::size_t growth_events{0};
    std::size_t cannibalized_synapses{0};
    std::size_t arena_capacity{0};
    std::size_t differential_checks{0};
    double max_probe_error{0.0};
};

class SnnNetwork final {
public:
    explicit SnnNetwork(SnnConfig config);
    SnnReport run();
    const SnnState& state() const { return state_; }
    OpenClSnnTopology make_opencl_topology() const;

private:
    SnnConfig config_;
    SplitMix64 rng_;
    SnnState state_;
    void initialize();
    int step(int t);
    int step_latency(int t, std::vector<uint8_t>& prev_spikes);
    void apply_somnia(int t);
    void apply_quantization();
    void maybe_grow_mycelial(int t);
    void cannibalize_weakest_edge(uint32_t src, uint32_t dst, float weight);
    double control_group_error(std::size_t count) const;
    SnnReport run_cpu();
};

} // namespace ag
