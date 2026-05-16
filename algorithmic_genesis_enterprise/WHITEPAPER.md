# Whitepaper: Algorithmic Genesis Enterprise Research Platform

**Untertitel:** Guided Discovery, Multi-Core Informatik-Algorithmus-Synthese und ausführbare Forschungsartefakte  
**Zielgruppe:** Entwickler, Forscher, Numeriker, Compiler-/Runtime-Engineers, Performance Engineers, technische Entscheider  
**Version:** Multi-Domain Discovery + Guided WebGUI + Domain Evaluation  
**Status:** Ausführbares Forschungs-Substrat, keine Produktionsgarantie ohne weitere Validierung

---

## Executive Summary

Algorithmic Genesis ist eine Enterprise-fähige Forschungsplattform zur automatisierten und geführten Entwicklung neuer mathematischer und informatischer Algorithmen. Das System nimmt menschliche Forschungsintuition in Form auswählbarer Formel-, Strategie- und Sicherheitsbausteine auf, übersetzt diese Auswahl in einen Suchdruck und erzeugt daraus ausführbare Algorithmus-Kandidaten mit Code, Manifest, Report, Tests und Domain-Evaluation.

Im Unterschied zu einem klassischen Optimierungsframework sucht Algorithmic Genesis nicht nur Parameter für bekannte Algorithmen. Es synthetisiert algorithmische Strukturen: numerische Update-Regeln, Root-Finding-Policies, Chaos-Maps, Signaltransformationen und Informatik-Kernalgorithmen wie Sortierung, Graphsuche, Scheduling, Parsing, Constraint Solving, Streaming, Kompression, Cache-Eviction, Load-Balancing und Anomaly Detection.

Das Projekt ist als lokales Forschungs-Substrat aufgebaut. Es kombiniert C++17, OpenCL, Python-WebGUI, JSONL-Archive, SNN-inspirierte Generatoren, GPU-residente Evaluationspfade, sichere Codeexporte und domänenspezifische Evaluatoren. Die WebGUI dient als Research Console: Nutzer wählen Kernbereich, Forschungsprofil und mindestens drei Formel-/Strategie-Teile. Das System erzeugt daraus ein Experiment-Manifest, startet die Discovery, exportiert Code und führt Validierungen aus.

---

## 1. Zweck des Projektes

Der Zweck von Algorithmic Genesis ist die Entwicklung einer Plattform, die neue algorithmische Kandidaten erzeugen kann, ohne dass ein Entwickler jede Regel manuell entwirft.

Die zentrale Frage lautet:

> Kann ein System aus menschlich gewählten mathematischen und informatischen Bausteinen neue ausführbare Algorithmusvarianten synthetisieren, bewerten, archivieren und als Code materialisieren?

Algorithmic Genesis beantwortet diese Frage mit einem integrierten Forschungsworkflow:

```text
Formel-/Strategie-Teile
→ Guided Search Bias
→ Evolutionäre/SNN-inspirierte Erzeugung
→ Fitness- und Novelty-Bewertung
→ Algorithmus-Synthese
→ Python/C++/OpenCL-Export
→ Tests
→ Domain-Evaluation
→ Discovery Archive
```

Das Ergebnis ist kein abstrakter Textvorschlag, sondern ein lauffähiger Algorithmus-Kandidat.

---

## 2. Was das Projekt nicht ist

Algorithmic Genesis ist nicht:

- ein fertiger Ersatz für etablierte numerische Libraries
- ein mathematischer Beweisgenerator
- ein garantiert optimaler Algorithmusfinder
- eine Blackbox, deren Ergebnisse ungeprüft produktiv eingesetzt werden sollten
- ein reines GPU-Benchmark-Projekt

Jeder erzeugte Algorithmus ist ein **Forschungsartefakt**. Er kann interessant, neuartig oder nützlich sein, muss aber gegen Baselines getestet werden.

---

## 3. Was das Projekt ist

Algorithmic Genesis ist:

- eine Algorithmus-Discovery-Plattform
- ein natives C++/OpenCL/Python-Forschungs-Substrat
- eine WebGUI-gesteuerte Experimentierumgebung
- ein Codegenerator für mathematische und informatische Algorithmus-Kandidaten
- ein Evaluationsframework für mehrere Domänen
- ein Archivsystem für Kandidaten, Fingerprints und Genealogie
- ein Sicherheitsrahmen für generierte Ausführung

---

## 4. Kernidee

Die Kernidee ist, Algorithmusentwicklung als Suchprozess über einem strukturierten Bausteinraum zu modellieren.

Ein Nutzer wählt beispielsweise:

```text
Domain: root_finding
Teile: tanh, curvature, secant_mix, bracket_guard, finite_sanitize, local_slope
Profil: safety_first
```

Das System übersetzt dies nicht nur in Metadaten, sondern in konkrete Suchsteuerung:

- erlaubte oder bevorzugte Token
- Feature-Druck
- Policy-Ausgangsgewichte
- Sicherheitsanforderungen
- Ausdrucksbudget
- Guided Seed
- Novelty- und Nontriviality-Anforderungen

Dann sucht das System nach einem neuen Kandidaten, der diese Signale nutzt.

---

## 5. Systemarchitektur

### 5.1 Übersicht

```text
+--------------------------+
| WebGUI Research Console  |
+------------+-------------+
             |
             v
+--------------------------+
| Experiment Manifest      |
+------------+-------------+
             |
             v
+--------------------------+
| Guided Discovery CLI     |
+------------+-------------+
             |
             v
+--------------------------+
| Generator Layer          |
| - Random/Evolution       |
| - SNN-guided generation  |
+------------+-------------+
             |
             v
+--------------------------+
| Evaluation Layer         |
| - CPU evaluator          |
| - OpenCL VM              |
| - Domain metrics         |
+------------+-------------+
             |
             v
+--------------------------+
| Algorithm Synthesis      |
| - Python export          |
| - C++ header export      |
| - OpenCL export          |
| - Manifest/README/Test   |
+------------+-------------+
             |
             v
+--------------------------+
| Archive + Reports        |
+--------------------------+
```

### 5.2 Native Core

Der C++-Kern enthält:

- Expression-/Genome-Repräsentation
- Fitness- und Novelty-Bewertung
- SNN-Modell
- OpenCL-Abstraktion
- Algorithmus-Synthese
- Substratmechaniken
- Archiv- und Fingerprint-Logik
- CLI-Befehle

### 5.3 Python-WebGUI

Die WebGUI ist bewusst ohne schwere externe Frameworks realisiert. Sie startet lokale Jobs, schreibt Manifeste, liest Resultate, zeigt Artefakte und führt Tests sowie Domain-Evaluationen aus.

---

## 6. Datenmodell

### 6.1 Kandidat

Ein Kandidat enthält:

```json
{
  "id": "...",
  "name": "...",
  "fitness": 0.93,
  "novelty": 0.91,
  "accuracy": 0.88,
  "stability": 0.96,
  "complexity": 0.83,
  "expression": "...",
  "ast_fingerprint": "...",
  "behavior_fingerprint": "...",
  "derivative_fingerprint": "...",
  "stability_fingerprint": "...",
  "complexity_fingerprint": "...",
  "parent_a": "...",
  "parent_b": "...",
  "birth_generation": 123,
  "mutation_trace": "..."
}
```

### 6.2 Algorithmus-Artefakt

Ein exportierter Algorithmus enthält:

```text
Name
Kind
Domain
Kernel Expression
Algorithm Summary
Mathematical Contract
Pseudocode
Complexity
Validation Protocol
Export Files
Manifest
Tests
```

### 6.3 Archiv

Das Archiv ist JSONL-basiert und dient als Gedächtnis des Systems. Es speichert Kandidaten, Fingerprints, Genealogie und Scores. Dadurch kann Novelty gegen vergangene Entdeckungen bewertet werden.

---

## 7. Formula-Part Guided Discovery

### 7.1 Warum Formula Parts?

Reine Zufallssuche ist ineffizient. Menschliche Intuition ist wertvoll. Formula Parts erlauben, Forschungsideen als Suchdruck einzuspeisen.

Beispiele:

```text
tanh
curvature
secant_mix
bracket_guard
finite_sanitize
local_slope
regula_falsi
cubic
scale
priority_queue
edge_relaxation
deadline_pressure
constraint_propagation
reuse_distance
tail_latency
```

### 7.2 Kategorien

Die Formula-Part Library enthält unter anderem:

- primitive Zustandsgrößen
- Geometrie
- Memory/History
- Nichtlinearitäten
- Normalisierung
- Policy-Ausgänge
- Sicherheitsmechanismen
- Strategien
- Informatik-Kerndomain-Bausteine

### 7.3 Suchwirkung

Die Auswahl beeinflusst:

- Guided Seed
- Expressionsbudget
- bevorzugte Primitive
- Mutation Bias
- Domain-spezifische Evaluationslogik
- Exporttyp
- Sicherheitsprofil

Damit ist die Auswahl nicht dekorativ, sondern algorithmisch wirksam.

---

## 8. Root Policy V2

Ein zentraler Durchbruch war die Erweiterung von einfachen Kerneln `K(x)` zu einer hochdimensionalen Root-Finding-Policy.

Statt:

```text
candidate = mid - step
```

verwendet Root Policy V2:

```text
RootContext[30] -> {
  bias,
  damping,
  secant_mix,
  bisection_mix,
  relaxation_mix,
  trust_delta
}
```

### 8.1 RootContext Features

```text
x, fx, lo, hi, flo, fhi, mid, width, relpos,
prev_x, prev_fx, prev2_x, prev2_fx,
bracket_slope, local_slope, prev_slope, curvature,
improvement, stagnation, trust, reject_rate,
scale, nfx, nflo, nfhi, nwidth, sign_change,
edge_balance, residual_ratio, width_ratio
```

### 8.2 Safe Candidate Mixer

Die Policy mischt:

- Bisektion
- Sekantenkandidat
- gelernte Relaxation

und erzwingt:

- Bracket Guard
- Finite Sanitize
- Fallback zu Bisektion
- Trust-/Reject-Dynamik
- Gas-Limit

Damit sucht das System nicht nur Formeln, sondern adaptive numerische Strategien.

---

## 9. Unterstützte mathematische Domains

### 9.1 Root Finding

Ziel: sichere Nullstellensuche mit adaptiven Update-Regeln.

Erzeugte Typen:

```text
root_refiner
```

Validierung:

- Bracket-Erhaltung
- Residual-Reduktion
- Finite Output
- Vergleich gegen Bisektion
- Gas-Limit

### 9.2 Chaotic Maps

Ziel: iterierte Abbildungen und dynamische Systeme.

Erzeugte Typen:

```text
chaotic_map_explorer
```

Metriken:

- Orbit-Entropie
- Lyapunov-ähnliche Divergenz
- Periodizität
- Attractor Spread
- Return-Map Diversity
- Vergleich gegen Logistic/Tent/Sine Map

### 9.3 Signal Transform

Ziel: adaptive Signaltransformationen und Morphing.

### 9.4 Fixed Point Dynamics

Ziel: Iterationen, die Fixpunkte finden oder untersuchen.

### 9.5 Sequence Generation

Ziel: Sequenzfortsetzung und rekurrente Muster.

### 9.6 Classification Boundary

Ziel: generierte Trennfunktionen.

### 9.7 Symbolic Identity Search

Ziel: Residuen- und Identitätssuche.

---

## 10. Unterstützte Informatik-Kerndomänen

Algorithmic Genesis wurde zu einer Multi-Core Computer Science Discovery Platform erweitert.

### 10.1 Sorting

Domain:

```text
sorting
```

Erzeugter Typ:

```text
adaptive_sorter
```

Ziel: adaptive Sortierstrategien, die Eigenschaften der Eingabe berücksichtigen.

### 10.2 Graph Shortest Path

Domain:

```text
graph_shortest_path
```

Erzeugter Typ:

```text
graph_pathfinder
```

Ziel: Pfadsuche unter Nutzung von Relaxation, Priorität, Heuristik und Kostenstruktur.

### 10.3 Scheduling

Domain:

```text
scheduling
```

Erzeugter Typ:

```text
schedule_optimizer
```

Ziel: Jobs auf Worker verteilen, Makespan und Fairness optimieren.

### 10.4 Parsing

Domain:

```text
parsing
```

Erzeugter Typ:

```text
parser_repairer
```

Ziel: Parser-/Repair-Strategien für Tokenfolgen.

### 10.5 Constraint Solving

Domain:

```text
constraint_solving
```

Erzeugter Typ:

```text
constraint_solver
```

Ziel: heuristische Variablen-/Werteordnung und Constraint Propagation.

### 10.6 Stream Processing

Domain:

```text
stream_processing
```

Erzeugter Typ:

```text
stream_processor
```

Ziel: Online-Metriken, Glättung, Anomalie- und Trenddruck.

### 10.7 Compression

Domain:

```text
compression
```

Erzeugter Typ:

```text
adaptive_compressor
```

Ziel: einfache adaptive Kodierungs- und Roundtrip-Strategien.

### 10.8 Cache Eviction

Domain:

```text
cache_eviction
```

Erzeugter Typ:

```text
cache_policy
```

Ziel: Ersetzungspolitiken mit Recency, Frequency und Reuse Distance.

### 10.9 Load Balancing

Domain:

```text
load_balancing
```

Erzeugter Typ:

```text
load_balancer
```

Ziel: Lastverteilung, Tail Latency und Varianzreduktion.

### 10.10 Anomaly Detection

Domain:

```text
anomaly_detection
```

Erzeugter Typ:

```text
anomaly_detector
```

Ziel: robuste Outlier-Erkennung über statistische Signale.

---

## 11. Code-Export

Jeder akzeptierte Algorithmus erzeugt:

```text
*_algorithm.py
*_algorithm.hpp
*_algorithm.cl
*_manifest.json
*_README.md
test_*_algorithm.py
```

### 11.1 Python

Der Python-Export ist direkt ausführbar und enthält:

- sanitize
- gas limit
- generierte Kernel
- Domain-Funktionen
- Demo
- RootContext/Policy bei Root-Finding
- CS-Domainfunktionen bei Informatikdomains

### 11.2 C++ Header

Der C++ Header dient als statische, einbettbare Referenz und legt ABI-nahe Strukturen an.

### 11.3 OpenCL

Der OpenCL Export erlaubt GPU-kompatible Kernel- oder Funktionsrepräsentationen.

### 11.4 Manifest

Das Manifest ist das auditierbare Objekt:

```json
{
  "name": "...",
  "kind": "...",
  "domain": "...",
  "kernel_expression": "...",
  "algorithm_score": 0.94,
  "nontriviality": 0.98,
  "fingerprints": {...},
  "export_files": [...]
}
```

---

## 12. OpenCL und GPU-Substrat

Algorithmic Genesis nutzt OpenCL für:

- Device Probe
- SNN-Simulation
- VM-Differentialtests
- OpenCL-Fitnesspfade
- potentielle Batch-Evaluation

Der GPU-Pfad ist nicht als Show-Effekt gedacht, sondern als Substrat für große Kandidatenmengen.

### 12.1 GPU-VM

Die GPU-VM führt Genom-/Bytecode-artige Kandidaten gegen Samples aus und wird gegen CPU-Referenz differenziell geprüft.

### 12.2 SNN

Das SNN dient als Generator- und Dynamikquelle. Es kann auf CPU oder OpenCL laufen.

### 12.3 Somnia, CCQ, Hardware Probe

Fortgeschrittene Substratmechaniken:

- Somnia Shadow Phase
- CCQ-inspirierte Gewichtquantisierung
- Continuous Differential Hardware Probing
- Zero-Sum Arena
- Numeric Poison Containment

---

## 13. Sicherheit

Generierte Algorithmen sind gefährlich, wenn sie unbegrenzt laufen oder NaN/Inf erzeugen. Algorithmic Genesis erzwingt daher Schutzmechanismen.

### 13.1 Numeric Poison Containment

Alle Exporte enthalten `sanitize()`.

### 13.2 Gas Limits

Operationen werden über `gas_left` begrenzt.

### 13.3 Bracket Guards

Root-Finding verlässt niemals absichtlich das Bracket.

### 13.4 Fallbacks

Wenn ein generierter Kandidat unsicher ist, fällt das System auf Baselines wie Bisektion zurück.

### 13.5 Differentialtests

OpenCL-Ergebnisse werden gegen CPU-Referenzpfade geprüft.

---

## 14. WebGUI Research Console

Die WebGUI bietet:

1. Experiment-Konfiguration
2. Auswahl von Formel-/Strategie-Teilen
3. Discovery-Start
4. Live-/Statusanzeige
5. Resultat
6. Report
7. Exportierte Artefakte
8. Python-Testausführung
9. Domain-Evaluation
10. Dateiansicht

Der Nutzer muss mindestens 3 Teile wählen. Empfohlen sind 5 bis 20.

---

## 15. Domain-Evaluation

Die WebGUI kann erzeugte Algorithmen domänenspezifisch bewerten.

### 15.1 Chaotic Maps

- Orbit-Plot
- Histogramm
- Return-Map
- Lyapunov-like
- Periodicity
- Attractor Spread
- Orbit Entropy
- Baseline-Vergleich

### 15.2 Root Finding

- Testfunktionen
- Bisection-Baseline
- Residual
- Bracket-Erhaltung

### 15.3 Informatikdomains

- Sorting: Sortedness
- Graph: Pfadkosten
- Scheduling: Makespan
- Constraints: Satisfied Assignment
- Compression: Ratio/Roundtrip
- Cache: Hits/Misses
- Load Balancing: Fairness
- Anomaly Detection: Outlier-Indizes

---

## 16. Entwicklerworkflow

### 16.1 Lokaler Build

1. Repository entpacken.
2. OpenCL via vcpkg installieren.
3. Visual-Studio-CMake verwenden.
4. Build erzeugen.
5. Tests ausführen.

### 16.2 Forschungslauf

1. WebGUI starten.
2. Domain wählen.
3. Teile wählen.
4. Experiment starten.
5. Resultat prüfen.
6. Tests ausführen.
7. Domain-Evaluation ausführen.
8. Kandidat archivieren.
9. Gegen Baselines vergleichen.

### 16.3 CLI-Forschung

Statt WebGUI kann ein Experiment-Manifest über `discover-guided` ausgeführt werden.

---

## 17. Bewertungskriterien für Kandidaten

Ein Kandidat ist interessant, wenn:

```text
accepted = true
algorithm_score hoch
nontriviality hoch
novelty hoch
stability ausreichend
vm_differential.max_abs_error klein
unique_behavior steigt
warnings leer
exportierte Tests ok
```

Kandidaten mit hoher Fitness, aber trivialem Verhalten, sind nicht automatisch wertvoll.

---

## 18. Beispiel: Root-Finding-Kandidat

Ein früher Kandidat erzeugte:

```text
K(x) = x³ - x
```

Dies war keine bewiesene Weltneuheit, aber ein valider Proof, dass das System aus evolutionärer Suche einen sicheren Root-Finding-Algorithmus exportieren kann.

Spätere Root Policy V2-Kandidaten nutzen:

```text
RootContext[30] -> Policy Outputs
```

und sind algorithmisch interessanter, weil sie adaptive Strategien entwickeln.

---

## 19. Beispiel: Chaotic Map Explorer

Ein WebGUI-Lauf erzeugte einen Chaos-Map-Kandidaten mit bounded recurrence:

```text
K(x) = tanh(tanh(sin(sin(x+x) + sin(x+x))))
```

Der exportierte Algorithmus erzeugt Orbits und berechnet eine Lyapunov-ähnliche Divergenzschätzung. Das ist forschungsrelevant, aber kein Beweis für echten mathematischen Chaoscharakter.

---

## 20. Grenzen des Systems

Das System kann Kandidaten erzeugen, aber:

- es beweist keine Optimalität
- es garantiert keine mathematische Neuheit
- es ersetzt keine wissenschaftliche Validierung
- es kann bekannte Strukturen neu kombinieren
- es kann viele strukturell verschiedene, aber verhaltensähnliche Kandidaten erzeugen
- GPU-Nutzung ist nicht in jedem Discovery-Schritt dominant
- manche Domänen haben derzeit Smoke-Evaluationen statt vollständiger Benchmarks

---

## 21. Nächste Forschungsschritte

Wichtige nächste Entwicklungsrichtungen:

1. stärkere Behavior-Diversity
2. Baseline-Benchmark-Suites pro Domain
3. automatische Paper-/Report-Generierung pro Kandidat
4. adaptive Domain-Metriken
5. persistent fused OpenCL kernels
6. Batch-Fitness für ganze Populationen
7. Symbolic Simplification und Anti-Triviality
8. Human-in-the-loop Experiment Planning
9. Cross-Domain Transfer: z. B. Scheduling-Ideen für Graphsuche
10. Multi-objective Pareto Archive

---

## 22. Enterprise-Aspekte

### 22.1 Reproduzierbarkeit

Jeder Lauf enthält:

- Seed
- Guided Seed
- Domain
- Profil
- Parts
- Archive
- Report
- Result JSON
- Exportdateien

### 22.2 Auditierbarkeit

Manifest und Report dokumentieren jeden Kandidaten.

### 22.3 Sicherheit

Gas-Limits, Sanitizing und Fallbacks minimieren Ausführungsrisiken.

### 22.4 Erweiterbarkeit

Neue Domains können ergänzt werden durch:

- Formula Parts
- Domain Generator
- Exportfunktion
- Testfunktion
- Domain-Evaluator
- WebGUI Labeling

---

## 23. Wie ein Entwickler das Projekt erweitert

### 23.1 Neue Domain hinzufügen

1. Domain-Namen definieren.
2. Formula Parts ergänzen.
3. Exportfunktion in Algorithmus-Synthese ergänzen.
4. Python-Testgenerator ergänzen.
5. Domain-Evaluator in WebGUI ergänzen.
6. Beispielmanifest hinzufügen.
7. Tests ergänzen.

### 23.2 Neue Formula Parts hinzufügen

1. Part ID definieren.
2. Kategorie und Beschreibung ergänzen.
3. Bias-Compiler anpassen.
4. WebGUI-Katalog aktualisieren.
5. Tests ausführen.

### 23.3 Neue Baseline hinzufügen

1. Evaluationsfunktion schreiben.
2. Metriken definieren.
3. JSON-Ausgabe erweitern.
4. GUI-Rendering ergänzen.

---

## 24. Architekturentscheidungen

### 24.1 Warum C++?

Für native Performance, klare Speicherlayouts, OpenCL-Integration und deterministische Builds.

### 24.2 Warum Python-WebGUI?

Für schnelle lokale Research-Interaktion, Artefaktanzeige, Tests und Domain-Evaluationen.

### 24.3 Warum OpenCL?

Für breite GPU-Unterstützung auf AMD, Intel und NVIDIA ohne CUDA-Lock-in.

### 24.4 Warum JSON/JSONL?

Für Auditierbarkeit, einfache Analyse, Archivierung und Tooling.

### 24.5 Warum Gas-Limits?

Weil generierte Algorithmen sonst unbegrenzt laufen könnten.

---

## 25. Fazit

Algorithmic Genesis ist eine Plattform zur erforschenden Algorithmus-Synthese. Sie verbindet menschliche Auswahl, evolutionäre Suche, SNN-inspirierte Erzeugung, GPU-beschleunigte Evaluierung, Codeexport und WebGUI-basierte Domain-Evaluation.

Das Projekt ist besonders wertvoll, weil es nicht bei Textvorschlägen stehen bleibt. Es erzeugt ausführbaren Code, Tests und Reports. Damit können Entwickler und Forscher neue algorithmische Ideen systematisch erzeugen, prüfen, vergleichen und weiterentwickeln.

Der aktuelle Stand ist ein belastbares Forschungs-Substrat. Die erzeugten Algorithmen sind Kandidaten, keine bewiesenen Theoreme. Der wissenschaftliche Wert entsteht durch Wiederholbarkeit, Archivierung, Vergleich gegen Baselines und menschliche Interpretation.

---

## 26. Kurzdefinition für Entwickler

> Algorithmic Genesis ist ein lokales C++/OpenCL/Python-System zur geführten Entdeckung, Bewertung und Code-Synthese neuer mathematischer und informatischer Algorithmus-Kandidaten. Die WebGUI übersetzt menschlich ausgewählte Formel-/Strategiebausteine in Suchdruck, startet Discovery-Läufe und exportiert getestete Python/C++/OpenCL-Artefakte samt Manifest, Report und Domain-Evaluation.
