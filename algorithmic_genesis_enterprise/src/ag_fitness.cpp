#include "ag/ag_fitness.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace ag {

double target_function(double x) {
    return std::sin(x) + 0.25 * x * x - 0.5 * std::cos(2.0 * x);
}

static double domain_target(const std::string& domain, double x) {
    if (domain == "sequence_generation") {
        const double k = std::floor((x + 16.0) * 3.0);
        return std::sin(k * 0.37) + std::cos(k * 0.11);
    }
    if (domain == "fixed_point_dynamics") {
        return std::tanh(1.2 * x) - 0.15 * x;
    }
    if (domain == "chaotic_maps") {
        const double y = std::sin(3.7 * x) * std::cos(1.9 * x);
        return 3.8 * y * (1.0 - std::abs(y));
    }
    if (domain == "signal_transform") {
        return 0.6 * std::sin(x) + 0.3 * std::sin(3.0 * x) + 0.1 * std::cos(7.0 * x);
    }
    if (domain == "root_finding") {
        return x * x * x - x - 0.25;
    }

    if (domain == "sorting") {
        return 0.45 * x + 0.35 * std::tanh(2.0 * x) + 0.20 * std::sin(5.0 * x);
    }
    if (domain == "graph_shortest_path") {
        return std::sin(1.7 * x) + 0.20 * std::cos(5.0 * x) + 0.15 * x;
    }
    if (domain == "scheduling") {
        return 0.5 * std::tanh(x) + 0.25 * std::sin(2.0 * x) - 0.10 * x;
    }
    if (domain == "parsing") {
        return std::sin(x) > 0.0 ? 0.75 : -0.25;
    }
    if (domain == "constraint_solving") {
        return std::cos(2.3 * x) + 0.10 * x * x;
    }
    if (domain == "stream_processing") {
        return 0.4 * std::sin(x) + 0.4 * std::sin(0.5 * x) + 0.2 * std::tanh(x);
    }
    if (domain == "compression") {
        return std::tanh(std::sin(4.0 * x) + 0.1 * x);
    }
    if (domain == "cache_eviction") {
        return std::sin(x) + 0.25 * std::cos(3.0 * x);
    }
    if (domain == "load_balancing") {
        return 0.3 * std::sin(2.0 * x) + 0.7 * std::tanh(x);
    }
    if (domain == "anomaly_detection") {
        return std::abs(x) > 1.1 ? 1.0 : -0.2 + 0.2 * std::sin(3.0 * x);
    }

    if (domain == "classification_boundary") {
        return x > 0.35 * std::sin(4.0 * x) ? 1.0 : -1.0;
    }
    if (domain == "symbolic_identity_search") {
        return std::sin(x) * std::sin(x) + std::cos(x) * std::cos(x);
    }
    return target_function(x);
}

FitnessDataset make_target_dataset(const FitnessConfig& cfg) {
    FitnessDataset ds;
    ds.x.reserve(static_cast<std::size_t>(cfg.samples));
    ds.y.reserve(static_cast<std::size_t>(cfg.samples));
    for (int i = 0; i < cfg.samples; ++i) {
        const double t = cfg.samples == 1 ? 0.0 : static_cast<double>(i) / static_cast<double>(cfg.samples - 1);
        const double x = cfg.x_min + t * (cfg.x_max - cfg.x_min);
        ds.x.push_back(x);
        ds.y.push_back(domain_target(cfg.domain, x));
    }
    return ds;
}

static double approx_uniqueness(const std::string& sig, const std::vector<std::string>& archive) {
    if (archive.empty()) return 1.0;
    std::size_t best_common = 0;
    for (const auto& other : archive) {
        std::size_t common = 0;
        const std::size_t n = std::min(sig.size(), other.size());
        for (std::size_t i = 0; i < n; ++i) common += sig[i] == other[i] ? 1u : 0u;
        best_common = std::max(best_common, common);
    }
    const double similarity = static_cast<double>(best_common) / static_cast<double>(std::max<std::size_t>(1, sig.size()));
    return std::max(0.0, 1.0 - similarity);
}

void evaluate_fitness(Genome& genome, const FitnessDataset& dataset, const FitnessConfig& cfg,
                      const std::vector<std::string>& archive_signatures) {
    if (dataset.x.empty()) {
        genome.fitness = -1e9;
        return;
    }

    double mse = 0.0;
    double roughness = 0.0;
    double prev = 0.0;
    bool have_prev = false;
    int ok = 0;

    for (std::size_t i = 0; i < dataset.x.size(); ++i) {
        const auto r = eval_genome(genome, dataset.x[i]);
        if (!r.ok || !std::isfinite(r.value)) {
            genome.fitness = -1e9;
            genome.accuracy = 0.0;
            genome.stability = 0.0;
            return;
        }
        const double e = r.value - dataset.y[i];
        mse += e * e;
        if (have_prev) roughness += std::abs(r.value - prev);
        prev = r.value;
        have_prev = true;
        ++ok;
    }

    mse /= static_cast<double>(ok);
    const double accuracy = 1.0 / (1.0 + mse);
    const double stability = 1.0 / (1.0 + roughness / static_cast<double>(std::max(1, ok - 1)));
    const double complexity_raw = static_cast<double>(genome.genes.size());
    const double complexity = std::min(1.0, std::log1p(complexity_raw) / std::log(65.0));
    const double novelty = approx_uniqueness(genome_signature(genome), archive_signatures);

    genome.accuracy = accuracy;
    genome.stability = stability;
    genome.complexity = complexity;
    genome.novelty = novelty;
    genome.fitness = cfg.accuracy_weight * accuracy +
                     cfg.stability_weight * stability +
                     cfg.complexity_weight * complexity +
                     cfg.novelty_weight * novelty;
}

} // namespace ag
