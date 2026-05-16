
from __future__ import annotations

from dataclasses import asdict, dataclass, field
from collections import deque
from typing import Iterable, Sequence, Any
import json
import math
import statistics

DEFAULT_GAS = 10_000


def sanitize(v: Any, limit: float = 1_000_000.0) -> float:
    """Numeric poison containment shared by all kernels."""
    try:
        x = float(v)
    except Exception:
        return 0.0
    if not math.isfinite(x):
        return 0.0
    return max(-limit, min(limit, x))


def expclamp(x: float) -> float:
    return math.exp(max(-20.0, min(20.0, sanitize(x))))


def logabs(x: float) -> float:
    return math.log(abs(sanitize(x)) + 1e-9)


@dataclass(frozen=True)
class AlgorithmLineage:
    """Exact generated artifacts fused by this project."""
    name: str
    kind: str
    domain: str
    kernel_expression: str
    source_run: str
    role_in_fusion: str
    algorithm_score: float
    nontriviality: float


LINEAGE: tuple[AlgorithmLineage, ...] = (
    AlgorithmLineage(
        name="ag_chaotic_map_explorer_0c55ac1bc71d",
        kind="chaotic_map_explorer",
        domain="chaotic_maps",
        kernel_expression="tanh(tanh(sin((sin((x + x)) + sin((x + x))))))",
        source_run="run_20260516_171657_17436",
        role_in_fusion="bounded entropic oscillator / exploration phase",
        algorithm_score=0.949405,
        nontriviality=0.984122,
    ),
    AlgorithmLineage(
        name="ag_chaotic_map_explorer_dc0f1a3431df",
        kind="chaotic_map_explorer",
        domain="chaotic_maps",
        kernel_expression="tanh(tanh(((x * cos(x)) / (x * cos(x)))))",
        source_run="run_20260516_173702_22107",
        role_in_fusion="contractive anchor / trust stabilizer",
        algorithm_score=0.952024,
        nontriviality=0.984937,
    ),
    AlgorithmLineage(
        name="ag_sequence_extrapolator_cf5cb9a3f619",
        kind="sequence_extrapolator",
        domain="sequence_generation",
        kernel_expression="((logabs(x) - logabs(tanh(x))) + tanh(tanh(x)))",
        source_run="run_20260516_182332_12601",
        role_in_fusion="delta-curvature predictor / temporal extrapolator",
        algorithm_score=0.965823,
        nontriviality=0.984883,
    ),
)


# ---------------------------------------------------------------------------
# Exact generated kernels
# ---------------------------------------------------------------------------

def k_explore(x: float, gas_left: int = DEFAULT_GAS) -> float:
    """Kernel from ag_chaotic_map_explorer_0c55ac1bc71d."""
    if gas_left <= 0:
        return 0.0
    x = sanitize(x)
    try:
        return sanitize(math.tanh(math.tanh(math.sin((math.sin((x + x)) + math.sin((x + x)))))))
    except Exception:
        return 0.0


def k_anchor(x: float, gas_left: int = DEFAULT_GAS) -> float:
    """Kernel from ag_chaotic_map_explorer_dc0f1a3431df.

    Except at numeric singularities, this kernel contracts to approximately
    tanh(tanh(1)), which is deliberately used as a stable anchor signal.
    """
    if gas_left <= 0:
        return 0.0
    x = sanitize(x)
    try:
        return sanitize(math.tanh(math.tanh(((x * math.cos(x)) / (x * math.cos(x))))))
    except Exception:
        return 0.0


def k_delta(x: float, gas_left: int = DEFAULT_GAS) -> float:
    """Kernel from ag_sequence_extrapolator_cf5cb9a3f619."""
    if gas_left <= 0:
        return 0.0
    x = sanitize(x)
    try:
        return sanitize(((logabs(x) - logabs(math.tanh(x))) + math.tanh(math.tanh(x))))
    except Exception:
        return 0.0


@dataclass
class TriadicConfig:
    """Configuration for the fused engine."""
    window: int = 64
    horizon: int = 16
    gas_limit: int = DEFAULT_GAS
    entropy_bins: int = 32
    learning_rate: float = 0.12
    shock_threshold: float = 3.0
    exploration_gain: float = 0.23
    anchor_gain: float = 0.37
    delta_gain: float = 0.19
    min_weight: float = 0.03
    max_abs_prediction: float = 1_000_000.0
    warmup: int = 4


@dataclass
class OnlineStats:
    count: int = 0
    mean: float = 0.0
    m2: float = 0.0

    def update(self, x: float) -> None:
        x = sanitize(x)
        self.count += 1
        d = x - self.mean
        self.mean += d / self.count
        self.m2 += d * (x - self.mean)

    @property
    def variance(self) -> float:
        return self.m2 / max(1, self.count - 1)

    @property
    def stdev(self) -> float:
        return math.sqrt(max(1e-12, self.variance))


@dataclass
class AttractorMemory:
    window: int
    bins: int
    values: deque[float] = field(default_factory=deque)
    pairs: deque[tuple[float, float]] = field(default_factory=deque)

    def push(self, x: float) -> None:
        x = sanitize(x)
        if self.values:
            self.pairs.append((self.values[-1], x))
        self.values.append(x)
        while len(self.values) > self.window:
            self.values.popleft()
        while len(self.pairs) > self.window:
            self.pairs.popleft()

    def entropy(self) -> float:
        if not self.values:
            return 0.0
        counts = [0] * self.bins
        lo, hi = -1.0, 1.0
        for v in self.values:
            u = max(lo, min(hi, sanitize(v)))
            idx = min(self.bins - 1, max(0, int((u - lo) / (hi - lo) * self.bins)))
            counts[idx] += 1
        total = sum(counts)
        if total <= 0:
            return 0.0
        h = 0.0
        for c in counts:
            if c:
                p = c / total
                h -= p * math.log(p + 1e-15, 2)
        return sanitize(h / math.log(self.bins, 2))

    def return_diversity(self) -> float:
        if not self.pairs:
            return 0.0
        bins = max(4, int(math.sqrt(self.bins * self.bins)))
        counts = [0] * (bins * bins)
        for a, b in self.pairs:
            ia = min(bins - 1, max(0, int((max(-1, min(1, a)) + 1) * 0.5 * bins)))
            ib = min(bins - 1, max(0, int((max(-1, min(1, b)) + 1) * 0.5 * bins)))
            counts[ia * bins + ib] += 1
        total = sum(counts)
        h = 0.0
        for c in counts:
            if c:
                p = c / total
                h -= p * math.log(p + 1e-15, 2)
        return sanitize(h / math.log(len(counts), 2))


@dataclass
class BranchPrediction:
    name: str
    value: float
    weight: float
    error_ema: float


@dataclass
class TriadicStep:
    index: int
    observation: float
    prediction: float
    residual: float
    anomaly_score: float
    regime: str
    trust: float
    chaos_phase: float
    anchor: float
    entropy: float
    return_diversity: float
    branch_predictions: list[BranchPrediction]


@dataclass
class TriadicReport:
    config: TriadicConfig
    lineage: tuple[AlgorithmLineage, ...]
    steps: list[TriadicStep]
    forecasts: list[float]
    metrics: dict[str, float]
    summary: dict[str, Any]

    def to_dict(self) -> dict[str, Any]:
        return {
            "config": asdict(self.config),
            "lineage": [asdict(x) for x in self.lineage],
            "steps": [
                {
                    **{k: v for k, v in asdict(s).items() if k != "branch_predictions"},
                    "branch_predictions": [asdict(b) for b in s.branch_predictions],
                }
                for s in self.steps
            ],
            "forecasts": self.forecasts,
            "metrics": self.metrics,
            "summary": self.summary,
        }

    def to_json(self, indent: int = 2) -> str:
        return json.dumps(self.to_dict(), indent=indent)


class TriadicGenesisEngine:
    """Triadic fusion of chaotic exploration, contractive anchoring and sequence extrapolation.

    The engine is intentionally not a classical forecast model. It is a new
    experimental execution model:
    - one generated chaotic kernel supplies bounded exploration pressure,
    - one generated chaotic kernel supplies a contraction anchor,
    - one generated sequence kernel supplies delta curvature,
    - a mycelial branch mixer adapts branch weights by recent error.
    """

    def __init__(self, config: TriadicConfig | None = None) -> None:
        self.config = config or TriadicConfig()
        self.stats = OnlineStats()
        self.memory = AttractorMemory(window=self.config.window, bins=self.config.entropy_bins)
        self.branch_error = {
            "delta_curvature": 1.0,
            "chaos_modulated": 1.0,
            "anchor_smoothed": 1.0,
            "mycelial_consensus": 1.0,
        }
        self.last_values: deque[float] = deque(maxlen=max(4, self.config.window))
        self.trust = 0.5
        self.chaos_phase = 0.123
        self._last_branch_values: dict[str, float] = {}

    def _soft_weights(self) -> dict[str, float]:
        inv = {k: 1.0 / (self.config.min_weight + max(1e-9, v)) for k, v in self.branch_error.items()}
        total = sum(inv.values()) or 1.0
        weights = {k: max(self.config.min_weight, v / total) for k, v in inv.items()}
        total = sum(weights.values()) or 1.0
        return {k: v / total for k, v in weights.items()}

    def _regime(self, anomaly: float, entropy: float, retdiv: float) -> str:
        if anomaly >= self.config.shock_threshold:
            return "shock"
        if entropy < 0.08 and retdiv < 0.08:
            return "contractive"
        if entropy > 0.45 or retdiv > 0.35:
            return "exploratory"
        return "drifting"

    def _branch_predict(self, observation: float) -> dict[str, float]:
        values = list(self.last_values)
        x = sanitize(observation)
        prev = values[-1] if values else x
        prev2 = values[-2] if len(values) > 1 else prev
        delta = sanitize(x - prev)
        curvature = sanitize(x - 2.0 * prev + prev2)

        self.chaos_phase = sanitize(math.tanh(k_explore(self.chaos_phase + 0.17 * delta)))
        anchor = k_anchor(self.chaos_phase + 1e-6)  # avoid singular zero path
        delta_curve = k_delta(delta + 0.2 * curvature)

        branch = {
            "delta_curvature": sanitize(x + delta + self.config.delta_gain * math.tanh(delta_curve),
                                       self.config.max_abs_prediction),
            "chaos_modulated": sanitize(x + delta * (1.0 + self.config.exploration_gain * self.chaos_phase),
                                        self.config.max_abs_prediction),
            "anchor_smoothed": sanitize((1.0 - self.config.anchor_gain) * x + self.config.anchor_gain * (x + delta * anchor),
                                        self.config.max_abs_prediction),
        }
        weights = self._soft_weights()
        consensus = sum(branch[k] * weights[k] for k in branch)
        branch["mycelial_consensus"] = sanitize(consensus, self.config.max_abs_prediction)
        self._last_branch_values = branch
        return branch

    def _update_branch_errors(self, observed: float, branch_values: dict[str, float]) -> None:
        for name, pred in branch_values.items():
            err = abs(sanitize(observed - pred))
            old = self.branch_error.get(name, 1.0)
            self.branch_error[name] = sanitize((1.0 - self.config.learning_rate) * old + self.config.learning_rate * err)

    def observe(self, value: float, index: int | None = None) -> TriadicStep:
        x = sanitize(value)
        idx = len(self.last_values) if index is None else int(index)

        if not self.last_values:
            prediction = x
            branch = {k: x for k in self.branch_error}
        else:
            branch = self._branch_predict(x)
            weights = self._soft_weights()
            prediction = sanitize(sum(branch[k] * weights.get(k, 0.0) for k in branch),
                                  self.config.max_abs_prediction)

        residual = sanitize(x - prediction)
        self.stats.update(x)
        z = abs(residual) / self.stats.stdev
        self.memory.push(math.tanh(x / (self.stats.stdev + 1e-9)))
        entropy = self.memory.entropy()
        return_diversity = self.memory.return_diversity()

        anchor = k_anchor(self.chaos_phase + 1e-6)
        anomaly = sanitize(z + 0.5 * abs(math.tanh(k_delta(residual))) + 0.25 * max(0.0, 0.2 - entropy))
        regime = self._regime(anomaly, entropy, return_diversity)

        # Trust is raised by low residual and reduced by shocks; anchor prevents collapse.
        self.trust = max(0.0, min(1.0, 0.85 * self.trust + 0.15 * (1.0 / (1.0 + anomaly)) + 0.05 * abs(anchor)))

        if len(self.last_values) >= 1:
            self._update_branch_errors(x, self._last_branch_values)

        self.last_values.append(x)

        weights = self._soft_weights()
        bps = [
            BranchPrediction(name=name, value=sanitize(branch[name]), weight=sanitize(weights.get(name, 0.0)),
                             error_ema=sanitize(self.branch_error.get(name, 0.0)))
            for name in branch
        ]

        return TriadicStep(
            index=idx,
            observation=x,
            prediction=prediction,
            residual=residual,
            anomaly_score=anomaly,
            regime=regime,
            trust=sanitize(self.trust),
            chaos_phase=sanitize(self.chaos_phase),
            anchor=sanitize(anchor),
            entropy=sanitize(entropy),
            return_diversity=sanitize(return_diversity),
            branch_predictions=bps,
        )

    def forecast(self, horizon: int | None = None) -> list[float]:
        horizon = self.config.horizon if horizon is None else max(0, int(horizon))
        if not self.last_values:
            return []
        # Simulate forward on a copy of minimal state to avoid altering live state.
        clone = self.clone()
        out: list[float] = []
        x = clone.last_values[-1]
        for _ in range(horizon):
            branch = clone._branch_predict(x)
            weights = clone._soft_weights()
            x = sanitize(sum(branch[k] * weights.get(k, 0.0) for k in branch), clone.config.max_abs_prediction)
            clone.last_values.append(x)
            clone.memory.push(math.tanh(x / (clone.stats.stdev + 1e-9)))
            out.append(x)
        return out

    def clone(self) -> "TriadicGenesisEngine":
        other = TriadicGenesisEngine(self.config)
        other.stats = OnlineStats(self.stats.count, self.stats.mean, self.stats.m2)
        other.memory = AttractorMemory(self.memory.window, self.memory.bins, deque(self.memory.values), deque(self.memory.pairs))
        other.branch_error = dict(self.branch_error)
        other.last_values = deque(self.last_values, maxlen=self.last_values.maxlen)
        other.trust = self.trust
        other.chaos_phase = self.chaos_phase
        other._last_branch_values = dict(self._last_branch_values)
        return other

    def run(self, values: Iterable[float], horizon: int | None = None) -> TriadicReport:
        steps = [self.observe(v, i) for i, v in enumerate(values)]
        forecasts = self.forecast(horizon)
        metrics = self.metrics(steps)
        return TriadicReport(
            config=self.config,
            lineage=LINEAGE,
            steps=steps,
            forecasts=forecasts,
            metrics=metrics,
            summary={
                "project": "Triadic Genesis Engine",
                "claim": "new project-level fusion of three generated algorithms; not a proof of mathematical novelty",
                "step_count": len(steps),
                "forecast_horizon": len(forecasts),
                "dominant_regime": _most_common([s.regime for s in steps]) if steps else "none",
                "final_trust": steps[-1].trust if steps else self.trust,
            },
        )

    def metrics(self, steps: Sequence[TriadicStep]) -> dict[str, float]:
        if not steps:
            return {}
        abs_errors = [abs(s.residual) for s in steps]
        anomalies = [s.anomaly_score for s in steps]
        trust = [s.trust for s in steps]
        return {
            "mae": sanitize(statistics.fmean(abs_errors)),
            "rmse": sanitize(math.sqrt(statistics.fmean([e * e for e in abs_errors]))),
            "max_abs_error": sanitize(max(abs_errors)),
            "mean_anomaly_score": sanitize(statistics.fmean(anomalies)),
            "max_anomaly_score": sanitize(max(anomalies)),
            "mean_trust": sanitize(statistics.fmean(trust)),
            "final_entropy": sanitize(steps[-1].entropy),
            "final_return_diversity": sanitize(steps[-1].return_diversity),
            "shock_count": float(sum(1 for s in steps if s.regime == "shock")),
        }


def _most_common(values: Sequence[str]) -> str:
    if not values:
        return "none"
    counts: dict[str, int] = {}
    for v in values:
        counts[v] = counts.get(v, 0) + 1
    return max(counts, key=counts.get)


def parse_series(text: str) -> list[float]:
    """Parse CSV/newline/space separated numeric text."""
    tokens = []
    for line in text.replace(";", ",").splitlines():
        tokens.extend(replace_commas(line).split())
    return [sanitize(t) for t in tokens if t.strip()]


def replace_commas(line: str) -> str:
    return line.replace(",", " ")


def synthetic_series(n: int = 160) -> list[float]:
    """Deterministic benchmark series with drift, seasonal signal and shocks."""
    out = []
    for i in range(n):
        x = 0.03 * i + math.sin(i * 0.17) + 0.35 * math.sin(i * 0.031)
        if i in {45, 92, 130}:
            x += 4.0 if i != 92 else -3.2
        out.append(sanitize(x))
    return out


def benchmark(values: Sequence[float] | None = None, horizon: int = 16) -> dict[str, Any]:
    values = list(values) if values is not None else synthetic_series()
    engine = TriadicGenesisEngine(TriadicConfig(horizon=horizon))
    report = engine.run(values, horizon=horizon)

    # One-step baselines.
    naive_errors = []
    delta_errors = []
    for i in range(2, len(values)):
        naive = values[i - 1]
        delta = values[i - 1] + (values[i - 1] - values[i - 2])
        naive_errors.append(abs(values[i] - naive))
        delta_errors.append(abs(values[i] - delta))

    tri_errors = [abs(s.residual) for s in report.steps[2:]]
    return {
        "engine": report.to_dict(),
        "baseline": {
            "naive_mae": sanitize(statistics.fmean(naive_errors) if naive_errors else 0.0),
            "delta_mae": sanitize(statistics.fmean(delta_errors) if delta_errors else 0.0),
            "triadic_mae": sanitize(statistics.fmean(tri_errors) if tri_errors else 0.0),
            "relative_to_naive": sanitize((statistics.fmean(tri_errors) / (statistics.fmean(naive_errors) + 1e-12)) if naive_errors and tri_errors else 0.0),
            "relative_to_delta": sanitize((statistics.fmean(tri_errors) / (statistics.fmean(delta_errors) + 1e-12)) if delta_errors and tri_errors else 0.0),
        },
    }
