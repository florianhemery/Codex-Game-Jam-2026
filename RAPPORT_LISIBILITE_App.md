# Rapport Lisibilite — Module App (Lot L11)

**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite app`

## Objectif

Extraire la logique applicative de `src/main.cpp` (469 lignes) vers `src/App/`, sans changement de comportement.

## NEW_SOURCES

| Fichier | Lignes | Role |
|---------|--------|------|
| `src/App/MainApp.hpp` | 16 | Facade `MainApp::run()` |
| `src/App/MainApp.cpp` | 34 | Init fenetre, pipeline, delegation boucle |
| `src/App/GameLoop.hpp` | 226 | Etat `Context`, passes rendu, helpers inline |
| `src/App/GameLoop.cpp` | 223 | Menu, simulation, rendu HUD, boucle principale |
| `src/App/CameraController.hpp` | 24 | API camera chase |
| `src/App/CameraController.cpp` | 77 | `initCamera`, `updateCamera` (decoupe lerp) |
| `src/App/LapTimer.hpp` | 31 | `LapTimerState` |
| `src/App/LapTimer.cpp` | 35 | `updateLapTimer` |
| `src/App/RacerColors.hpp` | 27 | Palette coureurs + ambiance piste |
| `src/App/RacerColors.cpp` | 37 | `colorForRacerIndex`, `ambianceForTrack` |

## Fichiers modifies

| Fichier | Avant | Apres | Role |
|---------|-------|-------|------|
| `src/main.cpp` | 469 | 13 | Point d'entree minimal |
| `CMakeLists.txt` | — | — | 5 `.cpp` App ajoutes a la cible `racer` |

## Conformite lisibilite

| Regle | Statut |
|-------|--------|
| Fichiers <= 300 lignes | OK (max : `GameLoop.hpp` 226L) |
| Fonctions <= 20 lignes | OK (`updateCamera` decoupe, `readSteerTarget` extrait) |
| Zero changement comportement | OK (refactor structurel pur) |
| Includes autres modules inchanges | OK |

## Verification build

Compilation manuelle des sources App + `main.cpp` avec `x86_64-w64-mingw32-clang++` : **OK**.  
`cmake --build build --target racer` bloque par erreur de permission sur `raylib-subbuild` (environnement).
