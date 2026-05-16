#!/usr/bin/env bash
set -euo pipefail
./build/ag_cli evolve --population 16 --generations 3 --seed 42 --json smoke_evolve.json
./build/ag_cli snn --neurons 32 --steps 20 --seed 42 --json smoke_snn.json
./build/ag_cli random --count 8 --seed 42
