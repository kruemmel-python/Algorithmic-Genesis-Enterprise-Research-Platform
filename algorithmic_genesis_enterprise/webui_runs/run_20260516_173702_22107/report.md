# Guided Algorithmic Genesis Experiment

## Experiment

- Name: `NewAlgo_2`
- Domain: `chaotic_maps`
- Profile: `aggressive`
- Generator: `snn`
- Fitness backend: `opencl-vm`
- Base seed: `2026`
- Guided seed: `9932943000844328532`
- Population: `256`
- Generations: `250`

## Selected Formula / Strategy Parts

- `tanh` — Bounded hyperbolic tangent. (nonlinearity/saturation)
- `curvature` — Difference between local and previous slopes. (slope/geometry)
- `secant_mix` — Weight for secant candidate. (policy_output/method_mix)
- `bracket_guard` — Never leave [lo, hi]. (safety/invariant)
- `finite_sanitize` — NaN/Inf containment. (safety/numeric)
- `width_ratio` — Width relative to previous width. (geometry/convergence)
- `nfx` — Residual scaled by bracket residual magnitude. (normalization/residual)
- `fx` — Residual at current candidate. (primitive/residual)
- `lo` — Lower bracket endpoint. (primitive/bracket)
- `bracket_slope` — Secant slope over [lo, hi]. (slope/secant)
- `secant` — Secant/interpolation candidate. (strategy/interpolation)
- `stagnation_reset` — Fallback when progress stalls. (safety/recovery)
- `nfhi` — Upper residual scaled by local scale. (normalization/bracket)
- `scale` — Numerical scale used for normalization. (normalization/safety)
- `relpos` — Position of x in [lo, hi]. (geometry/normalization)

## Search Interpretation

The selected parts are compiled into a guided seed, enlarged expressive genome budget, feature-pressure metadata and safety constraints. They act as human mathematical intuition injected into the Genesis search field, while the final candidate still has to pass novelty, nontriviality, VM differential checks and exported-code tests.

# Algorithmic Genesis: Executable Algorithm Discovery

## Acceptance

- Accepted: `true`
- Nontriviality: `0.984937`
- Algorithm score: `0.952024`

## Warnings

- archive behavior diversity is low; increase novelty pressure or switch domain

## Discovered Algorithm

- Name: `ag_chaotic_map_explorer_dc0f1a3431df`
- Kind: `chaotic_map_explorer`
- Domain: `chaotic_maps`
- Kernel: `K(x) = tanh(tanh(((x * cos(x)) / (x * cos(x)))))`

### What it does

A generated iterated-map algorithm that applies the discovered kernel as a bounded nonlinear recurrence and estimates orbit energy and Lyapunov-like divergence.

### Mathematical contract

Given x0 and an iteration budget, the algorithm returns a bounded orbit and a finite divergence estimate. It is observational, not a theorem of chaos.

### Pseudocode

```text
x=x0; for t in 0..steps: x=tanh(K(x)); accumulate orbit and log local separation; return orbit statistics
```

### Complexity

Time O(iterations * 102) for iterative modes or O(n * 102) for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.

### Validation protocol

Validate boundedness, reproducibility and finite Lyapunov-like estimates across seeds. Compare against logistic/tent-map baselines.

## Kernel Genealogy

- Kernel ID: `15856921624472168439`
- Parent A: `13732847496234342306`
- Parent B: `7881560602109991524`
- Birth generation: `250`
- Origin: `crossover`
- Mutation trace: `crossover:13732847496234342306+7881560602109991524;mutate`

## Fingerprints

- AST: `ast:85e494cdca18f36a`
- Behavior: `beh:026e86f0170b99c3`
- Derivative: `der:2fc36940b6aa46ea`
- Stability: `sta:d985140d2fa01a63`
- Complexity: `cmp:g102:c100`

## Exported Code

- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.py`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.hpp`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.cl`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_manifest.json`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_README.md`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\test_ag_chaotic_map_explorer_dc0f1a3431df_algorithm.py`

## Important interpretation

This is an executable mathematical algorithm candidate, not a proved theorem. It is suitable for empirical validation, comparison against known baselines and further evolutionary refinement.
