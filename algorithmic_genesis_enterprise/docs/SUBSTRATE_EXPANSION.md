# Enterprise Substrate Expansion

This document records the full professional expansion requested for Algorithmic Genesis.

## 1. GPU-resident Bytecode VM

Genomes are compiled into a compact linear bytecode:

```text
op:uint32 value:float a:uint32 b:uint32
```

The representation keeps the original DAG invariant: operands point backwards after modulo normalization. `vm-test` executes a differential test between CPU `eval_genome` and the OpenCL VM path.

## 2. Somnia Shadow-Phase

The SNN runtime supports a periodic consolidation phase:

```bash
--somnia --somnia-period 512 --somnia-duration 64 --somnia-decay 0.0005
```

During Somnia, input drive is reduced and weight decay is applied to inactive synapses. The goal is to suppress runaway potentiation and support stable attractor formation.

## 3. CCQ-inspired Quantization

`--ccq` / `--quantized-weights` projects weights onto an 8-bit codebook. `substrate-probe` validates the packed 4xINT8-per-uint32 representation and reports memory savings.

## 4. Mycelial Growth Field

`--mycelial-growth` enables zero-sum structural plasticity. The current production-safe mode does not grow unbounded arrays. Instead, new growth events cannibalize weak synapses inside a fixed arena.

## 5. Continuous Differential Hardware Probing

`--hardware-probe` records differential-check metadata and exposes drift fields in JSON. The CPU backend acts as the control baseline; OpenCL runs expose probe counts and maximum observed error.

## 6. Numeric Poison Containment

Generated math and kernels sanitize NaN/Inf deterministically to zero and clamp extreme values. This prevents single mutated candidates from poisoning full population evaluation.

## 7. Safe ABI Export

Generated Python, C++ and OpenCL exports are gas-limited or ABI-bounded. Native exports provide:

```cpp
extern "C" double evaluate_safely(const double* inputs, int* gas_left, int* error_code);
```

Every exported candidate is an artifact, not a trusted program.
