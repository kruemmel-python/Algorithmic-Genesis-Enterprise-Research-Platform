# Triadic Genesis Engine

**Triadic Genesis Engine** ist ein neues Forschungsprojekt, das drei unabhängig generierte Algorithmic-Genesis-Artefakte zu einem adaptiven Stream- und Zeitreihen-System verbindet.

Es ist kein einzelner Forecasting-Algorithmus und kein klassisches Machine-Learning-Modell. Es ist ein experimentelles Ausführungsmodell, in dem drei generierte Algorithmen als algorithmische Organe zusammenarbeiten:

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

## 1. Aktueller PowerShell-Start

### 1.1 In den Projektordner wechseln

```powershell
cd D:\triadic_genesis_engine_project
```

Prüfe, ob die Struktur stimmt:

```powershell
dir
dir .\src
dir .\src\triadic_genesis_engine
```

Erwartet:

```text
D:\triadic_genesis_engine_project\
  src\
    triadic_genesis_engine\
      __init__.py
      cli.py
      core.py
      webapp.py
  tests\
  docs\
  examples\
  source_algorithms\
  pyproject.toml
```

Wenn du nach dem Entpacken einen inneren Ordner hast, zuerst hineinwechseln:

```powershell
cd .\triadic_genesis_engine_project
```

## 2. Empfohlener Python-Aufruf unter Windows

Verwende möglichst Windows-Python über `py -3`, nicht MSYS-Python.

Prüfen:

```powershell
where python
where py
py -3 --version
```

Wenn `python` auf diesen Pfad zeigt, ist das MSYS-Python:

```text
C:\msys64\mingw64\bin\python.exe
```

Das funktioniert manchmal, ist aber für dieses Projekt nicht empfohlen. Nutze dann:

```powershell
py -3 -m ...
```

## 3. Variante A: Ohne Installation mit PYTHONPATH

In PowerShell muss `PYTHONPATH` so gesetzt werden:

```powershell
$env:PYTHONPATH = ".\src"
```

Danach Self-Test:

```powershell
py -3 -m triadic_genesis_engine.cli self-test
```

Erwartete Ausgabe:

```text
ok
```

Lineage anzeigen:

```powershell
py -3 -m triadic_genesis_engine.cli lineage
```

Demo starten:

```powershell
py -3 -m triadic_genesis_engine.cli demo --samples 160 --horizon 16
```

Benchmark ausführen:

```powershell
py -3 -m triadic_genesis_engine.cli benchmark --json benchmark.json
```

Forecast auf Beispieldaten:

```powershell
py -3 -m triadic_genesis_engine.cli forecast `
  --data examples\sample_series.csv `
  --horizon 24 `
  --json result.json
```

Lokale WebGUI starten:

```powershell
py -3 -m triadic_genesis_engine.cli web --host 127.0.0.1 --port 8777
```

Dann öffnen:

```text
http://127.0.0.1:8777
```

## 4. Variante B: Saubere Enterprise-Installation

Empfohlen für längere Arbeit:

```powershell
cd D:\triadic_genesis_engine_project
py -3 -m pip install -e .
```

Danach brauchst du `PYTHONPATH` nicht mehr.

Self-Test:

```powershell
py -3 -m triadic_genesis_engine.cli self-test
```

CLI über Modul:

```powershell
py -3 -m triadic_genesis_engine.cli lineage
py -3 -m triadic_genesis_engine.cli demo --samples 160 --horizon 16
py -3 -m triadic_genesis_engine.cli benchmark --json benchmark.json
```

Falls dein Python-Scripts-Verzeichnis im PATH ist, geht auch der Konsolenbefehl:

```powershell
triadic-genesis self-test
triadic-genesis lineage
triadic-genesis demo --samples 160 --horizon 16
triadic-genesis benchmark --json benchmark.json
triadic-genesis forecast --data examples\sample_series.csv --horizon 24 --json result.json
triadic-genesis web --host 127.0.0.1 --port 8777
```

## 5. Tests ausführen

Falls `pytest` noch nicht installiert ist:

```powershell
py -3 -m pip install pytest
```

Tests:

```powershell
py -3 -m pytest tests
```

Oder ohne pytest nur der interne Test:

```powershell
py -3 -m triadic_genesis_engine.cli self-test
```

## 6. Wichtige CLI-Befehle

| Zweck | PowerShell-Befehl |
|---|---|
| Self-Test | `py -3 -m triadic_genesis_engine.cli self-test` |
| Lineage anzeigen | `py -3 -m triadic_genesis_engine.cli lineage` |
| Demo | `py -3 -m triadic_genesis_engine.cli demo --samples 160 --horizon 16` |
| Benchmark | `py -3 -m triadic_genesis_engine.cli benchmark --json benchmark.json` |
| Forecast | `py -3 -m triadic_genesis_engine.cli forecast --data examples\sample_series.csv --horizon 24 --json result.json` |
| WebGUI | `py -3 -m triadic_genesis_engine.cli web --host 127.0.0.1 --port 8777` |

## 7. Eigene Daten verwenden

Die Datei kann CSV-, Leerzeichen- oder Zeilen-getrennte Zahlen enthalten.

Beispiel `my_series.csv`:

```text
0.0, 0.1, 0.15, 0.3, 0.28, 0.5, 0.9, 0.7
```

Forecast:

```powershell
py -3 -m triadic_genesis_engine.cli forecast `
  --data .\my_series.csv `
  --horizon 32 `
  --json .\my_result.json
```

Benchmark:

```powershell
py -3 -m triadic_genesis_engine.cli benchmark `
  --data .\my_series.csv `
  --horizon 32 `
  --json .\my_benchmark.json
```

## 8. Ausgabe verstehen

Ein Forecast-Report enthält:

```text
config
lineage
steps
forecasts
metrics
summary
```

Wichtige Felder:

| Feld | Bedeutung |
|---|---|
| `forecasts` | erzeugte Zukunftswerte |
| `metrics.mae` | mittlerer absoluter Fehler im internen Lauf |
| `metrics.rmse` | Root Mean Squared Error |
| `metrics.mean_anomaly_score` | mittlerer Anomaliedruck |
| `summary.final_entropy` | Attraktor-/Orbit-Entropie am Ende |
| `summary.final_return_diversity` | Return-Map-Diversität |
| `summary.mean_trust` | mittleres Vertrauen der Fusion |
| `lineage` | Herkunft der drei Originalalgorithmen |

## 9. Was das Projekt macht

Der Motor erzeugt mehrere Vorhersagezweige:

```text
delta_curvature
chaos_modulated
anchor_smoothed
mycelial_consensus
```

Diese Zweige werden online gewichtet. Ein Zweig, der zuletzt schlechter lag, verliert Gewicht; ein Zweig, der besser lag, gewinnt Gewicht. Dadurch entsteht eine adaptive Fusion.

## 10. Ursprung der drei verbundenen Algorithmen

Die drei Originalartefakte liegen in:

```text
source_algorithms\
```

Sie wurden aus `webui_runs.zip` übernommen und in `core.py` als exakte Kernel-Ausdrücke fest eingebettet.

| Algorithmus | Domain | Rolle im neuen System |
|---|---|---|
| `ag_chaotic_map_explorer_0c55ac1bc71d` | `chaotic_maps` | bounded entropic oscillator / exploration phase |
| `ag_chaotic_map_explorer_dc0f1a3431df` | `chaotic_maps` | contractive anchor / trust stabilizer |
| `ag_sequence_extrapolator_cf5cb9a3f619` | `sequence_generation` | delta-curvature predictor / temporal extrapolator |

Details stehen im Whitepaper:

```text
docs\WHITEPAPER.md
```

## 11. Häufige Fehler

### ModuleNotFoundError: No module named 'triadic_genesis_engine'

In PowerShell wurde vermutlich falsch gesetzt:

```powershell
set PYTHONPATH=src
```

Das ist CMD-Syntax, nicht PowerShell. Korrekt:

```powershell
$env:PYTHONPATH = ".\src"
py -3 -m triadic_genesis_engine.cli self-test
```

Oder Installation verwenden:

```powershell
py -3 -m pip install -e .
py -3 -m triadic_genesis_engine.cli self-test
```

### Falscher Python wird verwendet

Wenn das erscheint:

```text
C:\msys64\mingw64\bin\python.exe
```

nutze:

```powershell
py -3 -m triadic_genesis_engine.cli self-test
```

### WebGUI startet, aber Browser zeigt nichts

Prüfe, ob Port 8777 frei ist:

```powershell
py -3 -m triadic_genesis_engine.cli web --host 127.0.0.1 --port 8778
```

Dann öffnen:

```text
http://127.0.0.1:8778
```

## 12. Enterprise-Sicherheitsgrenzen

Dieses Projekt ist bewusst lokal und kontrolliert gehalten:

```text
keine externen Netzwerkaufrufe
lokale WebGUI auf 127.0.0.1
finite-sanitize für numerische Eingaben
bounded predictions
JSON-Ausgabe für Auditierbarkeit
Lineage der Originalalgorithmen eingebettet
keine dynamische Ausführung fremder Web-Inhalte
```

## 13. Wissenschaftliche Einordnung

Die Triadic Genesis Engine ist ein Forschungsartefakt. Sie beweist nicht, dass die drei Originalalgorithmen mathematische Weltneuheiten sind. Ihr Beitrag liegt darin, generierte Algorithmen nicht einzeln zu betrachten, sondern sie als wiederverwendbare algorithmische Organe zu einem neuen System zu verbinden.

Siehe:

```text
docs\WHITEPAPER.md
docs\ARCHITECTURE.md
docs\API.md
VALIDATION.md
```
