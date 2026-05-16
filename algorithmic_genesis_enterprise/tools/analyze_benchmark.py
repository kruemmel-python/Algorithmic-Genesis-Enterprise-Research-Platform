#!/usr/bin/env python3
"""Summarize Algorithmic Genesis benchmark-snn JSON.

Usage:
  python tools/analyze_benchmark.py benchmark_snn.json
"""
from __future__ import annotations

import json
import statistics
import sys
from collections import defaultdict
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: analyze_benchmark.py benchmark_snn.json", file=sys.stderr)
        return 2
    data = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
    groups: dict[str, list[dict]] = defaultdict(list)
    for run in data.get("runs", []):
        groups[run.get("label", "unknown")].append(run)

    print("label,backend,model,runs,wall_ms_mean,wall_ms_min,readback_ms_mean,transfer_bytes_mean,neuron_steps_per_second_mean")
    for label, runs in sorted(groups.items()):
        walls = [float(r["profile"].get("wall_ms", 0.0)) for r in runs]
        readbacks = [float(r["profile"].get("readback_ms", 0.0)) for r in runs]
        transfers = [float(r["profile"].get("transfer_bytes", 0.0)) for r in runs]
        throughput = [float(r["profile"].get("neuron_steps_per_second", 0.0)) for r in runs]
        first = runs[0]
        print(
            f"{label},"
            f"{first.get('backend','')},"
            f"{first.get('backend_model','')},"
            f"{len(runs)},"
            f"{statistics.mean(walls):.6f},"
            f"{min(walls):.6f},"
            f"{statistics.mean(readbacks):.6f},"
            f"{statistics.mean(transfers):.1f},"
            f"{statistics.mean(throughput):.3f}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
