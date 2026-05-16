# Algorithmic Genesis Enterprise Research Platform

**Version:** Multi-Core Computer Science Discovery + Guided WebGUI + Domain Evaluation  
**Zielgruppe:** Entwickler, Forscher, numerische Algorithmiker, Performance Engineers  
**Betriebssystem-Fokus:** Windows 11 / PowerShell / Visual Studio 2022 Build Tools  
**GPU-Pfad:** OpenCL über vcpkg  
**WebGUI:** lokale Research Console unter `http://127.0.0.1:8765`

---

## 1. Was ist dieses Projekt?

Algorithmic Genesis ist eine Forschungsplattform zur **geführten und autonomen Entdeckung neuer mathematischer und informatischer Algorithmen**.

Das System kombiniert:

- evolutionäre Suche
- SNN-inspirierte Generatoren
- OpenCL-beschleunigte Fitness- und VM-Pfade
- Discovery Archive
- Novelty Fingerprints
- Formula-/Strategy-Part Guided Discovery
- Code-Synthese nach Python, C++ Header und OpenCL
- WebGUI mit Forschungsworkflow
- Domain-spezifische Evaluationen
- Sicherheitsmechanismen wie Gas-Limits, NaN/Inf-Sanitizing und Bracket Guards

Der wichtigste Zweck ist nicht, bestehende Algorithmen nur nachzubauen, sondern **neue ausführbare Algorithmus-Kandidaten** zu erzeugen, zu testen, zu archivieren und vergleichbar zu machen.

---

## 2. Repository-Struktur

```text
algorithmic_genesis_enterprise/
  CMakeLists.txt
  include/ag/
  src/
  kernels/
  tests/
  tools/
  webui/
  experiments/
  docs/
  build_opencl/
  webui_runs/
```

Wichtige Bereiche:

```text
src/ag_algorithm.cpp       Algorithmus-Synthese und Code-Export
src/ag_genesis.cpp         Discovery Archive, Fingerprints, Reports
src/ag_substrate.cpp       Substratmechaniken, VM, CCQ, Sicherheit
src/ag_snn.cpp             CPU/OpenCL SNN
src/ag_opencl.cpp          OpenCL-Probe und OpenCL-Kernelpfade
webui/server.py            Research Console Backend
webui/static/              HTML/CSS/JS WebGUI
experiments/*.json         Guided Discovery Beispielmanifeste
docs/                      Architektur- und Evaluationsdokumentation
```

---

## 3. Voraussetzungen

### 3.1 Windows Build Tools

Installiert sein sollten:

- Visual Studio 2022 Build Tools
- C++ Build Tools
- Windows SDK
- CMake for Windows
- Python 3.11+
- Git
- vcpkg
- AMD/NVIDIA/Intel GPU-Treiber mit OpenCL Runtime

### 3.2 Wichtiger Hinweis zu CMake

Nutze unter Windows **nicht** das MSYS-CMake, sondern explizit:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" --version
```

Wenn `where cmake` zuerst `C:\msys64\usr\bin\cmake.exe` zeigt, setze den Pfad für die aktuelle Shell:

```powershell
$env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
```

---

## 4. Alle PowerShell-Befehle als Einzeiler

Alle folgenden Befehle sind bewusst als **PowerShell-Einzeiler** geschrieben. Du kannst sie einzeln kopieren.

---

## 5. Projektverzeichnis wechseln

```powershell
Set-Location D:\algorithmic_genesis_enterprise
```

---

## 6. Toolchain prüfen

```powershell
where.exe cmake
```

```powershell
where.exe cl
```

```powershell
where.exe git
```

```powershell
python --version
```

```powershell
echo $env:VCPKG_ROOT
```

---

## 7. CMake-Pfad temporär korrigieren

```powershell
$env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
```

---

## 8. vcpkg vorbereiten

```powershell
Set-Location C:\vcpkg; git pull; .\bootstrap-vcpkg.bat
```

---

## 9. OpenCL über vcpkg installieren

```powershell
C:\vcpkg\vcpkg.exe install opencl:x64-windows
```

Optional:

```powershell
C:\vcpkg\vcpkg.exe install clinfo:x64-windows
```

---

## 10. CPU-Build ohne OpenCL

```powershell
Set-Location D:\algorithmic_genesis_enterprise; if (Test-Path build_cpu) { Remove-Item build_cpu -Recurse -Force }; & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build_cpu -G "Visual Studio 17 2022" -A x64 -DAG_ENABLE_OPENCL=OFF -DAG_ENABLE_SQLITE=OFF
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\cmake.exe" --build build_cpu --config Release --parallel
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\ctest.exe" --test-dir build_cpu -C Release --output-on-failure
```

---

## 11. OpenCL-Build

```powershell
Set-Location D:\algorithmic_genesis_enterprise; if (Test-Path build_opencl) { Remove-Item build_opencl -Recurse -Force }; & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build_opencl -G "Visual Studio 17 2022" -A x64 -DAG_ENABLE_OPENCL=ON -DAG_ENABLE_SQLITE=OFF -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\cmake.exe" --build build_opencl --config Release --parallel
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\ctest.exe" --test-dir build_opencl -C Release --output-on-failure
```

---

## 12. OpenCL prüfen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe opencl-probe
```

---

## 13. CLI-Hilfe anzeigen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe --help
```

---

## 14. SNN ausführen

CPU:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe snn --neurons 2048 --connections 8 --steps 5000 --seed 42 --backend cpu --semantic latency --compact --profile --json snn_cpu_profile.json
```

OpenCL:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe snn --neurons 2048 --connections 8 --steps 5000 --seed 42 --backend opencl --semantic latency --local-size 256 --compact --profile --json snn_gpu_profile.json
```

Großer OpenCL-Substratlauf:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe snn --neurons 65536 --connections 8 --steps 10000 --seed 42 --backend opencl --semantic latency --somnia --somnia-period 512 --somnia-duration 64 --ccq --hardware-probe --hardware-probe-interval 256 --compact --profile --json substrate_snn.json
```

---

## 15. VM-Test und Substrat-Probe

GPU-VM-Differentialtest:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe vm-test --genome-length 64 --samples 512 --seed 42 --local-size 256 --json vm_test.json
```

CCQ-/Speicher-Probe:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe substrate-probe --weights 1000000 --seed 42 --json substrate_probe.json
```

---

## 16. Klassische Evolution

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe evolve --population 512 --generations 500 --seed 42 --domain chaotic_maps --generator snn --fitness-backend opencl-vm --vm-differential --archive genesis_archive.jsonl --save-top 64 --export-dir discoveries --report discovery_report.md --json genesis_result.json
```

---

## 17. Algorithmus-Discovery ohne WebGUI

Root-Finding:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-algorithm --domain root_finding --population 1024 --generations 1000 --seed 2026 --generator snn --fitness-backend opencl-vm --vm-differential --archive genesis_archive.jsonl --export-dir root_policy_algorithms --report root_policy_report.md --json root_policy_result.json
```

Chaotic Maps:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-algorithm --domain chaotic_maps --population 512 --generations 500 --seed 2026 --generator snn --fitness-backend opencl-vm --vm-differential --archive genesis_archive.jsonl --export-dir chaos_algorithms --report chaos_report.md --json chaos_result.json
```

Signal Transform:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-algorithm --domain signal_transform --population 512 --generations 500 --seed 2026 --generator snn --fitness-backend opencl-vm --vm-differential --archive genesis_archive.jsonl --export-dir signal_algorithms --report signal_report.md --json signal_result.json
```

---

## 18. Formula Parts anzeigen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe list-formula-parts --json formula_parts.json
```

---

## 19. Guided Discovery per Manifest

Allgemein:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_root_policy_example.json --archive guided_archive.jsonl --export-dir guided_algorithms --report guided_report.md --json guided_result.json
```

Sorting:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_sorting_example.json --archive guided_archive.jsonl --export-dir guided_sorting_algorithms --report guided_sorting_report.md --json guided_sorting_result.json
```

Graph Shortest Path:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_graph_example.json --archive guided_archive.jsonl --export-dir guided_graph_algorithms --report guided_graph_report.md --json guided_graph_result.json
```

Scheduling:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_scheduling_example.json --archive guided_archive.jsonl --export-dir guided_scheduling_algorithms --report guided_scheduling_report.md --json guided_scheduling_result.json
```

Constraint Solving:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_constraints_example.json --archive guided_archive.jsonl --export-dir guided_constraints_algorithms --report guided_constraints_report.md --json guided_constraints_result.json
```

Stream Processing:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_streaming_example.json --archive guided_archive.jsonl --export-dir guided_streaming_algorithms --report guided_streaming_report.md --json guided_streaming_result.json
```

---

## 20. WebGUI starten

```powershell
Set-Location D:\algorithmic_genesis_enterprise; python webui\server.py --host 127.0.0.1 --port 8765 --cli build_opencl\Release\ag_cli.exe
```

Browser öffnen:

```powershell
Start-Process "http://127.0.0.1:8765"
```

---

## 21. WebGUI-Workflow

1. WebGUI starten.
2. Domain auswählen.
3. Forschungsprofil auswählen.
4. Seed, Population, Generations setzen.
5. Mindestens 3 Formel-/Strategie-Teile auswählen.
6. Empfohlen: 5 bis 20 Teile.
7. Experiment starten.
8. Status aktualisieren.
9. Resultat prüfen.
10. Report lesen.
11. Exportierte Artefakte ansehen.
12. Exportierte Python-Tests ausführen.
13. Domain-Evaluation ausführen.

---

## 22. WebGUI-Run-Verzeichnis öffnen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Invoke-Item .\webui_runs
```

---

## 23. Exportierte Python-Tests ausführen

Einzelnen Test ausführen:

```powershell
Set-Location D:\algorithmic_genesis_enterprise\root_policy_v2_algorithms; python .\test_ag_root_refiner_10ef2ced78c6_algorithm.py
```

Alle Tests in einem Exportordner ausführen:

```powershell
Set-Location D:\algorithmic_genesis_enterprise\root_policy_v2_algorithms; Get-ChildItem test_*.py | ForEach-Object { python $_.FullName }
```

Alle Tests in einem WebGUI-Exportordner ausführen:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-ChildItem .\webui_runs -Recurse -Filter "test_*.py" | ForEach-Object { python $_.FullName }
```

---

## 24. JSON-Dateien anzeigen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-Content .\guided_result.json
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-Content .\root_policy_result.json
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-Content .\webui_runs\*\result.json
```

---

## 25. Reports anzeigen

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-Content .\guided_report.md
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Get-Content .\root_policy_report.md
```

---

## 26. Archiv-Statistiken

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe archive-stats --archive genesis_archive.jsonl
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe archive-stats --archive guided_archive.jsonl
```

---

## 27. Benchmarking

CPU vs OpenCL messen:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Measure-Command { .\build_opencl\Release\ag_cli.exe snn --neurons 2048 --connections 8 --steps 5000 --seed 42 --backend cpu --semantic latency --compact --profile --json snn_cpu_latency_profile.json }
```

```powershell
Set-Location D:\algorithmic_genesis_enterprise; Measure-Command { .\build_opencl\Release\ag_cli.exe snn --neurons 2048 --connections 8 --steps 5000 --seed 42 --backend opencl --semantic latency --compact --profile --json snn_gpu_latency_profile.json }
```

Automatischer SNN-Benchmark:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe benchmark-snn --neurons 2048 --connections 8 --steps 5000 --repeats 3 --local-sizes 64,128,256 --json benchmark_snn.json
```

Benchmark auswerten:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; python tools\analyze_benchmark.py benchmark_snn.json
```

---

## 28. Unterstützte mathematische Domains

```text
root_finding
chaotic_maps
signal_transform
fixed_point_dynamics
sequence_generation
classification_boundary
symbolic_identity_search
```

---

## 29. Unterstützte Informatik-Kernbereiche

```text
sorting
graph_shortest_path
scheduling
parsing
constraint_solving
stream_processing
compression
cache_eviction
load_balancing
anomaly_detection
```

---

## 30. Wichtige Output-Artefakte

Pro Algorithmus erzeugt das System typischerweise:

```text
*_algorithm.py
*_algorithm.hpp
*_algorithm.cl
*_manifest.json
*_README.md
test_*_algorithm.py
```

Bedeutung:

```text
*_algorithm.py        ausführbarer Python-Algorithmus
*_algorithm.hpp       C++ Header / sichere ABI-Variante
*_algorithm.cl        OpenCL Kernel oder OpenCL-kompatible Funktion
*_manifest.json       Metadaten, Fingerprints, Score, Vertrag
*_README.md           Erklärung des konkreten Algorithmus
test_*_algorithm.py   Validierungstest
```

---

## 31. Forschungsinterpretation

Ein akzeptierter Kandidat ist **kein mathematisch bewiesener Satz**. Er ist ein reproduzierbares, ausführbares Forschungsartefakt.

Gute Kandidaten erkennst du an:

```text
accepted: true
algorithm_score: hoch
nontriviality: hoch
novelty: hoch
unique_behavior: steigend
vm_differential.max_abs_error: klein
warnings: leer
exportierte Tests: ok
```

---

## 32. Troubleshooting

### Visual-Studio-Generator wird nicht gefunden

Symptom:

```text
Could not create named generator Visual Studio 17 2022
```

Fix:

```powershell
$env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
```

Dann erneut konfigurieren:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build_opencl -G "Visual Studio 17 2022" -A x64 -DAG_ENABLE_OPENCL=ON -DAG_ENABLE_SQLITE=OFF -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### OpenCL wird nicht gefunden

```powershell
C:\vcpkg\vcpkg.exe install opencl:x64-windows
```

### WebGUI startet nicht

```powershell
Set-Location D:\algorithmic_genesis_enterprise; python webui\server.py --host 127.0.0.1 --port 8765 --cli build_opencl\Release\ag_cli.exe
```

### Python-Test mit Wildcard funktioniert in CMD nicht

PowerShell verwenden:

```powershell
Get-ChildItem test_*.py | ForEach-Object { python $_.FullName }
```

### GPU-Auslastung niedrig

Das ist nicht automatisch ein Fehler. Viele Discovery-Phasen laufen CPU-seitig: Archiv, Fingerprints, Evolution, Codeexport, Report. OpenCL wird vor allem für VM-/Fitness-/SNN-Pfade genutzt.

---

## 33. Empfohlene Forschungsstrategie

Viele kurze Läufe:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; 1..10 | ForEach-Object { .\build_opencl\Release\ag_cli.exe discover-guided --experiment experiments\guided_root_policy_example.json --archive guided_archive.jsonl --export-dir "runs\root_seed_$($_)" --report "runs\root_seed_$($_).md" --json "runs\root_seed_$($_).json" }
```

Ein großer Lauf:

```powershell
Set-Location D:\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe discover-algorithm --domain root_finding --population 1024 --generations 1000 --seed 2026 --generator snn --fitness-backend opencl-vm --vm-differential --archive genesis_archive.jsonl --export-dir root_large_run --report root_large_run.md --json root_large_run.json
```

---

## 34. Sicherheitsprinzipien

Das Projekt verwendet mehrere Schutzmechanismen:

```text
sanitize()                 NaN/Inf containment
gas_left                   harte Operationsgrenze
bracket_guard              Intervallinvariante bei Root-Finding
finite_sanitize            Zahlenraumschutz
reject_penalty             schlechte Kandidaten werden gedämpft
trust_gate                 aggressive Schritte nur bei Vertrauen
OpenCL differential tests  CPU/GPU-Abgleich
```

---

## 35. Kurzfassung

```text
Build → WebGUI starten → Domain wählen → mindestens 3 Teile auswählen → Discovery starten → Artefakte testen → Domain-Evaluation ausführen → Kandidaten archivieren und vergleichen.
```
