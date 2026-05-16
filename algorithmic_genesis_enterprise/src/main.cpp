#include "ag/ag_algorithm.hpp"
#include "ag/ag_cli_common.hpp"
#include "ag/ag_ea.hpp"
#include "ag/ag_error.hpp"
#include "ag/ag_json.hpp"
#include "ag/ag_genesis.hpp"
#include "ag/ag_formula_parts.hpp"
#include "ag/ag_opencl.hpp"
#include "ag/ag_persist.hpp"
#include "ag/ag_rng.hpp"
#include "ag/ag_snn.hpp"
#include "ag/ag_substrate.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace ag;




static int cmd_list_formula_parts(const CliArgs& args) {
    const std::string json = formula_part_catalog_json();
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return 0;
}

static int cmd_discover_guided(const CliArgs& args) {
    const std::string manifest_path = args.get("--experiment", args.get("--manifest", "experiments/guided_experiment.json"));
    GuidedExperimentManifest manifest = read_guided_experiment_manifest(manifest_path);

    // CLI overrides for enterprise batch operation.
    manifest.archive_path = args.get("--archive", manifest.archive_path);
    manifest.export_dir = args.get("--export-dir", manifest.export_dir);
    manifest.report_path = args.get("--report", manifest.report_path);
    manifest.result_json = args.get("--json", manifest.result_json);

    validate_guided_experiment(manifest);
    AlgorithmSynthesisConfig cfg = guided_manifest_to_synthesis_config(manifest);
    AlgorithmSynthesisReport report = synthesize_algorithm(cfg);

    const std::string json = guided_synthesis_result_to_json(manifest, report);
    if (!manifest.result_json.empty()) write_file(manifest.result_json, json);
    if (!manifest.report_path.empty()) write_file(manifest.report_path, guided_report_markdown(manifest, report));

    std::cout << json << "\n";
    return report.accepted ? 0 : 4;
}


static int cmd_discover_algorithm(const CliArgs& args) {
    AlgorithmSynthesisConfig cfg;
    cfg.domain = args.get("--domain", "chaotic_maps");
    cfg.algorithm_kind = args.get("--algorithm-kind", "auto");
    cfg.generator = args.get("--generator", "snn");
    cfg.fitness_backend = args.get("--fitness-backend", "cpu");
    cfg.archive_path = args.get("--archive", "genesis_archive.jsonl");
    cfg.export_dir = args.get("--export-dir", "algorithm_discoveries");
    cfg.report_path = args.get("--report", "algorithm_report.md");
    cfg.population = static_cast<std::size_t>(args.get_int("--population", 256));
    cfg.generations = args.get_int("--generations", 100);
    cfg.genome_length = static_cast<std::size_t>(args.get_int("--genome-length", 48));
    cfg.samples = args.get_int("--samples", 128);
    cfg.save_top = static_cast<std::size_t>(args.get_int("--save-top", 32));
    cfg.seed = args.get_u64("--seed", 42);
    cfg.vm_differential = args.has("--vm-differential") || cfg.fitness_backend == "opencl-vm";
    cfg.require_nontrivial = !args.has("--allow-trivial");
    cfg.gas_limit = args.get_int("--gas-limit", 10000);

    AlgorithmSynthesisReport report = synthesize_algorithm(cfg);
    const std::string json = algorithm_synthesis_report_to_json(report);
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return report.accepted ? 0 : 4;
}

static int cmd_evolve(const CliArgs& args) {
    EvolutionConfig ecfg;
    ecfg.population = static_cast<std::size_t>(args.get_int("--population", 64));
    ecfg.generations = args.get_int("--generations", 50);
    ecfg.genome_length = static_cast<std::size_t>(args.get_int("--genome-length", 32));
    ecfg.mutation_rate = args.get_double("--mutation-rate", 0.08);
    ecfg.elite_count = args.get_int("--elite-count", 4);
    ecfg.seed = args.get_u64("--seed", 42);
    ecfg.generator = args.get("--generator", "random");
    ecfg.archive_path = args.get("--archive", "");
    ecfg.export_dir = args.get("--export-dir", "");
    ecfg.report_path = args.get("--report", "");
    ecfg.save_top = static_cast<std::size_t>(args.get_int("--save-top", 16));
    ecfg.fitness_backend = args.get("--fitness-backend", "cpu");
    ecfg.vm_differential = args.has("--vm-differential") || ecfg.fitness_backend == "opencl-vm";
    ecfg.vm_local_size = static_cast<std::size_t>(args.get_int("--local-size", 256));

    FitnessConfig fcfg;
    fcfg.domain = args.get("--domain", "approximation");
    fcfg.samples = args.get_int("--samples", 128);

    EvolutionEngine engine(ecfg, fcfg);
    EvolutionReport report = engine.run();

    const std::string json = evolution_report_to_json(report);
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);

    const std::string db_path = args.get("--db", "");
    if (!db_path.empty()) {
        AlgorithmStore store(db_path);
        if (store.enabled()) {
            store.open();
            store.save_genome(report.best, genome_to_expression(report.best), "best candidate from CLI evolve");
        }
    }

    std::cout << json << "\n";
    return 0;
}

static int cmd_snn(const CliArgs& args) {
    SnnConfig cfg;
    cfg.neurons = static_cast<std::size_t>(args.get_int("--neurons", 128));
    cfg.connections_per_neuron = static_cast<std::size_t>(args.get_int("--connections", 8));
    cfg.steps = args.get_int("--steps", 100);
    cfg.seed = args.get_u64("--seed", 42);
    cfg.backend = args.get("--backend", "auto");
    cfg.semantic = args.get("--semantic", "immediate");
    cfg.opencl_local_size = static_cast<std::size_t>(args.get_int("--local-size", 256));
    cfg.profile = args.has("--profile");
    cfg.emit_spikes_by_step = !args.has("--compact");
    cfg.somnia_enabled = args.has("--somnia");
    cfg.somnia_period = args.get_int("--somnia-period", 512);
    cfg.somnia_duration = args.get_int("--somnia-duration", 64);
    cfg.somnia_decay = static_cast<float>(args.get_double("--somnia-decay", 0.0005));
    cfg.quantized_weights = args.has("--quantized-weights") || args.has("--ccq");
    cfg.mycelial_growth = args.has("--mycelial-growth");
    cfg.max_synapses = static_cast<std::size_t>(args.get_int("--max-synapses", 0));
    cfg.growth_rate = args.get_double("--growth-rate", 0.0);
    cfg.hardware_probe = args.has("--hardware-probe");
    cfg.hardware_probe_interval = args.get_int("--hardware-probe-interval", 256);
    SnnNetwork net(cfg);
    SnnReport report = net.run();
    const std::string json = snn_report_to_json(report);
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return 0;
}

static int cmd_opencl_probe(const CliArgs& args) {
    OpenClProbe probe = probe_opencl();
    const std::string json = opencl_probe_to_json(probe);
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return probe.available ? 0 : 2;
}


static int cmd_benchmark(const CliArgs& args) {
    const std::size_t neurons = static_cast<std::size_t>(args.get_int("--neurons", 2048));
    const std::size_t connections = static_cast<std::size_t>(args.get_int("--connections", 8));
    const int steps = args.get_int("--steps", 5000);
    const uint64_t seed = args.get_u64("--seed", 42);
    const int repeats = std::max(1, args.get_int("--repeats", 3));
    const bool compact = !args.has("--emit-spikes");

    std::vector<std::size_t> local_sizes;
    const std::string local_arg = args.get("--local-sizes", "64,128,256");
    std::stringstream ls(local_arg);
    std::string token;
    while (std::getline(ls, token, ',')) {
        if (!token.empty()) local_sizes.push_back(static_cast<std::size_t>(std::stoul(token)));
    }
    if (local_sizes.empty()) local_sizes = {256};

    auto run_once = [&](const std::string& backend, const std::string& semantic, std::size_t local_size, uint64_t run_seed) {
        SnnConfig cfg;
        cfg.neurons = neurons;
        cfg.connections_per_neuron = connections;
        cfg.steps = steps;
        cfg.seed = run_seed;
        cfg.backend = backend;
        cfg.semantic = semantic;
        cfg.opencl_local_size = local_size;
        cfg.emit_spikes_by_step = !compact;
        cfg.profile = true;
        cfg.somnia_enabled = args.has("--somnia");
        cfg.somnia_period = args.get_int("--somnia-period", 512);
        cfg.somnia_duration = args.get_int("--somnia-duration", 64);
        cfg.somnia_decay = static_cast<float>(args.get_double("--somnia-decay", 0.0005));
        cfg.quantized_weights = args.has("--quantized-weights") || args.has("--ccq");
        cfg.hardware_probe = args.has("--hardware-probe");
        cfg.hardware_probe_interval = args.get_int("--hardware-probe-interval", 256);
        SnnNetwork net(cfg);
        return net.run();
    };

    std::ostringstream os;
    os << "{\"benchmark_version\":1";
    os << ",\"neurons\":" << neurons;
    os << ",\"connections\":" << connections;
    os << ",\"steps\":" << steps;
    os << ",\"repeats\":" << repeats;
    os << ",\"compact\":" << (compact ? "true" : "false");
    os << ",\"runs\":[";

    bool first = true;
    auto append_run = [&](const std::string& label, const SnnReport& r) {
        if (!first) os << ",";
        first = false;
        os << "{\"label\":\"" << json_escape(label) << "\"";
        os << ",\"backend\":\"" << json_escape(r.backend) << "\"";
        os << ",\"backend_model\":\"" << json_escape(r.backend_model) << "\"";
        os << ",\"backend_error\":\"" << json_escape(r.backend_error) << "\"";
        os << ",\"synapse_count\":" << r.synapse_count;
        os << ",\"total_spikes\":" << r.total_spikes;
        os << ",\"mean_voltage\":" << r.mean_voltage;
        os << ",\"mean_weight\":" << r.mean_weight;
        os << ",\"opencl\":" << opencl_probe_to_json(r.opencl);
        os << ",\"profile\":{";
        os << "\"wall_ms\":" << r.profile.wall_ms << ",";
        os << "\"setup_ms\":" << r.profile.setup_ms << ",";
        os << "\"kernel_ms\":" << r.profile.kernel_ms << ",";
        os << "\"readback_ms\":" << r.profile.readback_ms << ",";
        os << "\"local_size\":" << r.profile.local_size << ",";
        os << "\"global_neurons\":" << r.profile.global_neurons << ",";
        os << "\"transfer_bytes\":" << r.profile.transfer_bytes << ",";
        os << "\"neuron_steps_per_second\":" << r.profile.neuron_steps_per_second;
        os << "}}";
    };

    for (int r = 0; r < repeats; ++r) {
        append_run("cpu_immediate", run_once("cpu", "immediate", 0, seed + static_cast<uint64_t>(r)));
        append_run("cpu_one_tick_latency", run_once("cpu", "latency", 0, seed + static_cast<uint64_t>(r)));
        for (std::size_t local : local_sizes) {
            append_run("opencl_local_" + std::to_string(local),
                       run_once("opencl", "latency", local, seed + static_cast<uint64_t>(r)));
        }
    }

    os << "]}";
    const std::string json = os.str();
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return 0;
}


static int cmd_random(const CliArgs& args) {
    const auto count = static_cast<std::size_t>(args.get_int("--count", 16));
    const auto seed = args.get_u64("--seed", 42);
    OpenClProbe probe;
    auto xs = random_floats_opencl_or_cpu(seed, count, &probe);
    std::ostringstream os;
    const std::string backend = probe.available && probe.error.find("fallback") == std::string::npos ? "opencl" : "cpu";
    os << "{\"backend\":\"" << backend << "\",\"opencl\":" << opencl_probe_to_json(probe) << ",\"values\":[";
    for (std::size_t i = 0; i < xs.size(); ++i) {
        if (i) os << ",";
        os << xs[i];
    }
    os << "]}";
    const std::string json = os.str();
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return 0;
}

static int cmd_export_candidate(const CliArgs& args) {
    SplitMix64 rng(args.get_u64("--seed", 42));
    Genome g = make_random_genome(rng.next_u64(), static_cast<std::size_t>(args.get_int("--genome-length", 32)), rng);
    const std::string expression = args.get("--expression", "");
    if (!expression.empty()) {
        throw Error("direct expression import is intentionally not supported yet; use evolve --export-dir to export validated genomes");
    }
    CodeExportConfig cfg;
    cfg.output_dir = args.get("--out", "exports");
    cfg.module_name = args.get("--name", g.name);
    cfg.domain = args.get("--domain", "approximation");
    std::string error;
    if (!export_candidate_code(g, cfg, &error)) throw Error(error);
    std::cout << "{\"exported\":true,\"dir\":\"" << json_escape(cfg.output_dir) << "\",\"name\":\"" << json_escape(cfg.module_name) << "\"}\n";
    return 0;
}

static int cmd_archive_stats(const CliArgs& args) {
    const std::string path = args.get("--archive", "genesis_archive.jsonl");
    DiscoveryArchive archive(path);
    archive.load();
    const ArchiveStats st = archive.stats();
    std::cout << "{\"archive\":\"" << json_escape(path) << "\",\"entries\":" << st.entries
              << ",\"unique_ast\":" << st.unique_ast
              << ",\"unique_behavior\":" << st.unique_behavior << "}\n";
    return 0;
}


static int cmd_vm_test(const CliArgs& args) {
    SplitMix64 rng(args.get_u64("--seed", 42));
    Genome g = make_random_genome(rng.next_u64(), static_cast<std::size_t>(args.get_int("--genome-length", 32)), rng);
    FitnessConfig cfg;
    cfg.domain = args.get("--domain", "approximation");
    cfg.samples = args.get_int("--samples", 128);
    FitnessDataset dataset = make_target_dataset(cfg);
    VmDifferentialReport report = differential_test_bytecode_vm(g, dataset,
                                                                args.get_double("--tolerance", 1e-5),
                                                                static_cast<std::size_t>(args.get_int("--local-size", 256)));
    const std::string json = vm_differential_report_to_json(report);
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return (report.max_abs_error <= args.get_double("--tolerance", 1e-5) || report.backend == "cpu") ? 0 : 3;
}

static int cmd_substrate_probe(const CliArgs& args) {
    const std::size_t n = static_cast<std::size_t>(args.get_int("--weights", 1024));
    SplitMix64 rng(args.get_u64("--seed", 42));
    std::vector<float> w(n);
    for (float& x : w) x = 2.0f * rng.uniform01f();
    PackedWeights p = pack_weights_u8x4(w);
    std::vector<float> u = unpack_weights_u8x4(p);
    double max_err = 0.0;
    for (std::size_t i = 0; i < n; ++i) max_err = std::max(max_err, std::abs(static_cast<double>(w[i] - u[i])));
    std::ostringstream os;
    os << "{\"weights\":" << n
       << ",\"packed_words\":" << p.words.size()
       << ",\"raw_bytes\":" << (n * sizeof(float))
       << ",\"packed_bytes\":" << (p.words.size() * sizeof(uint32_t))
       << ",\"estimated_memory_saving\":0.75"
       << ",\"max_quantization_error\":" << max_err << "}";
    const std::string json = os.str();
    const std::string json_path = args.get("--json", "");
    if (!json_path.empty()) write_file(json_path, json);
    std::cout << json << "\n";
    return 0;
}


int main(int argc, char** argv) {
    try {
        CliArgs args(argc, argv);
        if (argc < 2 || args.has("--help") || args.has("-h")) {
            print_usage();
            return 0;
        }
        const std::string command = argv[1];
        if (command == "list-formula-parts") return cmd_list_formula_parts(args);
        if (command == "discover-guided") return cmd_discover_guided(args);
        if (command == "discover-algorithm") return cmd_discover_algorithm(args);
        if (command == "evolve") return cmd_evolve(args);
        if (command == "snn") return cmd_snn(args);
        if (command == "opencl-probe") return cmd_opencl_probe(args);
        if (command == "benchmark-snn") return cmd_benchmark(args);
        if (command == "random") return cmd_random(args);
        if (command == "archive-stats") return cmd_archive_stats(args);
        if (command == "export-candidate") return cmd_export_candidate(args);
        if (command == "vm-test") return cmd_vm_test(args);
        if (command == "substrate-probe") return cmd_substrate_probe(args);
        print_usage();
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ag_cli error: " << e.what() << "\n";
        return 1;
    }
}
