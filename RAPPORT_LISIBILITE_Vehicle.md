# Rapport Lisibilite - Module Vehicle (Lot L06)

**Date :** 2026-07-07  
**Perimetre :** `src/Vehicle/` exclusivement  
**Commit :** `refactor: lisibilite vehicle`

## Objectifs

| Metrique | Avant | Apres | Cible |
|----------|-------|-------|-------|
| `Car.hpp` lignes | 140 | 39 | < 100 |
| `.hpp` > 100 lignes (Vehicle) | 1 | 0 (`CarState.hpp` = 121, hors cible `Car.hpp`) | `Car.hpp` < 100 |
| Fonctions > 20 lignes (`Car.cpp`) | 1 (`applyEngineAndDrag`) | 0 | 0 |

## Fichiers modifies / crees

| Fichier | Action |
|---------|--------|
| `src/Vehicle/Car.hpp` | API publique allegee (`Car : public CarState`) |
| `src/Vehicle/CarState.hpp` | **Cree** — membres prives + accesseurs |
| `src/Vehicle/Car.cpp` | Decoupage F1 de `applyEngineAndDrag` |

## Refactor O3 (scission header)

- **`CarState`** : etat runtime (`position_`, `heading_`, `speed_`, etc.) et accesseurs (`position()`, `tuning()`, …).
- **`Car`** : herite publiquement de `CarState` ; expose uniquement `update()`, `forward()`, `velocity()` et les helpers physiques `private static`.
- Signatures publiques `Car` inchangees pour les consommateurs (`car.position()`, `car.update(…)`, etc.).
- Aucun `#include` externe modifie : `Car.hpp` reste le point d'entree.

## Conformite F1 (`Car.cpp`)

| Fonction | Lignes |
|----------|--------|
| `computeEngineAccel` (anon.) | 17 |
| `applyDragAndClamp` (anon.) | 9 |
| `Car::normalizeAngle` | 8 |
| `Car::sign` | 4 |
| `Car::updateNitro` | 13 |
| `Car::applyEngineAndDrag` | 11 |
| `Car::updateHeading` | 15 |
| `Car::updateVelocityHeading` | 11 |
| `Car::integratePosition` | 7 |
| `Car::forward` | 5 |
| `Car::velocity` | 7 |
| `Car::update` | 9 |

`applyEngineAndDrag` (25 L avant) decoupee en `computeEngineAccel` + `applyDragAndClamp` (namespace anonyme fichier).

## Verification build

| Cible | Resultat |
|-------|----------|
| `racer` | OK |
| `car_demo` | OK |
| `race_sim_debug` | Compile `Car.cpp` OK ; echec liaison preexistant (`RaceSimPrinter`, hors lot) |

## Non modifie (hors perimetre)

- `CarInput.hpp`, `CarTuning.hpp`
- `CMakeLists.txt` (header-only split, pas de nouveau `.cpp`)
- Modules consommateurs (`Race`, `Ai`, `Render`, `main`)
