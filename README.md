# Algorithmic Genesis Suite
<img width="1672" height="941" alt="ag" src="https://github.com/user-attachments/assets/2522ebb1-720d-4779-b3b3-38cf392078ec" />

**Zwei miteinander verbundene Forschungsprojekte für algorithmische Entdeckung, Code-Synthese und daraus abgeleitete adaptive Ausführungssysteme.**

Dieses Repository enthält nicht nur ein einzelnes Programm. Es enthält eine vollständige zweistufige Forschungsstrecke:

```text
algorithmic_genesis_enterprise/
  erzeugt, bewertet, exportiert und dokumentiert neue Algorithmus-Kandidaten

triadic_genesis_engine_project/
  verbindet drei erzeugte Algorithmus-Artefakte zu einem neuen adaptiven System
```

Die zentrale Idee lautet:

> **Algorithmic Genesis** erzeugt algorithmische Rohartefakte.  
> **Triadic Genesis Engine** zeigt, wie mehrere solcher Artefakte zu einem neuen, eigenständigen Forschungsprojekt fusioniert werden können.

Damit ist dieses Repository zugleich Generator, Experimentarchiv, Validierungsumgebung und Beispiel dafür, wie generierte Algorithmen nicht nur isoliert getestet, sondern als Bausteine größerer Systeme weiterverwendet werden können.

---

## Inhaltsverzeichnis

1. [Kurzfassung](#kurzfassung)
2. [Was liegt in diesem Repository?](#was-liegt-in-diesem-repository)
3. [Warum gibt es zwei Projekte?](#warum-gibt-es-zwei-projekte)
4. [Projekt 1: Algorithmic Genesis Enterprise](#projekt-1-algorithmic-genesis-enterprise)
5. [Projekt 2: Triadic Genesis Engine](#projekt-2-triadic-genesis-engine)
6. [Die Verbindung beider Projekte](#die-verbindung-beider-projekte)
7. [Die drei WebGUI-Runs als Ursprung der Triadic Genesis Engine](#die-drei-webgui-runs-als-ursprung-der-triadic-genesis-engine)
8. [Repository-Struktur](#repository-struktur)
9. [Schnellstart unter Windows / PowerShell](#schnellstart-unter-windows--powershell)
10. [Empfohlener Arbeitsablauf](#empfohlener-arbeitsablauf)
11. [Was passiert technisch?](#was-passiert-technisch)
12. [Validierung, Tests und Sicherheit](#validierung-tests-und-sicherheit)
13. [Wissenschaftliche Einordnung](#wissenschaftliche-einordnung)
14. [Für wen ist dieses Repository gedacht?](#für-wen-ist-dieses-repository-gedacht)
15. [Weiterführende Dokumentation](#weiterführende-dokumentation)

---

## Kurzfassung

Dieses Repository besteht aus zwei Hauptteilen:

| Ordner | Rolle | Technologie | Ergebnis |
|---|---|---|---|
| `algorithmic_genesis_enterprise/` | Discovery-, Synthese- und Evaluationsplattform | C++17, OpenCL, CMake, Python-WebGUI | Generierte Algorithmus-Artefakte, Reports, Exporte |
| `triadic_genesis_engine_project/` | Abgeleitetes Fusionsprojekt | Python-Package, CLI, WebGUI, Tests | Adaptives Stream-/Zeitreihen-System aus drei generierten Algorithmen |

**Algorithmic Genesis Enterprise** ist die Maschine, die neue Kandidaten erzeugt.  
**Triadic Genesis Engine** ist ein konkretes Folgeprojekt, das aus drei dieser Kandidaten konstruiert wurde.

Die drei Quellalgorithmen stammen aus WebGUI-Läufen im Ordner:

```text
algorithmic_genesis_enterprise/webui_runs/
```

Sie wurden anschließend in dieses Projekt übernommen:

```text
triadic_genesis_engine_project/source_algorithms/
```

Dort dienen sie als dokumentierte algorithmische Herkunftslinie für die neue Engine.

---

## Was liegt in diesem Repository?

Auf oberster Ebene liegen zwei eigenständige, aber logisch verbundene Projekte:

```text
.
├── algorithmic_genesis_enterprise/
└── triadic_genesis_engine_project/
```

Der erste Ordner ist die **Discovery-Plattform**. Dort werden algorithmische Kandidaten gesucht, bewertet, exportiert und archiviert.

Der zweite Ordner ist die **abgeleitete Engine**. Dort wird gezeigt, wie drei zuvor erzeugte Algorithmus-Artefakte zu einem neuen System verbunden werden.

Wichtig: Die beiden Unterprojekte besitzen eigene README-Dateien, weil sie separat startbar und testbar sind. Diese Root-README erklärt den Gesamtzusammenhang, die Architektur beider Teile und den Weg vom Generator zum daraus entstandenen Forschungsprojekt.

---

## Warum gibt es zwei Projekte?

Die Trennung ist bewusst gewählt.

### 1. `algorithmic_genesis_enterprise/` ist das Labor

Hier entstehen Kandidaten. Das Projekt stellt eine Umgebung bereit, in der mathematische und informatische Suchräume exploriert werden können. Es kombiniert unter anderem:

- evolutionäre Suche,
- SNN-inspirierte Generatoren,
- OpenCL-beschleunigte Bewertungspfade,
- eine lokale WebGUI für geführte Experimente,
- Code-Export nach Python, C++ Header und OpenCL,
- Domain-Evaluationen,
- Novelty-/Nontriviality-Scores,
- Reports, Manifeste und Discovery-Archive.

### 2. `triadic_genesis_engine_project/` ist ein abgeleitetes Forschungsprodukt

Hier werden drei vorher erzeugte Artefakte nicht nur abgelegt, sondern in eine neue Architektur überführt. Aus drei getrennten algorithmischen Kernen wird ein System für:

- Stream-Verarbeitung,
- Zeitreihen-Forecast,
- Anomalie-/Regime-Erkennung,
- adaptive Vertrauensgewichtung,
- deterministische Analyseberichte,
- CLI- und WebGUI-Nutzung.

### 3. Der Erkenntniswert liegt in der Verbindung

Das Repository demonstriert damit einen vollständigen Forschungszyklus:

```text
Idee
  ↓
Guided Discovery
  ↓
Algorithmus-Kandidat
  ↓
Export + Test + Report
  ↓
Analyse der algorithmischen Rolle
  ↓
Fusion mehrerer Artefakte
  ↓
Neues eigenständiges Projekt
```

Das ist der eigentliche Kern dieses Repositories: Nicht nur Algorithmen generieren, sondern generierte Algorithmen als Substrate für neue Systeme nutzbar machen.

---

## Projekt 1: Algorithmic Genesis Enterprise

```text
algorithmic_genesis_enterprise/
```

### Zweck

**Algorithmic Genesis Enterprise** ist eine Forschungsplattform zur geführten und autonomen Entdeckung neuer mathematischer und informatischer Algorithmus-Kandidaten.

Das Projekt stellt keinen klassischen Solver und keine Standardbibliothek dar. Es ist ein experimentelles Discovery-System, das aus Bausteinen, Suchstrategien und Bewertungsfunktionen ausführbare Kandidaten erzeugt.

### Kernfähigkeiten

- Native C++-Core-Library,
- CMake-basierter Windows-/Visual-Studio-Build,
- optionale OpenCL-Beschleunigung,
- SNN-LIF/STDP-Komponenten,
- evolutionäre Ausdruckssynthese,
- GPU-/CPU-Bewertungspfade,
- Formula-/Strategy-Part Guided Discovery,
- lokale WebGUI Research Console,
- Run-Archive unter `webui_runs/`,
- Export von Kandidaten nach Python, C++ Header und OpenCL,
- automatische README-/Manifest-/Test-Erzeugung je Artefakt,
- Domain-Evaluation, beispielsweise für `chaotic_maps`,
- Sicherheitsmechanismen wie Gas-Limits, Sanitizing und numerische Guardrails.

### Wichtige Unterordner

```text
algorithmic_genesis_enterprise/
├── include/ag/              C++ Header des nativen Kerns
├── src/                     C++ Implementierung
├── kernels/                 OpenCL-Kernel
├── tests/                   Native Tests
├── tools/                   Python-Hilfswerkzeuge und Benchmark-Tools
├── webui/                   Lokale Research Console
├── experiments/             Beispielmanifeste für Guided Discovery
├── docs/                    Architektur-, Roadmap- und Evaluationsdokumentation
├── webui_runs/              Konkrete WebGUI-Läufe und exportierte Artefakte
├── README.md                Detail-README des Discovery-Projekts
└── WHITEPAPER.md            Whitepaper des Discovery-Projekts
```

### Was dort passiert

Der typische Ablauf in diesem Projekt sieht so aus:

```text
Experiment konfigurieren
  ↓
Suchraum und Domain wählen
  ↓
Generator starten
  ↓
Kandidaten bewerten
  ↓
Novelty, Fitness und Nontriviality erfassen
  ↓
Kandidaten akzeptieren oder verwerfen
  ↓
Artefakte exportieren
  ↓
Tests und Reports erzeugen
```

Die WebGUI-Läufe erzeugen strukturierte Run-Ordner, zum Beispiel:

```text
algorithmic_genesis_enterprise/webui_runs/run_20260516_171657_17436/
algorithmic_genesis_enterprise/webui_runs/run_20260516_173702_22107/
algorithmic_genesis_enterprise/webui_runs/run_20260516_182332_12601/
```

Jeder Run enthält typischerweise:

```text
experiment_manifest.json
result.json
report.md
run.log
status.json
exports/
```

Im Ordner `exports/` liegen die konkret erzeugten Algorithmus-Artefakte, beispielsweise Python-Code, C++ Header, OpenCL-Code, Manifest, Tests und artefaktspezifische README-Dateien.

---

## Projekt 2: Triadic Genesis Engine

```text
triadic_genesis_engine_project/
```

### Zweck

**Triadic Genesis Engine** ist ein neues Forschungsprojekt, das aus drei durch Algorithmic Genesis erzeugten Algorithmen hervorgegangen ist.

Es ist kein einzelner Forecasting-Algorithmus und kein klassisches Machine-Learning-Modell. Es ist ein adaptives Fusionssystem, in dem drei generierte algorithmische Kerne unterschiedliche Rollen übernehmen:

```text
chaotic exploration kernel
        +
contractive anchor kernel
        +
sequence delta-curvature kernel
        ↓
mycelial branch mixer
        ↓
forecast + anomaly regime + trust dynamics
```

### Kernfähigkeiten

- Python-Package mit sauberer `src/`-Struktur,
- CLI über `python -m triadic_genesis_engine.cli`,
- optional installierbarer Konsolenbefehl `triadic-genesis`,
- lokale WebGUI,
- synthetische Demo-Daten,
- Forecast auf CSV-/Textdaten,
- Benchmark gegen Baselines,
- Lineage-Ausgabe der drei Quellalgorithmen,
- Self-Test,
- JSON-Reports,
- Dokumentation und Whitepaper.

### Wichtige Unterordner

```text
triadic_genesis_engine_project/
├── src/triadic_genesis_engine/       Package-Code
│   ├── cli.py                        Kommandozeilenoberfläche
│   ├── core.py                       Engine, Kernel, Mixer, Reports
│   ├── webapp.py                     Lokale WebGUI
│   └── __init__.py                   Package-Initialisierung
├── source_algorithms/                Die drei übernommenen Algorithmic-Genesis-Artefakte
├── tests/                            Pytest-basierte Tests
├── examples/                         Beispieldaten
├── outputs/                          Beispieloutputs und Benchmarkdaten
├── docs/                             API, Architektur, Whitepaper
├── README.md                         Detail-README der Engine
├── WHITEPAPER.md                     Projektwhitepaper
├── VALIDATION.md                     Validierungsnotizen
└── pyproject.toml                    Python-Projektdefinition
```

### Was dort passiert

Die Triadic Genesis Engine nimmt eine numerische Sequenz entgegen und verarbeitet sie über drei Rollen:

| Rolle | Quellalgorithmus | Funktion in der Engine |
|---|---|---|
| Exploration | `ag_chaotic_map_explorer_0c55ac1bc71d` | erzeugt begrenzten nichtlinearen Explorationsdruck |
| Anchor | `ag_chaotic_map_explorer_dc0f1a3431df` | stabilisiert Drift und wirkt als Trust-Anker |
| Delta-Curvature | `ag_sequence_extrapolator_cf5cb9a3f619` | extrapoliert lokale Differenz- und Krümmungssignale |

Der `Mycelial Branch Mixer` verbindet diese Signale zu einem adaptiven Branch-System. Dabei entstehen Prognosen, interne Zustandswerte, Fehlerdynamik, Anomalieindikatoren und Vertrauensgewichtungen.

---

## Die Verbindung beider Projekte

Die Beziehung lässt sich als Herkunftskette lesen:

```text
algorithmic_genesis_enterprise
  └── webui_runs
      ├── run_20260516_171657_17436
      │   └── ag_chaotic_map_explorer_0c55ac1bc71d
      ├── run_20260516_173702_22107
      │   └── ag_chaotic_map_explorer_dc0f1a3431df
      └── run_20260516_182332_12601
          └── ag_sequence_extrapolator_cf5cb9a3f619
              ↓
triadic_genesis_engine_project
  └── source_algorithms
      ├── ag_chaotic_map_explorer_0c55ac1bc71d_algorithm.py
      ├── ag_chaotic_map_explorer_dc0f1a3431df_algorithm.py
      ├── ag_sequence_extrapolator_cf5cb9a3f619_algorithm.py
      └── LINEAGE.md
              ↓
triadic_genesis_engine_project/src/triadic_genesis_engine/core.py
  └── fusionierte Engine-Logik
```

Diese Verbindung ist wichtig: Die Triadic Genesis Engine ist nicht aus dem Nichts entstanden. Sie ist eine gezielte Weiterverarbeitung konkreter Algorithmic-Genesis-Exports.

---

## Die drei WebGUI-Runs als Ursprung der Triadic Genesis Engine

Die Triadic Genesis Engine basiert auf drei Runs aus `algorithmic_genesis_enterprise/webui_runs/`.

### 1. `run_20260516_171657_17436`

```text
Name:          ag_chaotic_map_explorer_0c55ac1bc71d
Domain:        chaotic_maps
Kind:          chaotic_map_explorer
Score:         0.949405
Nontriviality: 0.984122
Rolle:         bounded entropic oscillator / exploration phase
```

Kernel:

```text
tanh(tanh(sin((sin((x + x)) + sin((x + x))))))
```

Dieser Kernel erzeugt ein begrenztes, nichtlineares Oszillationssignal. In der Triadic Genesis Engine wird er nicht als endgültig bewiesene chaotische Abbildung behandelt, sondern als stabil begrenzter Explorationsdruck.

### 2. `run_20260516_173702_22107`

```text
Name:          ag_chaotic_map_explorer_dc0f1a3431df
Domain:        chaotic_maps
Kind:          chaotic_map_explorer
Score:         0.952024
Nontriviality: 0.984937
Rolle:         contractive anchor / trust stabilizer
```

Kernel:

```text
tanh(tanh(((x * cos(x)) / (x * cos(x)))))
```

Dieser Kernel wirkt in vielen nichtsingulären Bereichen kontraktiv beziehungsweise stabilisierend. In der Triadic Genesis Engine wird daraus ein Anker- und Vertrauenssignal.

### 3. `run_20260516_182332_12601`

```text
Name:          ag_sequence_extrapolator_cf5cb9a3f619
Domain:        sequence_generation
Kind:          sequence_extrapolator
Score:         0.965823
Nontriviality: 0.984883
Rolle:         delta-curvature predictor / temporal extrapolator
```

Kernel:

```text
((logabs(x) - logabs(tanh(x))) + tanh(tanh(x)))
```

Dieser Kernel verarbeitet Differenz- und Krümmungssignale. In der Triadic Genesis Engine bildet er den zeitlichen Extrapolationszweig.

---

## Repository-Struktur

Gesamtansicht:

```text
.
├── README.md
├── algorithmic_genesis_enterprise/
│   ├── CMakeLists.txt
│   ├── include/ag/
│   ├── src/
│   ├── kernels/
│   ├── tests/
│   ├── tools/
│   ├── webui/
│   ├── experiments/
│   ├── docs/
│   ├── webui_runs/
│   ├── README.md
│   └── WHITEPAPER.md
└── triadic_genesis_engine_project/
    ├── pyproject.toml
    ├── src/triadic_genesis_engine/
    ├── source_algorithms/
    ├── tests/
    ├── examples/
    ├── outputs/
    ├── docs/
    ├── README.md
    ├── WHITEPAPER.md
    └── VALIDATION.md
```

### Welche README sollte man lesen?

| Datei | Zweck |
|---|---|
| `README.md` | Diese Datei. Gesamtüberblick über beide Projekte und ihre Beziehung. |
| `algorithmic_genesis_enterprise/README.md` | Detailstart für Build, OpenCL, WebGUI und Discovery-Plattform. |
| `triadic_genesis_engine_project/README.md` | Detailstart für Python-Engine, CLI, Forecast, WebGUI und Tests. |
| `algorithmic_genesis_enterprise/WHITEPAPER.md` | Wissenschaftlich-technische Einordnung der Discovery-Plattform. |
| `triadic_genesis_engine_project/WHITEPAPER.md` | Entstehung und Architektur der Triadic Genesis Engine. |
| `triadic_genesis_engine_project/source_algorithms/LINEAGE.md` | Herkunftslinie der drei Quellalgorithmen. |

---

## Schnellstart unter Windows / PowerShell

Alle Befehle sind als PowerShell-Einzeiler formuliert.

### 1. Repository betreten

Passe den Pfad an deinen lokalen Speicherort an:

```powershell
Set-Location D:\algorithmic_generator
```

Struktur prüfen:

```powershell
Get-ChildItem
```

Erwartet werden mindestens:

```text
algorithmic_genesis_enterprise
triadic_genesis_engine_project
README.md
```

---

### 2. Triadic Genesis Engine direkt testen

Dies ist der schnellste Einstieg, weil das zweite Projekt rein Python-basiert ist.

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli self-test
```

Erwartete Ausgabe:

```text
ok
```

Lineage der drei Quellalgorithmen anzeigen:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli lineage
```

Demo ausführen:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli demo --samples 160 --horizon 16
```

Benchmark schreiben:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli benchmark --json outputs\benchmark_local.json
```

Forecast auf Beispieldaten schreiben:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli forecast --data examples\sample_series.csv --horizon 24 --json outputs\result_local.json
```

WebGUI starten:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli web --host 127.0.0.1 --port 8777
```

Dann im Browser öffnen:

```text
http://127.0.0.1:8777
```

---

### 3. Triadic Genesis Engine sauber installieren

Für dauerhafte Arbeit empfiehlt sich eine editable Installation:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; py -3 -m pip install -e .
```

Danach kann die CLI ohne `PYTHONPATH` ausgeführt werden:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; py -3 -m triadic_genesis_engine.cli self-test
```

Falls dein Python-Scripts-Verzeichnis im `PATH` liegt:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; triadic-genesis self-test
```

Tests ausführen:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; py -3 -m pip install pytest; py -3 -m pytest tests
```

---

### 4. Algorithmic Genesis Enterprise: Toolchain prüfen

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; where.exe cmake; where.exe cl; where.exe git; py -3 --version
```

Unter Windows sollte CMake idealerweise aus dem offiziellen Installationspfad kommen:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" --version
```

Falls MSYS-CMake zuerst gefunden wird, kann der Pfad für die aktuelle PowerShell-Sitzung korrigiert werden:

```powershell
$env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
```

---

### 5. Algorithmic Genesis Enterprise: CPU-Build ohne OpenCL

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; if (Test-Path build_cpu) { Remove-Item build_cpu -Recurse -Force }; & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build_cpu -G "Visual Studio 17 2022" -A x64 -DAG_ENABLE_OPENCL=OFF -DAG_ENABLE_SQLITE=OFF
```

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\cmake.exe" --build build_cpu --config Release --parallel
```

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\ctest.exe" --test-dir build_cpu -C Release --output-on-failure
```

---

### 6. Algorithmic Genesis Enterprise: OpenCL-Build

Voraussetzung: `vcpkg` und OpenCL sind eingerichtet.

```powershell
Set-Location C:\vcpkg; git pull; .\bootstrap-vcpkg.bat
```

```powershell
C:\vcpkg\vcpkg.exe install opencl:x64-windows
```

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; if (Test-Path build_opencl) { Remove-Item build_opencl -Recurse -Force }; & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build_opencl -G "Visual Studio 17 2022" -A x64 -DAG_ENABLE_OPENCL=ON -DAG_ENABLE_SQLITE=OFF -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\cmake.exe" --build build_opencl --config Release --parallel
```

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\ctest.exe" --test-dir build_opencl -C Release --output-on-failure
```

OpenCL prüfen:

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; .\build_opencl\Release\ag_cli.exe opencl-probe
```

---

### 7. Algorithmic Genesis Enterprise: WebGUI starten

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; py -3 webui\server.py
```

Dann im Browser öffnen:

```text
http://127.0.0.1:8765
```

Die WebGUI dient dazu, Discovery-Läufe zu konfigurieren, zu starten, ihre Artefakte einzusehen und Reports beziehungsweise Exporte zu betrachten.

---

## Empfohlener Arbeitsablauf

Für neue Leser und Entwickler ist diese Reihenfolge sinnvoll:

### Schritt 1: Gesamtzusammenhang lesen

```text
README.md
```

Diese Root-Datei erklärt, warum zwei Projekte existieren und wie sie zusammenhängen.

### Schritt 2: Triadic Genesis Engine starten

```text
triadic_genesis_engine_project/
```

Dieses Projekt lässt sich schnell testen und zeigt unmittelbar das Ergebnis der Fusion.

Empfohlen:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli self-test
```

Danach:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli lineage
```

So sieht man sofort, welche drei Algorithmen verbunden wurden.

### Schritt 3: Quellalgorithmen im Ursprung prüfen

```text
algorithmic_genesis_enterprise/webui_runs/
```

Dort liegen die Run-Ordner, aus denen die drei Artefakte stammen.

### Schritt 4: Algorithmic Genesis Enterprise bauen

```text
algorithmic_genesis_enterprise/
```

Hier wird die eigentliche Discovery-Plattform gebaut und getestet.

### Schritt 5: Neue Discovery-Läufe erzeugen

Über die WebGUI können neue Läufe gestartet werden. Diese können später wiederum als Ausgangspunkt für weitere abgeleitete Projekte dienen.

---

## Was passiert technisch?

### Algorithmic Genesis Enterprise

Technisch arbeitet die Plattform mit einem mehrstufigen Such- und Evaluationsmodell:

```text
Bausteinraum
  ↓
Guided Seed / Profile / Domain
  ↓
Generator
  ↓
Kandidat
  ↓
Fitness-Bewertung
  ↓
Novelty- und Nontriviality-Prüfung
  ↓
Export
  ↓
Validierung
```

Die Kandidaten sind mathematische beziehungsweise algorithmische Ausdrücke, die in lauffähige Artefakte überführt werden. Je nach Lauf entstehen unter anderem:

- Python-Implementierungen,
- C++ Header,
- OpenCL-Kernel,
- JSON-Manifeste,
- Tests,
- README-Dateien,
- Reports.

### Triadic Genesis Engine

Die Engine nimmt drei solcher Kandidaten und ordnet ihnen neue Rollen zu.

Vereinfacht:

```text
Input-Zeitreihe
  ↓
Normalisierung / Statistik / Fenster
  ↓
Exploration-Kernel
  ↓
Anchor-Kernel
  ↓
Delta-Curvature-Kernel
  ↓
Branch Mixer
  ↓
Forecast + Regime + Trust + Report
```

Die ursprünglichen Algorithmen werden damit nicht einfach nebeneinander ausgeführt. Sie werden semantisch umgedeutet und als funktionale Organe in einer neuen Architektur verwendet.

---

## Validierung, Tests und Sicherheit

Beide Projekte behandeln generierte Algorithmen als Forschungsartefakte, nicht als ungeprüfte Produktivlogik.

### Sicherheitsprinzipien

- numerische Sanitization,
- Begrenzung nichtendlicher Werte,
- Gas-Limits beziehungsweise Ausführungsbudgets,
- deterministische Tests,
- explizite Manifeste,
- reproduzierbare Run-Verzeichnisse,
- Benchmark gegen Baselines,
- Trennung zwischen Generierung und abgeleiteter Nutzung.

### Wichtige Testpfade

Algorithmic Genesis Enterprise:

```powershell
Set-Location D:\algorithmic_generator\algorithmic_genesis_enterprise; & "C:\Program Files\CMake\bin\ctest.exe" --test-dir build_cpu -C Release --output-on-failure
```

Triadic Genesis Engine:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; py -3 -m pytest tests
```

Interner Self-Test der Triadic Genesis Engine:

```powershell
Set-Location D:\algorithmic_generator\triadic_genesis_engine_project; $env:PYTHONPATH = ".\src"; py -3 -m triadic_genesis_engine.cli self-test
```

---

## Wissenschaftliche Einordnung

Dieses Repository ist ein Forschungs- und Entwicklungsartefakt.

Es behauptet nicht, dass jeder generierte Kandidat mathematisch optimal, bewiesen neu oder produktiv sicher ist. Der Anspruch ist ein anderer:

1. Kandidaten werden systematisch erzeugt.
2. Ihre Herkunft wird dokumentiert.
3. Ihre Struktur wird exportiert.
4. Ihre Eigenschaften werden getestet.
5. Ihre Rolle kann in einem neuen System interpretiert werden.
6. Aus mehreren Kandidaten kann ein neues Projekt entstehen.

Damit liegt der Forschungswert nicht nur im einzelnen Kernel, sondern in der reproduzierbaren Pipeline vom generativen Experiment zum ausführbaren Folgeprojekt.

Die Triadic Genesis Engine ist dafür ein konkretes Beispiel: Drei getrennte WebGUI-Artefakte wurden als Exploration, Stabilisierung und Extrapolation interpretiert und in eine neue adaptive Engine überführt.

---

## Für wen ist dieses Repository gedacht?

Dieses Repository richtet sich an:

- Entwickler, die generative Algorithmensynthese untersuchen wollen,
- numerische Algorithmiker,
- Performance Engineers,
- OpenCL-/C++-Entwickler,
- Python-Tooling-Entwickler,
- Forscher im Bereich adaptiver Systeme,
- Personen, die aus generierten Artefakten größere Systeme konstruieren möchten.

Nicht ideal ist das Repository für Leser, die ausschließlich eine fertige Standardbibliothek suchen. Der Fokus liegt auf Forschung, Experiment, Architektur und nachvollziehbarer Weiterentwicklung.

---

## Weiterführende Dokumentation

### Algorithmic Genesis Enterprise

```text
algorithmic_genesis_enterprise/README.md
algorithmic_genesis_enterprise/WHITEPAPER.md
algorithmic_genesis_enterprise/docs/ARCHITECTURE.md
algorithmic_genesis_enterprise/docs/ALGORITHM_SYNTHESIS.md
algorithmic_genesis_enterprise/docs/ALGORITHM_SYNTHESIS_V2.md
algorithmic_genesis_enterprise/docs/WEBGUI_GUIDED_DISCOVERY.md
algorithmic_genesis_enterprise/docs/WEBGUI_DOMAIN_EVALUATION.md
algorithmic_genesis_enterprise/docs/WEBGUI_MULTI_DOMAIN_EVALUATION.md
algorithmic_genesis_enterprise/docs/CS_CORE_DISCOVERY.md
algorithmic_genesis_enterprise/docs/ROADMAP.md
```

### Triadic Genesis Engine

```text
triadic_genesis_engine_project/README.md
triadic_genesis_engine_project/WHITEPAPER.md
triadic_genesis_engine_project/VALIDATION.md
triadic_genesis_engine_project/docs/ARCHITECTURE.md
triadic_genesis_engine_project/docs/API.md
triadic_genesis_engine_project/docs/WHITEPAPER.md
triadic_genesis_engine_project/source_algorithms/LINEAGE.md
```

---

## Mentales Modell für GitHub-Leser

Wer dieses Repository öffnet, sollte es so lesen:

```text
1. Die Root-README erklärt das Gesamtsystem.
2. Algorithmic Genesis Enterprise ist der Generator und Experimentkern.
3. Die WebGUI-Runs enthalten konkrete erzeugte Artefakte.
4. Triadic Genesis Engine ist ein aus drei Artefakten gebautes neues Projekt.
5. Die Unter-READMEs erklären die praktische Bedienung der jeweiligen Ebene.
```

Oder noch kürzer:

```text
Algorithmic Genesis Enterprise erzeugt algorithmische Keime.
Triadic Genesis Engine zeigt, wie aus drei Keimen ein neues System wächst.
```

---

## Status

Dieses Repository ist ein professionell strukturierter Forschungsstand. Es ist ausführbar, testbar und dokumentiert, aber bewusst als R&D-System zu verstehen.

Die erzeugten Algorithmen und die daraus gebaute Triadic Genesis Engine sollten weiter validiert, erweitert und gegen zusätzliche Baselines getestet werden, bevor produktive oder sicherheitskritische Einsätze erwogen werden.
