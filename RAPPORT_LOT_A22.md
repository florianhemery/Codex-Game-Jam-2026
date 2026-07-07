# RAPPORT_LOT_A22 - Entry + Tools

Lot Phase 1 A22 : `src/main.cpp`, `src/tools/race_sim_debug.cpp`

## Regles appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech `racer` ajoute aux deux fichiers |
| F1/F4 | Decoupe de `main()` et `RunRace()` en sous-fonctions <= 20 lignes |
| F2/F3 | Retour a la ligne pour respecter 80 colonnes |
| F5 | Suppression de tous les commentaires dans les corps de fonctions |
| L1-L6 | Accolades, indentation 4 espaces, une declaration par ligne |
| V1-V2 | Helpers locaux en camelCase, constantes `k` prefix |
| C1-C5 | Constantes magiques extraites (`kAiCount`, `kLaps`, `kDt`, etc.) |
| G8 partiel | Helpers `namespace {}` -> classes `MainApp` / `RaceSimDebug` |
| A1-A8 | Functors `OpaquePass`/`LitPass`/`VfxPass` pour callbacks pipeline |
| E1-E6 | Retours codes inchanges (`main`, `race_sim_debug`) |
| K1-K6 | Structs d'etat (`Context`, `SimContext`, `LapTimerState`) |
| S1-S3 | Espacement operateurs, references `Type &var` |
| D1-D6 | Description fichier dans en-tete G1 (ex-commentaires fichier) |

## Fichiers modifies

- `src/main.cpp` : classe `MainApp` (menu, boucle course, rendu, VFX, camera)
- `src/tools/race_sim_debug.cpp` : classe `RaceSimDebug` (simulation headless)

## Compilation

| Cible | Resultat |
|-------|----------|
| `CMakeFiles/racer.dir/src/main.cpp.obj` | OK |
| `CMakeFiles/race_sim_debug.dir/src/tools/race_sim_debug.cpp.obj` | OK |
| `racer` (lien final) | Bloque par erreurs hors lot (`car_renderer.cpp`, `asset_registry.cpp`, lots paralleles) |

## Items transverses (non modifies, lot sequentiel)

| Item | Regle | Lot cible |
|------|-------|-----------|
| `#include "*.h"` (pas `.hpp`) | O2/H2 | S1 |
| API `DrawMenu`, `DrawHudEx`, `DrawCarEx`, `GetCarLightPoints` | N3 | S3 |
| `RacePhase::Racing` / `Finished`, `SurfaceStyle::Abimee` | N5 | S3 |
| `Ambiance::Midi`, `AubeDoree`, `Orage`, etc. | N5 | S3 |
| `AppState::Menu` / `Racing` (enum locale, PascalCase) | N5 | S3 si exporte |
| Appels raylib directs (`InitWindow`, `DrawText`, etc.) | A7 | Acceptable (wrappers moteur) |
| Conversion G8 stricte `GameApp` publique | G8 | S4 |
| Arborescence `src/tools/` snake_case | N2 | S2 |

## Changement fonctionnel

Aucun. Refactor structurel uniquement, logique de jeu et criteres de sante `RACE_SIM` preserves.
