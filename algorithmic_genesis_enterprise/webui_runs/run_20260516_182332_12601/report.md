# Guided Algorithmic Genesis Experiment

## Experiment

- Name: `newTest_2`
- Domain: `sequence_generation`
- Profile: `chaos_exploration`
- Generator: `snn`
- Fitness backend: `opencl-vm`
- Base seed: `2026`
- Guided seed: `11127991758665979999`
- Population: `256`
- Generations: `250`

## Selected Formula / Strategy Parts

- `tanh` — Bounded hyperbolic tangent. (nonlinearity/saturation)
- `curvature` — Difference between local and previous slopes. (slope/geometry)
- `secant_mix` — Weight for secant candidate. (policy_output/method_mix)
- `bracket_guard` — Never leave [lo, hi]. (safety/invariant)
- `finite_sanitize` — NaN/Inf containment. (safety/numeric)
- `zscore` — Standard-deviation anomaly score. (cs_core_anomaly/statistics)
- `recency` — LRU-style recency pressure. (cs_core_cache/eviction)
- `reuse_distance` — Reuse-distance prediction pressure. (cs_core_cache/prediction)
- `outlier_gate` — Bounded anomaly decision gate. (cs_core_anomaly/decision)
- `reconstruction_guard` — Decode/encode validation invariant. (cs_core_compression/safety)
- `edge_relaxation` — Relax candidate distances over edges. (cs_core_graph/dijkstra)
- `delimiter_stack` — Bounded stack for syntax matching. (cs_core_parsing/syntax)
- `fairness_penalty` — Penalize uneven load distribution. (cs_core_scheduling/load_balance)
- `deadline_pressure` — Deadline-aware scheduling pressure. (cs_core_scheduling/deadline)
- `mid` — Current interval midpoint. (geometry/bisection)
- `prev_x` — Previous accepted point. (memory/history)
- `edge_balance` — Balance between residuals near bracket edges. (geometry/bracket)
- `online_mean` — Streaming mean update. (cs_core_streaming/statistics)
- `history` — Residual improvement memory. (memory/trend)
- `residual_ratio` — Relative residual strength. (normalization/convergence)
- `nwidth` — log1p of interval width. (normalization/scale)
- `x` — Current candidate position. (primitive/state)
- `fx` — Residual at current candidate. (primitive/residual)
- `damping` — Learned step damping. (policy_output/step)
- `fhi` — Residual at upper bracket endpoint. (primitive/bracket)
- `bracket_slope` — Secant slope over [lo, hi]. (slope/secant)
- `bisection` — Safe midpoint fallback. (strategy/baseline)
- `secant` — Secant/interpolation candidate. (strategy/interpolation)
- `inverse_quadratic_hint` — Three-point curvature-aware interpolation pressure. (strategy/interpolation)
- `worker_load` — Current worker load signal. (cs_core_load_balancing/state)

## Search Interpretation

The selected parts are compiled into a guided seed, enlarged expressive genome budget, feature-pressure metadata and safety constraints. They act as human mathematical intuition injected into the Genesis search field, while the final candidate still has to pass novelty, nontriviality, VM differential checks and exported-code tests.

# Algorithmic Genesis: Executable Algorithm Discovery

## Acceptance

- Accepted: `true`
- Nontriviality: `0.984883`
- Algorithm score: `0.965823`

## Warnings

- archive behavior diversity is low; increase novelty pressure or switch domain

## Discovered Algorithm

- Name: `ag_sequence_extrapolator_cf5cb9a3f619`
- Kind: `sequence_extrapolator`
- Domain: `sequence_generation`
- Kernel: `K(x) = ((logabs(x) - logabs(tanh(x))) + tanh(tanh(x)))`

### What it does

A generated sequence extrapolator that converts recent deltas through the discovered kernel to predict the next value.

### Mathematical contract

Given at least two scalar observations, the algorithm predicts a finite next value from the latest value, local delta and the generated kernel.

### Pseudocode

```text
delta=x[n]-x[n-1]; curvature=K(delta); return sanitize(x[n]+delta+0.1*tanh(curvature))
```

### Complexity

Time O(iterations * 112) for iterative modes or O(n * 112) for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.

### Validation protocol

Validate CPU/Python/C++/OpenCL outputs on a shared sample grid and enforce finite outputs under NaN, Inf and gas exhaustion.

## Kernel Genealogy

- Kernel ID: `14942021777568159384`
- Parent A: `2974952580124812632`
- Parent B: `17911700625671222968`
- Birth generation: `250`
- Origin: `crossover`
- Mutation trace: `crossover:2974952580124812632+17911700625671222968;mutate`

## Fingerprints

- AST: `ast:fae4ba3a878cfa8a`
- Behavior: `beh:9e588a2004b2de10`
- Derivative: `der:834ef6de96edd73e`
- Stability: `sta:cec38449691193c1`
- Complexity: `cmp:g112:c100`

## Exported Code

- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.py`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.hpp`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.cl`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_manifest.json`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_README.md`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\test_ag_sequence_extrapolator_cf5cb9a3f619_algorithm.py`

## Important interpretation

This is an executable mathematical algorithm candidate, not a proved theorem. It is suitable for empirical validation, comparison against known baselines and further evolutionary refinement.
