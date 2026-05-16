#include "ag/ag_formula_parts.hpp"
#include "ag/ag_cli_common.hpp"
#include "ag/ag_json.hpp"
#include "ag/ag_error.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace ag {
namespace {

std::string read_all_text(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw Error("cannot read experiment manifest: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string json_get_string(const std::string& s, const std::string& key, const std::string& def) {
    std::regex re("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch m;
    if (std::regex_search(s, m, re)) return m[1].str();
    return def;
}

long long json_get_int64(const std::string& s, const std::string& key, long long def) {
    std::regex re("\"" + key + "\"\\s*:\\s*(-?[0-9]+)");
    std::smatch m;
    if (std::regex_search(s, m, re)) return std::stoll(m[1].str());
    return def;
}

bool json_get_bool(const std::string& s, const std::string& key, bool def) {
    std::regex re("\"" + key + "\"\\s*:\\s*(true|false)");
    std::smatch m;
    if (std::regex_search(s, m, re)) return m[1].str() == "true";
    return def;
}

std::vector<std::string> json_get_string_array(const std::string& s, const std::string& key) {
    std::vector<std::string> out;
    std::regex re("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]");
    std::smatch m;
    if (!std::regex_search(s, m, re)) return out;
    const std::string body = m[1].str();
    std::regex item("\"([^\"]+)\"");
    for (auto it = std::sregex_iterator(body.begin(), body.end(), item); it != std::sregex_iterator(); ++it) {
        out.push_back((*it)[1].str());
    }
    return out;
}

std::uint64_t fnv1a64(const std::string& s) {
    std::uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

std::string esc(const std::string& s) { return json_escape(s); }

bool catalog_contains(const std::vector<FormulaPart>& cat, const std::string& id) {
    return std::any_of(cat.begin(), cat.end(), [&](const FormulaPart& p){ return p.id == id; });
}

std::string part_category(const std::vector<FormulaPart>& cat, const std::string& id) {
    for (const FormulaPart& p : cat) if (p.id == id) return p.category;
    return "";
}

bool has_part(const GuidedExperimentManifest& m, const std::string& p) {
    return std::find(m.selected_parts.begin(), m.selected_parts.end(), p) != m.selected_parts.end();
}

} // namespace

std::vector<FormulaPart> formula_part_catalog() {
    return {
        {"x", "x", "primitive", "state", "Current candidate position.", 1.0, false},
        {"fx", "f(x)", "primitive", "residual", "Residual at current candidate.", 1.2, false},
        {"flo", "f(lo)", "primitive", "bracket", "Residual at lower bracket endpoint.", 1.0, false},
        {"fhi", "f(hi)", "primitive", "bracket", "Residual at upper bracket endpoint.", 1.0, false},
        {"lo", "lo", "primitive", "bracket", "Lower bracket endpoint.", 0.8, false},
        {"hi", "hi", "primitive", "bracket", "Upper bracket endpoint.", 0.8, false},
        {"mid", "midpoint", "geometry", "bisection", "Current interval midpoint.", 1.0, false},
        {"width", "width", "geometry", "scale", "Current interval width.", 1.1, false},
        {"relpos", "relative position", "geometry", "normalization", "Position of x in [lo, hi].", 1.1, false},
        {"edge_balance", "edge balance", "geometry", "bracket", "Balance between residuals near bracket edges.", 0.9, false},
        {"width_ratio", "width ratio", "geometry", "convergence", "Width relative to previous width.", 0.9, false},

        {"prev_x", "previous x", "memory", "history", "Previous accepted point.", 1.0, false},
        {"prev_fx", "previous f(x)", "memory", "history", "Residual at previous accepted point.", 1.0, false},
        {"prev2_x", "second previous x", "memory", "history", "Second previous accepted point.", 0.8, false},
        {"prev2_fx", "second previous f(x)", "memory", "history", "Residual at second previous point.", 0.8, false},
        {"history", "history", "memory", "trend", "Residual improvement memory.", 1.2, false},
        {"improvement", "improvement", "memory", "trend", "Current residual improvement.", 1.2, false},
        {"stagnation", "stagnation", "memory", "safety", "Counter for weak progress.", 1.1, false},

        {"bracket_slope", "bracket slope", "slope", "secant", "Secant slope over [lo, hi].", 1.4, false},
        {"local_slope", "local slope", "slope", "secant", "Local finite-difference slope.", 1.4, false},
        {"prev_slope", "previous slope", "slope", "history", "Previous finite-difference slope.", 1.0, false},
        {"curvature", "curvature", "slope", "geometry", "Difference between local and previous slopes.", 1.5, false},

        {"nfx", "normalized f(x)", "normalization", "residual", "Residual scaled by bracket residual magnitude.", 1.2, false},
        {"nflo", "normalized f(lo)", "normalization", "bracket", "Lower residual scaled by local scale.", 1.0, false},
        {"nfhi", "normalized f(hi)", "normalization", "bracket", "Upper residual scaled by local scale.", 1.0, false},
        {"nwidth", "log width", "normalization", "scale", "log1p of interval width.", 1.0, false},
        {"residual_ratio", "residual ratio", "normalization", "convergence", "Relative residual strength.", 1.2, false},
        {"scale", "scale", "normalization", "safety", "Numerical scale used for normalization.", 0.9, true},

        {"sin", "sin", "nonlinearity", "oscillation", "Sine nonlinearity.", 1.0, false},
        {"cos", "cos", "nonlinearity", "oscillation", "Cosine nonlinearity.", 0.8, false},
        {"tanh", "tanh", "nonlinearity", "saturation", "Bounded hyperbolic tangent.", 1.3, false},
        {"logabs", "logabs", "nonlinearity", "compression", "Safe logarithmic compression.", 1.0, true},
        {"expclamp", "expclamp", "nonlinearity", "amplification", "Safe bounded exponential.", 0.9, true},
        {"cubic", "cubic", "nonlinearity", "polynomial", "Cubic shaping term.", 1.1, false},
        {"reciprocal_safe", "safe reciprocal", "nonlinearity", "rational", "Safe reciprocal-like transformation.", 0.9, true},

        {"bias", "policy bias", "policy_output", "step", "Learned candidate direction.", 1.2, false},
        {"damping", "policy damping", "policy_output", "step", "Learned step damping.", 1.4, true},
        {"secant_mix", "secant mix", "policy_output", "method_mix", "Weight for secant candidate.", 1.5, false},
        {"bisection_mix", "bisection mix", "policy_output", "method_mix", "Weight for bisection candidate.", 1.3, true},
        {"relaxation_mix", "relaxation mix", "policy_output", "method_mix", "Weight for learned relaxation candidate.", 1.2, false},
        {"trust_delta", "trust delta", "policy_output", "trust", "Learned update of trust state.", 1.2, false},

        {"bisection", "bisection", "strategy", "baseline", "Safe midpoint fallback.", 1.5, true},
        {"secant", "secant", "strategy", "interpolation", "Secant/interpolation candidate.", 1.3, false},
        {"regula_falsi", "regula falsi", "strategy", "interpolation", "False-position candidate pressure.", 1.2, false},
        {"learned_relaxation", "learned relaxation", "strategy", "generated", "Generated kernel-based step.", 1.4, false},
        {"inverse_quadratic_hint", "inverse quadratic hint", "strategy", "interpolation", "Three-point curvature-aware interpolation pressure.", 1.0, false},

        {"bracket_guard", "bracket guard", "safety", "invariant", "Never leave [lo, hi].", 2.0, true},
        {"finite_sanitize", "finite sanitize", "safety", "numeric", "NaN/Inf containment.", 2.0, true},
        {"trust_gate", "trust gate", "safety", "trust", "Dampen learned steps when trust is low.", 1.6, true},
        {"stagnation_reset", "stagnation reset", "safety", "recovery", "Fallback when progress stalls.", 1.4, true},
        {"reject_penalty", "reject penalty", "safety", "learning", "Penalize repeatedly rejected candidates.", 1.3, true},
        {"gas_limit", "gas limit", "safety", "termination", "Hard operation budget.", 2.0, true},

        {"stable_sort", "stable sort", "cs_core_sorting", "correctness", "Preserve order of equal keys while sorting.", 1.5, true},
        {"inversion_pressure", "inversion pressure", "cs_core_sorting", "local_disorder", "Pressure against local inversions.", 1.2, false},
        {"adaptive_pivot", "adaptive pivot", "cs_core_sorting", "partition", "Learned pivot/key shaping for ordering.", 1.1, false},
        {"priority_queue", "priority queue", "cs_core_graph", "frontier", "Priority frontier for graph/search algorithms.", 1.3, false},
        {"edge_relaxation", "edge relaxation", "cs_core_graph", "dijkstra", "Relax candidate distances over edges.", 1.4, true},
        {"heuristic_potential", "heuristic potential", "cs_core_graph", "search_bias", "Learned potential for traversal ordering.", 1.2, false},
        {"deadline_pressure", "deadline pressure", "cs_core_scheduling", "deadline", "Deadline-aware scheduling pressure.", 1.2, false},
        {"makespan_minimize", "makespan minimize", "cs_core_scheduling", "objective", "Minimize maximum worker load.", 1.5, true},
        {"fairness_penalty", "fairness penalty", "cs_core_scheduling", "load_balance", "Penalize uneven load distribution.", 1.3, false},
        {"delimiter_stack", "delimiter stack", "cs_core_parsing", "syntax", "Bounded stack for syntax matching.", 1.4, true},
        {"repair_policy", "repair policy", "cs_core_parsing", "error_recovery", "Repair malformed token streams safely.", 1.3, true},
        {"precedence_pressure", "precedence pressure", "cs_core_parsing", "reduction", "Learned precedence/reduction preference.", 1.1, false},
        {"variable_ordering", "variable ordering", "cs_core_constraints", "search", "Variable ordering heuristic for CSP.", 1.4, false},
        {"value_ordering", "value ordering", "cs_core_constraints", "search", "Value ordering heuristic for CSP.", 1.3, false},
        {"constraint_propagation", "constraint propagation", "cs_core_constraints", "pruning", "Prune partial assignments safely.", 1.5, true},
        {"online_mean", "online mean", "cs_core_streaming", "statistics", "Streaming mean update.", 1.2, false},
        {"online_variance", "online variance", "cs_core_streaming", "statistics", "Streaming variance update.", 1.3, false},
        {"anomaly_pressure", "anomaly pressure", "cs_core_streaming", "signal", "Learned anomaly scoring pressure.", 1.3, false},
        {"run_length", "run length", "cs_core_compression", "encoding", "Run-length encoding pressure.", 1.2, false},
        {"delta_coding", "delta coding", "cs_core_compression", "encoding", "Delta encoding pressure.", 1.2, false},
        {"reconstruction_guard", "reconstruction guard", "cs_core_compression", "safety", "Decode/encode validation invariant.", 1.6, true},
        {"recency", "recency", "cs_core_cache", "eviction", "LRU-style recency pressure.", 1.2, false},
        {"frequency", "frequency", "cs_core_cache", "eviction", "LFU-style frequency pressure.", 1.2, false},
        {"reuse_distance", "reuse distance", "cs_core_cache", "prediction", "Reuse-distance prediction pressure.", 1.2, false},
        {"worker_load", "worker load", "cs_core_load_balancing", "state", "Current worker load signal.", 1.2, false},
        {"variance_penalty", "variance penalty", "cs_core_load_balancing", "fairness", "Penalize load variance.", 1.3, false},
        {"tail_latency", "tail latency", "cs_core_load_balancing", "objective", "Reduce worst-case completion time.", 1.4, false},
        {"zscore", "z-score", "cs_core_anomaly", "statistics", "Standard-deviation anomaly score.", 1.2, false},
        {"robust_median", "robust median", "cs_core_anomaly", "robustness", "Median-like robust center pressure.", 1.1, false},
        {"outlier_gate", "outlier gate", "cs_core_anomaly", "decision", "Bounded anomaly decision gate.", 1.5, true}
    };
}

std::string formula_part_catalog_json() {
    std::ostringstream os;
    os << "{\"version\":2,\"min_required\":3,\"recommended_min\":5,\"recommended_max\":20,\"parts\":[";
    const auto parts = formula_part_catalog();
    for (std::size_t i = 0; i < parts.size(); ++i) {
        const auto& p = parts[i];
        if (i) os << ",";
        os << "{\"id\":\"" << esc(p.id) << "\",\"label\":\"" << esc(p.label)
           << "\",\"category\":\"" << esc(p.category) << "\",\"role\":\"" << esc(p.role)
           << "\",\"description\":\"" << esc(p.description) << "\",\"default_weight\":" << p.default_weight
           << ",\"safety_critical\":" << (p.safety_critical ? "true" : "false") << "}";
    }
    os << "]}";
    return os.str();
}

std::string formula_part_ids_json_array(const std::vector<std::string>& ids) {
    std::ostringstream os;
    os << "[";
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (i) os << ",";
        os << "\"" << esc(ids[i]) << "\"";
    }
    os << "]";
    return os.str();
}

GuidedExperimentManifest read_guided_experiment_manifest(const std::string& path) {
    const std::string text = read_all_text(path);
    GuidedExperimentManifest m;
    m.name = json_get_string(text, "name", m.name);
    m.domain = json_get_string(text, "domain", m.domain);
    m.profile = json_get_string(text, "profile", m.profile);
    m.generator = json_get_string(text, "generator", m.generator);
    m.fitness_backend = json_get_string(text, "fitness_backend", m.fitness_backend);
    m.archive_path = json_get_string(text, "archive", json_get_string(text, "archive_path", m.archive_path));
    m.export_dir = json_get_string(text, "export_dir", m.export_dir);
    m.report_path = json_get_string(text, "report", json_get_string(text, "report_path", m.report_path));
    m.result_json = json_get_string(text, "json", json_get_string(text, "result_json", m.result_json));
    m.selected_parts = json_get_string_array(text, "selected_parts");
    m.population = static_cast<std::size_t>(std::max<long long>(1, json_get_int64(text, "population", static_cast<long long>(m.population))));
    m.generations = static_cast<int>(std::max<long long>(1, json_get_int64(text, "generations", m.generations)));
    m.genome_length = static_cast<std::size_t>(std::max<long long>(8, json_get_int64(text, "genome_length", static_cast<long long>(m.genome_length))));
    m.samples = static_cast<int>(std::max<long long>(8, json_get_int64(text, "samples", m.samples)));
    m.save_top = static_cast<std::size_t>(std::max<long long>(1, json_get_int64(text, "save_top", static_cast<long long>(m.save_top))));
    m.seed = static_cast<std::uint64_t>(json_get_int64(text, "seed", static_cast<long long>(m.seed)));
    m.vm_differential = json_get_bool(text, "vm_differential", m.vm_differential);
    m.require_nontrivial = json_get_bool(text, "require_nontrivial", m.require_nontrivial);
    m.gas_limit = static_cast<int>(std::max<long long>(100, json_get_int64(text, "gas_limit", m.gas_limit)));
    validate_guided_experiment(m);
    return m;
}

void validate_guided_experiment(const GuidedExperimentManifest& manifest) {
    if (manifest.selected_parts.size() < 3) {
        throw Error("guided discovery requires at least 3 selected formula/strategy parts");
    }
    const auto cat = formula_part_catalog();
    for (const auto& id : manifest.selected_parts) {
        if (!catalog_contains(cat, id)) {
            throw Error("unknown formula part in manifest: " + id);
        }
    }
    if (manifest.population < 8) throw Error("population must be >= 8");
    if (manifest.generations < 1) throw Error("generations must be >= 1");
}

std::uint64_t guided_seed(std::uint64_t base_seed, const std::vector<std::string>& parts,
                          const std::string& domain, const std::string& profile) {
    std::uint64_t h = base_seed ^ fnv1a64(domain) ^ (fnv1a64(profile) << 1);
    for (const auto& p : parts) {
        h ^= fnv1a64(p) + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
    }
    return h;
}

AlgorithmSynthesisConfig guided_manifest_to_synthesis_config(const GuidedExperimentManifest& m) {
    AlgorithmSynthesisConfig cfg;
    cfg.domain = m.domain;
    cfg.algorithm_kind = "auto";
    cfg.generator = m.generator;
    cfg.fitness_backend = m.fitness_backend;
    cfg.archive_path = m.archive_path;
    cfg.export_dir = m.export_dir;
    cfg.report_path = m.report_path;
    cfg.population = m.population;
    cfg.generations = m.generations;
    cfg.genome_length = m.genome_length;
    cfg.samples = m.samples;
    cfg.save_top = m.save_top;
    cfg.seed = guided_seed(m.seed, m.selected_parts, m.domain, m.profile);
    cfg.vm_differential = m.vm_differential || cfg.fitness_backend == "opencl-vm";
    cfg.require_nontrivial = m.require_nontrivial;
    cfg.gas_limit = m.gas_limit;

    // Selection pressure proxy: larger and more geometric/policy-rich selections increase expressive budget.
    const auto cat = formula_part_catalog();
    std::set<std::string> categories;
    for (const auto& p : m.selected_parts) categories.insert(part_category(cat, p));
    cfg.genome_length += std::min<std::size_t>(64, m.selected_parts.size() * 2 + categories.size() * 3);
    if (has_part(m, "curvature") || has_part(m, "inverse_quadratic_hint")) cfg.samples += 32;
    if (has_part(m, "finite_sanitize") || has_part(m, "bracket_guard")) cfg.require_nontrivial = true;
    if (m.profile == "novelty_max") cfg.save_top = std::max<std::size_t>(cfg.save_top, 64);
    if (m.profile == "safety_first") cfg.samples += 64;
    return cfg;
}

std::string guided_experiment_to_json(const GuidedExperimentManifest& m) {
    std::ostringstream os;
    os << "{\"name\":\"" << esc(m.name) << "\",\"domain\":\"" << esc(m.domain)
       << "\",\"profile\":\"" << esc(m.profile) << "\",\"generator\":\"" << esc(m.generator)
       << "\",\"fitness_backend\":\"" << esc(m.fitness_backend) << "\",\"seed\":" << m.seed
       << ",\"guided_seed\":" << guided_seed(m.seed, m.selected_parts, m.domain, m.profile)
       << ",\"population\":" << m.population << ",\"generations\":" << m.generations
       << ",\"genome_length\":" << m.genome_length << ",\"samples\":" << m.samples
       << ",\"selected_parts\":" << formula_part_ids_json_array(m.selected_parts) << "}";
    return os.str();
}

std::string guided_synthesis_result_to_json(const GuidedExperimentManifest& manifest,
                                            const AlgorithmSynthesisReport& report) {
    std::ostringstream os;
    os << "{\"guided\":true,\"experiment\":" << guided_experiment_to_json(manifest)
       << ",\"catalog_version\":2,\"selection_count\":" << manifest.selected_parts.size()
       << ",\"result\":" << algorithm_synthesis_report_to_json(report) << "}";
    return os.str();
}

std::string guided_report_markdown(const GuidedExperimentManifest& manifest,
                                   const AlgorithmSynthesisReport& report) {
    std::ostringstream os;
    os << "# Guided Algorithmic Genesis Experiment\n\n";
    os << "## Experiment\n\n";
    os << "- Name: `" << manifest.name << "`\n";
    os << "- Domain: `" << manifest.domain << "`\n";
    os << "- Profile: `" << manifest.profile << "`\n";
    os << "- Generator: `" << manifest.generator << "`\n";
    os << "- Fitness backend: `" << manifest.fitness_backend << "`\n";
    os << "- Base seed: `" << manifest.seed << "`\n";
    os << "- Guided seed: `" << guided_seed(manifest.seed, manifest.selected_parts, manifest.domain, manifest.profile) << "`\n";
    os << "- Population: `" << manifest.population << "`\n";
    os << "- Generations: `" << manifest.generations << "`\n\n";
    os << "## Selected Formula / Strategy Parts\n\n";
    const auto cat = formula_part_catalog();
    for (const auto& id : manifest.selected_parts) {
        auto it = std::find_if(cat.begin(), cat.end(), [&](const FormulaPart& p){ return p.id == id; });
        if (it != cat.end()) {
            os << "- `" << it->id << "` — " << it->description << " (" << it->category << "/" << it->role << ")\n";
        } else {
            os << "- `" << id << "`\n";
        }
    }
    os << "\n## Search Interpretation\n\n";
    os << "The selected parts are compiled into a guided seed, enlarged expressive genome budget, feature-pressure metadata and safety constraints. "
          "They act as human mathematical intuition injected into the Genesis search field, while the final candidate still has to pass novelty, "
          "nontriviality, VM differential checks and exported-code tests.\n\n";
    os << algorithm_discovery_report_markdown(report);
    return os.str();
}

} // namespace ag
