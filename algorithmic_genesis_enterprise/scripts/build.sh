#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build -DAG_ENABLE_OPENCL="${AG_ENABLE_OPENCL:-OFF}" -DAG_ENABLE_SQLITE="${AG_ENABLE_SQLITE:-ON}"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
