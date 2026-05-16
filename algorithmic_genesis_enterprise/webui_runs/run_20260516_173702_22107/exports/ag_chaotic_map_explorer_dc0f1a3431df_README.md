# ag_chaotic_map_explorer_dc0f1a3431df

## Purpose

A generated iterated-map algorithm that applies the discovered kernel as a bounded nonlinear recurrence and estimates orbit energy and Lyapunov-like divergence.

## Mathematical Kernel

`K(x) = tanh(tanh(((x * cos(x)) / (x * cos(x)))))`

## Contract

Given x0 and an iteration budget, the algorithm returns a bounded orbit and a finite divergence estimate. It is observational, not a theorem of chaos.

## Pseudocode

```text
x=x0; for t in 0..steps: x=tanh(K(x)); accumulate orbit and log local separation; return orbit statistics
```

## Complexity

Time O(iterations * 102) for iterative modes or O(n * 102) for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.

## Validation

Validate boundedness, reproducibility and finite Lyapunov-like estimates across seeds. Compare against logistic/tent-map baselines.

## Exported Files

- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.py`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.hpp`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_algorithm.cl`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_manifest.json`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\ag_chaotic_map_explorer_dc0f1a3431df_README.md`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_173702_22107\\exports\test_ag_chaotic_map_explorer_dc0f1a3431df_algorithm.py`
