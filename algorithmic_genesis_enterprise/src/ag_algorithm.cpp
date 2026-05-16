#include "ag/ag_algorithm.hpp"
#include "ag/ag_json.hpp"
#include "ag/ag_error.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <set>

namespace ag {
namespace {

std::string safe_symbol(std::string s) {
    for (char& c : s) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        if (!ok) c = '_';
    }
    if (s.empty() || (s[0] >= '0' && s[0] <= '9')) s = "ag_" + s;
    return s;
}

std::string op_py_expr(const std::string& expr) {
    std::string e = expr;
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

std::string op_cpp_expr(const std::string& expr) {
    std::string e = expr;
    auto rep = [&](const std::string& a, const std::string& b) {
        std::size_t pos = 0;
        while ((pos = e.find(a, pos)) != std::string::npos) {
            e.replace(pos, a.size(), b);
            pos += b.size();
        }
    };
    rep("sin(", "std::sin(");
    rep("cos(", "std::cos(");
    rep("tanh(", "std::tanh(");
    rep("expclamp(", "ag_expclamp(");
    rep("logabs(", "ag_logabs(");
    return e;
}

std::string op_cl_expr(const std::string& expr) {
    std::string e = expr;
    auto rep = [&](const std::string& a, const std::string& b) {
        std::size_t pos = 0;
        while ((pos = e.find(a, pos)) != std::string::npos) {
            e.replace(pos, a.size(), b);
            pos += b.size();
        }
    };
    rep("expclamp(", "ag_expclamp(");
    rep("logabs(", "ag_logabs(");
    return e;
}


static bool export_unary(Op op) {
    return op == Op::Sin || op == Op::Cos || op == Op::ExpClamp || op == Op::LogAbs || op == Op::Tanh;
}

static bool export_binary(Op op) {
    return op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::SafeDiv;
}

std::string export_expr_at(const Genome& genome, std::size_t i, int depth) {
    if (i >= genome.genes.size()) return "0.0";
    if (depth > 96) return "0.0";
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
    } else if (export_binary(g.op)) {
        const char* sym = g.op == Op::Add ? "+" : g.op == Op::Sub ? "-" : g.op == Op::Mul ? "*" : "/";
        os << "(" << export_expr_at(genome, idx_a, depth + 1) << " " << sym << " " << export_expr_at(genome, idx_b, depth + 1) << ")";
    } else if (export_unary(g.op)) {
        os << op_name(g.op) << "(" << export_expr_at(genome, idx_a, depth + 1) << ")";
    } else {
        os << "0.0";
    }
    return os.str();
}

std::string genome_to_export_expression(const Genome& genome) {
    if (genome.genes.empty()) return "0.0";
    return export_expr_at(genome, genome.genes.size() - 1, 0);
}

std::string algorithm_base_name(const std::string& kind, uint64_t id) {
    return safe_symbol("ag_" + kind + "_" + hex64(id).substr(0, 12));
}

std::string summary_for_kind(const std::string& kind) {
    if (kind == "chaotic_map_explorer") {
        return "A generated iterated-map algorithm that applies the discovered kernel as a bounded nonlinear recurrence and estimates orbit energy and Lyapunov-like divergence.";
    }
    if (kind == "root_refiner") {
        return "A generated bracketed root-refinement algorithm with a high-dimensional RootContext policy. The discovered kernel is lifted into K_policy(ctx), producing bias, damping, method-mix and trust updates from residuals, slopes, curvature, history, bracket geometry and safety state.";
    }
    if (kind == "fixed_point_iterator") {
        return "A generated fixed-point iterator that uses the discovered kernel as a contractive attractor transform with gas-limited convergence.";
    }
    if (kind == "signal_morpher") {
        return "A generated streaming transform that mixes an input signal with the discovered nonlinear kernel and a local memory term.";
    }
    if (kind == "sequence_extrapolator") {
        return "A generated sequence extrapolator that converts recent deltas through the discovered kernel to predict the next value.";
    }
    if (kind == "identity_residual_reducer") {
        return "A generated residual reducer that uses the discovered kernel to compress symbolic or numeric identity residuals toward zero.";
    }
    if (kind == "adaptive_sorter") {
        return "A generated comparison/key scheduling algorithm for stable adaptive sorting. The discovered kernel shapes local disorder, inversion pressure and tie-breaking while preserving sorted-output correctness.";
    }
    if (kind == "graph_pathfinder") {
        return "A generated shortest-path policy that combines Dijkstra-style safety with a learned edge-priority potential for graph traversal research.";
    }
    if (kind == "schedule_optimizer") {
        return "A generated scheduling heuristic that assigns jobs to machines using learned priority, deadline pressure and load-balance feedback.";
    }
    if (kind == "parser_repairer") {
        return "A generated parser/repair algorithm that tokenizes, balances delimiters and uses a learned policy to prefer safe reductions.";
    }
    if (kind == "constraint_solver") {
        return "A generated backtracking constraint solver with learned variable/value ordering and deterministic gas-limited search.";
    }
    if (kind == "stream_processor") {
        return "A generated streaming analytics algorithm for online mean/variance, anomaly pressure and bounded state updates.";
    }
    if (kind == "adaptive_compressor") {
        return "A generated lightweight compression heuristic combining run-length, delta and learned thresholding.";
    }
    if (kind == "cache_policy") {
        return "A generated cache-eviction policy that mixes recency, frequency and learned reuse pressure.";
    }
    if (kind == "load_balancer") {
        return "A generated load-balancing heuristic that assigns work to workers using learned load, variance and fairness pressure.";
    }
    if (kind == "anomaly_detector") {
        return "A generated anomaly detector using online robust statistics and learned deviation pressure.";
    }
    return "A generated scalar algorithm that wraps the discovered mathematical kernel into a gas-limited executable transform.";
}

std::string contract_for_kind(const std::string& kind) {
    if (kind == "root_refiner") {
        return "Given a continuous function f and an initial bracket [lo, hi] with opposite signs, the algorithm never leaves the bracket. Each candidate is generated by a high-dimensional RootContext policy using bracket geometry, residuals, secant/local slopes, curvature, improvement, stagnation, trust and reject-rate. A safe candidate mixer combines bisection, secant and learned relaxation, then clamps and validates the result. It returns the best point if convergence or gas exhaustion occurs.";
    }
    if (kind == "fixed_point_iterator") {
        return "Given an initial scalar x0, the algorithm produces a bounded orbit x[t+1] = mix(x[t], K(x[t])) and terminates on convergence or gas exhaustion.";
    }
    if (kind == "signal_morpher") {
        return "Given a finite signal, the algorithm returns a finite transformed signal of the same length. Each output is sanitized and bounded.";
    }
    if (kind == "sequence_extrapolator") {
        return "Given at least two scalar observations, the algorithm predicts a finite next value from the latest value, local delta and the generated kernel.";
    }
    if (kind == "chaotic_map_explorer") {
        return "Given x0 and an iteration budget, the algorithm returns a bounded orbit and a finite divergence estimate. It is observational, not a theorem of chaos.";
    }
    if (kind == "adaptive_sorter") {
        return "Given a finite list of scalar records, the algorithm returns a finite stable sorted list. The learned key may influence tie-breaking and pass scheduling, but the final correctness guard enforces nondecreasing order.";
    }
    if (kind == "graph_pathfinder") {
        return "Given a finite weighted graph with nonnegative weights, the algorithm returns a finite path and cost or a safe no-path result. Negative and nonfinite weights are sanitized.";
    }
    if (kind == "schedule_optimizer") {
        return "Given finite jobs and worker count, the algorithm returns deterministic assignments and load statistics under a gas limit.";
    }
    if (kind == "parser_repairer") {
        return "Given a finite token stream, the algorithm returns a bounded parse/repair report without recursion and without throwing on malformed input.";
    }
    if (kind == "constraint_solver") {
        return "Given finite variable domains and Python-callable constraints, the algorithm returns a satisfying assignment or a bounded failure result.";
    }
    if (kind == "stream_processor") {
        return "Given a finite stream, the algorithm emits bounded online statistics and anomaly signals using O(1) state.";
    }
    if (kind == "adaptive_compressor") {
        return "Given finite numeric data, the algorithm returns a reversible lightweight encoded representation and a decoder result.";
    }
    if (kind == "cache_policy") {
        return "Given finite access requests and cache capacity, the algorithm returns hits, misses and final cache state under deterministic eviction.";
    }
    if (kind == "load_balancer") {
        return "Given finite job costs and worker count, the algorithm returns assignments with bounded fairness and makespan statistics.";
    }
    if (kind == "anomaly_detector") {
        return "Given finite scalar observations, the algorithm returns indices and scores for bounded anomaly candidates.";
    }
    return "All exported functions are gas-limited, finite-sanitized and return a deterministic fallback value on numeric poison.";
}

std::string pseudocode_for_kind(const std::string& kind) {
    if (kind == "root_refiner") {
        return "initialize trust/history/stagnation; for t in 0..gas: build RootContext(mid, f(mid), lo, hi, flo, fhi, prev states, slopes, curvature, normalized residuals, trust, reject_rate); policy=K_policy(ctx)->{bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta}; candidate=mix(bisection, secant, learned_relaxation); clamp+validate; fallback to bisection if unsafe; shrink bracket; update trust/history/reject_rate; return best";
    }
    if (kind == "fixed_point_iterator") {
        return "x=x0; for t in 0..gas: y=sanitize(K(x)); next=0.65*x+0.35*tanh(y); stop if |next-x|<tol; x=next; return x";
    }
    if (kind == "signal_morpher") {
        return "state=0; for each sample s: k=K(s+0.25*state); y=0.70*s+0.30*tanh(k); state=0.8*state+0.2*y; emit sanitize(y)";
    }
    if (kind == "sequence_extrapolator") {
        return "delta=x[n]-x[n-1]; curvature=K(delta); return sanitize(x[n]+delta+0.1*tanh(curvature))";
    }
    if (kind == "chaotic_map_explorer") {
        return "x=x0; for t in 0..steps: x=tanh(K(x)); accumulate orbit and log local separation; return orbit statistics";
    }
    if (kind == "adaptive_sorter") {
        return "compute learned key for each value and local context; stable-sort by value plus learned tie-break; verify nondecreasing order; repair if necessary";
    }
    if (kind == "graph_pathfinder") {
        return "initialize frontier with start; repeatedly pop lowest cost plus learned priority; relax sanitized edges; reconstruct path when goal is reached";
    }
    if (kind == "schedule_optimizer") {
        return "rank jobs by learned priority; repeatedly assign next job to worker minimizing load plus learned pressure; return loads and makespan";
    }
    if (kind == "parser_repairer") {
        return "scan tokens; maintain bounded stack; repair mismatched delimiters; reduce simple arithmetic spans using learned precedence pressure";
    }
    if (kind == "constraint_solver") {
        return "order variables by domain size plus learned pressure; try values in learned order; backtrack under gas limit; return assignment";
    }
    if (kind == "stream_processor") {
        return "for each item: update mean/variance; compute learned anomaly pressure; emit summary and state";
    }
    if (kind == "adaptive_compressor") {
        return "scan values; form runs when delta is below learned threshold; emit literal/delta-run packets; decode for verification";
    }
    if (kind == "cache_policy") {
        return "for each request: hit if present; otherwise evict item with worst recency/frequency/learned reuse score; update state";
    }
    if (kind == "load_balancer") {
        return "for each job: choose worker minimizing projected makespan plus learned fairness penalty; return assignment";
    }
    if (kind == "anomaly_detector") {
        return "update robust online statistics; mark samples whose learned score exceeds adaptive threshold";
    }
    return "for each input x: y=sanitize(K(x)); emit y";
}

std::string complexity_for_kind(const std::string& kind, std::size_t kernel_cost) {
    std::ostringstream os;
    os << "Time O(iterations * " << kernel_cost << ") for iterative modes or O(n * " << kernel_cost
       << ") for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.";
    if (kind == "root_refiner") os << " The bracket invariant is maintained by constant-time checks.";
    return os.str();
}

std::string validation_for_kind(const std::string& kind) {
    if (kind == "root_refiner") {
        return "Validate on polynomials, trigonometric roots, adversarial flat functions and discontinuity-like numeric poison. Check bracket preservation, finite output, monotonic best residual tracking, gas termination, RootContext feature finiteness, policy output bounds, trust/reject dynamics and agreement with bisection fallback invariants.";
    }
    if (kind == "chaotic_map_explorer") {
        return "Validate boundedness, reproducibility and finite Lyapunov-like estimates across seeds. Compare against logistic/tent-map baselines.";
    }
    if (kind == "signal_morpher") {
        return "Validate finite outputs, gain bounds, impulse response, step response and spectral distortion against identity and tanh baselines.";
    }
    if (kind == "adaptive_sorter") {
        return "Validate sortedness, stability on equal values, finite output and comparison against Python sorted.";
    }
    if (kind == "graph_pathfinder") {
        return "Validate path feasibility, cost consistency and comparison against a Dijkstra baseline on small graphs.";
    }
    if (kind == "schedule_optimizer") {
        return "Validate every job assigned exactly once, finite loads and makespan against round-robin and greedy baselines.";
    }
    if (kind == "parser_repairer") {
        return "Validate malformed input containment, delimiter repair and bounded parse tree output.";
    }
    if (kind == "constraint_solver") {
        return "Validate returned assignments satisfy all constraints and failure is bounded under gas exhaustion.";
    }
    if (kind == "stream_processor") {
        return "Validate online statistics against batch statistics and anomaly-score finiteness.";
    }
    if (kind == "adaptive_compressor") {
        return "Validate decode(encode(x)) approximately reconstructs x and reports compression ratio.";
    }
    if (kind == "cache_policy") {
        return "Validate deterministic hits/misses and capacity invariant.";
    }
    if (kind == "load_balancer") {
        return "Validate assignment cardinality, load conservation and finite fairness score.";
    }
    if (kind == "anomaly_detector") {
        return "Validate finite scores, deterministic anomaly indices and robustness to NaN/Inf input.";
    }
    return "Validate CPU/Python/C++/OpenCL outputs on a shared sample grid and enforce finite outputs under NaN, Inf and gas exhaustion.";
}

void write_python_algorithm(const AlgorithmArtifact& a, const std::filesystem::path& p, int gas_limit) {
    const std::string expr = op_py_expr(a.kernel_expression);
    std::ofstream py(p);
    py << "\"\"\"Executable mathematical algorithm generated by Algorithmic Genesis.\n";
    py << "Name: " << a.name << "\nKind: " << a.kind << "\nDomain: " << a.domain << "\n";
    py << "This file is deterministic, gas-limited and numeric-poison-contained.\n\"\"\"\n";
    py << "from __future__ import annotations\nimport math\nfrom typing import Callable, Iterable\n\n";
    py << "DEFAULT_GAS = " << gas_limit << "\n\n";
    py << "def sanitize(v: float) -> float:\n";
    py << "    try:\n        x = float(v)\n    except Exception:\n        return 0.0\n";
    py << "    if not math.isfinite(x):\n        return 0.0\n";
    py << "    return max(-1_000_000.0, min(1_000_000.0, x))\n\n";
    py << "def expclamp(x: float) -> float:\n    return math.exp(max(-20.0, min(20.0, sanitize(x))))\n\n";
    py << "def logabs(x: float) -> float:\n    return math.log(abs(sanitize(x)) + 1e-9)\n\n";
    py << "def kernel(x: float, gas_left: int = DEFAULT_GAS) -> float:\n";
    py << "    if gas_left <= 0:\n        return 0.0\n";
    py << "    x = sanitize(x)\n";
    py << "    try:\n        return sanitize(" << expr << ")\n";
    py << "    except Exception:\n        return 0.0\n\n";
    py << "def transform_scalar(x: float, gas_left: int = DEFAULT_GAS) -> tuple[float, int]:\n";
    py << "    if gas_left <= 0:\n        return 0.0, 1\n";
    py << "    return kernel(x, gas_left - 1), 0\n\n";
    py << "def fixed_point(x0: float, iterations: int = 128, tol: float = 1e-9, gas_left: int = DEFAULT_GAS) -> tuple[float, int]:\n";
    py << "    x = sanitize(x0)\n    for _ in range(max(0, iterations)):\n        gas_left -= 1\n        if gas_left <= 0:\n            return x, 1\n        y = sanitize(0.65 * x + 0.35 * math.tanh(kernel(x, gas_left)))\n        if abs(y - x) <= tol:\n            return y, 0\n        x = y\n    return x, 0\n\n";
    py << "def signal_morph(signal: Iterable[float], gas_left: int = DEFAULT_GAS) -> tuple[list[float], int]:\n";
    py << "    out: list[float] = []\n    state = 0.0\n    for s in signal:\n        gas_left -= 1\n        if gas_left <= 0:\n            return out, 1\n        x = sanitize(float(s) + 0.25 * state)\n        y = sanitize(0.70 * float(s) + 0.30 * math.tanh(kernel(x, gas_left)))\n        state = sanitize(0.8 * state + 0.2 * y)\n        out.append(y)\n    return out, 0\n\n";
    py << "def predict_next(values: list[float], gas_left: int = DEFAULT_GAS) -> tuple[float, int]:\n";
    py << "    if len(values) < 2 or gas_left <= 0:\n        return 0.0, 1\n";
    py << "    a, b = sanitize(values[-2]), sanitize(values[-1])\n    d = sanitize(b - a)\n    return sanitize(b + d + 0.1 * math.tanh(kernel(d, gas_left - 1))), 0\n\n";
    py << "def explore_map(x0: float, iterations: int = 256, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    x = sanitize(x0)\n    orbit = []\n    lyap = 0.0\n    eps = 1e-5\n    for _ in range(max(0, iterations)):\n        gas_left -= 2\n        if gas_left <= 0:\n            return {'orbit': orbit, 'lyapunov_like': lyap / max(1, len(orbit))}, 1\n        x1 = sanitize(math.tanh(kernel(x, gas_left)))\n        xp = sanitize(math.tanh(kernel(x + eps, gas_left)))\n        lyap += math.log(abs((xp - x1) / eps) + 1e-9)\n        x = x1\n        orbit.append(x)\n    return {'orbit': orbit, 'lyapunov_like': lyap / max(1, len(orbit))}, 0\n\n";

    py << "ROOT_FEATURES = (\n"
          "    'x','fx','lo','hi','flo','fhi','mid','width','relpos',\n"
          "    'prev_x','prev_fx','prev2_x','prev2_fx',\n"
          "    'bracket_slope','local_slope','prev_slope','curvature',\n"
          "    'improvement','stagnation','trust','reject_rate',\n"
          "    'scale','nfx','nflo','nfhi','nwidth','sign_change',\n"
          "    'edge_balance','residual_ratio','width_ratio'\n"
          ")\n\n";
    py << "def _safe_div(a: float, b: float) -> float:\n"
          "    b = sanitize(b)\n"
          "    if abs(b) <= 1e-12:\n"
          "        return 0.0\n"
          "    return sanitize(a / b)\n\n";
    py << "def make_root_context(x: float, fx: float, lo: float, hi: float, flo: float, fhi: float,\n";
    py << "                      prev_x: float, prev_fx: float, prev2_x: float, prev2_fx: float,\n";
    py << "                      prev_best: float, best_r: float, stagnation: float, trust: float,\n";
    py << "                      reject_rate: float, initial_width: float) -> dict[str, float]:\n";
    py << "    lo = sanitize(lo); hi = sanitize(hi); x = sanitize(x); fx = sanitize(fx)\n";
    py << "    flo = sanitize(flo); fhi = sanitize(fhi)\n";
    py << "    width = abs(hi - lo) + 1e-12\n";
    py << "    mid = sanitize(0.5 * (lo + hi))\n";
    py << "    relpos = max(0.0, min(1.0, (x - lo) / width))\n";
    py << "    bracket_slope = _safe_div(fhi - flo, width)\n";
    py << "    local_slope = _safe_div(fx - prev_fx, x - prev_x)\n";
    py << "    prev_slope = _safe_div(prev_fx - prev2_fx, prev_x - prev2_x)\n";
    py << "    curvature = sanitize(local_slope - prev_slope)\n";
    py << "    improvement = sanitize(prev_best - best_r)\n";
    py << "    scale = abs(flo) + abs(fhi) + abs(fx) + 1e-9\n";
    py << "    nfx = sanitize(fx / scale)\n";
    py << "    nflo = sanitize(flo / scale)\n";
    py << "    nfhi = sanitize(fhi / scale)\n";
    py << "    nwidth = sanitize(math.log1p(width))\n";
    py << "    sign_change = -1.0 if flo * fhi <= 0.0 else 1.0\n";
    py << "    edge_balance = sanitize((abs(flo) - abs(fhi)) / (abs(flo) + abs(fhi) + 1e-9))\n";
    py << "    residual_ratio = sanitize(abs(fx) / (min(abs(flo), abs(fhi)) + 1e-9))\n";
    py << "    width_ratio = sanitize(width / (abs(initial_width) + 1e-12))\n";
    py << "    return {\n";
    py << "        'x': x, 'fx': fx, 'lo': lo, 'hi': hi, 'flo': flo, 'fhi': fhi,\n";
    py << "        'mid': mid, 'width': width, 'relpos': relpos,\n";
    py << "        'prev_x': sanitize(prev_x), 'prev_fx': sanitize(prev_fx),\n";
    py << "        'prev2_x': sanitize(prev2_x), 'prev2_fx': sanitize(prev2_fx),\n";
    py << "        'bracket_slope': bracket_slope, 'local_slope': local_slope,\n";
    py << "        'prev_slope': prev_slope, 'curvature': curvature,\n";
    py << "        'improvement': improvement, 'stagnation': sanitize(stagnation),\n";
    py << "        'trust': max(0.0, min(1.0, sanitize(trust))),\n";
    py << "        'reject_rate': max(0.0, min(1.0, sanitize(reject_rate))),\n";
    py << "        'scale': scale, 'nfx': nfx, 'nflo': nflo, 'nfhi': nfhi,\n";
    py << "        'nwidth': nwidth, 'sign_change': sign_change,\n";
    py << "        'edge_balance': edge_balance, 'residual_ratio': residual_ratio,\n";
    py << "        'width_ratio': width_ratio,\n";
    py << "    }\n\n";
    py << "def context_kernel(x: float, fx: float, flo: float, fhi: float, history: float, width: float, gas_left: int = DEFAULT_GAS) -> float:\n";
    py << "    # Backward-compatible scalar contextual primitive.\n";
    py << "    if gas_left <= 0:\n        return 0.0\n";
    py << "    x = sanitize(x); fx = sanitize(fx); flo = sanitize(flo); fhi = sanitize(fhi)\n";
    py << "    history = sanitize(history); width = abs(sanitize(width)) + 1e-12\n";
    py << "    kx = kernel(x, gas_left - 1)\n";
    py << "    kfx = kernel(0.25 * fx, gas_left - 1)\n";
    py << "    kedge = kernel(0.5 * (flo + fhi), gas_left - 1)\n";
    py << "    slope_hint = sanitize((fhi - flo) / width)\n";
    py << "    residual_pressure = sanitize(math.tanh(fx) - 0.5 * math.tanh(flo + fhi))\n";
    py << "    return sanitize(0.42 * kx + 0.22 * kfx - 0.16 * kedge + 0.12 * history + 0.08 * math.tanh(slope_hint) + residual_pressure)\n\n";
    py << "def root_policy(ctx: dict[str, float], gas_left: int = DEFAULT_GAS) -> dict[str, float]:\n";
    py << "    # High-dimensional generated root policy.\n";
    py << "    # The discovered scalar kernel is evaluated on many normalized context projections.\n";
    py << "    # Outputs are bounded controls for a safe candidate mixer, not direct roots.\n";
    py << "    if gas_left <= 0:\n";
    py << "        return {'bias': 0.0, 'damping': 0.0, 'secant_mix': 0.0, 'bisection_mix': 1.0, 'relaxation_mix': 0.0, 'trust_delta': -0.1}\n";
    py << "    kx = kernel(ctx['x'], gas_left - 1)\n";
    py << "    kres = kernel(ctx['nfx'] + 0.25 * ctx['edge_balance'], gas_left - 1)\n";
    py << "    kslope = kernel(math.tanh(ctx['bracket_slope']) + 0.5 * math.tanh(ctx['local_slope']), gas_left - 1)\n";
    py << "    kcurve = kernel(math.tanh(ctx['curvature']) - 0.2 * ctx['stagnation'], gas_left - 1)\n";
    py << "    ktrust = kernel(ctx['trust'] - ctx['reject_rate'] + ctx['improvement'], gas_left - 1)\n";
    py << "    instability = max(0.0, min(1.0, abs(math.tanh(ctx['curvature'])) + ctx['reject_rate'] + 0.25 * ctx['stagnation']))\n";
    py << "    bias = math.tanh(0.30 * kx + 0.30 * kres + 0.20 * kslope - 0.10 * kcurve + 0.10 * ktrust)\n";
    py << "    damping = max(0.02, min(1.0, 0.65 + 0.25 * math.tanh(ktrust) - 0.45 * instability))\n";
    py << "    secant_mix = max(0.0, min(1.0, 0.35 + 0.25 * math.tanh(kslope) + 0.20 * ctx['trust'] - 0.35 * instability))\n";
    py << "    relaxation_mix = max(0.0, min(1.0, 0.30 + 0.25 * math.tanh(kres + kcurve) + 0.20 * ctx['trust'] - 0.25 * ctx['reject_rate']))\n";
    py << "    bisection_mix = max(0.10, min(1.0, 1.0 - 0.5 * secant_mix - 0.4 * relaxation_mix + 0.45 * instability))\n";
    py << "    trust_delta = sanitize(0.05 * math.tanh(ktrust + ctx['improvement']) - 0.08 * ctx['reject_rate'] - 0.03 * ctx['stagnation'])\n";
    py << "    return {\n";
    py << "        'bias': sanitize(bias), 'damping': sanitize(damping),\n";
    py << "        'secant_mix': sanitize(secant_mix), 'bisection_mix': sanitize(bisection_mix),\n";
    py << "        'relaxation_mix': sanitize(relaxation_mix), 'trust_delta': trust_delta,\n";
    py << "    }\n\n";
    py << "def _candidate_mixer(ctx: dict[str, float], policy: dict[str, float]) -> float:\n";
    py << "    lo, hi, width, mid = ctx['lo'], ctx['hi'], ctx['width'], ctx['mid']\n";
    py << "    secant = mid\n";
    py << "    if abs(ctx['bracket_slope']) > 1e-12:\n";
    py << "        secant = ctx['x'] - ctx['fx'] / ctx['bracket_slope']\n";
    py << "    learned = mid - 0.5 * width * policy['damping'] * math.tanh(policy['bias'])\n";
    py << "    wb = policy['bisection_mix']; ws = policy['secant_mix']; wr = policy['relaxation_mix']\n";
    py << "    z = wb + ws + wr + 1e-12\n";
    py << "    cand = (wb * mid + ws * secant + wr * learned) / z\n";
    py << "    return min(max(sanitize(cand), lo), hi)\n\n";
    py << "def root_refine(f: Callable[[float], float], lo: float, hi: float, iterations: int = 128, gas_left: int = DEFAULT_GAS) -> tuple[float, int]:\n";
    py << "    lo, hi = sanitize(lo), sanitize(hi)\n";
    py << "    if lo == hi:\n        return lo, 2\n";
    py << "    if lo > hi:\n        lo, hi = hi, lo\n";
    py << "    initial_width = abs(hi - lo) + 1e-12\n";
    py << "    flo, fhi = sanitize(f(lo)), sanitize(f(hi))\n";
    py << "    if flo == 0.0:\n        return lo, 0\n";
    py << "    if fhi == 0.0:\n        return hi, 0\n";
    py << "    if flo * fhi > 0.0:\n        return sanitize(0.5 * (lo + hi)), 3\n";
    py << "    best_x, best_r = (lo, abs(flo)) if abs(flo) <= abs(fhi) else (hi, abs(fhi))\n";
    py << "    prev_x = prev2_x = 0.5 * (lo + hi)\n";
    py << "    prev_fx = prev2_fx = sanitize(f(prev_x))\n";
    py << "    trust = 0.35\n    reject_rate = 0.0\n    stagnation = 0.0\n";
    py << "    for _ in range(max(0, iterations)):\n";
    py << "        gas_left -= 8\n";
    py << "        if gas_left <= 0:\n            return best_x, 1\n";
    py << "        mid = sanitize(0.5 * (lo + hi))\n";
    py << "        fm = sanitize(f(mid))\n";
    py << "        old_best = best_r\n";
    py << "        ctx = make_root_context(mid, fm, lo, hi, flo, fhi, prev_x, prev_fx, prev2_x, prev2_fx, old_best, best_r, stagnation, trust, reject_rate, initial_width)\n";
    py << "        policy = root_policy(ctx, gas_left)\n";
    py << "        cand = _candidate_mixer(ctx, policy)\n";
    py << "        if abs(cand - lo) < 1e-15 or abs(cand - hi) < 1e-15 or not math.isfinite(cand):\n";
    py << "            cand = mid\n";
    py << "        fc = sanitize(f(cand))\n";
    py << "        rejected = False\n";
    py << "        if abs(fm) < abs(fc) or not math.isfinite(fc):\n";
    py << "            cand, fc = mid, fm\n";
    py << "            rejected = True\n";
    py << "        if abs(fc) < best_r:\n";
    py << "            best_x, best_r = cand, abs(fc)\n";
    py << "        improvement = old_best - best_r\n";
    py << "        stagnation = 0.0 if improvement > max(1e-14, 1e-6 * old_best) else min(64.0, stagnation + 1.0)\n";
    py << "        reject_rate = sanitize(0.90 * reject_rate + 0.10 * (1.0 if rejected else 0.0))\n";
    py << "        trust = max(0.0, min(1.0, trust + policy['trust_delta'] + (0.025 if improvement > 0 else -0.015)))\n";
    py << "        prev2_x, prev2_fx = prev_x, prev_fx\n";
    py << "        prev_x, prev_fx = cand, fc\n";
    py << "        if fc == 0.0 or abs(hi - lo) <= 1e-12:\n            return cand, 0\n";
    py << "        if (flo < 0 and fc > 0) or (flo > 0 and fc < 0):\n";
    py << "            hi, fhi = cand, fc\n";
    py << "        else:\n";
    py << "            lo, flo = cand, fc\n";
    py << "    return best_x, 0\n\n";
    py << "def adaptive_sort(values: Iterable[float], gas_left: int = DEFAULT_GAS) -> tuple[list[float], int]:\n";
    py << "    records = [(sanitize(v), i) for i, v in enumerate(values)]\n";
    py << "    if gas_left <= 0:\n        return [v for v, _ in records], 1\n";
    py << "    def learned_key(item):\n        v, i = item\n        return (v, 1e-9 * math.tanh(kernel(v + 0.001 * i, gas_left - 1)), i)\n";
    py << "    out = [v for v, _ in sorted(records, key=learned_key)]\n";
    py << "    if any(out[i] > out[i+1] for i in range(len(out)-1)):\n        out = sorted(out)\n";
    py << "    return out, 0\n\n";
    py << "def graph_shortest_path(graph: dict, start, goal, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    import heapq\n";
    py << "    dist = {start: 0.0}; parent = {}; heap = [(0.0, start)]\n";
    py << "    seen = set()\n";
    py << "    while heap:\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0:\n            return {'path': [], 'cost': math.inf, 'reached': False}, 1\n";
    py << "        cost, node = heapq.heappop(heap)\n";
    py << "        if node in seen:\n            continue\n";
    py << "        seen.add(node)\n";
    py << "        if node == goal:\n";
    py << "            path = [node]\n";
    py << "            while path[-1] in parent:\n                path.append(parent[path[-1]])\n";
    py << "            path.reverse()\n";
    py << "            return {'path': path, 'cost': sanitize(cost), 'reached': True}, 0\n";
    py << "        for nxt, w in graph.get(node, []):\n";
    py << "            ww = abs(sanitize(w))\n";
    py << "            priority_bias = 1e-6 * math.tanh(kernel(ww + len(seen), gas_left - 1))\n";
    py << "            nc = cost + ww\n";
    py << "            if nc < dist.get(nxt, math.inf):\n";
    py << "                dist[nxt] = nc; parent[nxt] = node\n";
    py << "                heapq.heappush(heap, (nc + priority_bias, nxt))\n";
    py << "    return {'path': [], 'cost': math.inf, 'reached': False}, 0\n\n";
    py << "def schedule_jobs(jobs: Iterable[float], workers: int = 2, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    workers = max(1, int(workers)); loads = [0.0] * workers; assignment = []\n";
    py << "    ordered = [(sanitize(j), i) for i, j in enumerate(jobs)]\n";
    py << "    ordered.sort(key=lambda p: -(abs(p[0]) + 0.01 * math.tanh(kernel(p[0], gas_left - 1))))\n";
    py << "    for cost, idx in ordered:\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0:\n            return {'assignment': assignment, 'loads': loads, 'makespan': max(loads) if loads else 0.0}, 1\n";
    py << "        best = min(range(workers), key=lambda w: loads[w] + cost + 0.01 * math.tanh(kernel(loads[w] - cost, gas_left - 1)))\n";
    py << "        loads[best] = sanitize(loads[best] + abs(cost)); assignment.append((idx, best))\n";
    py << "    return {'assignment': sorted(assignment), 'loads': loads, 'makespan': max(loads), 'fairness': max(loads)-min(loads)}, 0\n\n";
    py << "def parse_and_repair(tokens: Iterable[str], gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    pairs = {')': '(', ']': '[', '}': '{'}; openers = set(pairs.values())\n";
    py << "    stack = []; repairs = []; out = []\n";
    py << "    for tok in tokens:\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0:\n            return {'balanced': False, 'repairs': repairs, 'tokens': out}, 1\n";
    py << "        if tok in openers:\n            stack.append(tok); out.append(tok)\n";
    py << "        elif tok in pairs:\n            if stack and stack[-1] == pairs[tok]:\n                stack.pop(); out.append(tok)\n";
    py << "            else:\n                repairs.append('insert_' + pairs[tok]); out.append(pairs[tok]); out.append(tok)\n";
    py << "        else:\n            out.append(tok)\n";
    py << "    while stack:\n        repairs.append('close_' + stack.pop())\n";
    py << "    return {'balanced': len(repairs) == 0, 'repairs': repairs, 'tokens': out}, 0\n\n";
    py << "def solve_constraints(domains: dict, constraints: list, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    vars_ = list(domains.keys())\n";
    py << "    vars_.sort(key=lambda k: (len(domains[k]), math.tanh(kernel(len(domains[k]), gas_left - 1))))\n";
    py << "    assign = {}\n";
    py << "    def ok_partial():\n";
    py << "        try:\n            return all(c(assign) for c in constraints)\n";
    py << "        except KeyError:\n            return True\n";
    py << "    def bt(i, gas):\n";
    py << "        if gas <= 0: return False, gas\n";
    py << "        if i == len(vars_): return all(c(assign) for c in constraints), gas\n";
    py << "        v = vars_[i]\n";
    py << "        vals = list(domains[v]); vals.sort(key=lambda x: math.tanh(kernel(sanitize(x), gas - 1)))\n";
    py << "        for val in vals:\n";
    py << "            assign[v] = val\n";
    py << "            if ok_partial():\n";
    py << "                done, gas = bt(i+1, gas-1)\n";
    py << "                if done: return True, gas\n";
    py << "            assign.pop(v, None)\n";
    py << "        return False, gas\n";
    py << "    done, gas_left = bt(0, gas_left)\n";
    py << "    return ({'assignment': dict(assign), 'satisfied': bool(done)}, 0 if done else 4)\n\n";
    py << "def stream_analyze(values: Iterable[float], gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    n = 0; mean = 0.0; m2 = 0.0; anomalies = []\n";
    py << "    for i, raw in enumerate(values):\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0:\n            return {'count': n, 'mean': mean, 'variance': m2 / max(1, n-1), 'anomalies': anomalies}, 1\n";
    py << "        x = sanitize(raw); n += 1\n";
    py << "        delta = x - mean; mean += delta / n; m2 += delta * (x - mean)\n";
    py << "        std = math.sqrt(max(1e-12, m2 / max(1, n-1)))\n";
    py << "        score = abs(x - mean) / std + abs(math.tanh(kernel(x - mean, gas_left - 1)))\n";
    py << "        if n > 4 and score > 3.0:\n            anomalies.append((i, sanitize(score)))\n";
    py << "    return {'count': n, 'mean': mean, 'variance': m2 / max(1, n-1), 'anomalies': anomalies}, 0\n\n";
    py << "def compress_adaptive(values: Iterable[float], gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    vals = [sanitize(v) for v in values]\n";
    py << "    if not vals: return {'packets': [], 'decoded': [], 'ratio': 1.0}, 0\n";
    py << "    packets = []; decoded = [] ; i = 0\n";
    py << "    while i < len(vals):\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0: return {'packets': packets, 'decoded': decoded, 'ratio': 1.0}, 1\n";
    py << "        base = vals[i]; run = 1; threshold = 1e-9 + 0.01 * abs(math.tanh(kernel(base, gas_left - 1)))\n";
    py << "        while i + run < len(vals) and abs(vals[i+run] - base) <= threshold:\n            run += 1\n";
    py << "        packets.append(('run', base, run)); decoded.extend([base] * run); i += run\n";
    py << "    raw = max(1, len(vals)); enc = max(1, len(packets) * 3)\n";
    py << "    return {'packets': packets, 'decoded': decoded, 'ratio': enc / raw}, 0\n\n";
    py << "def cache_simulate(requests: Iterable, capacity: int = 4, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    capacity = max(1, int(capacity)); cache = {}; hits = 0; misses = 0\n";
    py << "    for t, r in enumerate(requests):\n";
    py << "        gas_left -= 1\n";
    py << "        if gas_left <= 0: return {'hits': hits, 'misses': misses, 'cache': list(cache.keys())}, 1\n";
    py << "        if r in cache:\n            hits += 1; cache[r]['freq'] += 1; cache[r]['last'] = t\n";
    py << "        else:\n            misses += 1\n";
    py << "            if len(cache) >= capacity:\n";
    py << "                victim = min(cache, key=lambda k: cache[k]['freq'] * 0.4 + cache[k]['last'] * 0.001 + 0.01 * math.tanh(kernel(cache[k]['freq'], gas_left - 1)))\n";
    py << "                cache.pop(victim, None)\n";
    py << "            cache[r] = {'freq': 1, 'last': t}\n";
    py << "    return {'hits': hits, 'misses': misses, 'cache': list(cache.keys())}, 0\n\n";
    py << "def load_balance(jobs: Iterable[float], workers: int = 2, gas_left: int = DEFAULT_GAS) -> tuple[dict, int]:\n";
    py << "    return schedule_jobs(jobs, workers, gas_left)\n\n";
    py << "def detect_anomalies(values: Iterable[float], gas_left: int = DEFAULT_GAS) -> tuple[list[int], int]:\n";
    py << "    stats, err = stream_analyze(values, gas_left)\n";
    py << "    return [i for i, _ in stats.get('anomalies', [])], err\n\n";    py << "def run_demo() -> None:\n";
    py << "    print('algorithm:', '" << a.name << "')\n";
    py << "    print('kernel(0.5)=', kernel(0.5))\n";
    py << "    print('fixed_point=', fixed_point(0.25)[0])\n";
    py << "    print('predict_next=', predict_next([0.0, 0.5, 0.75])[0])\n\n";
    py << "if __name__ == '__main__':\n    run_demo()\n";
}

void write_cpp_algorithm(const AlgorithmArtifact& a, const std::filesystem::path& p, int gas_limit) {
    const std::string expr = op_cpp_expr(a.kernel_expression);
    std::ofstream h(p);
    h << "#pragma once\n#include <algorithm>\n#include <cmath>\n#include <cstddef>\n\n";
    h << "namespace " << safe_symbol(a.name) << " {\n";
    h << "constexpr int DEFAULT_GAS = " << gas_limit << ";\n";
    h << "inline double sanitize(double v){ return std::isfinite(v) ? std::clamp(v, -1000000.0, 1000000.0) : 0.0; }\n";
    h << "inline double ag_expclamp(double x){ return std::exp(std::clamp(sanitize(x), -20.0, 20.0)); }\n";
    h << "inline double ag_logabs(double x){ return std::log(std::abs(sanitize(x)) + 1e-9); }\n";
    h << "inline double kernel(double x){ x = sanitize(x); return sanitize(" << expr << "); }\n";
    h << "inline double context_kernel(double x, double fx, double flo, double fhi, double history, double width){\n";
    h << "    x=sanitize(x); fx=sanitize(fx); flo=sanitize(flo); fhi=sanitize(fhi); history=sanitize(history); width=std::abs(sanitize(width))+1e-12;\n";
    h << "    const double kx=kernel(x); const double kfx=kernel(0.25*fx); const double kedge=kernel(0.5*(flo+fhi));\n";
    h << "    const double slope_hint=sanitize((fhi-flo)/width); const double residual_pressure=sanitize(std::tanh(fx)-0.5*std::tanh(flo+fhi));\n";
    h << "    return sanitize(0.42*kx + 0.22*kfx - 0.16*kedge + 0.12*history + 0.08*std::tanh(slope_hint) + residual_pressure);\n";
    h << "}\n";
    h << "struct RootContext{ double x,fx,lo,hi,flo,fhi,mid,width,relpos,prev_x,prev_fx,prev2_x,prev2_fx,bracket_slope,local_slope,prev_slope,curvature,improvement,stagnation,trust,reject_rate,scale,nfx,nflo,nfhi,nwidth,sign_change,edge_balance,residual_ratio,width_ratio; };\n";
    h << "struct RootPolicy{ double bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta; };\n";
    h << "inline double safe_div(double a,double b){ return std::abs(b)<=1e-12 ? 0.0 : sanitize(a/b); }\n";
    h << "inline RootContext make_root_context(double x,double fx,double lo,double hi,double flo,double fhi,double prev_x,double prev_fx,double prev2_x,double prev2_fx,double prev_best,double best_r,double stagnation,double trust,double reject_rate,double initial_width){\n";
    h << "    RootContext c{}; c.x=sanitize(x); c.fx=sanitize(fx); c.lo=sanitize(lo); c.hi=sanitize(hi); c.flo=sanitize(flo); c.fhi=sanitize(fhi); c.width=std::abs(c.hi-c.lo)+1e-12; c.mid=sanitize(0.5*(c.lo+c.hi)); c.relpos=std::clamp((c.x-c.lo)/c.width,0.0,1.0); c.prev_x=sanitize(prev_x); c.prev_fx=sanitize(prev_fx); c.prev2_x=sanitize(prev2_x); c.prev2_fx=sanitize(prev2_fx); c.bracket_slope=safe_div(c.fhi-c.flo,c.width); c.local_slope=safe_div(c.fx-c.prev_fx,c.x-c.prev_x); c.prev_slope=safe_div(c.prev_fx-c.prev2_fx,c.prev_x-c.prev2_x); c.curvature=sanitize(c.local_slope-c.prev_slope); c.improvement=sanitize(prev_best-best_r); c.stagnation=sanitize(stagnation); c.trust=std::clamp(sanitize(trust),0.0,1.0); c.reject_rate=std::clamp(sanitize(reject_rate),0.0,1.0); c.scale=std::abs(c.flo)+std::abs(c.fhi)+std::abs(c.fx)+1e-9; c.nfx=sanitize(c.fx/c.scale); c.nflo=sanitize(c.flo/c.scale); c.nfhi=sanitize(c.fhi/c.scale); c.nwidth=sanitize(std::log1p(c.width)); c.sign_change=(c.flo*c.fhi<=0.0)?-1.0:1.0; c.edge_balance=sanitize((std::abs(c.flo)-std::abs(c.fhi))/(std::abs(c.flo)+std::abs(c.fhi)+1e-9)); c.residual_ratio=sanitize(std::abs(c.fx)/(std::min(std::abs(c.flo),std::abs(c.fhi))+1e-9)); c.width_ratio=sanitize(c.width/(std::abs(initial_width)+1e-12)); return c; }\n";
    h << "inline RootPolicy root_policy(const RootContext& c){ const double kx=kernel(c.x); const double kres=kernel(c.nfx+0.25*c.edge_balance); const double kslope=kernel(std::tanh(c.bracket_slope)+0.5*std::tanh(c.local_slope)); const double kcurve=kernel(std::tanh(c.curvature)-0.2*c.stagnation); const double ktrust=kernel(c.trust-c.reject_rate+c.improvement); const double instability=std::clamp(std::abs(std::tanh(c.curvature))+c.reject_rate+0.25*c.stagnation,0.0,1.0); RootPolicy p{}; p.bias=sanitize(std::tanh(0.30*kx+0.30*kres+0.20*kslope-0.10*kcurve+0.10*ktrust)); p.damping=std::clamp(0.65+0.25*std::tanh(ktrust)-0.45*instability,0.02,1.0); p.secant_mix=std::clamp(0.35+0.25*std::tanh(kslope)+0.20*c.trust-0.35*instability,0.0,1.0); p.relaxation_mix=std::clamp(0.30+0.25*std::tanh(kres+kcurve)+0.20*c.trust-0.25*c.reject_rate,0.0,1.0); p.bisection_mix=std::clamp(1.0-0.5*p.secant_mix-0.4*p.relaxation_mix+0.45*instability,0.10,1.0); p.trust_delta=sanitize(0.05*std::tanh(ktrust+c.improvement)-0.08*c.reject_rate-0.03*c.stagnation); return p; }\n";
    h << "inline double fixed_point(double x0, int iterations, int* gas_left, int* error_code){\n";
    h << "    if(!gas_left || !error_code){ return 0.0; }\n    double x=sanitize(x0); *error_code=0;\n";
    h << "    for(int i=0;i<iterations;++i){ if(--(*gas_left)<=0){*error_code=1; return x;} double y=sanitize(0.65*x+0.35*std::tanh(kernel(x))); if(std::abs(y-x)<1e-9) return y; x=y; }\n";
    h << "    return x;\n}\n";
    h << "inline std::size_t signal_morph(const double* input, double* output, std::size_t n, int* gas_left, int* error_code){\n";
    h << "    if(!input || !output || !gas_left || !error_code){ return 0; } *error_code=0; double state=0.0;\n";
    h << "    for(std::size_t i=0;i<n;++i){ if(--(*gas_left)<=0){*error_code=1; return i;} const double x=sanitize(input[i]+0.25*state); const double y=sanitize(0.70*input[i]+0.30*std::tanh(kernel(x))); state=sanitize(0.8*state+0.2*y); output[i]=y; }\n";
    h << "    return n;\n}\n";
    h << "} // namespace " << safe_symbol(a.name) << "\n\n";
    h << "extern \"C\" inline double evaluate_safely(const double* inputs, int* gas_left, int* error_code){\n";
    h << "    if(!inputs || !gas_left || !error_code){ return 0.0; }\n";
    h << "    if(*gas_left <= 0){ *error_code = 1; return 0.0; }\n";
    h << "    --(*gas_left); *error_code = 0; return " << safe_symbol(a.name) << "::kernel(inputs[0]);\n";
    h << "}\n";
}

void write_opencl_algorithm(const AlgorithmArtifact& a, const std::filesystem::path& p) {
    const std::string expr = op_cl_expr(a.kernel_expression);
    std::ofstream cl(p);
    cl << "inline float ag_sanitize(float v){ uint i=as_uint(v); return ((i & 0x7F800000u)==0x7F800000u) ? 0.0f : clamp(v, -1000000.0f, 1000000.0f); }\n";
    cl << "inline float ag_expclamp(float x){ return exp(clamp(ag_sanitize(x), -20.0f, 20.0f)); }\n";
    cl << "inline float ag_logabs(float x){ return log(fabs(ag_sanitize(x)) + 1e-9f); }\n";
    cl << "inline float kernel(float x){ x=ag_sanitize(x); return ag_sanitize((float)(" << expr << ")); }\n";
    cl << "inline float context_kernel(float x, float fx, float flo, float fhi, float history, float width){\n";
    cl << "    x=ag_sanitize(x); fx=ag_sanitize(fx); flo=ag_sanitize(flo); fhi=ag_sanitize(fhi); history=ag_sanitize(history); width=fabs(ag_sanitize(width))+1e-12f;\n";
    cl << "    float kx=kernel(x); float kfx=kernel(0.25f*fx); float kedge=kernel(0.5f*(flo+fhi));\n";
    cl << "    float slope_hint=ag_sanitize((fhi-flo)/width); float residual_pressure=ag_sanitize(tanh(fx)-0.5f*tanh(flo+fhi));\n";
    cl << "    return ag_sanitize(0.42f*kx + 0.22f*kfx - 0.16f*kedge + 0.12f*history + 0.08f*tanh(slope_hint) + residual_pressure);\n";
    cl << "}\n";
    cl << "typedef struct{ float x,fx,lo,hi,flo,fhi,mid,width,relpos,prev_x,prev_fx,prev2_x,prev2_fx,bracket_slope,local_slope,prev_slope,curvature,improvement,stagnation,trust,reject_rate,scale,nfx,nflo,nfhi,nwidth,sign_change,edge_balance,residual_ratio,width_ratio; } RootContext;\n";
    cl << "typedef struct{ float bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta; } RootPolicy;\n";
    cl << "inline float ag_safe_div(float a,float b){ return fabs(b)<=1e-12f ? 0.0f : ag_sanitize(a/b); }\n";
    cl << "inline RootPolicy root_policy(RootContext c){ float kx=kernel(c.x); float kres=kernel(c.nfx+0.25f*c.edge_balance); float kslope=kernel(tanh(c.bracket_slope)+0.5f*tanh(c.local_slope)); float kcurve=kernel(tanh(c.curvature)-0.2f*c.stagnation); float ktrust=kernel(c.trust-c.reject_rate+c.improvement); float instability=clamp(fabs(tanh(c.curvature))+c.reject_rate+0.25f*c.stagnation,0.0f,1.0f); RootPolicy p; p.bias=ag_sanitize(tanh(0.30f*kx+0.30f*kres+0.20f*kslope-0.10f*kcurve+0.10f*ktrust)); p.damping=clamp(0.65f+0.25f*tanh(ktrust)-0.45f*instability,0.02f,1.0f); p.secant_mix=clamp(0.35f+0.25f*tanh(kslope)+0.20f*c.trust-0.35f*instability,0.0f,1.0f); p.relaxation_mix=clamp(0.30f+0.25f*tanh(kres+kcurve)+0.20f*c.trust-0.25f*c.reject_rate,0.0f,1.0f); p.bisection_mix=clamp(1.0f-0.5f*p.secant_mix-0.4f*p.relaxation_mix+0.45f*instability,0.10f,1.0f); p.trust_delta=ag_sanitize(0.05f*tanh(ktrust+c.improvement)-0.08f*c.reject_rate-0.03f*c.stagnation); return p; }\n";
    cl << "__kernel void transform_scalar(__global const float* xs, __global float* ys, const uint n){ uint i=get_global_id(0); if(i<n){ ys[i]=kernel(xs[i]); }}\n";
    cl << "__kernel void fixed_point_batch(__global const float* x0, __global float* out, const uint n, const uint iterations){ uint i=get_global_id(0); if(i<n){ float x=ag_sanitize(x0[i]); for(uint k=0;k<iterations;++k){ x=ag_sanitize(0.65f*x + 0.35f*tanh(kernel(x))); } out[i]=x; }}\n";
}

void write_manifest(const AlgorithmArtifact& a, const std::filesystem::path& p) {
    std::ofstream m(p);
    m << algorithm_artifact_to_json(a) << "\n";
}

void write_readme(const AlgorithmArtifact& a, const std::filesystem::path& p) {
    std::ofstream md(p);
    md << "# " << a.name << "\n\n";
    md << "## Purpose\n\n" << a.algorithm_summary << "\n\n";
    md << "## Mathematical Kernel\n\n`K(x) = " << a.kernel_expression << "`\n\n";
    if (a.kind == "root_refiner") {
        md << "## Contextual Update Kernel\n\n";
        md << "The exported root-finder does not use only `K(x)`. It builds a high-dimensional `RootContext` with bracket geometry, residuals, secant/local slopes, curvature, improvement, stagnation, trust, reject-rate and normalized scale features. `root_policy(ctx)` emits `bias`, `damping`, `secant_mix`, `bisection_mix`, `relaxation_mix` and `trust_delta`; a safe candidate mixer combines bisection, secant and learned relaxation while preserving the bracket invariant.\n\n";
    }
    md << "## Contract\n\n" << a.mathematical_contract << "\n\n";
    md << "## Pseudocode\n\n```text\n" << a.pseudocode << "\n```\n\n";
    md << "## Complexity\n\n" << a.complexity << "\n\n";
    md << "## Validation\n\n" << a.validation_protocol << "\n\n";
    md << "## Exported Files\n\n";
    for (const auto& f : a.export_files) md << "- `" << f << "`\n";
}

void write_test_script(const AlgorithmArtifact& a, const std::filesystem::path& p) {
    std::ofstream t(p);
    const std::string mod = a.name + "_algorithm";
    t << "import math\n";
    t << "import " << mod << " as alg\n\n";
    t << "def test_finite_kernel():\n";
    t << "    for x in [-10,-1,0,0.5,1,10,float('nan'),float('inf')]:\n";
    t << "        y = alg.kernel(x)\n";
    t << "        assert math.isfinite(y)\n\n";
    t << "def test_gas_limit():\n";
    t << "    y, err = alg.transform_scalar(0.5, 0)\n";
    t << "    assert err == 1\n\n";
    t << "def test_algorithm_smoke():\n";
    t << "    y, err = alg.fixed_point(0.25, iterations=16)\n";
    t << "    assert math.isfinite(y)\n";
    t << "    ys, err = alg.signal_morph([0.0, 0.5, 1.0])\n";
    t << "    assert len(ys) == 3\n";
    t << "    assert all(math.isfinite(v) for v in ys)\n";
    t << "    z, err = alg.predict_next([0.0, 0.5, 0.75])\n";
    t << "    assert math.isfinite(z)\n";
    t << "    r, err = alg.root_refine(lambda x: x*x - 2.0, 0.0, 2.0, iterations=64)\n";
    t << "    assert err in (0, 1)\n";
    t << "    assert 0.0 <= r <= 2.0\n";
    t << "    assert abs(r*r - 2.0) < 1e-4\n";
    t << "    ctxv = alg.context_kernel(0.5, -0.2, -1.0, 1.0, 0.1, 2.0)\n";
    t << "    assert math.isfinite(ctxv)\n";
    t << "    ctx = alg.make_root_context(0.5, -0.2, 0.0, 2.0, -2.0, 2.0, 0.4, -0.3, 0.3, -0.4, 0.5, 0.2, 0.0, 0.5, 0.0, 2.0)\n";
    t << "    assert all(k in ctx for k in alg.ROOT_FEATURES)\n";
    t << "    pol = alg.root_policy(ctx)\n";
    t << "    assert all(math.isfinite(v) for v in pol.values())\n";
    t << "    sorted_vals, err = alg.adaptive_sort([3, 1, 2, 2])\n";
    t << "    assert sorted_vals == sorted(sorted_vals)\n";
    t << "    graph = {'a': [('b', 1.0), ('c', 5.0)], 'b': [('c', 1.0)], 'c': []}\n";
    t << "    path, err = alg.graph_shortest_path(graph, 'a', 'c')\n";
    t << "    assert path['reached'] and path['path'][0] == 'a' and path['path'][-1] == 'c'\n";
    t << "    sched, err = alg.schedule_jobs([3, 1, 2, 4], workers=2)\n";
    t << "    assert len(sched['assignment']) == 4 and len(sched['loads']) == 2\n";
    t << "    parsed, err = alg.parse_and_repair(['(', 'x', '+', '1', ']'])\n";
    t << "    assert 'tokens' in parsed and isinstance(parsed['repairs'], list)\n";
    t << "    sol, err = alg.solve_constraints({'x': [1,2,3], 'y': [1,2,3]}, [lambda a: 'x' not in a or 'y' not in a or a['x'] < a['y']])\n";
    t << "    assert sol['satisfied']\n";
    t << "    stream, err = alg.stream_analyze([1,1,1,10,1,1])\n";
    t << "    assert stream['count'] == 6 and math.isfinite(stream['mean'])\n";
    t << "    comp, err = alg.compress_adaptive([1,1,1,2,2])\n";
    t << "    assert len(comp['decoded']) == 5\n";
    t << "    cache, err = alg.cache_simulate(['a','b','a','c','a'], capacity=2)\n";
    t << "    assert cache['hits'] >= 1\n";
    t << "    lb, err = alg.load_balance([1,2,3], workers=2)\n";
    t << "    assert len(lb['loads']) == 2\n";
    t << "    idx, err = alg.detect_anomalies([0,0,0,10,0,0])\n";
    t << "    assert isinstance(idx, list)\n";
    t << "\nif __name__ == '__main__':\n";
    t << "    test_finite_kernel(); test_gas_limit(); test_algorithm_smoke(); print('ok')\n";
}

} // namespace

std::string choose_algorithm_kind(const std::string& domain, const std::string& requested) {
    if (!requested.empty() && requested != "auto") return requested;
    if (domain == "chaotic_maps") return "chaotic_map_explorer";
    if (domain == "root_finding") return "root_refiner";
    if (domain == "fixed_point_dynamics") return "fixed_point_iterator";
    if (domain == "signal_transform") return "signal_morpher";
    if (domain == "sequence_generation") return "sequence_extrapolator";
    if (domain == "symbolic_identity_search") return "identity_residual_reducer";
    if (domain == "sorting") return "adaptive_sorter";
    if (domain == "graph_shortest_path") return "graph_pathfinder";
    if (domain == "scheduling") return "schedule_optimizer";
    if (domain == "parsing") return "parser_repairer";
    if (domain == "constraint_solving") return "constraint_solver";
    if (domain == "stream_processing") return "stream_processor";
    if (domain == "compression") return "adaptive_compressor";
    if (domain == "cache_eviction") return "cache_policy";
    if (domain == "load_balancing") return "load_balancer";
    if (domain == "anomaly_detection") return "anomaly_detector";
    return "generated_scalar_transform";
}

double nontriviality_score(const Genome& genome) {
    if (genome.genes.empty()) return 0.0;
    const std::string expression = genome_to_expression(genome);
    const bool expression_uses_x = expression.find('x') != std::string::npos;
    if (!expression_uses_x) return std::min(0.20, std::clamp(genome.novelty, 0.0, 1.0) * 0.20);
    std::set<Op> ops;
    int constants = 0;
    int binary = 0;
    int unary = 0;
    int self_div = 0;
    int variable_refs = 0;
    for (const Gene& g : genome.genes) {
        ops.insert(g.op);
        if (g.op == Op::Const) ++constants;
        if (g.op == Op::Add || g.op == Op::Sub || g.op == Op::Mul || g.op == Op::SafeDiv) ++binary;
        if (g.op == Op::Sin || g.op == Op::Cos || g.op == Op::ExpClamp || g.op == Op::LogAbs || g.op == Op::Tanh) ++unary;
        if (g.op == Op::SafeDiv && g.a == g.b) ++self_div;
        if (g.op == Op::VarX) ++variable_refs;
    }
    const double n = static_cast<double>(genome.genes.size());
    double score = 0.0;
    score += std::min(1.0, static_cast<double>(ops.size()) / 7.0) * 0.25;
    score += std::min(1.0, static_cast<double>(binary) / std::max(1.0, n * 0.25)) * 0.20;
    score += std::min(1.0, static_cast<double>(unary) / std::max(1.0, n * 0.20)) * 0.15;
    score += std::min(1.0, static_cast<double>(variable_refs) / std::max(1.0, n * 0.10)) * 0.20;
    score += std::clamp(genome.novelty, 0.0, 1.0) * 0.20;
    score -= std::min(0.35, 0.08 * static_cast<double>(self_div));
    if (constants > static_cast<int>(genome.genes.size() * 0.60)) score -= 0.20;
    if (expression.find("/ sin(") != std::string::npos || expression.find("/ cos(") != std::string::npos || expression.find("/ tanh(") != std::string::npos) score -= 0.10;
    if (expression.size() < 8) score -= 0.20;

    // Behavior-level nontriviality: structurally large DAGs may still collapse to constants
    // such as (A - A). Penalize near-zero variance over a canonical probe grid.
    double sum = 0.0;
    double sum2 = 0.0;
    int count = 0;
    for (double x : {-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0}) {
        const EvalResult r = eval_genome(genome, x);
        const double y = (r.ok && std::isfinite(r.value)) ? std::clamp(r.value, -1000.0, 1000.0) : 0.0;
        sum += y;
        sum2 += y * y;
        ++count;
    }
    const double mean = sum / static_cast<double>(count);
    const double variance = std::max(0.0, sum2 / static_cast<double>(count) - mean * mean);
    if (variance < 1e-8) score = std::min(score, 0.25);
    else if (variance < 1e-4) score = std::min(score, 0.45);

    return std::clamp(score, 0.0, 1.0);
}

AlgorithmSynthesisReport synthesize_algorithm(const AlgorithmSynthesisConfig& cfg) {
    EvolutionConfig ecfg;
    ecfg.population = cfg.population;
    ecfg.generations = cfg.generations;
    ecfg.genome_length = cfg.genome_length;
    ecfg.seed = cfg.seed;
    ecfg.generator = cfg.generator;
    ecfg.archive_path = cfg.archive_path;
    ecfg.export_dir.clear(); // Algorithm export is richer than raw candidate export.
    ecfg.report_path.clear();
    ecfg.save_top = cfg.save_top;
    ecfg.fitness_backend = cfg.fitness_backend;
    ecfg.vm_differential = cfg.vm_differential || cfg.fitness_backend == "opencl-vm";
    ecfg.vm_local_size = 256;

    FitnessConfig fcfg;
    fcfg.domain = cfg.domain;
    fcfg.samples = cfg.samples;
    // Discovery mode: increase pressure against behavior collapse and trivial identities.
    fcfg.novelty_weight = std::max(fcfg.novelty_weight, 0.22);
    fcfg.complexity_weight = std::max(fcfg.complexity_weight, 0.12);

    EvolutionEngine engine(ecfg, fcfg);
    EvolutionReport ev = engine.run();

    Genome selected = ev.best;
    double best_score = -1.0;
    for (const Genome& g : ev.final_population) {
        const double nt = nontriviality_score(g);
        const double score = 0.55 * g.fitness + 0.25 * g.novelty + 0.20 * nt;
        if (score > best_score) {
            best_score = score;
            selected = g;
        }
    }

    AlgorithmSynthesisReport report;
    report.evolution = std::move(ev);
    report.accepted = true;

    AlgorithmArtifact artifact;
    artifact.kind = choose_algorithm_kind(cfg.domain, cfg.algorithm_kind);
    artifact.domain = cfg.domain;
    artifact.name = algorithm_base_name(artifact.kind, selected.id);
    artifact.kernel_genome = selected;
    artifact.kernel_expression = genome_to_export_expression(selected);
    artifact.nontriviality = nontriviality_score(selected);
    artifact.algorithm_score = best_score;
    artifact.algorithm_summary = summary_for_kind(artifact.kind);
    artifact.mathematical_contract = contract_for_kind(artifact.kind);
    artifact.pseudocode = pseudocode_for_kind(artifact.kind);
    artifact.complexity = complexity_for_kind(artifact.kind, selected.genes.size());
    artifact.validation_protocol = validation_for_kind(artifact.kind);

    if (cfg.require_nontrivial && artifact.nontriviality < 0.35) {
        report.accepted = false;
        report.warnings.push_back("selected kernel has low nontriviality; exported for inspection but not marked as accepted");
    }
    if (report.evolution.archive_after.unique_behavior <= 1 && report.evolution.archive_after.entries > 1) {
        report.warnings.push_back("archive behavior diversity is low; increase novelty pressure or switch domain");
    }

    std::filesystem::create_directories(cfg.export_dir);
    const std::filesystem::path dir(cfg.export_dir);
    const std::string base = artifact.name;
    artifact.export_files = {
        (dir / (base + "_algorithm.py")).string(),
        (dir / (base + "_algorithm.hpp")).string(),
        (dir / (base + "_algorithm.cl")).string(),
        (dir / (base + "_manifest.json")).string(),
        (dir / (base + "_README.md")).string(),
        (dir / ("test_" + base + "_algorithm.py")).string()
    };

    write_python_algorithm(artifact, artifact.export_files[0], cfg.gas_limit);
    write_cpp_algorithm(artifact, artifact.export_files[1], cfg.gas_limit);
    write_opencl_algorithm(artifact, artifact.export_files[2]);
    write_manifest(artifact, artifact.export_files[3]);
    write_readme(artifact, artifact.export_files[4]);
    write_test_script(artifact, artifact.export_files[5]);

    report.artifact = artifact;

    if (!cfg.report_path.empty()) {
        std::ofstream out(cfg.report_path);
        out << algorithm_discovery_report_markdown(report);
    }
    return report;
}

std::string algorithm_artifact_to_json(const AlgorithmArtifact& a) {
    std::ostringstream os;
    os << "{";
    os << "\"name\":\"" << json_escape(a.name) << "\",";
    os << "\"kind\":\"" << json_escape(a.kind) << "\",";
    os << "\"domain\":\"" << json_escape(a.domain) << "\",";
    os << "\"kernel_expression\":\"" << json_escape(a.kernel_expression) << "\",";
    os << "\"kernel_arity\":\"" << (a.kind == "root_refiner" ? "root_policy_v2:RootContext[30]->{bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta}" : "scalar:x") << "\",";
    os << "\"contextual_update\":" << (a.kind == "root_refiner" ? "true" : "false") << ",";
    os << "\"algorithm_summary\":\"" << json_escape(a.algorithm_summary) << "\",";
    os << "\"mathematical_contract\":\"" << json_escape(a.mathematical_contract) << "\",";
    os << "\"pseudocode\":\"" << json_escape(a.pseudocode) << "\",";
    os << "\"complexity\":\"" << json_escape(a.complexity) << "\",";
    os << "\"validation_protocol\":\"" << json_escape(a.validation_protocol) << "\",";
    os << "\"nontriviality\":" << a.nontriviality << ",";
    os << "\"algorithm_score\":" << a.algorithm_score << ",";
    os << "\"kernel\":" << genome_to_json(a.kernel_genome) << ",";
    os << "\"export_files\":[";
    for (std::size_t i = 0; i < a.export_files.size(); ++i) {
        if (i) os << ",";
        os << "\"" << json_escape(a.export_files[i]) << "\"";
    }
    os << "]}";
    return os.str();
}

std::string algorithm_synthesis_report_to_json(const AlgorithmSynthesisReport& report) {
    std::ostringstream os;
    os << "{";
    os << "\"accepted\":" << (report.accepted ? "true" : "false") << ",";
    os << "\"artifact\":" << algorithm_artifact_to_json(report.artifact) << ",";
    os << "\"evolution_best\":" << genome_to_json(report.evolution.best) << ",";
    os << "\"archive\":{";
    os << "\"before_entries\":" << report.evolution.archive_before.entries << ",";
    os << "\"after_entries\":" << report.evolution.archive_after.entries << ",";
    os << "\"unique_ast\":" << report.evolution.archive_after.unique_ast << ",";
    os << "\"unique_behavior\":" << report.evolution.archive_after.unique_behavior << "},";
    os << "\"vm_differential\":" << vm_differential_report_to_json(report.evolution.vm_report) << ",";
    os << "\"warnings\":[";
    for (std::size_t i = 0; i < report.warnings.size(); ++i) {
        if (i) os << ",";
        os << "\"" << json_escape(report.warnings[i]) << "\"";
    }
    os << "]}";
    return os.str();
}

std::string algorithm_discovery_report_markdown(const AlgorithmSynthesisReport& report) {
    const AlgorithmArtifact& a = report.artifact;
    std::ostringstream md;
    md << "# Algorithmic Genesis: Executable Algorithm Discovery\n\n";
    md << "## Acceptance\n\n";
    md << "- Accepted: `" << (report.accepted ? "true" : "false") << "`\n";
    md << "- Nontriviality: `" << a.nontriviality << "`\n";
    md << "- Algorithm score: `" << a.algorithm_score << "`\n\n";
    if (!report.warnings.empty()) {
        md << "## Warnings\n\n";
        for (const auto& w : report.warnings) md << "- " << w << "\n";
        md << "\n";
    }
    md << "## Discovered Algorithm\n\n";
    md << "- Name: `" << a.name << "`\n";
    md << "- Kind: `" << a.kind << "`\n";
    md << "- Domain: `" << a.domain << "`\n";
    md << "- Kernel: `K(x) = " << a.kernel_expression << "`\n\n";
    md << "### What it does\n\n" << a.algorithm_summary << "\n\n";
    md << "### Mathematical contract\n\n" << a.mathematical_contract << "\n\n";
    md << "### Pseudocode\n\n```text\n" << a.pseudocode << "\n```\n\n";
    md << "### Complexity\n\n" << a.complexity << "\n\n";
    md << "### Validation protocol\n\n" << a.validation_protocol << "\n\n";
    md << "## Kernel Genealogy\n\n";
    md << "- Kernel ID: `" << a.kernel_genome.id << "`\n";
    md << "- Parent A: `" << a.kernel_genome.parent_a << "`\n";
    md << "- Parent B: `" << a.kernel_genome.parent_b << "`\n";
    md << "- Birth generation: `" << a.kernel_genome.birth_generation << "`\n";
    md << "- Origin: `" << a.kernel_genome.origin << "`\n";
    md << "- Mutation trace: `" << a.kernel_genome.mutation_trace << "`\n\n";
    md << "## Fingerprints\n\n";
    md << "- AST: `" << a.kernel_genome.ast_fingerprint << "`\n";
    md << "- Behavior: `" << a.kernel_genome.behavior_fingerprint << "`\n";
    md << "- Derivative: `" << a.kernel_genome.derivative_fingerprint << "`\n";
    md << "- Stability: `" << a.kernel_genome.stability_fingerprint << "`\n";
    md << "- Complexity: `" << a.kernel_genome.complexity_fingerprint << "`\n\n";
    md << "## Exported Code\n\n";
    for (const auto& f : a.export_files) md << "- `" << f << "`\n";
    md << "\n## Important interpretation\n\n";
    md << "This is an executable mathematical algorithm candidate, not a proved theorem. It is suitable for empirical validation, comparison against known baselines and further evolutionary refinement.\n";
    return md.str();
}

} // namespace ag
