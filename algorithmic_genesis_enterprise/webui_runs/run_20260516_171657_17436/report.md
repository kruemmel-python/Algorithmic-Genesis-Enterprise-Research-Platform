# Guided Algorithmic Genesis Experiment

## Experiment

- Name: `chaotic_maps_exploration`
- Domain: `chaotic_maps`
- Profile: `chaos_exploration`
- Generator: `snn`
- Fitness backend: `opencl-vm`
- Base seed: `2026`
- Guided seed: `5158663407634940192`
- Population: `256`
- Generations: `250`

## Selected Formula / Strategy Parts

- `tanh` — Bounded hyperbolic tangent. (nonlinearity/saturation)
- `curvature` — Difference between local and previous slopes. (slope/geometry)
- `secant_mix` — Weight for secant candidate. (policy_output/method_mix)
- `bracket_guard` — Never leave [lo, hi]. (safety/invariant)
- `finite_sanitize` — NaN/Inf containment. (safety/numeric)
- `width` — Current interval width. (geometry/scale)
- `prev2_fx` — Residual at second previous point. (memory/history)
- `prev2_x` — Second previous accepted point. (memory/history)
- `cubic` — Cubic shaping term. (nonlinearity/polynomial)
- `scale` — Numerical scale used for normalization. (normalization/safety)
- `flo` — Residual at lower bracket endpoint. (primitive/bracket)
- `local_slope` — Local finite-difference slope. (slope/secant)
- `regula_falsi` — False-position candidate pressure. (strategy/interpolation)

## Search Interpretation

The selected parts are compiled into a guided seed, enlarged expressive genome budget, feature-pressure metadata and safety constraints. They act as human mathematical intuition injected into the Genesis search field, while the final candidate still has to pass novelty, nontriviality, VM differential checks and exported-code tests.

# Algorithmic Genesis: Executable Algorithm Discovery

## Acceptance

- Accepted: `true`
- Nontriviality: `0.984122`
- Algorithm score: `0.949405`

## Discovered Algorithm

- Name: `ag_chaotic_map_explorer_0c55ac1bc71d`
- Kind: `chaotic_map_explorer`
- Domain: `chaotic_maps`
- Kernel: `K(x) = tanh(tanh(sin((sin((x + x)) + sin((x + x))))))`

### What it does

A generated iterated-map algorithm that applies the discovered kernel as a bounded nonlinear recurrence and estimates orbit energy and Lyapunov-like divergence.

### Mathematical contract

Given x0 and an iteration budget, the algorithm returns a bounded orbit and a finite divergence estimate. It is observational, not a theorem of chaos.

### Pseudocode

```text
x=x0; for t in 0..steps: x=tanh(K(x)); accumulate orbit and log local separation; return orbit statistics
```

### Complexity

Time O(iterations * 101) for iterative modes or O(n * 101) for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.

### Validation protocol

Validate boundedness, reproducibility and finite Lyapunov-like estimates across seeds. Compare against logistic/tent-map baselines.

## Kernel Genealogy

- Kernel ID: `888805736780203503`
- Parent A: `11478962082037989604`
- Parent B: `14577481835215239426`
- Birth generation: `250`
- Origin: `crossover`
- Mutation trace: `crossover:11478962082037989604+14577481835215239426;mutate`

## Fingerprints

- AST: `ast:9534a739d9cb4451`
- Behavior: `beh:bf62a54eb230e4d0`
- Derivative: `der:87fc1ac08725c10f`
- Stability: `sta:d985130d2fa018b0`
- Complexity: `cmp:g101:c100`

## Exported Code

- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\ag_chaotic_map_explorer_0c55ac1bc71d_algorithm.py`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\ag_chaotic_map_explorer_0c55ac1bc71d_algorithm.hpp`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\ag_chaotic_map_explorer_0c55ac1bc71d_algorithm.cl`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\ag_chaotic_map_explorer_0c55ac1bc71d_manifest.json`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\ag_chaotic_map_explorer_0c55ac1bc71d_README.md`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_171657_17436\\exports\test_ag_chaotic_map_explorer_0c55ac1bc71d_algorithm.py`

## Important interpretation

This is an executable mathematical algorithm candidate, not a proved theorem. It is suitable for empirical validation, comparison against known baselines and further evolutionary refinement.
