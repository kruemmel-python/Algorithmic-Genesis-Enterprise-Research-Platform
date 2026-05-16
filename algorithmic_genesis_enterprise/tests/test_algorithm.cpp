#include "ag/ag_algorithm.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

using namespace ag;

int main_algorithm_test() {
    Genome g;
    g.id = 123;
    g.name = "test_kernel";
    g.novelty = 0.8;
    g.genes = {
        Gene{Op::VarX, 0.0f, 0, 0},
        Gene{Op::Sin, 0.0f, 0, 0},
        Gene{Op::Tanh, 0.0f, 1, 0},
        Gene{Op::Mul, 0.0f, 2, 0}
    };
    assert(nontriviality_score(g) > 0.2);
    assert(choose_algorithm_kind("root_finding", "auto") == "root_refiner");
    assert(choose_algorithm_kind("signal_transform", "auto") == "signal_morpher");

    AlgorithmSynthesisConfig cfg;
    cfg.population = 8;
    cfg.generations = 2;
    cfg.genome_length = 8;
    cfg.samples = 32;
    cfg.seed = 7;
    cfg.domain = "signal_transform";
    cfg.generator = "random";
    cfg.archive_path = "";
    cfg.export_dir = "algorithm_test_exports";
    cfg.report_path = "algorithm_test_report.md";
    cfg.require_nontrivial = false;
    AlgorithmSynthesisReport r = synthesize_algorithm(cfg);
    assert(!r.artifact.name.empty());
    assert(!r.artifact.kernel_expression.empty());
    assert(!r.artifact.export_files.empty());
    for (const auto& f : r.artifact.export_files) {
        assert(std::filesystem::exists(f));
    }
    const std::string js = algorithm_synthesis_report_to_json(r);
    assert(js.find("\"artifact\"") != std::string::npos);
    assert(js.find("kernel_expression") != std::string::npos);
    return 0;
}

static int run_algorithm_test = main_algorithm_test();
