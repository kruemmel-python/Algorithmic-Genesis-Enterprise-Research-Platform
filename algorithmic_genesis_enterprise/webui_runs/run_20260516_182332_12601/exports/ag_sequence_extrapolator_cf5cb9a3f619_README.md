# ag_sequence_extrapolator_cf5cb9a3f619

## Purpose

A generated sequence extrapolator that converts recent deltas through the discovered kernel to predict the next value.

## Mathematical Kernel

`K(x) = ((logabs(x) - logabs(tanh(x))) + tanh(tanh(x)))`

## Contract

Given at least two scalar observations, the algorithm predicts a finite next value from the latest value, local delta and the generated kernel.

## Pseudocode

```text
delta=x[n]-x[n-1]; curvature=K(delta); return sanitize(x[n]+delta+0.1*tanh(curvature))
```

## Complexity

Time O(iterations * 112) for iterative modes or O(n * 112) for vector modes; memory O(1) streaming, O(n) only when an output vector is requested.

## Validation

Validate CPU/Python/C++/OpenCL outputs on a shared sample grid and enforce finite outputs under NaN, Inf and gas exhaustion.

## Exported Files

- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.py`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.hpp`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_algorithm.cl`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_manifest.json`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\ag_sequence_extrapolator_cf5cb9a3f619_README.md`
- `D:\\algorithmic_genesis_enterprise\\webui_runs\\run_20260516_182332_12601\\exports\test_ag_sequence_extrapolator_cf5cb9a3f619_algorithm.py`
