# WebGUI Multi-Domain Evaluation

The Research Console now evaluates generated algorithms across mathematics and core computer-science domains.

## Supported domain evaluators

- `chaotic_maps`: orbit entropy, Lyapunov-like divergence, periodicity, attractor spread, return-map diversity, baseline comparison against logistic/tent/sine maps.
- `root_finding`: generated `root_refine` is compared against a bisection baseline on quadratic, cubic, trigonometric, flat and oscillatory cases.
- `signal_transform`: generated `signal_morph` is tested on step, sine, impulse and ramp signals for finiteness, energy ratio and smoothness.
- `fixed_point_dynamics`: generated `fixed_point` is tested on multiple seeds for convergence and one-step consistency.
- `sequence_generation`: generated `predict_next` is tested on arithmetic, constant and adversarial sequences.
- `classification_boundary`: generated scalar boundary is evaluated on a sign-classification smoke dataset.
- `symbolic_identity_search`: generated residual kernel is scanned for finite residual behavior and residual statistics.
- `sorting`: generated `adaptive_sort` is checked against sorted output.
- `graph_shortest_path`: generated graph search is checked against a known shortest path.
- `scheduling`: generated scheduler is measured by makespan and fairness.
- `parsing`: generated parser/repairer is measured by repair count.
- `constraint_solving`: generated CSP solver is checked for satisfied assignment.
- `stream_processing`: generated stream analyzer is checked for online statistics and anomaly markers.
- `compression`: generated compressor is checked for roundtrip length and compression ratio.
- `cache_eviction`: generated cache policy is evaluated by hit ratio.
- `load_balancing`: generated balancer is measured by makespan/fairness.
- `anomaly_detection`: generated detector is measured by detected outlier indices.

## Endpoint

```text
GET /api/job/<job_id>/domain-eval
```

The response is also persisted as:

```text
webui_runs/<job_id>/domain_evaluation.json
```

## Research interpretation

These evaluators are not mathematical proofs. They are smoke/benchmark gates designed to decide whether a generated algorithm candidate deserves deeper comparison against production-grade baselines.
