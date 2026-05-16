# Algorithm Synthesis V2: Kontextsensitiver Root-Finder

Diese Version erweitert `discover-algorithm` für die Domäne `root_finding`.

## Vorher

Der erzeugte Root-Finder nutzte nur einen skalaren Kernel:

```text
K(x)
```

Das war robust, aber mathematisch begrenzt, weil der Update-Schritt die Funktion selbst kaum sah.

## Jetzt

Der exportierte Algorithmus enthält zusätzlich:

```text
K_ctx(x, f(x), f(lo), f(hi), history, width)
```

Die Update-Regel nutzt damit:

- aktuelle Position `x`
- aktuellen Residualwert `f(x)`
- Randwerte `f(lo)` und `f(hi)`
- residuale Verbesserungshistorie `history`
- aktuelle Intervallbreite `width`

## Sicherheitsinvariante

Trotz kontextsensitiver Update-Regel bleibt der Algorithmus bracketed:

```text
candidate = clamp(mid - step, lo, hi)
```

Wenn der Kontextschritt den Residualwert nicht verbessert, fällt der Algorithmus auf Bisektion zurück.

## Exportierte Artefakte

`discover-algorithm --domain root_finding` erzeugt:

- Python-Code mit `context_kernel(...)`
- C++ Header mit `context_kernel(...)`
- OpenCL Kernel mit `context_kernel(...)`
- Manifest mit `"kernel_arity":"contextual:x,fx,flo,fhi,history,width"`
- README und Testdatei

## Test

```bat
python root_algorithms\test_<name>_algorithm.py
```

Der Test prüft:

- finite Kernel-Ausgabe
- Gas-Limit
- `context_kernel`
- Root-Finding auf `x*x - 2`
- Klammer-Invariante
