# Executable Mathematical Algorithm Synthesis

This layer turns Algorithmic Genesis from an expression discoverer into an algorithm author.

## Command

```bash
ag_cli discover-algorithm \
  --domain chaotic_maps \
  --population 512 \
  --generations 500 \
  --generator snn \
  --fitness-backend opencl-vm \
  --vm-differential \
  --archive genesis_archive.jsonl \
  --export-dir algorithm_discoveries \
  --report algorithm_report.md \
  --json algorithm_result.json
```

## What is generated

Each run evolves a mathematical kernel `K(x)` and wraps it into a named algorithm artifact:

- Python implementation with finite sanitization and gas limits
- C++ header with `extern "C"` safe ABI
- OpenCL kernel for batch evaluation
- JSON manifest
- Markdown report
- Python smoke test

## Algorithm kinds

`--algorithm-kind auto` maps domains to algorithm families:

| Domain | Algorithm family |
|---|---|
| `chaotic_maps` | `chaotic_map_explorer` |
| `root_finding` | `root_refiner` |
| `fixed_point_dynamics` | `fixed_point_iterator` |
| `signal_transform` | `signal_morpher` |
| `sequence_generation` | `sequence_extrapolator` |
| `symbolic_identity_search` | `identity_residual_reducer` |
| other | `generated_scalar_transform` |

## Important interpretation

Generated artifacts are executable mathematical algorithm candidates, not proven theorems.
They must be validated against baselines and stress tests. The system records genealogy,
fingerprints and nontriviality scores so discoveries can be audited and refined.
