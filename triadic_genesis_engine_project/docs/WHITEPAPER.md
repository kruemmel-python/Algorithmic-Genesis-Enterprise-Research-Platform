# Whitepaper: Triadic Genesis Engine

**Untertitel:** Von drei automatisch generierten Algorithmus-Artefakten zu einem neuen adaptiven Ausführungsmodell für Stream-Prognose, Regime-Erkennung und Trust-Dynamik.

**Projektname:** Triadic Genesis Engine  
**Version:** 1.0  
**Status:** Forschungsartefakt, nicht produktiv freigegebener mathematischer Beweis

---

## Executive Summary

Die **Triadic Genesis Engine** entstand aus drei neuartigen, durch Algorithmic Genesis erzeugten Originalalgorithmen. Diese Algorithmen waren zunächst separate Artefakte aus unterschiedlichen WebGUI-Discovery-Läufen. Zwei davon stammen aus der Domäne `chaotic_maps`, einer aus der Domäne `sequence_generation`.

Statt die Algorithmen isoliert zu bewerten, verfolgt dieses Projekt eine neue Forschungsrichtung:

> Generierte Algorithmen werden als algorithmische Organe betrachtet, die zu einem neuen adaptiven System verbunden werden können.

Aus dieser Idee entstand ein neues Projekt mit eigenem Namen, eigener Architektur, CLI, WebGUI, Tests, Dokumentation und Benchmarking:

```text
Triadic Genesis Engine
```

Der Name beschreibt die drei Quellorgane:

1. Chaotic exploration kernel
2. Contractive anchor kernel
3. Sequence delta-curvature kernel

Diese drei Komponenten werden durch einen **Mycelial Branch Mixer** verbunden. Das System erzeugt daraus Vorhersagen, Anomalie- und Regime-Signale sowie eine adaptive Trust-Dynamik.

---

## 1. Ausgangspunkt: Algorithmic Genesis und webui_runs.zip

Die drei Ausgangsalgorithmen wurden nicht manuell geschrieben. Sie wurden zuvor durch die Algorithmic-Genesis-Plattform erzeugt, exportiert und als Artefakte in `webui_runs.zip` bereitgestellt.

Jedes Artefakt enthält:

```text
Algorithmusname
Domain
Kernel-Ausdruck
Algorithm score
Nontriviality
Manifest
exportierten Python-Code
Validierungsmetadaten
```

Die drei Algorithmen wurden anschließend analysiert und nicht unverändert als getrennte Programme belassen. Stattdessen wurde geprüft:

```text
Welche Dynamik erzeugt der Kernel?
Welche Rolle könnte er in einem größeren System spielen?
Ist er explorativ, stabilisierend oder vorhersagend?
Wie lässt er sich sicher und deterministisch kombinieren?
```

Das Ergebnis dieser Analyse war die Rollenverteilung, auf der die Triadic Genesis Engine basiert.

---

## 2. Der gewählte Projektname

Der Projektname lautet:

```text
Triadic Genesis Engine
```

### 2.1 Bedeutung von „Triadic“

„Triadic“ verweist auf die drei Ursprungskerne:

```text
1. Exploration
2. Anchoring
3. Extrapolation
```

Diese drei Funktionen entsprechen drei fundamentalen Kräften in adaptiven Systemen:

| Kraft | Bedeutung |
|---|---|
| Exploration | Neue Zustandsräume untersuchen |
| Stabilisierung | Drift und numerische Eskalation begrenzen |
| Extrapolation | Aus lokaler Historie Zukunft ableiten |

### 2.2 Bedeutung von „Genesis“

„Genesis“ verweist auf die Entstehung aus der Algorithmic-Genesis-Plattform. Die drei Algorithmen wurden durch vorherige Discovery-Läufe generiert und anschließend zu einem neuen System transformiert.

### 2.3 Bedeutung von „Engine“

„Engine“ bedeutet, dass das Projekt kein statischer Bericht ist. Es ist ein ausführbares System mit:

```text
Python-Package
CLI
WebGUI
Tests
Benchmark
JSON-Reports
Dokumentation
```

---

## 3. Die drei Originalalgorithmen

## 3.1 Originalalgorithmus 1: ag_chaotic_map_explorer_0c55ac1bc71d

### Metadaten

```text
Name:              ag_chaotic_map_explorer_0c55ac1bc71d
Domain:            chaotic_maps
Kind:              chaotic_map_explorer
Algorithm score:   0.949405
Nontriviality:     0.984122
Rolle im Projekt:  bounded entropic oscillator / exploration phase
```

### Kernel

```text
K₁(x) = tanh(tanh(sin((sin((x + x)) + sin((x + x))))))
```

Vereinfacht gelesen:

```text
K₁(x) ≈ tanh(tanh(sin(2 · sin(2x))))
```

### Technische Bedeutung

Dieser Kernel ist stark begrenzt, aber nichtlinear. Durch `sin`, verschachtelte `sin`-Terme und doppelte `tanh`-Sättigung entsteht ein Signal, das kontrolliert oszillatorisch ist.

Er ist nicht geeignet, allein ein vollständiges Prognosesystem zu bilden. Seine Stärke liegt in der Erzeugung eines **bounded exploration pressure**:

```text
nichtlinear
endlich
saturiert
wiederholbar
numerisch stabil
```

### Rolle in der Triadic Genesis Engine

Im neuen Projekt wird dieser Algorithmus als **chaotic exploration kernel** genutzt.

Er erzeugt eine interne Phase:

```text
chaos_phase
```

Diese Phase moduliert Vorhersagezweige und hilft dem System, nicht vollständig in lineare oder naive Extrapolation zu kollabieren.

### Warum nicht direkt als chaotische Weltneuheit?

Der Kernel ist ein automatisch synthetisierter mathematischer Ausdruck. Er ist nicht als bewiesene neue chaotische Abbildung zu verstehen. Seine Nützlichkeit im neuen Projekt liegt nicht in einem theoretisch bewiesenen Chaosgrad, sondern in seiner Funktion als stabil begrenzter nichtlinearer Oszillator.

---

## 3.2 Originalalgorithmus 2: ag_chaotic_map_explorer_dc0f1a3431df

### Metadaten

```text
Name:              ag_chaotic_map_explorer_dc0f1a3431df
Domain:            chaotic_maps
Kind:              chaotic_map_explorer
Algorithm score:   0.952024
Nontriviality:     0.984937
Rolle im Projekt:  contractive anchor / trust stabilizer
```

### Kernel

```text
K₂(x) = tanh(tanh(((x · cos(x)) / (x · cos(x)))))
```

Für die meisten nichtsingulären Werte ist der Quotient näherungsweise:

```text
(x · cos(x)) / (x · cos(x)) ≈ 1
```

Damit nähert sich der Kernel:

```text
K₂(x) ≈ tanh(tanh(1))
```

### Technische Bedeutung

Für sich allein betrachtet wirkt dieser Kernel weniger explorativ. Er tendiert zu einem stabilen, kontraktiven Wert. Genau daraus entsteht seine Rolle im neuen System:

```text
Stabilisierung
Driftbegrenzung
Trust-Anker
Kontraktionssignal
```

Wo der erste Kernel explorative Nichtlinearität einbringt, liefert der zweite Kernel eine Art stabilen Gegenpol.

### Rolle in der Triadic Genesis Engine

Im neuen Projekt wird dieser Algorithmus als **contractive anchor kernel** verwendet.

Er beeinflusst:

```text
anchor_smoothed branch
trust dynamics
bounded correction
drift damping
```

### Warum ist dieser Algorithmus trotz scheinbarer Trivialität nützlich?

Wenn man ihn allein bewertet, könnte man sagen: Der Kernel kollabiert oft auf einen nahezu konstanten Wert. In einer Fusion ist das jedoch kein Fehler, sondern eine nutzbare Eigenschaft.

In komplexen Systemen braucht man nicht nur Neuheit und Exploration, sondern auch:

```text
Anker
Dämpfung
Stabilitätsreferenzen
Fallback-Signale
```

Dieser Algorithmus übernimmt genau diese Rolle.

---

## 3.3 Originalalgorithmus 3: ag_sequence_extrapolator_cf5cb9a3f619

### Metadaten

```text
Name:              ag_sequence_extrapolator_cf5cb9a3f619
Domain:            sequence_generation
Kind:              sequence_extrapolator
Algorithm score:   0.965823
Nontriviality:     0.984883
Rolle im Projekt:  delta-curvature predictor / temporal extrapolator
```

### Kernel

```text
K₃(x) = (logabs(x) - logabs(tanh(x))) + tanh(tanh(x))
```

### Technische Bedeutung

Dieser Kernel verarbeitet lokale Differenzen und kann als nichtlineare Transformation von Delta- oder Krümmungsinformationen genutzt werden.

Der Ausdruck enthält:

```text
logabs(x)
logabs(tanh(x))
tanh(tanh(x))
```

Das erzeugt eine Mischung aus:

```text
logarithmischer Kompression
Sättigung
lokaler Nichtlinearität
Delta-Sensitivität
```

### Rolle in der Triadic Genesis Engine

Dieser Algorithmus ist der zentrale **temporal extrapolator**.

Er wird genutzt, um aus:

```text
delta = x_t - x_{t-1}
curvature = x_t - 2x_{t-1} + x_{t-2}
```

eine Vorhersagekomponente zu erzeugen.

### Warum ist er der stärkste Prognosebaustein?

Im Gegensatz zu den beiden chaotic-map-Algorithmen ist dieser Kernel direkt auf Sequenzfortschreibung ausgerichtet. Darum bildet er den Kern des Zweigs:

```text
delta_curvature
```

Dieser Zweig ist für lokale Fortschreibung und Krümmungsinterpretation zuständig.

---

## 4. Warum diese drei verbunden wurden

Die drei Algorithmen decken komplementäre Rollen ab:

| Algorithmus | Primäre Kraft | Risiko allein | Nutzen in Fusion |
|---|---|---|---|
| `0c55...` | Exploration | kann mild oszillieren oder degenerieren | erzeugt nichtlinearen Suchdruck |
| `dc0...` | Stabilisierung | kann zu konstant wirken | verhindert unkontrollierten Drift |
| `cf5...` | Extrapolation | kein vollständiges Stream-System | liefert zeitliche Prognosekraft |

Die Fusion folgt dem Prinzip:

```text
Exploration ohne Stabilität driftet.
Stabilität ohne Exploration kollabiert.
Extrapolation ohne Regime-Signal ist blind.
```

Erst die Verbindung erzeugt ein vollständigeres System.

---

## 5. Neue Architektur: Mycelial Branch Mixer

Die Triadic Genesis Engine führt einen neuen Verbindungspunkt ein:

```text
Mycelial Branch Mixer
```

Der Begriff „mycelial“ ist technisch gemeint. Er steht nicht für Dekoration, sondern für eine konkrete Struktur:

```text
mehrere Vorhersagezweige
gewichtete Verbindungen
online aktualisierte Branch-Gewichte
Fehler-Rückkopplung
lokale Zustandsanpassung
```

### 5.1 Verarbeitungsfluss

```text
observation stream
  ↓
online statistics
  ↓
attractor memory
  ↓
three generated kernels
  ↓
branch predictors
  ↓
error-adaptive weights
  ↓
consensus forecast
  ↓
regime + anomaly + trust
```

### 5.2 Branches

Die Engine erzeugt mehrere Vorhersagezweige:

```text
delta_curvature
chaos_modulated
anchor_smoothed
mycelial_consensus
```

### 5.3 Branch-Gewichte

Jeder Branch besitzt eine Fehlerhistorie. Aus der Fehlerhistorie werden Soft-Weights berechnet.

Ein Branch, der zuletzt schlecht war, wird gedämpft. Ein Branch, der zuletzt gut war, wird verstärkt.

Dadurch entsteht ein adaptives Ensemble, aber nicht im klassischen Sinne: Die Ensemble-Mitglieder sind nicht normale Modelle, sondern aus generierten mathematischen Kernen abgeleitete algorithmische Organe.

---

## 6. Ausführungsmodell

### 6.1 Eingang

Das System verarbeitet eine numerische Sequenz:

```text
x₀, x₁, x₂, ...
```

Diese kann aus CSV-, Leerzeichen- oder Zeilen-getrennten Werten stammen.

### 6.2 Online-Zustand

Während des Laufs hält das System:

```text
Online mean
Online variance
Attractor memory
Return-map diversity
Anomaly score
Trust value
Branch error estimates
Chaos phase
```

### 6.3 Forecast

Aus dem aktuellen Zustand berechnet die Engine:

```text
delta_curvature forecast
chaos_modulated forecast
anchor_smoothed forecast
weighted consensus forecast
```

### 6.4 Regime-Erkennung

Die Engine klassifiziert grob:

```text
contractive
drifting
exploratory
shock
```

Diese Regime sind keine wissenschaftlich endgültigen Klassen. Sie dienen als Diagnose, wie sich der Stream im aktuellen Fenster verhält.

---

## 7. Was am neuen Projekt neu ist

Die einzelnen Quellalgorithmen sind automatisch erzeugte Forschungskandidaten. Das neue Projekt besteht aber nicht nur darin, sie zu sammeln.

Neu ist die Transformation:

```text
drei getrennte generierte Algorithmen
→ funktionale Rollenbestimmung
→ algorithmische Organ-Zuweisung
→ gemeinsames Ausführungsmodell
→ adaptive Branch-Gewichtung
→ Stream-Prognose + Regime + Trust
```

Das ist der Kernbeitrag der Triadic Genesis Engine.

---

## 8. Abgrenzung von bekannten Ansätzen

### Kein klassisches Ensemble

Klassische Ensembles kombinieren meist trainierte Modelle. Hier werden nicht trainierte ML-Modelle kombiniert, sondern generierte algorithmische Kernel mit fest definierten mathematischen Ausdrücken.

### Kein neuronales Netzwerk

Es gibt keine Backpropagation, keine Gewichts-Matrix und kein Training im üblichen Sinne. Die Anpassung erfolgt online über Branch-Fehler.

### Kein reines Forecasting-Modell

Die Engine gibt nicht nur Prognosen aus, sondern zusätzlich:

```text
anomaly score
regime
trust
entropy
return-map diversity
lineage
branch predictions
```

### Kein Beweis einer mathematischen Weltneuheit

Die Engine ist ein neues Projekt-Artefakt und ein neues Kombinationsmodell. Sie beweist nicht, dass die Originalkernel mathematisch noch nie zuvor existiert haben.

---

## 9. Validierung

Das Projekt enthält:

```text
tests/test_engine.py
VALIDATION.md
benchmark CLI
self-test CLI
synthetic sample series
JSON export
```

Im ursprünglichen Validierungslauf zeigte die Fusion auf einer synthetischen Beispielserie:

```text
naive_mae:          0.24698
delta_mae:          0.30121
triadic_mae:        0.13171
relative_to_naive:  0.53327
relative_to_delta:  0.43727
```

Das bedeutet: Auf dieser Beispielserie war die Fusion deutlich besser als naive Last-Value- und einfache Delta-Fortschreibung.

Wichtig: Das ist kein universeller Beweis. Es ist ein Hinweis, dass die Fusion als Forschungsrichtung brauchbar ist.

---

## 10. Risiken und Scheitermodi

### 10.1 Schlechter als Baselines

Auf glatten linearen Signalen kann ein simpler linearer Forecast besser sein.

### 10.2 Überreaktion auf Schocks

Die explorative Komponente kann frühe Ausreißer überbewerten.

### 10.3 Zu starke Kontraktion

Der Anchor kann in manchen Regimen zu konservativ wirken.

### 10.4 Scheingenauigkeit

Wenn nur synthetische Daten genutzt werden, kann das Ergebnis zu optimistisch sein.

### 10.5 Keine formale Optimalität

Die Engine garantiert keine optimale Vorhersage, keine minimale MAE und keine beweisbare Dominanz gegenüber klassischen Verfahren.

---

## 11. Sicherheits- und Enterprise-Grenzen

Das Projekt ist bewusst lokal und kontrolliert:

```text
keine externen Netzwerkaufrufe
lokale WebGUI auf 127.0.0.1
finite-sanitize für Eingaben
bounded predictions
JSON-Reports
eingebettete Lineage
keine dynamische Internetabhängigkeit
```

Damit ist das Projekt für lokale Forschungs- und Audit-Workflows geeignet.

---

## 12. Bedienung im Forschungsbetrieb

### Self-Test

```powershell
py -3 -m triadic_genesis_engine.cli self-test
```

### Lineage

```powershell
py -3 -m triadic_genesis_engine.cli lineage
```

### Benchmark

```powershell
py -3 -m triadic_genesis_engine.cli benchmark --json benchmark.json
```

### Eigene Daten

```powershell
py -3 -m triadic_genesis_engine.cli forecast `
  --data examples\sample_series.csv `
  --horizon 24 `
  --json result.json
```

### WebGUI

```powershell
py -3 -m triadic_genesis_engine.cli web --host 127.0.0.1 --port 8777
```

---

## 13. Wissenschaftliche Bedeutung

Die wichtigste Erkenntnis ist nicht, dass einer der drei Ursprungskerne allein eine endgültige neue mathematische Theorie darstellt.

Die wichtigste Erkenntnis ist:

> Generierte Algorithmen können als wiederverwendbare algorithmische Organe betrachtet und zu neuen Systemarchitekturen verbunden werden.

Damit verschiebt sich Algorithmic Genesis von:

```text
ein Algorithmus wird erzeugt
```

zu:

```text
Algorithmus-Artefakte werden zu neuen algorithmischen Ökosystemen komponiert
```

Die Triadic Genesis Engine ist das erste konkrete Projekt in dieser Richtung.

---

## 14. Nächste Forschungsschritte

1. Reale Zeitreihen testen
2. Mehr Quellalgorithmen als Organe einbinden
3. Branch-Mixer selbst evolutionär erzeugen
4. Domain-spezifische Metriken ergänzen
5. Automatische Ablation durchführen
6. Drift- und Schock-Benchmarks erweitern
7. Export als Microservice vorbereiten
8. Vergleich gegen ARIMA, exponential smoothing und einfache ML-Baselines
9. Mehrere Triadic Engines als Population evolvieren
10. Whitebox-Erklärbarkeit der Branch-Gewichte verbessern

---

## 15. Schlussfolgerung

Die **Triadic Genesis Engine** ist aus drei neuartigen generierten Originalalgorithmen entstanden:

```text
ag_chaotic_map_explorer_0c55ac1bc71d
ag_chaotic_map_explorer_dc0f1a3431df
ag_sequence_extrapolator_cf5cb9a3f619
```

Jeder dieser Algorithmen brachte eine andere Kraft mit:

```text
Exploration
Stabilisierung
Extrapolation
```

Das neue Projekt verbindet diese Kräfte zu einem ausführbaren System:

```text
Triadic Genesis Engine
```

Es erzeugt Forecasts, Anomalie-Signale, Regime-Klassifikation, Trust-Dynamik und dokumentierte Lineage. Es ist kein Beweis endgültiger mathematischer Neuheit, aber ein professionelles Forschungsartefakt für eine neue Idee:

> automatisch generierte Algorithmen als kombinierbare Organe einer neuen algorithmischen Architektur.
