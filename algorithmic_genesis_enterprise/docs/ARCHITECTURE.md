# Architecture

## Layer 1: Portable Substrate Runtime

The original brief requests a bare-metal custom OS. This repository uses a portable C++ runtime boundary
instead. It exposes deterministic memory-owning data structures and explicit feature probes. The next step
toward bare metal would be replacing the process runtime with a custom allocator and direct device queue layer.

## Layer 2: SNN and Evolution Engine

The SNN stores neurons in struct-of-arrays layout:

- voltage
- threshold
- leak_rate
- last_spike_time
- CSR-like connection offsets
- connection indices
- weights

This avoids per-neuron heap allocation and makes future OpenCL/CUDA kernels straightforward.

The evolutionary engine stores genomes as linear genes. Each gene references earlier genes, producing an
acyclic expression graph. This deliberately avoids Python/C++ object trees during hot evaluation.

## Layer 3: Mathematical Engine

The current mathematical engine evaluates expression graphs against a target function and rewards:

- accuracy
- novelty
- stability
- complexity

The design supports adding symbolic export, category-diagram export, and LLVM code generation later.

## Layer 4: Persistence

SQLite stores candidate identity, score vector, expression, and signature. If SQLite is absent, the core still builds.

## Extension points

- replace `eval_genome` with bytecode/JIT execution
- move SNN `step` into OpenCL
- use archive signatures for novelty search at population scale
- add graph isomorphism checks
- persist full genome blobs


## OpenCL execution layer

The OpenCL layer now has three concrete responsibilities:

1. `opencl-probe` creates a real context and command queue and reports platform/device.
2. `random` executes the `generate_random_numbers` kernel and only falls back to CPU on runtime failure.
3. `snn --backend auto|opencl` can execute a compact GPU-resident LIF simulation with per-neuron
   membrane updates and reduction kernels for spike counts / mean voltage.

This intentionally separates two SNN models:

- CPU SNN: recurrent graph with STDP and mutable edge weights.
- OpenCL SNN: flat data-parallel LIF field optimized for GPU execution and backend verification.

The JSON output includes `backend`, `opencl`, and `backend_error`, so benchmarks can distinguish
real GPU execution from fallback execution.


## OpenCL recurrent LIF/STDP backend

The production GPU path uses CSR synapse buffers generated from the same seeded topology as the CPU network. The host stores outgoing adjacency as the canonical representation and builds incoming CSR mirrors for atomics-free voltage integration.

### Memory contract

| Buffer | Shape | Purpose |
|---|---:|---|
| `outgoing_offsets` | `neurons + 1` | edge ranges for each source neuron |
| `outgoing_indices` | `synapses` | destination neuron for each edge |
| `weights` | `synapses` | mutable STDP weights |
| `incoming_offsets` | `neurons + 1` | incoming edge ranges for each destination |
| `incoming_sources` | `synapses` | source neuron for each incoming edge |
| `incoming_edge_ids` | `synapses` | index into canonical `weights` |
| `voltage` | `neurons` | membrane state |
| `spikes` / `prev_spikes` | `neurons` | current and previous spike frontier |
| `last_spike_time` | `neurons` | STDP timing state |

### Scheduling

Per simulation step:

1. `lif_recurrent_step` integrates incoming spikes from `prev_spikes`, applies leak and deterministic counter-based external drive, writes `spikes`, and updates `last_spike_time`.
2. `stdp_update_edges` updates mutable edge weights for active source neurons from the previous spike frontier.
3. `reduce_u32` computes spikes per step.
4. `copy_u32` advances `spikes` into `prev_spikes`.

### Rationale

The design avoids non-portable OpenCL float atomics. It accepts a one-step synaptic latency, which is reported in JSON as `backend_model: "lif_recurrent_stdp_one_tick_latency"`. This is an explicit architecture tradeoff, not a hidden approximation.

## Profiling and OpenCL tuning layer

The OpenCL backend is now instrumented at the API boundary. It reports:

- total wall-clock execution time,
- setup/program/buffer time,
- kernel enqueue time,
- host readback time,
- host transfer volume,
- selected workgroup size,
- global neuron range,
- neuron-steps per second.

The profiling design is intentionally host-side and portable. OpenCL event profiling can be added later,
but host-side timing already exposes the dominant enterprise bottleneck: frequent CPU/GPU synchronization.

### Compact mode

The original SNN JSON returned one spike count per step. This is useful for visualization, but it forces
host readback once per step. Compact mode accumulates spike counts on the device and reads the total once,
which makes GPU benchmarking representative of compute throughput rather than JSON/debug telemetry.

### Fair CPU/GPU comparison

The CPU backend supports both:

- `lif_recurrent_stdp_immediate`
- `lif_recurrent_stdp_one_tick_latency_cpu`

The OpenCL backend uses:

- `lif_recurrent_stdp_one_tick_latency`

The one-tick mode avoids non-portable float atomics and gives a stable cross-vendor execution model.
