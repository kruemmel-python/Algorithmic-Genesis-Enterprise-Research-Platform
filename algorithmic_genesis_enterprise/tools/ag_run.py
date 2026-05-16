#!/usr/bin/env python3
"""Python orchestration wrapper for the native Algorithmic Genesis CLI."""
from __future__ import annotations
import argparse
import json
import subprocess
from pathlib import Path
from typing import Any

def run(binary: Path, population: int, generations: int, seed: int, db: Path | None) -> dict[str, Any]:
    cmd = [
        str(binary), "evolve",
        "--population", str(population),
        "--generations", str(generations),
        "--seed", str(seed),
    ]
    if db is not None:
        cmd += ["--db", str(db)]
    completed = subprocess.run(cmd, check=True, text=True, capture_output=True)
    return json.loads(completed.stdout)

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--population", type=int, default=64)
    parser.add_argument("--generations", type=int, default=50)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--db", type=Path)
    args = parser.parse_args()
    result = run(args.binary, args.population, args.generations, args.seed, args.db)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
