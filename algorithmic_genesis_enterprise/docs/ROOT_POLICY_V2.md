# Root Policy V2: High-Dimensional Algorithm Synthesis

This version upgrades root-finding synthesis from a scalar contextual update `K_ctx(x, f(x), f(lo), f(hi), history, width)` to a high-dimensional policy:

```text
RootContext[30] -> RootPolicy{bias, damping, secant_mix, bisection_mix, relaxation_mix, trust_delta}
```

## RootContext fields

```text
x, fx, lo, hi, flo, fhi, mid, width, relpos,
prev_x, prev_fx, prev2_x, prev2_fx,
bracket_slope, local_slope, prev_slope, curvature,
improvement, stagnation, trust, reject_rate,
scale, nfx, nflo, nfhi, nwidth, sign_change,
edge_balance, residual_ratio, width_ratio
```

## Algorithmic flow

1. Build a RootContext from bracket, residual, slope, curvature and history.
2. Evaluate the discovered scalar kernel over multiple normalized context projections.
3. Produce a bounded RootPolicy:
   - `bias`
   - `damping`
   - `secant_mix`
   - `bisection_mix`
   - `relaxation_mix`
   - `trust_delta`
4. Mix candidates:
   - bisection midpoint
   - bracket secant estimate
   - learned relaxation step
5. Clamp to the bracket.
6. Reject unsafe or non-improving candidates.
7. Update trust, reject rate, stagnation and history.
8. Preserve bracket invariants.

## Why this matters

This is no longer a formula search over `K(x)`. It is adaptive numerical strategy synthesis.
The generated algorithm can learn when to behave like bisection, when to behave like a secant method,
when to dampen steps, and when to distrust its learned update.
