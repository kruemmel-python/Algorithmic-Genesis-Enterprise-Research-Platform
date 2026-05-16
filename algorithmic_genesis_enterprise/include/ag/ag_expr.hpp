#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace ag {

enum class Op : uint8_t {
    Const = 0,
    VarX = 1,
    Add = 2,
    Sub = 3,
    Mul = 4,
    SafeDiv = 5,
    Sin = 6,
    Cos = 7,
    ExpClamp = 8,
    LogAbs = 9,
    Tanh = 10
};

struct Gene {
    Op op{Op::Const};
    float value{0.0f};
    uint16_t a{0};
    uint16_t b{0};
};

struct Genome {
    uint64_t id{0};
    std::string name;
    std::vector<Gene> genes;
    double fitness{0.0};
    double novelty{0.0};
    double accuracy{0.0};
    double stability{0.0};
    double complexity{0.0};

    // Genesis provenance: enables genealogy, novelty auditing and reproducible discovery.
    uint64_t parent_a{0};
    uint64_t parent_b{0};
    int birth_generation{0};
    std::string origin{"random"};
    std::string mutation_trace;
    std::string ast_fingerprint;
    std::string behavior_fingerprint;
    std::string derivative_fingerprint;
    std::string stability_fingerprint;
    std::string complexity_fingerprint;
};

struct EvalResult {
    bool ok{false};
    double value{0.0};
    std::string error;
};

std::string op_name(Op op);
std::string genome_to_expression(const Genome& genome);
EvalResult eval_genome(const Genome& genome, double x);
std::string genome_signature(const Genome& genome);
Genome make_random_genome(uint64_t id, std::size_t length, class SplitMix64& rng);
void mutate_genome(Genome& genome, class SplitMix64& rng, double rate);
Genome crossover(uint64_t id, const Genome& lhs, const Genome& rhs, class SplitMix64& rng);
bool validate_genome(const Genome& genome, std::string* reason = nullptr);

} // namespace ag
