# Roadmap

1. Proof of concept
   - current repository
   - deterministic CLI
   - CPU fallback

2. Benchmark
   - run `tools/benchmark.py`
   - add perf counters
   - record hardware metadata

3. Profiling
   - profile expression evaluation
   - profile SNN step loop
   - identify allocation hotspots

4. API stabilization
   - freeze genome serialization
   - freeze CLI JSON schema

5. Native acceleration
   - port random generation fully to OpenCL host path
   - port SNN step to OpenCL
   - add optional LLVM expression compiler

6. Testsuite
   - add fuzz tests for genome validation
   - add long-run stochastic regression tests
   - add persistence migration tests

7. Documentation
   - publish candidate scoring methodology
   - document failure modes and unsafe assumptions

8. Einsatzgrenzen
   - not suitable for safety-critical autonomous code generation
   - generated expressions must remain sandboxed
   - novelty is not proof of mathematical value
