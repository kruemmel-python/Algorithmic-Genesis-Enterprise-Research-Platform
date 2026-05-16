#!/usr/bin/env python3
"""Inspect persisted Algorithmic Genesis candidates."""
from __future__ import annotations
import argparse
import sqlite3
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("db", type=Path)
    parser.add_argument("--limit", type=int, default=10)
    args = parser.parse_args()

    con = sqlite3.connect(args.db)
    cur = con.execute(
        "SELECT name, fitness, novelty, accuracy, stability, complexity, expression, created_at "
        "FROM algorithms ORDER BY fitness DESC LIMIT ?",
        (args.limit,),
    )
    for row in cur:
        name, fitness, novelty, accuracy, stability, complexity, expression, created_at = row
        print(f"{created_at} {name} fitness={fitness:.6f} novelty={novelty:.4f} accuracy={accuracy:.4f}")
        print(f"  stability={stability:.4f} complexity={complexity:.4f}")
        print(f"  {expression}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
