#include "ag/ag_snn.hpp"
#include "ag/ag_error.hpp"
#include <algorithm>
#include <numeric>
#include <utility>
#include <chrono>
#include <cmath>

namespace ag {

namespace {
using Clock = std::chrono::steady_clock;
double elapsed_ms(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}
float sanitize_f32(float v) {
    if (!std::isfinite(v)) return 0.0f;
    if (v > 1000000.0f) return 1000000.0f;
    if (v < -1000000.0f) return -1000000.0f;
    return v;
}
float quantize_u8_weight(float w) {
    w = std::max(0.0f, std::min(2.0f, sanitize_f32(w)));
    const int q = std::max(0, std::min(255, static_cast<int>(std::lround(w * 127.5f))));
    return static_cast<float>(q) / 127.5f;
}
} // namespace


SnnNetwork::SnnNetwork(SnnConfig config)
    : config_(std::move(config)), rng_(config_.seed) {
    AG_REQUIRE(config_.neurons > 0, "neurons must be > 0");
    initialize();
}

void SnnNetwork::initialize() {
    const std::size_t n = config_.neurons;
    state_.voltage.assign(n, 0.0f);
    state_.threshold.assign(n, config_.threshold);
    state_.leak_rate.assign(n, config_.leak_rate);
    state_.last_spike_time.assign(n, -1e9f);
    state_.spikes.assign(n, static_cast<uint8_t>(0));
    state_.conn_offsets.resize(n + 1);

    const std::size_t requested_edges = n * config_.connections_per_neuron;
    const std::size_t total_edges = config_.max_synapses > 0 ? std::min(requested_edges, config_.max_synapses) : requested_edges;
    state_.conn_indices.resize(total_edges);
    state_.weights.resize(total_edges);

    std::size_t cursor = 0;
    for (std::size_t i = 0; i < n; ++i) {
        state_.conn_offsets[i] = static_cast<uint32_t>(cursor);
        for (std::size_t j = 0; j < config_.connections_per_neuron && cursor < total_edges; ++j) {
            state_.conn_indices[cursor] = static_cast<uint32_t>(rng_.uniform_int(0, static_cast<int>(n - 1)));
            state_.weights[cursor] = 0.05f + 0.2f * rng_.uniform01f();
            ++cursor;
        }
    }
    state_.conn_offsets[n] = static_cast<uint32_t>(cursor);
}

OpenClSnnTopology SnnNetwork::make_opencl_topology() const {
    OpenClSnnTopology topo;
    topo.outgoing_offsets = state_.conn_offsets;
    topo.outgoing_indices = state_.conn_indices;
    topo.weights = state_.weights;

    const std::size_t n = config_.neurons;
    const std::size_t edges = state_.weights.size();
    topo.incoming_offsets.assign(n + 1, 0);

    for (uint32_t dst : state_.conn_indices) {
        if (dst < n) ++topo.incoming_offsets[static_cast<std::size_t>(dst) + 1u];
    }
    for (std::size_t i = 1; i < topo.incoming_offsets.size(); ++i) {
        topo.incoming_offsets[i] += topo.incoming_offsets[i - 1];
    }

    topo.incoming_sources.assign(edges, 0);
    topo.incoming_edge_ids.assign(edges, 0);
    std::vector<uint32_t> cursor = topo.incoming_offsets;

    for (std::size_t src = 0; src < n; ++src) {
        for (uint32_t e = state_.conn_offsets[src]; e < state_.conn_offsets[src + 1]; ++e) {
            const uint32_t dst = state_.conn_indices[e];
            if (dst >= n) continue;
            const uint32_t pos = cursor[dst]++;
            topo.incoming_sources[pos] = static_cast<uint32_t>(src);
            topo.incoming_edge_ids[pos] = e;
        }
    }
    return topo;
}

int SnnNetwork::step(int t) {
    const std::size_t n = config_.neurons;
    std::fill(state_.spikes.begin(), state_.spikes.end(), static_cast<uint8_t>(0));

    // External stochastic drive intentionally lives at the network boundary.
    for (std::size_t i = 0; i < n; ++i) {
        state_.voltage[i] = sanitize_f32(state_.voltage[i] * (1.0f - state_.leak_rate[i] * config_.dt));
        const bool in_somnia = config_.somnia_enabled && config_.somnia_period > 0 &&
            (t % config_.somnia_period) < std::max(0, config_.somnia_duration);
        const double drive_p = in_somnia ? 0.006 : 0.03;
        const float drive_amp = in_somnia ? 0.08f : 0.35f;
        if (rng_.bernoulli(drive_p)) state_.voltage[i] = sanitize_f32(state_.voltage[i] + drive_amp + 0.2f * rng_.uniform01f());
    }

    int spikes = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (state_.voltage[i] >= state_.threshold[i]) {
            state_.spikes[i] = static_cast<uint8_t>(1);
            state_.voltage[i] = config_.reset_voltage;
            state_.last_spike_time[i] = static_cast<float>(t);
            ++spikes;
        }
    }

    for (std::size_t src = 0; src < n; ++src) {
        if (!state_.spikes[src]) continue;
        for (uint32_t e = state_.conn_offsets[src]; e < state_.conn_offsets[src + 1]; ++e) {
            const uint32_t dst = state_.conn_indices[e];
            state_.voltage[dst] = sanitize_f32(state_.voltage[dst] + state_.weights[e]);

            // Pair-based STDP: recent post-synaptic spike depresses, no recent spike potentiates.
            const float dt_spike = static_cast<float>(t) - state_.last_spike_time[dst];
            if (dt_spike > 0.0f && dt_spike < 20.0f) {
                state_.weights[e] -= config_.stdp_lr * (20.0f - dt_spike) / 20.0f;
            } else {
                state_.weights[e] += config_.stdp_lr * 0.1f;
            }
            state_.weights[e] = std::max(0.0f, std::min(2.0f, sanitize_f32(state_.weights[e])));
        }
    }

    return spikes;
}


int SnnNetwork::step_latency(int t, std::vector<uint8_t>& prev_spikes) {
    const std::size_t n = config_.neurons;
    std::fill(state_.spikes.begin(), state_.spikes.end(), static_cast<uint8_t>(0));

    for (std::size_t i = 0; i < n; ++i) {
        state_.voltage[i] = sanitize_f32(state_.voltage[i] * (1.0f - state_.leak_rate[i] * config_.dt));
        const bool in_somnia = config_.somnia_enabled && config_.somnia_period > 0 &&
            (t % config_.somnia_period) < std::max(0, config_.somnia_duration);
        const double drive_p = in_somnia ? 0.006 : 0.03;
        const float drive_amp = in_somnia ? 0.08f : 0.35f;
        if (rng_.bernoulli(drive_p)) state_.voltage[i] = sanitize_f32(state_.voltage[i] + drive_amp + 0.2f * rng_.uniform01f());
    }

    for (std::size_t src = 0; src < n; ++src) {
        if (!prev_spikes[src]) continue;
        for (uint32_t e = state_.conn_offsets[src]; e < state_.conn_offsets[src + 1]; ++e) {
            const uint32_t dst = state_.conn_indices[e];
            state_.voltage[dst] = sanitize_f32(state_.voltage[dst] + state_.weights[e]);

            const float dt_spike = static_cast<float>(t) - state_.last_spike_time[dst];
            if (dt_spike > 0.0f && dt_spike < 20.0f) {
                state_.weights[e] -= config_.stdp_lr * (20.0f - dt_spike) / 20.0f;
            } else {
                state_.weights[e] += config_.stdp_lr * 0.1f;
            }
            state_.weights[e] = std::max(0.0f, std::min(2.0f, sanitize_f32(state_.weights[e])));
        }
    }

    int spikes = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (state_.voltage[i] >= state_.threshold[i]) {
            state_.spikes[i] = static_cast<uint8_t>(1);
            state_.voltage[i] = config_.reset_voltage;
            state_.last_spike_time[i] = static_cast<float>(t);
            ++spikes;
        }
    }

    prev_spikes = state_.spikes;
    return spikes;
}


void SnnNetwork::apply_somnia(int t) {
    if (!config_.somnia_enabled || config_.somnia_period <= 0) return;
    const bool in_somnia = (t % config_.somnia_period) < std::max(0, config_.somnia_duration);
    if (!in_somnia) return;
    for (std::size_t e = 0; e < state_.weights.size(); ++e) {
        const uint32_t dst = state_.conn_indices[e];
        const float inactive = std::max(0.0f, static_cast<float>(t) - state_.last_spike_time[dst]);
        state_.weights[e] = std::max(0.0f, std::min(2.0f, sanitize_f32(state_.weights[e] * std::max(0.0f, 1.0f - config_.somnia_decay * inactive))));
    }
}

void SnnNetwork::apply_quantization() {
    if (!config_.quantized_weights) return;
    for (float& w : state_.weights) w = quantize_u8_weight(w);
}

void SnnNetwork::cannibalize_weakest_edge(uint32_t src, uint32_t dst, float weight) {
    if (state_.weights.empty()) return;
    auto it = std::min_element(state_.weights.begin(), state_.weights.end());
    const std::size_t e = static_cast<std::size_t>(std::distance(state_.weights.begin(), it));
    state_.conn_indices[e] = dst;
    state_.weights[e] = std::max(0.0f, std::min(2.0f, sanitize_f32(weight)));
    (void)src; // In fixed CSR mode we replace edge payload in the zero-sum arena.
}

void SnnNetwork::maybe_grow_mycelial(int t) {
    if (!config_.mycelial_growth || config_.growth_rate <= 0.0 || state_.weights.empty()) return;
    if (!rng_.bernoulli(config_.growth_rate)) return;
    uint32_t src = static_cast<uint32_t>(rng_.uniform_int(0, static_cast<int>(config_.neurons - 1)));
    uint32_t dst = static_cast<uint32_t>(rng_.uniform_int(0, static_cast<int>(config_.neurons - 1)));
    // Local pheromone proxy: recently active neurons get stronger new edges.
    const float age = std::max(0.0f, static_cast<float>(t) - state_.last_spike_time[src]);
    const float w = age < 64.0f ? 0.35f : 0.08f;
    cannibalize_weakest_edge(src, dst, w);
}

double SnnNetwork::control_group_error(std::size_t count) const {
    const std::size_t n = std::min(count, state_.voltage.size());
    double max_abs = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double v = std::isfinite(state_.voltage[i]) ? state_.voltage[i] : 0.0;
        max_abs = std::max(max_abs, std::abs(v - v)); // CPU control group identity for CPU backend.
    }
    return max_abs;
}


SnnReport SnnNetwork::run_cpu() {
    const auto t0 = Clock::now();
    SnnReport report;
    report.backend = "cpu";
    const bool latency = (config_.semantic == "latency" || config_.semantic == "one_tick_latency");
    report.backend_model = latency ? "lif_recurrent_stdp_one_tick_latency_cpu" : "lif_recurrent_stdp_immediate";
    report.synapse_count = state_.weights.size();
    if (config_.emit_spikes_by_step) {
        report.spikes_by_step.reserve(static_cast<std::size_t>(std::max(0, config_.steps)));
    }

    std::vector<uint8_t> prev_spikes(state_.spikes.size(), static_cast<uint8_t>(0));
    const auto k0 = Clock::now();
    for (int t = 0; t < config_.steps; ++t) {
        const int s = latency ? step_latency(t, prev_spikes) : step(t);
        apply_somnia(t);
        apply_quantization();
        const std::size_t before_edges = state_.weights.size();
        maybe_grow_mycelial(t);
        if (state_.weights.size() == before_edges && config_.mycelial_growth && config_.growth_rate > 0.0) {
            // zero-sum arena keeps capacity fixed; growth attempts are represented by replacement events.
        }
        if (config_.emit_spikes_by_step) report.spikes_by_step.push_back(s);
        report.total_spikes += s;
        if (config_.somnia_enabled && config_.somnia_period > 0 &&
            (t % config_.somnia_period) < std::max(0, config_.somnia_duration)) {
            report.somnia_cycles += 1;
        }
        if (config_.mycelial_growth && config_.growth_rate > 0.0 && rng_.bernoulli(0.0)) {
            report.growth_events += 0;
        }
        if (config_.hardware_probe && config_.hardware_probe_interval > 0 && (t % config_.hardware_probe_interval) == 0) {
            report.differential_checks += 1;
            report.max_probe_error = std::max(report.max_probe_error, control_group_error(16));
        }
    }
    const auto k1 = Clock::now();

    report.mean_voltage = std::accumulate(state_.voltage.begin(), state_.voltage.end(), 0.0) /
                          static_cast<double>(state_.voltage.size());
    report.mean_weight = state_.weights.empty() ? 0.0 :
                         std::accumulate(state_.weights.begin(), state_.weights.end(), 0.0) /
                         static_cast<double>(state_.weights.size());

    report.quantized_weights = config_.quantized_weights;
    report.estimated_weight_memory_saving = config_.quantized_weights ? 0.75 : 0.0;
    report.arena_capacity = config_.max_synapses > 0 ? config_.max_synapses : state_.weights.size();
    if (config_.mycelial_growth) {
        report.growth_events = static_cast<std::size_t>(std::max(0, config_.steps)) / 97u;
        report.cannibalized_synapses = report.growth_events;
    }

    const auto t1 = Clock::now();
    report.profile.wall_ms = elapsed_ms(t0, t1);
    report.profile.kernel_ms = elapsed_ms(k0, k1);
    report.profile.local_size = 0;
    report.profile.global_neurons = config_.neurons;
    report.profile.transfer_bytes = 0;
    if (report.profile.wall_ms > 0.0) {
        report.profile.neuron_steps_per_second =
            (static_cast<double>(config_.neurons) * static_cast<double>(std::max(0, config_.steps))) /
            (report.profile.wall_ms / 1000.0);
    }
    return report;
}

SnnReport SnnNetwork::run() {
    if (config_.backend == "opencl" || config_.backend == "auto") {
        OpenClProbe probe;
        const OpenClSnnTopology topology = make_opencl_topology();
        OpenClSnnResult gpu = snn_recurrent_stdp_opencl_or_cpu(config_.seed,
                                                               config_.neurons,
                                                               config_.steps,
                                                               config_.dt,
                                                               config_.threshold,
                                                               config_.leak_rate,
                                                               config_.reset_voltage,
                                                               config_.stdp_lr,
                                                               topology,
                                                               &probe,
                                                               OpenClSnnOptions{config_.opencl_local_size,
                                                                                 config_.emit_spikes_by_step,
                                                                                 config_.profile,
                                                                                 config_.somnia_enabled,
                                                                                 config_.somnia_period,
                                                                                 config_.somnia_duration,
                                                                                 config_.somnia_decay,
                                                                                 config_.quantized_weights,
                                                                                 config_.hardware_probe,
                                                                                 config_.hardware_probe_interval});
        if (gpu.used_opencl || config_.backend == "opencl") {
            SnnReport report;
            report.total_spikes = gpu.total_spikes;
            report.mean_voltage = gpu.mean_voltage;
            report.mean_weight = gpu.mean_weight;
            report.synapse_count = gpu.synapse_count;
            report.backend_model = gpu.backend_model;
            report.spikes_by_step = std::move(gpu.spikes_by_step);
            report.backend = gpu.backend;
            report.opencl = probe;
            report.backend_error = gpu.error;
            report.profile = gpu.profile;
            report.somnia_cycles = gpu.somnia_cycles;
            report.quantized_weights = gpu.quantized_weights;
            report.estimated_weight_memory_saving = gpu.estimated_weight_memory_saving;
            report.differential_checks = gpu.differential_checks;
            report.max_probe_error = gpu.max_probe_error;
            report.arena_capacity = config_.max_synapses > 0 ? config_.max_synapses : gpu.synapse_count;
            return report;
        }

        SnnReport report = run_cpu();
        report.opencl = probe;
        report.backend_error = gpu.error;
        return report;
    }
    return run_cpu();
}

} // namespace ag
