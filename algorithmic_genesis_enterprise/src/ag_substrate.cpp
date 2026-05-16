#include "ag/ag_substrate.hpp"
#include "ag/ag_json.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace ag {
namespace {
float sanitize(float v) {
    if (!std::isfinite(v)) return 0.0f;
    if (v > 1000000.0f) return 1000000.0f;
    if (v < -1000000.0f) return -1000000.0f;
    return v;
}
uint32_t q8(float w) {
    w = std::max(0.0f, std::min(2.0f, sanitize(w)));
    return static_cast<uint32_t>(std::max(0, std::min(255, static_cast<int>(std::lround(w * 127.5f)))));
}
} // namespace

PackedWeights pack_weights_u8x4(const std::vector<float>& weights) {
    PackedWeights p;
    p.original_count = weights.size();
    p.words.assign((weights.size() + 3u) / 4u, 0u);
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const uint32_t q = q8(weights[i]);
        const std::size_t word = i / 4u;
        const uint32_t shift = static_cast<uint32_t>((i % 4u) * 8u);
        p.words[word] |= (q << shift);
    }
    return p;
}

std::vector<float> unpack_weights_u8x4(const PackedWeights& packed) {
    std::vector<float> out(packed.original_count, 0.0f);
    for (std::size_t i = 0; i < out.size(); ++i) {
        const std::size_t word = i / 4u;
        const uint32_t shift = static_cast<uint32_t>((i % 4u) * 8u);
        const uint32_t q = (packed.words[word] >> shift) & 0xffu;
        out[i] = static_cast<float>(q) / 127.5f;
    }
    return out;
}

std::vector<OpenClVmInstruction> compile_genome_to_vm_bytecode(const Genome& genome) {
    std::vector<OpenClVmInstruction> code;
    code.reserve(genome.genes.size());
    for (std::size_t i = 0; i < genome.genes.size(); ++i) {
        const Gene& g = genome.genes[i];
        OpenClVmInstruction ins;
        ins.op = static_cast<uint32_t>(g.op);
        ins.value = g.value;
        ins.a = static_cast<uint32_t>(g.a % (i + 1u));
        ins.b = static_cast<uint32_t>(g.b % (i + 1u));
        if (ins.a == i && i > 0) ins.a = static_cast<uint32_t>(i - 1u);
        if (ins.b == i && i > 0) ins.b = static_cast<uint32_t>(i - 1u);
        code.push_back(ins);
    }
    return code;
}

std::vector<float> eval_genome_bytecode_cpu(const std::vector<OpenClVmInstruction>& bytecode,
                                            const std::vector<float>& xs) {
    OpenClProbe ignored;
    return eval_bytecode_vm_opencl_or_cpu(bytecode, xs, &ignored, 256).values;
}

VmDifferentialReport differential_test_bytecode_vm(const Genome& genome,
                                                   const FitnessDataset& dataset,
                                                   double tolerance,
                                                   std::size_t local_size) {
    (void)tolerance;
    VmDifferentialReport r;
    std::vector<float> xs;
    xs.reserve(dataset.x.size());
    for (double x : dataset.x) xs.push_back(static_cast<float>(x));

    const auto bytecode = compile_genome_to_vm_bytecode(genome);
    std::vector<float> cpu(xs.size(), 0.0f);
    for (std::size_t i = 0; i < xs.size(); ++i) {
        EvalResult e = eval_genome(genome, xs[i]);
        cpu[i] = e.ok ? static_cast<float>(e.value) : 0.0f;
    }

    OpenClProbe probe;
    OpenClVmResult gpu = eval_bytecode_vm_opencl_or_cpu(bytecode, xs, &probe, local_size);
    r.backend = gpu.backend;
    r.opencl_available = probe.available;
    r.samples = xs.size();
    r.kernel_ms = gpu.kernel_ms;
    r.readback_ms = gpu.readback_ms;
    r.error = gpu.error;

    double sum = 0.0;
    for (std::size_t i = 0; i < xs.size(); ++i) {
        const double d = std::abs(static_cast<double>(cpu[i]) - static_cast<double>(gpu.values[i]));
        r.max_abs_error = std::max(r.max_abs_error, d);
        sum += d;
    }
    r.mean_abs_error = xs.empty() ? 0.0 : sum / static_cast<double>(xs.size());
    return r;
}

std::string vm_differential_report_to_json(const VmDifferentialReport& r) {
    std::ostringstream os;
    os << "{";
    os << "\"backend\":\"" << json_escape(r.backend) << "\",";
    os << "\"opencl_available\":" << (r.opencl_available ? "true" : "false") << ",";
    os << "\"samples\":" << r.samples << ",";
    os << "\"max_abs_error\":" << r.max_abs_error << ",";
    os << "\"mean_abs_error\":" << r.mean_abs_error << ",";
    os << "\"kernel_ms\":" << r.kernel_ms << ",";
    os << "\"readback_ms\":" << r.readback_ms << ",";
    os << "\"error\":\"" << json_escape(r.error) << "\"";
    os << "}";
    return os.str();
}

} // namespace ag
