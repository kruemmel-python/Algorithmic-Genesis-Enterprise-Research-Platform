# API

```python
from triadic_genesis_engine import TriadicGenesisEngine, TriadicConfig

values = [1.0, 1.2, 1.3, 2.1]
engine = TriadicGenesisEngine(TriadicConfig(horizon=8))
report = engine.run(values)

print(report.metrics)
print(report.forecasts)
print(report.to_json())
```

## Main classes

- `TriadicConfig`
- `TriadicGenesisEngine`
- `TriadicStep`
- `TriadicReport`

## Main functions

- `sanitize`
- `k_explore`
- `k_anchor`
- `k_delta`
- `synthetic_series`
- `benchmark`
- `parse_series`
