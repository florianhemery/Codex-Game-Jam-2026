# Rapport Lisibilite - Lot L10 Tools

**Module :** `src/Tools/`  
**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite tools`

## Objectif

Scinder `RaceSimDebug.cpp` monolithique en simulation (`RaceSimDebug`) et affichage (`RaceSimPrinter`), avec `runRace()` reduit a trois etapes et toutes les fonctions <= 20 lignes (F1).

## NEW_SOURCES

| Fichier | Role |
|---------|------|
| `src/Tools/RaceSimDebug.hpp` | Declarations simulation : `SimContext`, constantes, API `RaceSimDebug` |
| `src/Tools/RaceSimDebug.cpp` | Implementation simulation + `main()` ; inclut `RaceSimPrinter.cpp` |
| `src/Tools/RaceSimPrinter.hpp` | Declarations affichage console (classement, logs, erreurs) |
| `src/Tools/RaceSimPrinter.cpp` | Implementation affichage |

**CMake (hors lot, a consolider) :** ajouter `src/Tools/RaceSimPrinter.cpp` a la cible `race_sim_debug` et retirer le `#include "Tools/RaceSimPrinter.cpp"` de `RaceSimDebug.cpp` pour une compilation multi-unites propre.

## Refactor `runRace()`

Avant (orchestration inline) :

```
init header -> simulateSteps -> printStandings -> evaluateHealth -> verdict
```

Apres (resume lisible) :

```cpp
bool RaceSimDebug::runRace(int presetIndex)
{
    SimContext ctx = initSimulation(presetIndex);

    advanceFrames(ctx);
    return printResults(ctx);
}
```

| Etape | Responsabilite |
|-------|----------------|
| `initSimulation()` | Charge preset, construit `SimContext`, affiche en-tete |
| `advanceFrames()` | Boucle de simulation (ex-`simulateSteps`) |
| `printResults()` | Classement, sante, erreurs, verdict final |

## Decoupage des responsabilites

| Classe | Fichiers | Contenu |
|--------|----------|---------|
| `RaceSimDebug` | `.hpp` / `.cpp` | Metriques, sante, boucle de simulation |
| `RaceSimPrinter` | `.hpp` / `.cpp` | Tous les `printf` (header, logs, classement, erreurs) |

## Conformite F1 (fonctions <= 20 lignes)

| Fonction | Lignes | Statut |
|----------|--------|--------|
| `runRace` | 6 | OK |
| `initSimulation` | 8 | OK |
| `advanceFrames` | 17 | OK |
| `printResults` | 8 | OK |
| `printStandingRows` (ancien 23L) | 13 | OK (extrait `printStandingRowAt`) |
| `main` | 18 | OK (helpers `isValidPresetIndex`, `runAllPresets`) |
| Autres fonctions | <= 16 | OK |

## Metriques

| Indicateur | Avant | Apres |
|------------|-------|-------|
| Fichiers `src/Tools/` | 1 | 4 |
| Lignes `RaceSimDebug.cpp` | 316 | 191 (+ include printer) |
| Classe anonyme | oui | non (headers publics outil) |
| Fonctions > 20L | 1 (`printStandingRows`) | 0 |

## Verification build

```
cmake --build build --target race_sim_debug
```

**Resultat :** compilation et liaison OK.
