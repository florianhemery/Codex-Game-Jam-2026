# Rapport Lot A08 - Core-Components (components.h)

## Fichiers modifies

- `src/engine/core/components.h`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech standard (projet `racer`) |
| H2 | `#pragma once` remplace par `#ifndef COMPONENTS_H_` / `#define` / `#endif` |
| F2 | Ligne `#include "raylib.h"` avec commentaire inline supprimee (< 80 cols) |
| F5 | Commentaires inline sur membres supprimes |
| L1-L6 | Indentation 4 espaces, lignes vides entre structs, includes groupes |
| G2 | Ligne vide entre definitions de types |
| D1 | Commentaires de bloc deplaces vers la description de fichier G1 |

## Items transverses (lots sequentiels)

| Regle | Detail | Lot cible |
|-------|--------|-----------|
| O2 | Extension `.h` -> `.hpp` | S1 |
| N2 | Fichier `components.h` / dossier `core` -> PascalCase | S2 |
| O3 | Huit types dans un header (`TransformComponent`, `KinematicsComponent`, `RenderMeshComponent`, `LapProgressComponent`, `NameComponent`, `PlayerTag`, `AiTag`) | S4 |
| N3 | Symboles publics PascalCase (`TransformComponent`, etc.) | S3 |
| K1 | Attributs membres sans suffixe `_` (`meshId`, `velocityHeading`, etc.) | S3 |
| A7 | Include `raylib.h` pour types `Vector3` / `Color` uniquement (acceptable, info) | - |

## Build

Cible impactee : `engine_core`, `engine_core_test` (via `frame_snapshot.cpp`, `engine_core_test.cpp`).
