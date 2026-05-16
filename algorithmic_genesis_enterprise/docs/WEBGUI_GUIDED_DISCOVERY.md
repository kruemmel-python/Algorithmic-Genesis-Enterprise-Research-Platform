# Formula-Part Guided Discovery WebGUI

This layer turns human mathematical intuition into a guided Genesis search.

## Start

```bat
D:
cd \algorithmic_genesis_enterprise
python webui\server.py --host 127.0.0.1 --port 8765 --cli build_opencl\Release\ag_cli.exe
```

Open:

```text
http://127.0.0.1:8765
```

## Workflow

1. Select a domain, profile, population and generations.
2. Select at least 3 formula/strategy parts.
3. Start the experiment.
4. The server writes an experiment manifest and runs:

```bat
ag_cli.exe discover-guided --experiment <manifest>
```

5. After completion, inspect result JSON, report, generated Python/C++/OpenCL code and run exported tests.

## CLI without WebGUI

```bat
build_opencl\Release\ag_cli.exe list-formula-parts --json formula_parts.json
```

Create `experiments\guided_root.json`:

```json
{
  "name": "guided_root_policy",
  "domain": "root_finding",
  "profile": "safety_first",
  "generator": "snn",
  "fitness_backend": "opencl-vm",
  "seed": 2026,
  "population": 256,
  "generations": 250,
  "selected_parts": [
    "tanh",
    "curvature",
    "secant_mix",
    "bracket_guard",
    "finite_sanitize"
  ],
  "archive": "guided_archive.jsonl",
  "export_dir": "guided_algorithms",
  "report": "guided_report.md",
  "json": "guided_result.json"
}
```

Run:

```bat
build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_root.json
```

## Research interpretation

Selected parts are not just labels. The manifest is compiled into:
- guided seed perturbation,
- expressive genome budget,
- domain/profile pressure,
- safety constraints,
- export/report metadata.

The generated algorithm still has to pass nontriviality scoring, VM differential checks, safe code export and generated Python tests.
