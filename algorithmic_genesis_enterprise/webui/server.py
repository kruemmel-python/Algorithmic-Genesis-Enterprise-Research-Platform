#!/usr/bin/env python3
"""
Algorithmic Genesis Research WebGUI server.

No external framework is required. It serves a browser UI, writes experiment
manifests, starts guided discovery jobs, captures status, exposes artifacts and
lets researchers run generated Python tests.

Run from repository root:
    python webui/server.py --host 127.0.0.1 --port 8765 --cli build_opencl/Release/ag_cli.exe
"""
from __future__ import annotations

import argparse
import json
import math
import os
import queue
import shutil
import subprocess
import sys
import threading
import time
import importlib.util
import statistics
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

ROOT = Path(__file__).resolve().parents[1]
STATIC = Path(__file__).resolve().parent / "static"
RUNS = ROOT / "webui_runs"
RUNS.mkdir(exist_ok=True)

DEFAULT_CATALOG = {
    "version": 2,
    "min_required": 3,
    "recommended_min": 5,
    "recommended_max": 20,
    "parts": [
        {"id":"x","label":"x","category":"primitive","role":"state","description":"Current candidate position."},
        {"id":"fx","label":"f(x)","category":"primitive","role":"residual","description":"Residual at current point."},
        {"id":"flo","label":"f(lo)","category":"primitive","role":"bracket","description":"Lower bracket residual."},
        {"id":"fhi","label":"f(hi)","category":"primitive","role":"bracket","description":"Upper bracket residual."},
        {"id":"width","label":"width","category":"geometry","role":"scale","description":"Current bracket width."},
        {"id":"relpos","label":"relative position","category":"geometry","role":"normalization","description":"Position inside bracket."},
        {"id":"history","label":"history","category":"memory","role":"trend","description":"Residual improvement memory."},
        {"id":"improvement","label":"improvement","category":"memory","role":"trend","description":"Current improvement signal."},
        {"id":"stagnation","label":"stagnation","category":"memory","role":"safety","description":"Progress stall counter."},
        {"id":"bracket_slope","label":"bracket slope","category":"slope","role":"secant","description":"Secant slope across bracket."},
        {"id":"local_slope","label":"local slope","category":"slope","role":"secant","description":"Local finite-difference slope."},
        {"id":"curvature","label":"curvature","category":"slope","role":"geometry","description":"Change in local slope."},
        {"id":"nfx","label":"normalized f(x)","category":"normalization","role":"residual","description":"Scale-free residual."},
        {"id":"residual_ratio","label":"residual ratio","category":"normalization","role":"convergence","description":"Relative residual strength."},
        {"id":"sin","label":"sin","category":"nonlinearity","role":"oscillation","description":"Sine shaping."},
        {"id":"cos","label":"cos","category":"nonlinearity","role":"oscillation","description":"Cosine shaping."},
        {"id":"tanh","label":"tanh","category":"nonlinearity","role":"saturation","description":"Bounded saturation."},
        {"id":"logabs","label":"logabs","category":"nonlinearity","role":"compression","description":"Safe logarithmic compression."},
        {"id":"expclamp","label":"expclamp","category":"nonlinearity","role":"amplification","description":"Safe bounded exponential."},
        {"id":"cubic","label":"cubic","category":"nonlinearity","role":"polynomial","description":"Cubic shaping."},
        {"id":"bias","label":"policy bias","category":"policy_output","role":"step","description":"Learned candidate direction."},
        {"id":"damping","label":"policy damping","category":"policy_output","role":"step","description":"Learned step damping."},
        {"id":"secant_mix","label":"secant mix","category":"policy_output","role":"method_mix","description":"Secant candidate weight."},
        {"id":"bisection_mix","label":"bisection mix","category":"policy_output","role":"method_mix","description":"Bisection weight."},
        {"id":"relaxation_mix","label":"relaxation mix","category":"policy_output","role":"method_mix","description":"Learned relaxation weight."},
        {"id":"trust_delta","label":"trust delta","category":"policy_output","role":"trust","description":"Trust update."},
        {"id":"bisection","label":"bisection","category":"strategy","role":"baseline","description":"Safe midpoint fallback."},
        {"id":"secant","label":"secant","category":"strategy","role":"interpolation","description":"Secant candidate."},
        {"id":"regula_falsi","label":"regula falsi","category":"strategy","role":"interpolation","description":"False-position pressure."},
        {"id":"learned_relaxation","label":"learned relaxation","category":"strategy","role":"generated","description":"Generated kernel step."},
        {"id":"bracket_guard","label":"bracket guard","category":"safety","role":"invariant","description":"Never leave bracket."},
        {"id":"finite_sanitize","label":"finite sanitize","category":"safety","role":"numeric","description":"NaN/Inf containment."},
        {"id":"trust_gate","label":"trust gate","category":"safety","role":"trust","description":"Dampen low-trust steps."},
        {"id":"stagnation_reset","label":"stagnation reset","category":"safety","role":"recovery","description":"Fallback on stagnation."},
        {"id":"reject_penalty","label":"reject penalty","category":"safety","role":"learning","description":"Penalize rejected candidates."},
        {"id":"gas_limit","label":"gas limit","category":"safety","role":"termination","description":"Operation budget."},
        {"id":"stable_sort","label":"stable sort","category":"cs_core_sorting","role":"correctness","description":"Stable sorted-output invariant."},
        {"id":"inversion_pressure","label":"inversion pressure","category":"cs_core_sorting","role":"local_disorder","description":"Local inversion pressure."},
        {"id":"adaptive_pivot","label":"adaptive pivot","category":"cs_core_sorting","role":"partition","description":"Learned pivot/key shaping."},
        {"id":"priority_queue","label":"priority queue","category":"cs_core_graph","role":"frontier","description":"Graph frontier ordering."},
        {"id":"edge_relaxation","label":"edge relaxation","category":"cs_core_graph","role":"dijkstra","description":"Distance relaxation invariant."},
        {"id":"heuristic_potential","label":"heuristic potential","category":"cs_core_graph","role":"search_bias","description":"Learned traversal potential."},
        {"id":"deadline_pressure","label":"deadline pressure","category":"cs_core_scheduling","role":"deadline","description":"Deadline-aware scheduling pressure."},
        {"id":"makespan_minimize","label":"makespan minimize","category":"cs_core_scheduling","role":"objective","description":"Minimize maximum worker load."},
        {"id":"fairness_penalty","label":"fairness penalty","category":"cs_core_scheduling","role":"load_balance","description":"Penalize uneven load."},
        {"id":"delimiter_stack","label":"delimiter stack","category":"cs_core_parsing","role":"syntax","description":"Bounded syntax stack."},
        {"id":"repair_policy","label":"repair policy","category":"cs_core_parsing","role":"error_recovery","description":"Safe parse repair."},
        {"id":"precedence_pressure","label":"precedence pressure","category":"cs_core_parsing","role":"reduction","description":"Reduction preference."},
        {"id":"variable_ordering","label":"variable ordering","category":"cs_core_constraints","role":"search","description":"CSP variable ordering."},
        {"id":"value_ordering","label":"value ordering","category":"cs_core_constraints","role":"search","description":"CSP value ordering."},
        {"id":"constraint_propagation","label":"constraint propagation","category":"cs_core_constraints","role":"pruning","description":"Safe pruning pressure."},
        {"id":"online_mean","label":"online mean","category":"cs_core_streaming","role":"statistics","description":"Online mean update."},
        {"id":"online_variance","label":"online variance","category":"cs_core_streaming","role":"statistics","description":"Online variance update."},
        {"id":"anomaly_pressure","label":"anomaly pressure","category":"cs_core_streaming","role":"signal","description":"Learned anomaly signal."},
        {"id":"run_length","label":"run length","category":"cs_core_compression","role":"encoding","description":"Run-length pressure."},
        {"id":"delta_coding","label":"delta coding","category":"cs_core_compression","role":"encoding","description":"Delta coding pressure."},
        {"id":"reconstruction_guard","label":"reconstruction guard","category":"cs_core_compression","role":"safety","description":"Decode guard."},
        {"id":"recency","label":"recency","category":"cs_core_cache","role":"eviction","description":"LRU pressure."},
        {"id":"frequency","label":"frequency","category":"cs_core_cache","role":"eviction","description":"LFU pressure."},
        {"id":"reuse_distance","label":"reuse distance","category":"cs_core_cache","role":"prediction","description":"Reuse-distance pressure."},
        {"id":"worker_load","label":"worker load","category":"cs_core_load_balancing","role":"state","description":"Worker load signal."},
        {"id":"variance_penalty","label":"variance penalty","category":"cs_core_load_balancing","role":"fairness","description":"Load variance penalty."},
        {"id":"tail_latency","label":"tail latency","category":"cs_core_load_balancing","role":"objective","description":"Worst-case completion pressure."},
        {"id":"zscore","label":"z-score","category":"cs_core_anomaly","role":"statistics","description":"Standard deviation anomaly score."},
        {"id":"robust_median","label":"robust median","category":"cs_core_anomaly","role":"robustness","description":"Robust center pressure."},
        {"id":"outlier_gate","label":"outlier gate","category":"cs_core_anomaly","role":"decision","description":"Bounded anomaly gate."}
    ]
}


class Job:
    def __init__(self, job_id: str, manifest: dict, cli: Path):
        self.job_id = job_id
        self.manifest = manifest
        self.cli = cli
        self.dir = RUNS / job_id
        self.dir.mkdir(parents=True, exist_ok=True)
        self.manifest_path = self.dir / "experiment_manifest.json"
        self.result_path = self.dir / "result.json"
        self.report_path = self.dir / "report.md"
        self.export_dir = self.dir / "exports"
        self.log_path = self.dir / "run.log"
        self.status_path = self.dir / "status.json"
        self.process: subprocess.Popen[str] | None = None
        self.started_at = time.time()
        self.finished_at: float | None = None
        self.state = "queued"
        self.error = ""
        self.returncode: int | None = None

    def write_status(self):
        payload = {
            "job_id": self.job_id,
            "state": self.state,
            "returncode": self.returncode,
            "error": self.error,
            "started_at": self.started_at,
            "finished_at": self.finished_at,
            "manifest": self.manifest,
            "paths": {
                "dir": str(self.dir),
                "manifest": str(self.manifest_path),
                "result": str(self.result_path),
                "report": str(self.report_path),
                "export_dir": str(self.export_dir),
                "log": str(self.log_path),
            }
        }
        self.status_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        return payload

    def run(self):
        self.state = "running"
        self.manifest["archive"] = str(self.dir / "guided_archive.jsonl")
        self.manifest["export_dir"] = str(self.export_dir)
        self.manifest["report"] = str(self.report_path)
        self.manifest["json"] = str(self.result_path)
        self.manifest_path.write_text(json.dumps(self.manifest, indent=2), encoding="utf-8")
        self.write_status()
        cmd = [str(self.cli), "discover-guided", "--experiment", str(self.manifest_path)]
        with self.log_path.open("w", encoding="utf-8") as log:
            log.write("COMMAND: " + " ".join(cmd) + "\n\n")
            log.flush()
            try:
                self.process = subprocess.Popen(cmd, cwd=str(ROOT), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                assert self.process.stdout is not None
                for line in self.process.stdout:
                    log.write(line)
                    log.flush()
                self.returncode = self.process.wait()
                self.state = "finished" if self.returncode == 0 else "failed"
            except Exception as exc:  # noqa: BLE001
                self.error = str(exc)
                self.returncode = 99
                self.state = "failed"
            finally:
                self.finished_at = time.time()
                self.write_status()


class AppState:
    def __init__(self, cli: Path):
        self.cli = cli
        self.jobs: dict[str, Job] = {}
        self.lock = threading.Lock()

    def start_job(self, manifest: dict) -> Job:
        selected = manifest.get("selected_parts", [])
        if not isinstance(selected, list) or len(selected) < 3:
            raise ValueError("At least 3 formula/strategy parts must be selected.")
        job_id = time.strftime("run_%Y%m%d_%H%M%S") + "_" + str(int(time.time() * 1000) % 100000)
        job = Job(job_id, manifest, self.cli)
        with self.lock:
            self.jobs[job_id] = job
        thread = threading.Thread(target=job.run, daemon=True)
        thread.start()
        return job

    def get_job(self, job_id: str) -> Job | None:
        with self.lock:
            return self.jobs.get(job_id)


def guess_cli(cli_arg: str | None) -> Path:
    if cli_arg:
        return Path(cli_arg).resolve()
    candidates = [
        ROOT / "build_opencl" / "Release" / "ag_cli.exe",
        ROOT / "build" / "Release" / "ag_cli.exe",
        ROOT / "build" / "ag_cli",
        ROOT / "build_opencl" / "ag_cli",
    ]
    for p in candidates:
        if p.exists():
            return p
    return candidates[0]


def load_catalog(cli: Path) -> dict:
    if cli.exists():
        try:
            out = subprocess.check_output([str(cli), "list-formula-parts"], cwd=str(ROOT), text=True, timeout=15)
            return json.loads(out)
        except Exception:
            pass
    return DEFAULT_CATALOG


def json_response(handler: BaseHTTPRequestHandler, payload: object, status: int = 200):
    data = json.dumps(payload, indent=2).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(data)))
    handler.end_headers()
    handler.wfile.write(data)


def text_response(handler: BaseHTTPRequestHandler, payload: str, content_type: str = "text/plain; charset=utf-8", status: int = 200):
    data = payload.encode("utf-8", errors="replace")
    handler.send_response(status)
    handler.send_header("Content-Type", content_type)
    handler.send_header("Content-Length", str(len(data)))
    handler.end_headers()
    handler.wfile.write(data)



# ----------------------------- Domain Evaluation -----------------------------

def _sanitize_num(v: object) -> float:
    try:
        x = float(v)
    except Exception:
        return 0.0
    if not math.isfinite(x):
        return 0.0
    return max(-1_000_000.0, min(1_000_000.0, x))


def _load_generated_algorithm(export_dir: Path):
    py_files = sorted(export_dir.glob("*_algorithm.py"))
    py_files = [p for p in py_files if not p.name.startswith("test_")]
    if not py_files:
        raise FileNotFoundError("No generated *_algorithm.py artifact found.")
    target = py_files[0]
    module_name = "ag_webgui_generated_" + str(abs(hash(str(target))))
    spec = importlib.util.spec_from_file_location(module_name, target)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load generated module from {target}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module, target


def _histogram(values: list[float], bins: int = 48, lo: float = -1.0, hi: float = 1.0) -> list[int]:
    out = [0 for _ in range(bins)]
    width = max(1e-12, hi - lo)
    for v in values:
        x = max(lo, min(hi, _sanitize_num(v)))
        idx = min(bins - 1, max(0, int((x - lo) / width * bins)))
        out[idx] += 1
    return out


def _entropy_from_counts(counts: list[int]) -> float:
    total = sum(counts)
    if total <= 0:
        return 0.0
    h = 0.0
    for c in counts:
        if c > 0:
            p = c / total
            h -= p * math.log(p + 1e-15, 2)
    return h


def _finite_ratio(values: list[float]) -> float:
    if not values:
        return 0.0
    return sum(1 for v in values if math.isfinite(v)) / len(values)


def _periodicity(values: list[float], max_lag: int = 96) -> dict:
    tail = values[len(values)//2:] if len(values) > 64 else values
    if len(tail) < 8:
        return {"best_period": 0, "mse": 0.0, "periodicity_score": 0.0}
    best_lag = 1
    best_mse = float("inf")
    upper = min(max_lag, len(tail)//2)
    for lag in range(1, upper + 1):
        n = len(tail) - lag
        if n <= 0:
            continue
        mse = sum((tail[i] - tail[i + lag]) ** 2 for i in range(n)) / n
        if mse < best_mse:
            best_mse = mse
            best_lag = lag
    score = 1.0 / (1.0 + 1000.0 * best_mse)
    return {"best_period": best_lag, "mse": _sanitize_num(best_mse), "periodicity_score": _sanitize_num(score)}


def _return_map_entropy(values: list[float], bins: int = 32) -> float:
    if len(values) < 2:
        return 0.0
    counts = [0 for _ in range(bins * bins)]
    for a, b in zip(values[:-1], values[1:]):
        ia = min(bins - 1, max(0, int((max(-1.0, min(1.0, a)) + 1.0) * 0.5 * bins)))
        ib = min(bins - 1, max(0, int((max(-1.0, min(1.0, b)) + 1.0) * 0.5 * bins)))
        counts[ia * bins + ib] += 1
    return _entropy_from_counts(counts)


def _lyapunov_from_map(fn, x0: float, steps: int, eps: float = 1e-5) -> tuple[list[float], float]:
    x = _sanitize_num(x0)
    orbit: list[float] = []
    lyap = 0.0
    for _ in range(max(0, steps)):
        try:
            x1 = _sanitize_num(fn(x))
            xp = _sanitize_num(fn(x + eps))
            lyap += math.log(abs((xp - x1) / eps) + 1e-9)
            x = x1
            orbit.append(x)
        except Exception:
            orbit.append(0.0)
            x = 0.0
    return orbit, lyap / max(1, len(orbit))


def _generated_orbit(module, x0: float, steps: int) -> tuple[list[float], float]:
    if hasattr(module, "explore_map"):
        result, err = module.explore_map(x0, iterations=steps)
        orbit = [_sanitize_num(x) for x in result.get("orbit", [])]
        lyap = _sanitize_num(result.get("lyapunov_like", 0.0))
        return orbit, lyap
    if hasattr(module, "kernel"):
        return _lyapunov_from_map(lambda x: math.tanh(module.kernel(x)), x0, steps)
    raise AttributeError("Generated artifact has neither explore_map nor kernel.")


def _logistic_orbit(x0: float, steps: int, r: float = 4.0) -> tuple[list[float], float]:
    # Map [-1, 1] -> [0, 1], run logistic, map back to [-1, 1].
    y0 = max(0.0, min(1.0, 0.5 * (_sanitize_num(x0) + 1.0)))
    def f_unit(y: float) -> float:
        return max(0.0, min(1.0, r * y * (1.0 - y)))
    y = y0
    orbit = []
    lyap = 0.0
    for _ in range(max(0, steps)):
        deriv = abs(r * (1.0 - 2.0 * y))
        lyap += math.log(deriv + 1e-9)
        y = f_unit(y)
        orbit.append(2.0 * y - 1.0)
    return orbit, lyap / max(1, len(orbit))


def _tent_orbit(x0: float, steps: int, mu: float = 2.0) -> tuple[list[float], float]:
    y = max(0.0, min(1.0, 0.5 * (_sanitize_num(x0) + 1.0)))
    orbit = []
    lyap = math.log(abs(mu) + 1e-9)
    for _ in range(max(0, steps)):
        y = mu * y if y < 0.5 else mu * (1.0 - y)
        y = max(0.0, min(1.0, y))
        orbit.append(2.0 * y - 1.0)
    return orbit, lyap


def _sine_orbit(x0: float, steps: int, r: float = 1.0) -> tuple[list[float], float]:
    y = max(0.0, min(1.0, 0.5 * (_sanitize_num(x0) + 1.0)))
    orbit = []
    lyap = 0.0
    for _ in range(max(0, steps)):
        deriv = abs(r * math.pi * math.cos(math.pi * y))
        lyap += math.log(deriv + 1e-9)
        y = max(0.0, min(1.0, r * math.sin(math.pi * y)))
        orbit.append(2.0 * y - 1.0)
    return orbit, lyap / max(1, len(orbit))


def _metrics_for_orbit(name: str, orbit: list[float], lyap: float) -> dict:
    clean = [_sanitize_num(x) for x in orbit]
    if not clean:
        return {"name": name, "error": "empty orbit"}
    hist = _histogram(clean, bins=48)
    finite = _finite_ratio(clean)
    spread = max(clean) - min(clean)
    mean = statistics.fmean(clean)
    stdev = statistics.pstdev(clean) if len(clean) > 1 else 0.0
    entropy = _entropy_from_counts(hist) / math.log(48, 2)
    rm_entropy = _return_map_entropy(clean) / math.log(32 * 32, 2)
    period = _periodicity(clean)
    lyap_class = "chaotic" if lyap > 0.05 else ("contractive" if lyap < -0.05 else "neutral")
    return {
        "name": name,
        "samples": len(clean),
        "finite_ratio": finite,
        "bounded": all(-1.000001 <= x <= 1.000001 for x in clean),
        "mean": _sanitize_num(mean),
        "stdev": _sanitize_num(stdev),
        "attractor_spread": _sanitize_num(spread),
        "orbit_entropy": _sanitize_num(entropy),
        "return_map_diversity": _sanitize_num(rm_entropy),
        "lyapunov_like": _sanitize_num(lyap),
        "lyapunov_band": lyap_class,
        "periodicity": period,
        "histogram": hist,
        "orbit_sample": clean[:512],
        "return_map_sample": [[clean[i], clean[i+1]] for i in range(min(512, len(clean)-1))],
    }


def _sensitivity(module, steps: int = 512, eps: float = 1e-6) -> dict:
    seeds = [-0.75, -0.25, 0.123, 0.5, 0.85]
    divergences = []
    for x0 in seeds:
        a, _ = _generated_orbit(module, x0, steps)
        b, _ = _generated_orbit(module, x0 + eps, steps)
        n = min(len(a), len(b))
        if n > 0:
            divergences.append(abs(a[-1] - b[-1]) / eps)
    if not divergences:
        return {"mean_final_divergence_per_eps": 0.0, "max_final_divergence_per_eps": 0.0}
    return {
        "mean_final_divergence_per_eps": _sanitize_num(statistics.fmean(divergences)),
        "max_final_divergence_per_eps": _sanitize_num(max(divergences)),
    }


def evaluate_chaotic_maps(job: Job, steps: int = 1024) -> dict:
    module, artifact = _load_generated_algorithm(job.export_dir)
    generated_orbits = []
    lyaps = []
    for seed in [-0.75, -0.25, 0.123, 0.5, 0.85]:
        orbit, lyap = _generated_orbit(module, seed, steps)
        generated_orbits.extend(orbit)
        lyaps.append(lyap)
    generated = _metrics_for_orbit("generated", generated_orbits, statistics.fmean(lyaps) if lyaps else 0.0)

    logistic, log_lyap = _logistic_orbit(0.123, steps)
    tent, tent_lyap = _tent_orbit(0.123, steps)
    sine, sine_lyap = _sine_orbit(0.123, steps)
    baselines = [
        _metrics_for_orbit("logistic_r4", logistic, log_lyap),
        _metrics_for_orbit("tent_mu2", tent, tent_lyap),
        _metrics_for_orbit("sine_r1", sine, sine_lyap),
    ]

    # Research score rewards finite bounded dynamics, high diversity and non-degenerate spread.
    score = (
        0.25 * generated["finite_ratio"] +
        0.20 * float(generated["bounded"]) +
        0.20 * generated["orbit_entropy"] +
        0.15 * generated["return_map_diversity"] +
        0.10 * min(1.0, generated["attractor_spread"]) +
        0.10 * (1.0 if generated["lyapunov_band"] in ("neutral", "chaotic") else 0.35)
    )

    payload = {
        "domain": "chaotic_maps",
        "artifact": artifact.name,
        "steps_per_seed": steps,
        "seed_count": 5,
        "generated": generated,
        "baselines": baselines,
        "sensitivity_to_seed": _sensitivity(module),
        "research_score": _sanitize_num(score),
        "interpretation": {
            "orbit_entropy": "0..1 normalized Shannon entropy over orbit histogram; higher means richer coverage.",
            "return_map_diversity": "0..1 normalized entropy of (x_t, x_{t+1}) pairs; higher means richer map geometry.",
            "periodicity_score": "1 means strongly periodic; near 0 means non-repeating or noisy.",
            "lyapunov_band": "contractive, neutral, or chaotic based on the Lyapunov-like estimate.",
            "research_score": "Heuristic composite for ranking generated maps; not a proof of novelty.",
        },
    }
    (job.dir / "domain_evaluation.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return payload



def _load_generated_module(job: Job):
    py_files = sorted(job.export_dir.glob("*_algorithm.py"))
    py_files = [p for p in py_files if not p.name.startswith("test_")]
    if not py_files:
        raise RuntimeError("no generated Python algorithm found")
    path = py_files[0]
    spec = importlib.util.spec_from_file_location(path.stem, path)
    if not spec or not spec.loader:
        raise RuntimeError("cannot import generated algorithm")
    mod = importlib.util.module_from_spec(spec)
    sys.modules[path.stem] = mod
    spec.loader.exec_module(mod)
    return mod, path


def _bisection_baseline(f, lo: float, hi: float, iterations: int = 80) -> tuple[float, float, int]:
    lo = _sanitize_num(lo); hi = _sanitize_num(hi)
    flo = _sanitize_num(f(lo)); fhi = _sanitize_num(f(hi))
    if flo == 0.0:
        return lo, 0.0, 0
    if fhi == 0.0:
        return hi, 0.0, 0
    if flo * fhi > 0.0:
        mid = 0.5 * (lo + hi)
        return mid, abs(_sanitize_num(f(mid))), 3
    best_x, best_r = (lo, abs(flo)) if abs(flo) <= abs(fhi) else (hi, abs(fhi))
    for i in range(max(1, iterations)):
        mid = 0.5 * (lo + hi)
        fm = _sanitize_num(f(mid))
        if abs(fm) < best_r:
            best_x, best_r = mid, abs(fm)
        if abs(fm) <= 1e-12:
            return mid, abs(fm), 0
        if (flo < 0 and fm > 0) or (flo > 0 and fm < 0):
            hi, fhi = mid, fm
        else:
            lo, flo = mid, fm
    return best_x, best_r, 0


def evaluate_root_finding(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "root_refine"):
        return {"domain": "root_finding", "available": False, "message": "Generated module has no root_refine function."}

    cases = [
        {"name": "sqrt2_quadratic", "lo": 0.0, "hi": 2.0, "f": lambda x: x*x - 2.0},
        {"name": "cubic_shift", "lo": 1.0, "hi": 2.0, "f": lambda x: x*x*x - 3.0},
        {"name": "sin_pi", "lo": 3.0, "hi": 4.0, "f": math.sin},
        {"name": "flat_cubic", "lo": -1.0, "hi": 2.0, "f": lambda x: (x - 0.25)**3},
        {"name": "oscillatory_safe", "lo": 0.1, "hi": 1.0, "f": lambda x: math.cos(5.0*x) - 0.2},
    ]

    rows = []
    successes = 0
    total_ratio = 0.0
    for c in cases:
        f = c["f"]
        try:
            x, err = mod.root_refine(f, c["lo"], c["hi"], iterations=96)
            residual = abs(_sanitize_num(f(x)))
            bx, br, berr = _bisection_baseline(f, c["lo"], c["hi"], 96)
            in_bracket = min(c["lo"], c["hi"]) - 1e-12 <= x <= max(c["lo"], c["hi"]) + 1e-12
            finite = math.isfinite(float(x)) and math.isfinite(float(residual))
            ok = finite and in_bracket and residual <= max(1e-5, br * 20.0 + 1e-12)
            successes += 1 if ok else 0
            ratio = min(10.0, residual / (br + 1e-12))
            total_ratio += ratio
            rows.append({
                "case": c["name"],
                "x": _sanitize_num(x),
                "residual": _sanitize_num(residual),
                "error_code": err,
                "baseline_x": _sanitize_num(bx),
                "baseline_residual": _sanitize_num(br),
                "baseline_error_code": berr,
                "in_bracket": in_bracket,
                "finite": finite,
                "pass": ok,
                "residual_ratio_to_bisection": _sanitize_num(ratio),
            })
        except Exception as exc:  # noqa: BLE001
            rows.append({"case": c["name"], "error": str(exc), "pass": False})
            total_ratio += 10.0

    robustness = successes / max(1, len(cases))
    residual_competitiveness = 1.0 / (1.0 + total_ratio / max(1, len(cases)))
    return {
        "domain": "root_finding",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "root_refine_vs_bisection",
        "cases": rows,
        "successes": successes,
        "total_cases": len(cases),
        "robustness": _sanitize_num(robustness),
        "residual_competitiveness": _sanitize_num(residual_competitiveness),
        "score": _sanitize_num(0.65 * robustness + 0.35 * residual_competitiveness),
    }


def evaluate_signal_transform(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "signal_morph"):
        return {"domain": "signal_transform", "available": False, "message": "Generated module has no signal_morph function."}
    signals = {
        "step": [0.0]*16 + [1.0]*16,
        "sine": [math.sin(i * 0.18) for i in range(64)],
        "impulse": [1.0 if i == 16 else 0.0 for i in range(64)],
        "ramp": [i / 63.0 for i in range(64)],
    }
    rows = []
    finite_all = 0
    for name, sig in signals.items():
        out, err = mod.signal_morph(sig)
        out = [_sanitize_num(v) for v in out]
        finite = _finite_ratio(out)
        finite_all += 1 if finite >= 1.0 and len(out) == len(sig) else 0
        in_energy = sum(x*x for x in sig) + 1e-12
        out_energy = sum(x*x for x in out) + 1e-12
        smoothness = sum(abs(out[i] - out[i-1]) for i in range(1, len(out))) / max(1, len(out)-1)
        rows.append({
            "case": name,
            "length": len(out),
            "finite_ratio": finite,
            "energy_ratio": _sanitize_num(out_energy / in_energy),
            "smoothness": _sanitize_num(smoothness),
            "error_code": err,
            "sample": out[:32],
        })
    score = finite_all / max(1, len(signals))
    return {
        "domain": "signal_transform",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "signal_morph_finiteness_energy_smoothness",
        "cases": rows,
        "score": _sanitize_num(score),
    }


def evaluate_fixed_point(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "fixed_point"):
        return {"domain": "fixed_point_dynamics", "available": False, "message": "Generated module has no fixed_point function."}
    seeds = [-2.0, -1.0, -0.25, 0.0, 0.25, 1.0, 2.0]
    rows = []
    finite_count = 0
    stable_count = 0
    for x0 in seeds:
        y, err = mod.fixed_point(x0, iterations=128)
        y = _sanitize_num(y)
        finite = math.isfinite(y)
        finite_count += 1 if finite else 0
        # One-step consistency check if kernel exists.
        residual = 0.0
        if hasattr(mod, "kernel"):
            y2 = _sanitize_num(0.65 * y + 0.35 * math.tanh(mod.kernel(y)))
            residual = abs(y2 - y)
        stable = finite and residual < 1e-6
        stable_count += 1 if stable else 0
        rows.append({"x0": x0, "fixed_point": y, "error_code": err, "residual": _sanitize_num(residual), "stable": stable})
    return {
        "domain": "fixed_point_dynamics",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "fixed_point_convergence_smoke",
        "cases": rows,
        "finite_ratio": _sanitize_num(finite_count / max(1, len(seeds))),
        "stable_ratio": _sanitize_num(stable_count / max(1, len(seeds))),
        "score": _sanitize_num(0.5 * finite_count / max(1, len(seeds)) + 0.5 * stable_count / max(1, len(seeds))),
    }


def evaluate_sequence_generation(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "predict_next"):
        return {"domain": "sequence_generation", "available": False, "message": "Generated module has no predict_next function."}
    cases = [
        {"name": "linear", "values": [1.0, 2.0, 3.0, 4.0], "expected": 5.0},
        {"name": "half_steps", "values": [0.0, 0.5, 1.0, 1.5], "expected": 2.0},
        {"name": "constant", "values": [2.0, 2.0, 2.0], "expected": 2.0},
        {"name": "decay", "values": [8.0, 4.0, 2.0], "expected": 0.0},  # intentionally adversarial for linear extrapolation
    ]
    rows = []
    err_sum = 0.0
    finite = 0
    for c in cases:
        pred, err = mod.predict_next(c["values"])
        pred = _sanitize_num(pred)
        abs_err = abs(pred - c["expected"])
        err_sum += min(100.0, abs_err)
        finite += 1 if math.isfinite(pred) else 0
        rows.append({"case": c["name"], "prediction": pred, "expected": c["expected"], "abs_error": _sanitize_num(abs_err), "error_code": err})
    return {
        "domain": "sequence_generation",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "next_value_prediction_smoke",
        "cases": rows,
        "finite_ratio": _sanitize_num(finite / max(1, len(cases))),
        "mean_abs_error": _sanitize_num(err_sum / max(1, len(cases))),
        "score": _sanitize_num(1.0 / (1.0 + err_sum / max(1, len(cases)))),
    }


def evaluate_classification_boundary(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "kernel"):
        return {"domain": "classification_boundary", "available": False, "message": "Generated module has no kernel function."}
    data = [(-2.0, -1), (-1.0, -1), (-0.2, -1), (0.2, 1), (1.0, 1), (2.0, 1)]
    correct = 0
    rows = []
    for x, label in data:
        y = _sanitize_num(mod.kernel(x))
        pred = 1 if y >= 0.0 else -1
        ok = pred == label
        correct += 1 if ok else 0
        rows.append({"x": x, "kernel": y, "label": label, "prediction": pred, "correct": ok})
    return {
        "domain": "classification_boundary",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "sign_boundary_smoke",
        "cases": rows,
        "accuracy": _sanitize_num(correct / max(1, len(data))),
        "score": _sanitize_num(correct / max(1, len(data))),
    }


def evaluate_symbolic_identity(job: Job) -> dict:
    mod, path = _load_generated_module(job)
    if not hasattr(mod, "kernel"):
        return {"domain": "symbolic_identity_search", "available": False, "message": "Generated module has no kernel function."}
    xs = [i / 10.0 for i in range(-30, 31)]
    vals = [_sanitize_num(mod.kernel(x)) for x in xs]
    finite = _finite_ratio(vals)
    mean_abs = sum(abs(v) for v in vals) / max(1, len(vals))
    max_abs = max((abs(v) for v in vals), default=0.0)
    variation = statistics.pstdev(vals) if len(vals) > 1 else 0.0
    # For identity residual reducers, low residual with nonzero variation is interesting.
    score = finite * (1.0 / (1.0 + mean_abs)) * min(1.0, 0.25 + variation)
    return {
        "domain": "symbolic_identity_search",
        "available": True,
        "algorithm_file": path.name,
        "benchmark": "identity_residual_scan",
        "samples": len(vals),
        "finite_ratio": _sanitize_num(finite),
        "mean_abs_residual": _sanitize_num(mean_abs),
        "max_abs_residual": _sanitize_num(max_abs),
        "variation": _sanitize_num(variation),
        "sample": vals[:32],
        "score": _sanitize_num(score),
    }

def evaluate_cs_domain(job: Job) -> dict:
    domain = str(job.manifest.get("domain", ""))
    mod, path = _load_generated_module(job)
    metrics = {"domain": domain, "available": True, "algorithm_file": path.name}

    try:
        if domain == "sorting":
            vals = [5, 1, 3, 3, -2, 8, 0]
            out, err = mod.adaptive_sort(vals)
            metrics.update({
                "benchmark": "adaptive_sort",
                "sorted": out == sorted(out),
                "stable_smoke": True,
                "input_size": len(vals),
                "error_code": err,
                "score": 1.0 if out == sorted(vals) else 0.0,
            })
        elif domain == "graph_shortest_path":
            graph = {"a": [("b", 1.0), ("c", 5.0)], "b": [("c", 1.0), ("d", 4.0)], "c": [("d", 1.0)], "d": []}
            res, err = mod.graph_shortest_path(graph, "a", "d")
            metrics.update({
                "benchmark": "graph_shortest_path",
                "path": res.get("path"),
                "cost": res.get("cost"),
                "reached": res.get("reached"),
                "baseline_cost": 3.0,
                "error_code": err,
                "score": 1.0 if res.get("reached") and abs(float(res.get("cost", 999))-3.0) < 1e-6 else 0.5,
            })
        elif domain == "scheduling":
            res, err = mod.schedule_jobs([7, 3, 2, 5, 11, 4], workers=3)
            metrics.update({
                "benchmark": "schedule_jobs",
                "loads": res.get("loads"),
                "makespan": res.get("makespan"),
                "fairness": res.get("fairness"),
                "error_code": err,
                "score": 1.0 / (1.0 + float(res.get("fairness", 999))),
            })
        elif domain == "parsing":
            res, err = mod.parse_and_repair(["(", "x", "+", "[", "1", ")", "}"])
            metrics.update({
                "benchmark": "parse_and_repair",
                "repairs": res.get("repairs"),
                "repair_count": len(res.get("repairs", [])),
                "balanced_initially": res.get("balanced"),
                "error_code": err,
                "score": 1.0 / (1.0 + len(res.get("repairs", []))),
            })
        elif domain == "constraint_solving":
            res, err = mod.solve_constraints({"x": [1,2,3], "y": [1,2,3], "z": [1,2,3]},
                                             [lambda a: ("x" not in a or "y" not in a) or a["x"] < a["y"],
                                              lambda a: ("y" not in a or "z" not in a) or a["y"] <= a["z"]])
            metrics.update({
                "benchmark": "solve_constraints",
                "assignment": res.get("assignment"),
                "satisfied": res.get("satisfied"),
                "error_code": err,
                "score": 1.0 if res.get("satisfied") else 0.0,
            })
        elif domain == "stream_processing":
            res, err = mod.stream_analyze([1,1,1,8,1,1,10,1])
            metrics.update({
                "benchmark": "stream_analyze",
                "count": res.get("count"),
                "mean": res.get("mean"),
                "variance": res.get("variance"),
                "anomalies": res.get("anomalies"),
                "error_code": err,
                "score": min(1.0, len(res.get("anomalies", [])) / 2.0),
            })
        elif domain == "compression":
            res, err = mod.compress_adaptive([1,1,1,2,2,3,3,3,3])
            ok = len(res.get("decoded", [])) == 9
            metrics.update({
                "benchmark": "compress_adaptive",
                "packets": len(res.get("packets", [])),
                "ratio": res.get("ratio"),
                "roundtrip_length_ok": ok,
                "error_code": err,
                "score": (1.0 if ok else 0.0) * max(0.0, 1.0 - float(res.get("ratio", 1.0)) / 3.0),
            })
        elif domain == "cache_eviction":
            res, err = mod.cache_simulate(["a","b","a","c","a","b","d","a"], capacity=2)
            metrics.update({
                "benchmark": "cache_simulate",
                "hits": res.get("hits"),
                "misses": res.get("misses"),
                "cache": res.get("cache"),
                "error_code": err,
                "score": float(res.get("hits", 0)) / max(1.0, float(res.get("hits", 0) + res.get("misses", 0))),
            })
        elif domain == "load_balancing":
            res, err = mod.load_balance([4,8,2,7,1,3], workers=3)
            metrics.update({
                "benchmark": "load_balance",
                "loads": res.get("loads"),
                "makespan": res.get("makespan"),
                "fairness": res.get("fairness"),
                "error_code": err,
                "score": 1.0 / (1.0 + float(res.get("fairness", 999))),
            })
        elif domain == "anomaly_detection":
            idx, err = mod.detect_anomalies([0,0,0,10,0,0,-9,0])
            metrics.update({
                "benchmark": "detect_anomalies",
                "indices": idx,
                "error_code": err,
                "score": min(1.0, len(idx) / 2.0),
            })
        else:
            return {"domain": domain, "available": False, "message": "No CS domain evaluation available for this domain."}
    except Exception as exc:  # noqa: BLE001
        return {"domain": domain, "available": False, "error": str(exc)}

    target = job.dir / "domain_evaluation.json"
    target.write_text(json.dumps(metrics, indent=2), encoding="utf-8")
    return metrics

def evaluate_domain(job: Job) -> dict:
    domain = str(job.manifest.get("domain", ""))
    if domain == "chaotic_maps":
        return evaluate_chaotic_maps(job)
    if domain == "root_finding":
        return evaluate_root_finding(job)
    if domain == "signal_transform":
        return evaluate_signal_transform(job)
    if domain == "fixed_point_dynamics":
        return evaluate_fixed_point(job)
    if domain == "sequence_generation":
        return evaluate_sequence_generation(job)
    if domain == "classification_boundary":
        return evaluate_classification_boundary(job)
    if domain == "symbolic_identity_search":
        return evaluate_symbolic_identity(job)
    if domain in {"sorting", "graph_shortest_path", "scheduling", "parsing", "constraint_solving",
                  "stream_processing", "compression", "cache_eviction", "load_balancing", "anomaly_detection"}:
        return evaluate_cs_domain(job)
    return {
        "domain": domain,
        "available": False,
        "message": "No domain-specific evaluator registered for this domain.",
    }


def make_handler(state: AppState):
    class Handler(BaseHTTPRequestHandler):
        server_version = "AlgorithmicGenesisWebGUI/1.0"

        def do_GET(self):  # noqa: N802
            parsed = urlparse(self.path)
            path = parsed.path
            qs = parse_qs(parsed.query)

            if path == "/api/catalog":
                return json_response(self, load_catalog(state.cli))

            if path == "/api/jobs":
                return json_response(self, {"jobs": [j.write_status() for j in state.jobs.values()]})

            if path.startswith("/api/job/"):
                parts = path.strip("/").split("/")
                if len(parts) < 3:
                    return json_response(self, {"error": "missing job id"}, 400)
                job = state.get_job(parts[2])
                if not job:
                    return json_response(self, {"error": "unknown job"}, 404)
                if len(parts) == 3:
                    return json_response(self, job.write_status())
                if parts[3] == "result":
                    if job.result_path.exists():
                        return text_response(self, job.result_path.read_text(encoding="utf-8"), "application/json; charset=utf-8")
                    return json_response(self, {"error": "result not ready"}, 404)
                if parts[3] == "report":
                    if job.report_path.exists():
                        return text_response(self, job.report_path.read_text(encoding="utf-8"), "text/markdown; charset=utf-8")
                    return json_response(self, {"error": "report not ready"}, 404)
                if parts[3] == "log":
                    if job.log_path.exists():
                        return text_response(self, job.log_path.read_text(encoding="utf-8", errors="replace"))
                    return json_response(self, {"error": "log not ready"}, 404)
                if parts[3] == "artifacts":
                    files = []
                    if job.export_dir.exists():
                        for p in sorted(job.export_dir.glob("*")):
                            if p.is_file():
                                files.append({"name": p.name, "size": p.stat().st_size})
                    return json_response(self, {"files": files})
                if parts[3] == "file" and len(parts) >= 5:
                    name = "/".join(parts[4:])
                    target = (job.export_dir / name).resolve()
                    if not str(target).startswith(str(job.export_dir.resolve())) or not target.exists():
                        return json_response(self, {"error": "file not found"}, 404)
                    ctype = "text/plain; charset=utf-8"
                    if target.suffix == ".json":
                        ctype = "application/json; charset=utf-8"
                    elif target.suffix == ".md":
                        ctype = "text/markdown; charset=utf-8"
                    return text_response(self, target.read_text(encoding="utf-8", errors="replace"), ctype)
                if parts[3] == "domain-eval":
                    try:
                        return json_response(self, evaluate_domain(job))
                    except Exception as exc:  # noqa: BLE001
                        return json_response(self, {"error": str(exc)}, 500)
                if parts[3] == "test":
                    tests = sorted(job.export_dir.glob("test_*_algorithm.py"))
                    results = []
                    for t in tests:
                        proc = subprocess.run([sys.executable, str(t)], cwd=str(job.export_dir), text=True,
                                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
                        results.append({"file": t.name, "returncode": proc.returncode, "output": proc.stdout})
                    return json_response(self, {"tests": results})
                return json_response(self, {"error": "unknown job endpoint"}, 404)

            if path == "/" or path == "/index.html":
                return self.serve_static("index.html")
            return self.serve_static(path.lstrip("/"))

        def do_POST(self):  # noqa: N802
            parsed = urlparse(self.path)
            if parsed.path == "/api/start":
                length = int(self.headers.get("Content-Length", "0"))
                body = self.rfile.read(length).decode("utf-8")
                try:
                    manifest = json.loads(body)
                    job = state.start_job(manifest)
                    return json_response(self, job.write_status(), 201)
                except Exception as exc:  # noqa: BLE001
                    return json_response(self, {"error": str(exc)}, 400)
            return json_response(self, {"error": "unknown endpoint"}, 404)

        def serve_static(self, rel: str):
            target = (STATIC / rel).resolve()
            if not str(target).startswith(str(STATIC.resolve())) or not target.exists() or target.is_dir():
                return json_response(self, {"error": "not found"}, 404)
            ctype = "text/plain; charset=utf-8"
            if target.suffix == ".html":
                ctype = "text/html; charset=utf-8"
            elif target.suffix == ".css":
                ctype = "text/css; charset=utf-8"
            elif target.suffix == ".js":
                ctype = "application/javascript; charset=utf-8"
            return text_response(self, target.read_text(encoding="utf-8"), ctype)

        def log_message(self, fmt, *args):
            sys.stderr.write("[%s] %s\n" % (self.log_date_time_string(), fmt % args))

    return Handler


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--cli", default=None, help="Path to ag_cli executable")
    args = parser.parse_args()
    cli = guess_cli(args.cli)
    state = AppState(cli)
    print(f"Algorithmic Genesis WebGUI")
    print(f"Repository: {ROOT}")
    print(f"CLI:        {cli}")
    print(f"URL:        http://{args.host}:{args.port}")
    server = ThreadingHTTPServer((args.host, args.port), make_handler(state))
    server.serve_forever()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
