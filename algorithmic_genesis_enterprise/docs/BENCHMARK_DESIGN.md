# Benchmark Design

## Baseline

- CPU expression evaluation with linear genomes.
- SNN simulation with struct-of-arrays layout.
- Deterministic SplitMix64 RNG.

## Metrics

- candidates evaluated per second
- best fitness after N generations
- memory per genome
- SNN steps per second
- spike rate stability
- persistence throughput

## Hypotheses

1. Linear genomes reduce allocation pressure compared with expression-tree objects.
2. Struct-of-arrays SNN improves cache locality compared with vector-of-neuron objects.
3. Novelty archive slows premature convergence but may reduce maximum short-run accuracy.

## Counter-hypotheses

1. Small populations may be dominated by RNG noise.
2. Novelty score may reward useless structural drift.
3. Expression interpretation may become the bottleneck before memory layout matters.

## Measurement risks

- compiler optimization differences
- CPU frequency scaling
- OpenCL driver startup overhead
- SQLite fsync behavior
