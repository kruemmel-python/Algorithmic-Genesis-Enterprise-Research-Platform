#include "ag/ag_genesis.hpp"
#include "ag/ag_error.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>

namespace ag {
namespace {


uint64_t fnv1a(const std::string& s) {
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= static_cast<uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

std::string bucket(double v, double scale = 10.0) {
    if (!std::isfinite(v)) return "nan";
    const long long q = static_cast<long long>(std::llround(v * scale));
    return std::to_string(q);
}

std::string now_utcish() {
    using clock = std::chrono::system_clock;
    const auto t = clock::to_time_t(clock::now());
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return os.str();
}

std::string safe_name(const std::string& name) {
    std::string out;
    for (char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty() || (out[0] >= '0' && out[0] <= '9')) out = "ag_" + out;
    return out;
}

std::string expression_to_python(std::string e) {
    // The expression language is intentionally close to Python; map protected ops.
    auto rep = [&](const std::string& a, const std::string& b) {
        std::size_t pos = 0;
        while ((pos = e.find(a, pos)) != std::string::npos) {
            e.replace(pos, a.size(), b);
            pos += b.size();
        }
    };
    rep("sin(", "math.sin(");
    rep("cos(", "math.cos(");
    rep("tanh(", "math.tanh(");
    rep("expclamp(", "expclamp(");
    rep("logabs(", "logabs(");
    return e;
}


std::string extract_json_string(const std::string& line, const std::string& key) {
    const std::string pat = "\"" + key + "\":\"";
    const auto p = line.find(pat);
    if (p == std::string::npos) return {};
    const auto start = p + pat.size();
    auto end = start;
    while (end < line.size()) {
        if (line[end] == '"' && line[end - 1] != '\\') break;
        ++end;
    }
    return line.substr(start, end - start);
}

} // namespace

FingerprintSet compute_fingerprints(const Genome& genome, const FitnessConfig& cfg) {
    FingerprintSet fp;
    const std::string expr = genome_to_expression(genome);
    fp.ast = "ast:" + hex64(fnv1a(genome_signature(genome)));

    std::ostringstream behavior;
    std::ostringstream derivative;
    std::ostringstream stability;
    const double lo = cfg.x_min;
    const double hi = cfg.x_max;
    const int samples = std::max(16, std::min(128, cfg.samples));
    double prev = 0.0;
    bool have_prev = false;
    int finite_count = 0;
    int sign_changes = 0;
    double roughness = 0.0;
    for (int i = 0; i < samples; ++i) {
        const double x = lo + (hi - lo) * static_cast<double>(i) / static_cast<double>(samples - 1);
        const EvalResult r = eval_genome(genome, x);
        const double y = (r.ok && std::isfinite(r.value)) ? std::clamp(r.value, -1000.0, 1000.0) : 0.0;
        behavior << bucket(y, 5.0) << ';';
        if (have_prev) {
            const double dy = y - prev;
            derivative << bucket(dy, 5.0) << ';';
            roughness += std::abs(dy);
            if ((prev < 0.0 && y > 0.0) || (prev > 0.0 && y < 0.0)) ++sign_changes;
        }
        prev = y;
        have_prev = true;
        ++finite_count;
    }
    stability << "finite=" << finite_count << ";sign=" << sign_changes << ";rough=" << bucket(roughness, 1.0);
    fp.behavior = "beh:" + hex64(fnv1a(behavior.str()));
    fp.derivative = "der:" + hex64(fnv1a(derivative.str()));
    fp.stability = "sta:" + hex64(fnv1a(stability.str()));
    fp.complexity = "cmp:g" + std::to_string(genome.genes.size()) + ":c" + bucket(genome.complexity, 100.0);
    return fp;
}

void attach_fingerprints(Genome& genome, const FingerprintSet& fp) {
    genome.ast_fingerprint = fp.ast;
    genome.behavior_fingerprint = fp.behavior;
    genome.derivative_fingerprint = fp.derivative;
    genome.stability_fingerprint = fp.stability;
    genome.complexity_fingerprint = fp.complexity;
}

double novelty_against_archive(const FingerprintSet& fp, const std::vector<FingerprintSet>& archive) {
    if (archive.empty()) return 1.0;
    double best_similarity = 0.0;
    for (const auto& other : archive) {
        double sim = 0.0;
        if (fp.ast == other.ast) sim += 0.40;
        if (fp.behavior == other.behavior) sim += 0.35;
        if (fp.derivative == other.derivative) sim += 0.15;
        if (fp.stability == other.stability) sim += 0.07;
        if (fp.complexity == other.complexity) sim += 0.03;
        best_similarity = std::max(best_similarity, sim);
    }
    return std::clamp(1.0 - best_similarity, 0.0, 1.0);
}

DiscoveryArchive::DiscoveryArchive(std::string path) : path_(std::move(path)) {}

bool DiscoveryArchive::load() {
    entries_.clear();
    if (path_.empty()) return false;
    std::ifstream in(path_);
    if (!in) return true;
    std::string line;
    while (std::getline(in, line)) {
        ArchiveEntry e;
        e.expression = extract_json_string(line, "expression");
        e.domain = extract_json_string(line, "domain");
        e.backend = extract_json_string(line, "backend");
        e.created_at = extract_json_string(line, "created_at");
        e.fingerprints.ast = extract_json_string(line, "ast_fingerprint");
        e.fingerprints.behavior = extract_json_string(line, "behavior_fingerprint");
        e.fingerprints.derivative = extract_json_string(line, "derivative_fingerprint");
        e.fingerprints.stability = extract_json_string(line, "stability_fingerprint");
        e.fingerprints.complexity = extract_json_string(line, "complexity_fingerprint");
        entries_.push_back(std::move(e));
    }
    return true;
}

bool DiscoveryArchive::append(const ArchiveEntry& entry) {
    if (path_.empty()) return false;
    std::filesystem::create_directories(std::filesystem::path(path_).parent_path().empty() ? "." : std::filesystem::path(path_).parent_path());
    std::ofstream out(path_, std::ios::app);
    if (!out) return false;
    const Genome& g = entry.genome;
    auto esc = [](const std::string& s) {
        std::string r;
        for (char c : s) {
            if (c == '"') r += "\\\"";
            else if (c == '\\') r += "\\\\";
            else if (c == '\n') r += "\\n";
            else r += c;
        }
        return r;
    };
    out << "{"
        << "\"created_at\":\"" << now_utcish() << "\","
        << "\"id\":" << g.id << ","
        << "\"name\":\"" << esc(g.name) << "\","
        << "\"domain\":\"" << esc(entry.domain) << "\","
        << "\"backend\":\"" << esc(entry.backend) << "\","
        << "\"seed\":\"" << esc(entry.seed) << "\","
        << "\"fitness\":" << g.fitness << ","
        << "\"novelty\":" << g.novelty << ","
        << "\"accuracy\":" << g.accuracy << ","
        << "\"stability\":" << g.stability << ","
        << "\"complexity\":" << g.complexity << ","
        << "\"parent_a\":" << g.parent_a << ","
        << "\"parent_b\":" << g.parent_b << ","
        << "\"birth_generation\":" << g.birth_generation << ","
        << "\"origin\":\"" << esc(g.origin) << "\","
        << "\"mutation_trace\":\"" << esc(g.mutation_trace) << "\","
        << "\"expression\":\"" << esc(entry.expression) << "\","
        << "\"ast_fingerprint\":\"" << esc(entry.fingerprints.ast) << "\","
        << "\"behavior_fingerprint\":\"" << esc(entry.fingerprints.behavior) << "\","
        << "\"derivative_fingerprint\":\"" << esc(entry.fingerprints.derivative) << "\","
        << "\"stability_fingerprint\":\"" << esc(entry.fingerprints.stability) << "\","
        << "\"complexity_fingerprint\":\"" << esc(entry.fingerprints.complexity) << "\""
        << "}\n";
    return true;
}

std::vector<FingerprintSet> DiscoveryArchive::fingerprints() const {
    std::vector<FingerprintSet> out;
    out.reserve(entries_.size());
    for (const auto& e : entries_) out.push_back(e.fingerprints);
    return out;
}

ArchiveStats DiscoveryArchive::stats() const {
    ArchiveStats s;
    s.entries = entries_.size();
    std::set<std::string> ast, beh;
    for (const auto& e : entries_) {
        if (!e.fingerprints.ast.empty()) ast.insert(e.fingerprints.ast);
        if (!e.fingerprints.behavior.empty()) beh.insert(e.fingerprints.behavior);
    }
    s.unique_ast = ast.size();
    s.unique_behavior = beh.size();
    return s;
}

bool export_candidate_code(const Genome& genome, const CodeExportConfig& cfg, std::string* error) {
    try {
        std::filesystem::create_directories(cfg.output_dir);
        const std::string name = safe_name(cfg.module_name.empty() ? genome.name : cfg.module_name);
        const std::string expr = genome_to_expression(genome);
        const std::string pyexpr = expression_to_python(expr);

        {
            std::ofstream py(std::filesystem::path(cfg.output_dir) / (name + ".py"));
            py << "\"\"\"Generated by Algorithmic Genesis. Domain: " << cfg.domain << "\"\"\"\n";
            py << "import math\n\n";
            py << "def expclamp(x: float) -> float:\n    return math.exp(max(-20.0, min(20.0, x)))\n\n";
            py << "def logabs(x: float) -> float:\n    return math.log(abs(x) + 1e-9)\n\n";
            py << "def candidate(x: float, gas_left: int = 10000) -> float:\n";
            py << "    if gas_left <= 0:\n        return 0.0\n";
            py << "    try:\n        y = float(" << pyexpr << ")\n";
            py << "        return 0.0 if not math.isfinite(y) else max(-1e6, min(1e6, y))\n";
            py << "    except Exception:\n        return 0.0\n";
            py << "\ndef evaluate_safely(x: float, gas_left: int = 10000) -> tuple[float, int]:\n";
            py << "    if gas_left <= 0:\n        return 0.0, 1\n";
            py << "    return candidate(x, gas_left - " << genome.genes.size() << "), 0\n";
        }
        {
            std::ofstream h(std::filesystem::path(cfg.output_dir) / (name + ".hpp"));
            h << "#pragma once\n#include <cmath>\n#include <algorithm>\n";
            h << "inline double ag_expclamp(double x){ return std::exp(std::clamp(x, -20.0, 20.0)); }\n";
            h << "inline double ag_logabs(double x){ return std::log(std::abs(x) + 1e-9); }\n";
            std::string ce = expr;
            auto rep = [&](const std::string& a, const std::string& b){ size_t pos=0; while((pos=ce.find(a,pos))!=std::string::npos){ ce.replace(pos,a.size(),b); pos+=b.size(); }};
            rep("sin(", "std::sin("); rep("cos(", "std::cos("); rep("tanh(", "std::tanh("); rep("expclamp(", "ag_expclamp("); rep("logabs(", "ag_logabs(");
            h << "inline double ag_sanitize(double v){ return std::isfinite(v) ? std::clamp(v, -1000000.0, 1000000.0) : 0.0; }\n";
            h << "inline double " << name << "(double x){ return ag_sanitize(" << ce << "); }\n";
            h << "extern \"C\" inline double evaluate_safely(const double* inputs, int* gas_left, int* error_code){\n";
            h << "    if(!inputs || !gas_left || !error_code){ return 0.0; }\n";
            h << "    const int cost = " << genome.genes.size() << ";\n";
            h << "    if(*gas_left < cost){ *error_code = 1; return 0.0; }\n";
            h << "    *gas_left -= cost; *error_code = 0; return " << name << "(inputs[0]);\n";
            h << "}\n";
        }
        {
            std::ofstream cl(std::filesystem::path(cfg.output_dir) / (name + ".cl"));
            std::string cle = expr;
            auto rep = [&](const std::string& a, const std::string& b){ size_t pos=0; while((pos=cle.find(a,pos))!=std::string::npos){ cle.replace(pos,a.size(),b); pos+=b.size(); }};
            rep("expclamp(", "ag_expclamp("); rep("logabs(", "ag_logabs(");
            cl << "inline float ag_expclamp(float x){ return exp(clamp(x, -20.0f, 20.0f)); }\n";
            cl << "inline float ag_logabs(float x){ return log(fabs(x) + 1e-9f); }\n";
            cl << "inline float ag_sanitize(float v){ uint i=as_uint(v); return ((i & 0x7F800000u)==0x7F800000u) ? 0.0f : clamp(v, -1000000.0f, 1000000.0f); }\n";
            cl << "__kernel void eval_candidate(__global const float* xs, __global float* ys, const uint n, const int gas_limit){ uint i=get_global_id(0); if(i<n){ if(gas_limit < " << genome.genes.size() << "){ ys[i]=0.0f; return; } float x=xs[i]; ys[i]=ag_sanitize((float)(" << cle << ")); }}\n";
        }
        return true;
    } catch (const std::exception& e) {
        if (error) *error = e.what();
        return false;
    }
}

std::string discovery_report_markdown(const Genome& best,
                                      const std::vector<Genome>& population,
                                      const ArchiveStats& before,
                                      const ArchiveStats& after,
                                      const FitnessConfig& cfg,
                                      const std::string& backend,
                                      const std::string& generator) {
    std::ostringstream md;
    md << "# Algorithmic Genesis Discovery Report\n\n";
    md << "## Run Summary\n\n";
    md << "- Domain: `" << cfg.domain << "`\n";
    md << "- Backend: `" << backend << "`\n";
    md << "- Generator: `" << generator << "`\n";
    md << "- Archive entries before: " << before.entries << "\n";
    md << "- Archive entries after: " << after.entries << "\n";
    md << "- Unique AST fingerprints after: " << after.unique_ast << "\n";
    md << "- Unique behavior fingerprints after: " << after.unique_behavior << "\n\n";
    md << "## Best Candidate\n\n";
    md << "- Name: `" << best.name << "`\n";
    md << "- ID: `" << best.id << "`\n";
    md << "- Fitness: " << best.fitness << "\n";
    md << "- Novelty: " << best.novelty << "\n";
    md << "- Accuracy: " << best.accuracy << "\n";
    md << "- Stability: " << best.stability << "\n";
    md << "- Complexity: " << best.complexity << "\n";
    md << "- Expression: `" << genome_to_expression(best) << "`\n\n";
    md << "## Fingerprints\n\n";
    md << "- AST: `" << best.ast_fingerprint << "`\n";
    md << "- Behavior: `" << best.behavior_fingerprint << "`\n";
    md << "- Derivative: `" << best.derivative_fingerprint << "`\n";
    md << "- Stability: `" << best.stability_fingerprint << "`\n";
    md << "- Complexity: `" << best.complexity_fingerprint << "`\n\n";
    md << "## Genealogy\n\n";
    md << "- Parent A: `" << best.parent_a << "`\n";
    md << "- Parent B: `" << best.parent_b << "`\n";
    md << "- Birth generation: " << best.birth_generation << "\n";
    md << "- Origin: `" << best.origin << "`\n";
    md << "- Mutation trace: `" << best.mutation_trace << "`\n\n";
    md << "## Top Population Slice\n\n";
    md << "| Rank | Name | Fitness | Novelty | Expression |\n";
    md << "|---:|---|---:|---:|---|\n";
    const std::size_t n = std::min<std::size_t>(10, population.size());
    for (std::size_t i = 0; i < n; ++i) {
        md << "|" << (i + 1) << "|`" << population[i].name << "`|" << population[i].fitness << "|" << population[i].novelty << "|`" << genome_to_expression(population[i]) << "`|\n";
    }
    md << "\n## Interpretation\n\n";
    md << "The candidate is not accepted as a theorem. It is an executable mathematical artifact with recorded provenance, behavior and structural fingerprints. Further validation should compare it against known baselines and stress it outside the training domain.\n";
    return md.str();
}

std::vector<Genome> generate_population_from_snn(std::size_t population,
                                                 std::size_t genome_length,
                                                 uint64_t seed,
                                                 int birth_generation,
                                                 std::size_t virtual_neurons,
                                                 int virtual_steps) {
    SplitMix64 rng(seed ^ 0x9e3779b97f4a7c15ull);
    std::vector<Genome> out;
    out.reserve(population);
    for (std::size_t i = 0; i < population; ++i) {
        Genome g;
        g.id = rng.next_u64();
        g.name = "ag_snn_" + hex64(g.id);
        g.origin = "snn_grammar";
        g.birth_generation = birth_generation;
        g.genes.reserve(genome_length);
        uint64_t membrane = seed ^ (i * 0x9e3779b97f4a7c15ull);
        for (std::size_t j = 0; j < genome_length; ++j) {
            // Virtual spike resonance: neuron id + time fold into grammar token.
            membrane ^= rng.next_u64() + static_cast<uint64_t>(virtual_neurons * 31 + virtual_steps * 17 + j);
            membrane ^= membrane >> 12; membrane *= 0x27d4eb2d165667c5ull; membrane ^= membrane >> 25;
            const int token = static_cast<int>(membrane % 11);
            Gene gene;
            gene.op = static_cast<Op>(token);
            gene.value = static_cast<float>(-2.0 + 4.0 * rng.uniform01());
            gene.a = static_cast<uint16_t>(j == 0 ? 0 : rng.uniform_int(0, static_cast<int>(j)));
            gene.b = static_cast<uint16_t>(j == 0 ? 0 : rng.uniform_int(0, static_cast<int>(j)));
            g.genes.push_back(gene);
        }
        std::string reason;
        if (!validate_genome(g, &reason)) {
            g = make_random_genome(g.id, genome_length, rng);
            g.origin = "snn_grammar_repaired";
            g.birth_generation = birth_generation;
            g.mutation_trace = "repair:" + reason;
        }
        out.push_back(std::move(g));
    }
    return out;
}

} // namespace ag
