#include "ag/ag_cli_common.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace ag {

CliArgs::CliArgs(int argc, char** argv) {
    for (int i = 0; i < argc; ++i) args.emplace_back(argv[i]);
}

bool CliArgs::has(const std::string& key) const {
    for (const auto& a : args) if (a == key) return true;
    return false;
}

std::string CliArgs::get(const std::string& key, const std::string& default_value) const {
    for (std::size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == key) return args[i + 1];
    }
    return default_value;
}

int CliArgs::get_int(const std::string& key, int default_value) const {
    return std::stoi(get(key, std::to_string(default_value)));
}

unsigned long long CliArgs::get_u64(const std::string& key, unsigned long long default_value) const {
    return std::stoull(get(key, std::to_string(default_value)));
}

double CliArgs::get_double(const std::string& key, double default_value) const {
    return std::stod(get(key, std::to_string(default_value)));
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write file: " + path);
    out << content;
}

void print_usage() {
    std::cout <<
        "Algorithmic Genesis CLI\n\n"
        "Commands:\n"
        "  list-formula-parts [--json file]  # catalog for guided WebGUI / experiments\n"
        "  discover-guided --experiment file.json [--archive file] [--export-dir dir] [--report file] [--json file]\n"
        "      # guided algorithm discovery from at least 3 selected formula/strategy parts\n"
        "  discover-algorithm --domain name --population N --generations N --seed N\n"
        "      [--algorithm-kind auto|root_refiner|fixed_point_iterator|signal_morpher|chaotic_map_explorer]\n"
        "      [--generator random|snn] [--fitness-backend cpu|opencl-vm] [--vm-differential]\n"
        "      [--archive file.jsonl] [--export-dir dir] [--report file.md] [--json file]\n"
        "      # evolves a kernel and exports a named executable mathematical algorithm\n"
        "  evolve --population N --generations N --seed N [--domain name] [--generator random|snn]\n"
        "      [--archive file.jsonl] [--save-top N] [--export-dir dir] [--report file.md] [--json file]\n"
        "  snn --neurons N --steps N --seed N [--connections N] [--backend cpu|auto|opencl]\n"
        "      [--semantic immediate|latency] [--local-size 64|128|256] [--compact] [--profile]\n"
        "      [--somnia --somnia-period N --somnia-duration N --somnia-decay F]\n"
        "      [--ccq|--quantized-weights] [--mycelial-growth --max-synapses N --growth-rate F]\n"
        "      [--hardware-probe --hardware-probe-interval N] [--json file]\n"
        "  benchmark-snn --neurons N --connections N --steps N --repeats N [--local-sizes 64,128,256]\n"
        "      [--somnia] [--ccq] [--hardware-probe] [--json file]\n"
        "  opencl-probe [--json file]\n"
        "  random --count N --seed N [--json file]  # uses OpenCL when available\n"
        "  archive-stats --archive file.jsonl\n"
        "  export-candidate --out dir --name module_name --seed N  # gas-limited safe ABI export\n"
        "  vm-test --genome-length N --samples N --seed N [--local-size N] [--json file]\n"
        "  substrate-probe --weights N --seed N [--json file]\n";
}

} // namespace ag
