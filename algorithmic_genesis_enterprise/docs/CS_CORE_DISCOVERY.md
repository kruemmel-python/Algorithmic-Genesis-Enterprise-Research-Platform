# Multi-Core Computer Science Algorithm Discovery

Algorithmic Genesis now supports guided discovery beyond numerical mathematics. A researcher can choose a core computer-science domain, select at least three formula/strategy parts, and let the system synthesize an executable algorithm artifact.

## Supported core CS domains

| Domain | Generated kind | Exported main function |
|---|---|---|
| `sorting` | `adaptive_sorter` | `adaptive_sort(values)` |
| `graph_shortest_path` | `graph_pathfinder` | `graph_shortest_path(graph, start, goal)` |
| `scheduling` | `schedule_optimizer` | `schedule_jobs(jobs, workers)` |
| `parsing` | `parser_repairer` | `parse_and_repair(tokens)` |
| `constraint_solving` | `constraint_solver` | `solve_constraints(domains, constraints)` |
| `stream_processing` | `stream_processor` | `stream_analyze(values)` |
| `compression` | `adaptive_compressor` | `compress_adaptive(values)` |
| `cache_eviction` | `cache_policy` | `cache_simulate(requests, capacity)` |
| `load_balancing` | `load_balancer` | `load_balance(jobs, workers)` |
| `anomaly_detection` | `anomaly_detector` | `detect_anomalies(values)` |

The original mathematical domains remain available: `root_finding`, `chaotic_maps`, `signal_transform`, `fixed_point_dynamics`, `sequence_generation`, `classification_boundary`, and `symbolic_identity_search`.

## Formula/strategy parts

The catalog now includes CS-specific pressure fields:

- Sorting: `stable_sort`, `inversion_pressure`, `adaptive_pivot`
- Graph search: `priority_queue`, `edge_relaxation`, `heuristic_potential`
- Scheduling: `deadline_pressure`, `makespan_minimize`, `fairness_penalty`
- Parsing: `delimiter_stack`, `repair_policy`, `precedence_pressure`
- Constraint solving: `variable_ordering`, `value_ordering`, `constraint_propagation`
- Streaming: `online_mean`, `online_variance`, `anomaly_pressure`
- Compression: `run_length`, `delta_coding`, `reconstruction_guard`
- Cache policy: `recency`, `frequency`, `reuse_distance`
- Load balancing: `worker_load`, `variance_penalty`, `tail_latency`
- Anomaly detection: `zscore`, `robust_median`, `outlier_gate`

These selections are compiled into a guided seed and search pressure. The final algorithm must still pass generated tests and finite/gas-limited safety checks.

## WebGUI workflow

1. Start the WebGUI.
2. Select a domain such as `graph_shortest_path`.
3. Select at least three relevant parts, for example:
   - `priority_queue`
   - `edge_relaxation`
   - `heuristic_potential`
   - `finite_sanitize`
   - `gas_limit`
4. Start discovery.
5. Run exported Python tests.
6. Run Domain Evaluation to obtain domain-specific smoke metrics.

## CLI example

```bat
build_opencl\Release\ag_cli.exe discover-guided ^
  --experiment experiments\guided_sorting_example.json ^
  --archive guided_archive.jsonl ^
  --export-dir guided_algorithms ^
  --report guided_report.md ^
  --json guided_result.json
```

## Research interpretation

Generated CS algorithms are research candidates. They are not guaranteed to beat established algorithms. Their value is that they synthesize new heuristic variants with explicit contracts, safe execution boundaries, source code exports, and reproducible manifests.
