#include "ag/ag_ea.hpp"
#include "ag/ag_error.hpp"
#include <algorithm>
#include <fstream>
#include <numeric>

namespace ag {

EvolutionEngine::EvolutionEngine(EvolutionConfig config, FitnessConfig fitness_config)
    : config_(config),
      fitness_config_(fitness_config),
      rng_(config.seed),
      dataset_(make_target_dataset(fitness_config)) {
    if (!config_.archive_path.empty()) {
        DiscoveryArchive archive(config_.archive_path);
        archive.load();
        discovery_fingerprints_ = archive.fingerprints();
    }
    AG_REQUIRE(config_.population >= 4, "population must be >= 4");
    AG_REQUIRE(config_.genome_length >= 2, "genome_length must be >= 2");
    AG_REQUIRE(config_.elite_count >= 1, "elite_count must be >= 1");
}

Genome EvolutionEngine::tournament(const std::vector<Genome>& population) {
    Genome best = population[static_cast<std::size_t>(rng_.uniform_int(0, static_cast<int>(population.size() - 1)))];
    for (int i = 0; i < 3; ++i) {
        const Genome& candidate = population[static_cast<std::size_t>(rng_.uniform_int(0, static_cast<int>(population.size() - 1)))];
        if (candidate.fitness > best.fitness) best = candidate;
    }
    return best;
}

EvolutionReport EvolutionEngine::run() {
    std::vector<Genome> population;
    population.reserve(config_.population);
    if (config_.generator == "snn") {
        population = generate_population_from_snn(config_.population, config_.genome_length, config_.seed, 0);
    } else {
        for (std::size_t i = 0; i < config_.population; ++i) {
            Genome g = make_random_genome(rng_.next_u64(), config_.genome_length, rng_);
            g.birth_generation = 0;
            population.push_back(std::move(g));
        }
    }

    DiscoveryArchive disk_archive(config_.archive_path);
    if (!config_.archive_path.empty()) disk_archive.load();
    const ArchiveStats before_stats = disk_archive.stats();

    EvolutionReport report;
    report.backend = config_.fitness_backend == "opencl-vm" ? "opencl-vm" : "cpu";
    report.generator = config_.generator;
    report.archive_before = before_stats;

    for (int gen = 0; gen < config_.generations; ++gen) {
        for (auto& g : population) {
            evaluate_fitness(g, dataset_, fitness_config_, archive_);
            FingerprintSet fp = compute_fingerprints(g, fitness_config_);
            attach_fingerprints(g, fp);
            const double hist_novelty = novelty_against_archive(fp, discovery_fingerprints_);
            g.novelty = 0.5 * g.novelty + 0.5 * hist_novelty;
            g.fitness = fitness_config_.accuracy_weight * g.accuracy +
                        fitness_config_.stability_weight * g.stability +
                        fitness_config_.complexity_weight * g.complexity +
                        fitness_config_.novelty_weight * g.novelty;
        }

        std::sort(population.begin(), population.end(), [](const Genome& a, const Genome& b) {
            return a.fitness > b.fitness;
        });

        report.best_fitness_by_generation.push_back(population.front().fitness);
        const double mean = std::accumulate(population.begin(), population.end(), 0.0,
            [](double s, const Genome& g) { return s + g.fitness; }) / static_cast<double>(population.size());
        report.mean_fitness_by_generation.push_back(mean);

        archive_.push_back(genome_signature(population.front()));
        if (archive_.size() > 512) archive_.erase(archive_.begin());

        std::vector<Genome> next;
        const int elites = std::min<int>(config_.elite_count, static_cast<int>(population.size()));
        for (int i = 0; i < elites; ++i) next.push_back(population[static_cast<std::size_t>(i)]);

        while (next.size() < population.size()) {
            Genome a = tournament(population);
            Genome b = tournament(population);
            Genome child = crossover(rng_.next_u64(), a, b, rng_);
            mutate_genome(child, rng_, config_.mutation_rate);
            child.birth_generation = gen + 1;
            next.push_back(std::move(child));
        }

        population = std::move(next);
    }

    for (auto& g : population) {
        evaluate_fitness(g, dataset_, fitness_config_, archive_);
        FingerprintSet fp = compute_fingerprints(g, fitness_config_);
        attach_fingerprints(g, fp);
        const double hist_novelty = novelty_against_archive(fp, discovery_fingerprints_);
        g.novelty = 0.5 * g.novelty + 0.5 * hist_novelty;
        g.fitness = fitness_config_.accuracy_weight * g.accuracy +
                    fitness_config_.stability_weight * g.stability +
                    fitness_config_.complexity_weight * g.complexity +
                    fitness_config_.novelty_weight * g.novelty;
    }
    std::sort(population.begin(), population.end(), [](const Genome& a, const Genome& b) {
        return a.fitness > b.fitness;
    });

    report.best = population.front();
    if (config_.vm_differential) {
        report.vm_report = differential_test_bytecode_vm(report.best, dataset_, 1e-5, config_.vm_local_size);
    }

    if (!config_.archive_path.empty()) {
        const std::size_t n = std::min(config_.save_top, population.size());
        for (std::size_t i = 0; i < n; ++i) {
            ArchiveEntry e;
            e.genome = population[i];
            e.expression = genome_to_expression(population[i]);
            e.fingerprints = compute_fingerprints(population[i], fitness_config_);
            e.domain = fitness_config_.domain;
            e.backend = report.backend;
            e.seed = std::to_string(config_.seed);
            disk_archive.append(e);
        }
        disk_archive.load();
        report.archive_after = disk_archive.stats();
    } else {
        report.archive_after = report.archive_before;
    }

    if (!config_.export_dir.empty()) {
        CodeExportConfig xcfg;
        xcfg.output_dir = config_.export_dir;
        xcfg.module_name = report.best.name;
        xcfg.domain = fitness_config_.domain;
        std::string error;
        (void)export_candidate_code(report.best, xcfg, &error);
    }

    if (!config_.report_path.empty()) {
        std::ofstream out(config_.report_path);
        out << discovery_report_markdown(report.best, population, report.archive_before, report.archive_after,
                                         fitness_config_, report.backend, report.generator);
    }

    report.final_population = std::move(population);
    return report;
}

} // namespace ag
