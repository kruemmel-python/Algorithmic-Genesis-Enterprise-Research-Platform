#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 webui/server.py --host 127.0.0.1 --port 8765 --cli build/ag_cli
