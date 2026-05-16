# Architecture: Triadic Genesis Engine

## 1. Source analysis

The three source algorithms had complementary behavior:

| Algorithm | Strength | Weakness | Fusion role |
|---|---|---|---|
| Chaotic map `0c55...` | bounded nonlinear orbit | can collapse to mild bounded oscillation | exploration phase |
| Chaotic map `dc0...` | strong contractive stability | low behavior diversity | trust anchor |
| Sequence extrapolator `cf5...` | local delta curvature | not a complete stream system alone | forecast branch |

## 2. New execution model

The engine introduces a mycelial branch mixer:

```text
observation stream
  ↓
online stats + attractor memory
  ↓
three generated kernels
  ↓
branch predictors
  ↓
error-adaptive weights
  ↓
consensus forecast
  ↓
regime + anomaly + trust
```

## 3. Branches

### delta_curvature

Uses the sequence extrapolator kernel on local delta and curvature.

### chaos_modulated

Uses the chaotic exploration kernel as bounded phase pressure to modulate extrapolation.

### anchor_smoothed

Uses the contractive chaotic kernel as stabilizer.

### mycelial_consensus

Weighted consensus of the branches, where weights are updated by recent prediction error.

## 4. Metrics

The report emits:

- MAE
- RMSE
- max absolute error
- mean anomaly score
- max anomaly score
- mean trust
- final entropy
- final return-map diversity
- shock count

## 5. Failure modes

This is a research prototype. It can be worse than simple baselines on smooth linear signals and can overreact to early shocks. Use the included benchmark before relying on it.
