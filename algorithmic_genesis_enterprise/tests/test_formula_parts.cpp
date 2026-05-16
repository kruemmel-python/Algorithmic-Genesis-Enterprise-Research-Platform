#include "ag/ag_formula_parts.hpp"
#include <cassert>
#include <fstream>
#include <string>

int main_formula_parts_test() {
    auto parts = ag::formula_part_catalog();
    assert(parts.size() >= 40);
    std::string json = ag::formula_part_catalog_json();
    assert(json.find("bracket_guard") != std::string::npos);
    assert(json.find("secant_mix") != std::string::npos);

    const char* path = "guided_manifest_test.json";
    {
        std::ofstream out(path);
        out << "{"
            << "\"name\":\"test\","
            << "\"domain\":\"root_finding\","
            << "\"profile\":\"safety_first\","
            << "\"selected_parts\":[\"tanh\",\"curvature\",\"secant_mix\",\"bracket_guard\"],"
            << "\"population\":16,"
            << "\"generations\":2,"
            << "\"seed\":7"
            << "}";
    }
    auto m = ag::read_guided_experiment_manifest(path);
    assert(m.selected_parts.size() == 4);
    auto cfg = ag::guided_manifest_to_synthesis_config(m);
    assert(cfg.domain == "root_finding");
    assert(cfg.population == 16);
    assert(cfg.genome_length > m.genome_length);
    return 0;
}

static int run_formula_parts_test = main_formula_parts_test();
