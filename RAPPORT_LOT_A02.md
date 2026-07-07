# RAPPORT_LOT_A02 - Track

**Lot :** A02  
**Fichiers :** `src/track/track.h`, `src/track/track.cpp`  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style track`

## Regles corrigees localement

| Regle | Action |
|-------|--------|
| **G1** | Header Epitech (`racer`) ajoute en tete des deux fichiers |
| **H2** | `#pragma once` remplace par `#ifndef TRACK_H_` / `#define` / `#endif` |
| **F1** | `Make` decoupe en `AppendEastStraight`, `AppendNorthCurve`, `AppendWestStraight`, `AppendSouthCurve` ; `ProjectPosition` delegue a `SampleSegment` |
| **F2** | Toutes les lignes <= 80 colonnes |
| **F5** | Commentaires supprimes dans tous les corps de fonctions |
| **L4** | Accolades ouvrantes des definitions de fonctions sur ligne separee |
| **C2** | Ternaire imbrique remplace par `if` / `else if` dans `SampleSegment` |
| **E6** | `PointAtDistance` restructure avec un seul `return` final |
| **G8** (partiel) | Helper `Length` (namespace anonyme) -> `Track::SegmentLength` static prive |
| **A2** | `size_t` remplace par `std::size_t` |
| **G6** | Constantes `kCurveSegments` / `kStraightSegments` en `constexpr` |
| **D5** | Methodes de lecture deja `const`, conservees |

## Build

Cibles impactees compilees avec succes :

- `track_demo`
- `race_sim_debug`

Commande : `cmake --build build --target track_demo race_sim_debug`

## Items transverses (lots sequentiels)

| Regle | Item | Lot prevu |
|-------|------|-----------|
| **O2** | Extension `.h` au lieu de `.hpp` | S1 |
| **N2** | Fichiers `track.h` / `track.cpp` en snake_case, dossier `src/track/` | S2 |
| **O3** | `SurfaceStyle`, `TrackDef` et `Track` dans le meme fichier | S4 |
| **N3** | API publique PascalCase (`Make`, `Presets`, `StartPosition`, `ProjectPosition`, etc.) | S3 |
| **N5** | Valeurs enum `SurfaceStyle::Propre`, `SurfaceStyle::Abimee` | S3 |
| **G8** | `TrackDef` reste un struct libre hors classe | S4 (scission prevue) |
| **K1** | Convention attributs `_suffix` deja presente ; harmonisation globale | S3 |

## Notes

- Aucun changement fonctionnel : generation de waypoints, projection, distances et presets inchanges.
- Tirets doubles (`--`) remplaces par des virgules dans les chaines de presets.
- `#include "track/track.h"` inchange (interdit cross-lot).
