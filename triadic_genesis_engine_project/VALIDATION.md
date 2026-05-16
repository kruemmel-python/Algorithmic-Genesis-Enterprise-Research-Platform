# Validation

Validated locally with Python 3.13.5.

```bash
PYTHONPATH=src python -m unittest discover -s tests
PYTHONPATH=src python -m triadic_genesis_engine.cli self-test
PYTHONPATH=src python -m triadic_genesis_engine.cli benchmark --json benchmark.json
PYTHONPATH=src python -m triadic_genesis_engine.cli forecast --data examples/sample_series.csv --horizon 8 --json result.json
```

Results:

- Unit tests: passed.
- Self-test: passed.
- Benchmark JSON written.
- Forecast JSON written.

The sandbox printed an unrelated spreadsheet runtime warmup warning during interpreter startup; the commands exited successfully.
