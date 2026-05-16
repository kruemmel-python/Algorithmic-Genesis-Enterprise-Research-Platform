#!/usr/bin/env python3
"""Simple benchmark driver for repeated CLI evolution runs."""
from __future__ import annotations
import argparse
import json
import statistics
import subprocess
import time
from pathlib import Path

def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--binary", type=Path, required=True)
    p.add_argument("--runs", type=int, default=5)
    p.add_argument("--population", type=int, default=64)
    p.add_argument("--generations", type=int, default=30)
    args = p.parse_args()

    durations: list[float] = []
    fitness: list[float] = []
    for i in range(args.runs):
        start = time.perf_counter()
        cp = subprocess.run([
            str(args.binary), "evolve",
            "--population", str(args.population),
            "--generations", str(args.generations),
            "--seed", str(1000 + i),
        ], check=True, text=True, capture_output=True)
        durations.append(time.perf_counter() - start)
        fitness.append(json.loads(cp.stdout)["best"]["fitness"])

    print(json.dumps({
        "runs": args.runs,
        "duration_mean_s": statistics.mean(durations),
        "duration_stdev_s": statistics.stdev(durations) if len(durations) > 1 else 0.0,
        "fitness_mean": statistics.mean(fitness),
        "fitness_max": max(fitness),
    }, indent=2))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
