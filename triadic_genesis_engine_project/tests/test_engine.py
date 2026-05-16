
import json
import math
import unittest

from triadic_genesis_engine import TriadicGenesisEngine, TriadicConfig
from triadic_genesis_engine.core import (
    k_explore, k_anchor, k_delta, synthetic_series, benchmark, parse_series
)


class TriadicEngineTests(unittest.TestCase):
    def test_kernels_are_finite(self):
        for x in [-10, -1, 0, 1e-9, 1, 10, float("nan"), float("inf")]:
            self.assertTrue(math.isfinite(k_explore(x)))
            self.assertTrue(math.isfinite(k_anchor(x)))
            self.assertTrue(math.isfinite(k_delta(x)))

    def test_engine_runs_and_forecasts(self):
        values = synthetic_series(96)
        report = TriadicGenesisEngine(TriadicConfig(horizon=12)).run(values)
        self.assertEqual(len(report.steps), 96)
        self.assertEqual(len(report.forecasts), 12)
        self.assertTrue(all(math.isfinite(x) for x in report.forecasts))
        self.assertIn("mae", report.metrics)
        self.assertIn("dominant_regime", report.summary)

    def test_json_serializable(self):
        values = parse_series("1,2,3\n4 5")
        report = TriadicGenesisEngine(TriadicConfig(horizon=4)).run(values)
        dumped = report.to_json()
        self.assertIn("Triadic Genesis Engine", dumped)
        self.assertEqual(len(json.loads(dumped)["forecasts"]), 4)

    def test_benchmark(self):
        out = benchmark(synthetic_series(80), horizon=8)
        self.assertIn("engine", out)
        self.assertIn("baseline", out)
        self.assertGreaterEqual(out["baseline"]["relative_to_naive"], 0.0)


if __name__ == "__main__":
    unittest.main()
