# Genesis Discovery Layer

This layer re-centers Algorithmic Genesis on its original purpose: creation, evaluation, preservation and explanation of new mathematical artifacts.

## Seven implemented capabilities

| Capability | Implementation |
|---|---|
| Discovery Archive | `DiscoveryArchive` JSONL persistence with provenance and fingerprints |
| Historical Novelty | AST, behavior, derivative, stability and complexity fingerprints |
| Mathematical Domains | Multiple target domains selected by `--domain` |
| SNN as Generator | `--generator snn` creates expression genomes from spike-resonance token streams |
| Genealogy | Parent IDs, birth generation, origin and mutation trace on every genome |
| Code Synthesis | Export to Python, C++ and OpenCL evaluators |
| Discovery Reports | Markdown report with best candidate, fingerprints, archive growth and genealogy |

## Recommended run

```bash
ag_cli evolve \
  --population 512 \
  --generations 500 \
  --seed 42 \
  --domain chaotic_maps \
  --generator snn \
  --archive genesis_archive.jsonl \
  --save-top 64 \
  --export-dir discoveries \
  --report discovery_report.md \
  --json result.json
```

## Important interpretation

The system does not prove that a generated artifact is mathematically novel in the human literature. It records structural and behavioral novelty relative to its own archive and exposes artifacts for further validation.
