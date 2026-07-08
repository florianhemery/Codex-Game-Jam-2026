# Rapport Lot A01 - Vehicle (car.h / car.cpp)

## Fichiers modifies

- `src/vehicle/car.h`
- `src/vehicle/car.cpp`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech standard (projet `racer`) |
| H2 | `#pragma once` remplace par `#ifndef CAR_H_` / `#define` / `#endif` |
| F1 | `Car::Update` decoupe en methodes `CarPhysics` (<= 20 lignes chacune) |
| F2 | Lignes > 80 colonnes reformatees |
| F5 | Commentaires supprimes dans les corps de fonctions |
| G8 (partiel) | `NormalizeAngle` / `Sign` (namespace anonyme) -> `CarPhysics` static |
| G5 | Helpers fichier-local encapsules dans `CarPhysics` (portee fichier) |
| V2 | `struct Car` -> `class Car` (comportement interne) ; POD inchanges |
| L1-L6 | Indentation 4 espaces, accolades, espaces operateurs |
| A1 | References `const` sur parametres non modifies |
| E6 | Un seul `return` par fonction helper |
| G2 | Ligne vide entre implementations de fonctions |

## Items transverses (lots sequentiels)

| Regle | Detail | Lot cible |
|-------|--------|-----------|
| O2 | Extension `.h` -> `.hpp` | S1 |
| N2 | Fichier `car.h` / dossier `vehicle` -> `Car.hpp` / `Vehicle/` | S2 |
| O3 | Trois types dans un header (`CarInput`, `CarTuning`, `Car`) | S4 |
| N3 | API publique PascalCase (`Update`, `Forward`, `Velocity`) | S3 |
| K1 | Convention attributs membres (`_suffix` ou prefixe) | S3 |
| D2 | Attributs `Car` publics (acces direct depuis race/ai/render) | S4 |
| G8 (strict) | `CarPhysics` reste classe interne `.cpp`, pas integration dans `Car` | S4 |
| Duplication | `NormalizeAngle` aussi present dans `ai_driver.cpp` | S4 |

## Build

Cible impactee : `racer`, `race_sim_debug`, `car_demo` (via `car.cpp`).
