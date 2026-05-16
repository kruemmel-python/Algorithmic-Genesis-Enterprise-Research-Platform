# WebGUI Domain Evaluation Panels

This module adds domain-specific research evaluation to the Algorithmic Genesis Research Console.

## Scope

The first implemented domain panel is `chaotic_maps`.

After a WebGUI experiment finishes and exports a generated algorithm, click:

```text
Domain-Evaluation ausführen
```

The server imports the exported `*_algorithm.py` artifact in an isolated dynamic module namespace and evaluates the generated map against known baselines.

## Chaotic Maps Metrics

The evaluation computes:

| Metric | Meaning |
|---|---|
| `orbit_entropy` | Normalized Shannon entropy of the orbit histogram. High values indicate broad attractor coverage. |
| `return_map_diversity` | Normalized entropy of `(x_t, x_{t+1})` pairs. High values indicate richer map geometry. |
| `lyapunov_like` | Finite-difference divergence estimate. Positive values suggest sensitive dependence. |
| `lyapunov_band` | `contractive`, `neutral`, or `chaotic`. |
| `periodicity.best_period` | Best repeating lag detected in the second half of the orbit. |
| `periodicity.periodicity_score` | High means strongly periodic. |
| `attractor_spread` | `max(orbit)-min(orbit)`. |
| `finite_ratio` | Fraction of finite values. |
| `bounded` | Whether the orbit stayed in `[-1, 1]`. |
| `sensitivity_to_seed` | Divergence after perturbing initial state by epsilon. |

## Baselines

The panel compares generated maps against:

- Logistic map `r=4`
- Tent map `mu=2`
- Sine map `r=1`

The comparison is empirical. It is not a proof of novelty.

## Visuals

The WebGUI renders:

1. Orbit time series.
2. Histogram / attractor coverage.
3. Return map `x_t -> x_{t+1}`.
4. Baseline comparison table.
5. Raw `domain_evaluation.json`.

## API

```http
GET /api/job/<job_id>/domain-eval
```

For `chaotic_maps`, the server writes:

```text
webui_runs/<job_id>/domain_evaluation.json
```

## Research Use

Use this panel to reject degenerate maps early:

- Very low entropy
- Very high periodicity
- Tiny attractor spread
- Non-finite values
- No distinction from simple baselines

A promising candidate typically has bounded finite output, nontrivial spread, nonzero return-map diversity, and either neutral or positive Lyapunov-like divergence.
