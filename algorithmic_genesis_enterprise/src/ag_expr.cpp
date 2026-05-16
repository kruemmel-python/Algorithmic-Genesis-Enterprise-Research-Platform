#include "ag/ag_expr.hpp"
#include "ag/ag_rng.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_set>

namespace ag {

std::string op_name(Op op) {
    switch (op) {
        case Op::Const: return "const";
        case Op::VarX: return "x";
        case Op::Add: return "add";
        case Op::Sub: return "sub";
        case Op::Mul: return "mul";
        case Op::SafeDiv: return "sdiv";
        case Op::Sin: return "sin";
        case Op::Cos: return "cos";
        case Op::ExpClamp: return "expclamp";
        case Op::LogAbs: return "logabs";
        case Op::Tanh: return "tanh";
    }
    return "unknown";
}

static bool unary(Op op) {
    return op == Op::Sin || op == Op::Cos || op == Op::ExpClamp || op == Op::LogAbs || op == Op::Tanh;
}

static bool binary(Op op) {
    return op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::SafeDiv;
}

static double clamp_exp_arg(double x) {
    if (x > 8.0) return 8.0;
    if (x < -8.0) return -8.0;
    return x;
}

static double sanitize_numeric(double v) {
    // Numeric Poison Containment: generated math is not allowed to leak NaN/Inf
    // into the substrate. Invalid values are deterministically collapsed to 0.
    if (!std::isfinite(v)) return 0.0;
    if (v > 1e6) return 1e6;
    if (v < -1e6) return -1e6;
    return v;
}

EvalResult eval_genome(const Genome& genome, double x) {
    if (genome.genes.empty()) return {false, 0.0, "empty genome"};
    std::vector<double> values(genome.genes.size(), 0.0);
    for (std::size_t i = 0; i < genome.genes.size(); ++i) {
        const Gene& g = genome.genes[i];
        auto idx_a = static_cast<std::size_t>(g.a % (i + 1));
        auto idx_b = static_cast<std::size_t>(g.b % (i + 1));
        if (idx_a == i && i > 0) idx_a = i - 1;
        if (idx_b == i && i > 0) idx_b = i - 1;

        double v = 0.0;
        switch (g.op) {
            case Op::Const: v = static_cast<double>(g.value); break;
            case Op::VarX: v = x; break;
            case Op::Add: v = values[idx_a] + values[idx_b]; break;
            case Op::Sub: v = values[idx_a] - values[idx_b]; break;
            case Op::Mul: v = values[idx_a] * values[idx_b]; break;
            case Op::SafeDiv: v = values[idx_a] / (std::abs(values[idx_b]) + 1e-6); break;
            case Op::Sin: v = std::sin(values[idx_a]); break;
            case Op::Cos: v = std::cos(values[idx_a]); break;
            case Op::ExpClamp: v = std::exp(clamp_exp_arg(values[idx_a])); break;
            case Op::LogAbs: v = std::log(std::abs(values[idx_a]) + 1e-6); break;
            case Op::Tanh: v = std::tanh(values[idx_a]); break;
        }
        v = sanitize_numeric(v);
        values[i] = v;
    }
    return {true, values.back(), ""};
}

static std::string expr_at(const Genome& genome, std::size_t i, int depth) {
    if (i >= genome.genes.size()) return "?";
    if (depth > 6) return "...";
    const Gene& g = genome.genes[i];
    auto idx_a = static_cast<std::size_t>(g.a % (i + 1));
    auto idx_b = static_cast<std::size_t>(g.b % (i + 1));
    if (idx_a == i && i > 0) idx_a = i - 1;
    if (idx_b == i && i > 0) idx_b = i - 1;

    std::ostringstream os;
    if (g.op == Op::Const) {
        os << g.value;
    } else if (g.op == Op::VarX) {
        os << "x";
    } else if (binary(g.op)) {
        const char* sym = g.op == Op::Add ? "+" : g.op == Op::Sub ? "-" : g.op == Op::Mul ? "*" : "/";
        os << "(" << expr_at(genome, idx_a, depth + 1) << " " << sym << " " << expr_at(genome, idx_b, depth + 1) << ")";
    } else if (unary(g.op)) {
        os << op_name(g.op) << "(" << expr_at(genome, idx_a, depth + 1) << ")";
    } else {
        os << "?";
    }
    return os.str();
}

std::string genome_to_expression(const Genome& genome) {
    if (genome.genes.empty()) return "<empty>";
    return expr_at(genome, genome.genes.size() - 1, 0);
}

std::string genome_signature(const Genome& genome) {
    std::ostringstream os;
    for (const auto& g : genome.genes) {
        os << static_cast<int>(g.op) << ':' << static_cast<int>(g.value * 1000.0f) << ':' << g.a << ':' << g.b << ';';
    }
    return os.str();
}

Genome make_random_genome(uint64_t id, std::size_t length, SplitMix64& rng) {
    Genome genome;
    genome.id = id;
    genome.name = "ag_" + hex64(id);
    genome.origin = "random";
    genome.mutation_trace = "birth:random";
    genome.genes.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        Gene g;
        int maxop = static_cast<int>(Op::Tanh);
        g.op = static_cast<Op>(rng.uniform_int(0, maxop));
        if (i < 2) g.op = (i == 0) ? Op::VarX : Op::Const;
        g.value = static_cast<float>(rng.normal01());
        g.a = static_cast<uint16_t>(rng.uniform_int(0, static_cast<int>(i)));
        g.b = static_cast<uint16_t>(rng.uniform_int(0, static_cast<int>(i)));
        genome.genes.push_back(g);
    }
    return genome;
}

void mutate_genome(Genome& genome, SplitMix64& rng, double rate) {
    for (std::size_t i = 0; i < genome.genes.size(); ++i) {
        Gene& g = genome.genes[i];
        if (rng.bernoulli(rate)) g.op = static_cast<Op>(rng.uniform_int(0, static_cast<int>(Op::Tanh)));
        if (rng.bernoulli(rate)) g.value += static_cast<float>(0.25 * rng.normal01());
        if (rng.bernoulli(rate)) g.a = static_cast<uint16_t>(rng.uniform_int(0, static_cast<int>(i)));
        if (rng.bernoulli(rate)) g.b = static_cast<uint16_t>(rng.uniform_int(0, static_cast<int>(i)));
    }
    if (!genome.genes.empty()) genome.genes[0].op = Op::VarX;
    if (!genome.mutation_trace.empty()) genome.mutation_trace += ";";
    genome.mutation_trace += "mutate";
}

Genome crossover(uint64_t id, const Genome& lhs, const Genome& rhs, SplitMix64& rng) {
    Genome child;
    child.id = id;
    child.name = "ag_" + hex64(id);
    child.parent_a = lhs.id;
    child.parent_b = rhs.id;
    child.origin = "crossover";
    child.mutation_trace = "crossover:" + std::to_string(lhs.id) + "+" + std::to_string(rhs.id);
    const std::size_t n = std::min(lhs.genes.size(), rhs.genes.size());
    child.genes.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
        child.genes[i] = rng.bernoulli(0.5) ? lhs.genes[i] : rhs.genes[i];
        child.genes[i].a = static_cast<uint16_t>(child.genes[i].a % (i + 1));
        child.genes[i].b = static_cast<uint16_t>(child.genes[i].b % (i + 1));
    }
    return child;
}

bool validate_genome(const Genome& genome, std::string* reason) {
    if (genome.genes.empty()) {
        if (reason) *reason = "genome is empty";
        return false;
    }
    for (std::size_t i = 0; i < genome.genes.size(); ++i) {
        if (static_cast<int>(genome.genes[i].op) > static_cast<int>(Op::Tanh)) {
            if (reason) *reason = "invalid op";
            return false;
        }
    }
    return true;
}

} // namespace ag
