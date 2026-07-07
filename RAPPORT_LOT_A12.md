# Rapport Lot A12 - RHI Graph (render_graph.h / render_graph.cpp)

## Fichiers modifies

- `src/engine/rhi/render_graph.h`
- `src/engine/rhi/render_graph.cpp`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech standard (projet `racer`) |
| H2 | `#pragma once` remplace par `#ifndef RENDER_GRAPH_H_` / `#define` / `#endif` |
| F1 | `Execute` et `AcquireTransientTarget` decoupes (`executePass`, `tryReusePooledTarget`) |
| F2 | Lignes > 80 colonnes reformatees |
| F5 | Commentaires supprimes dans les corps de fonctions |
| L1-L6 | Indentation 4 espaces, accolades, espaces operateurs, sauts de ligne |
| V1-V2 | `&` attache au nom ; `PassContext` et `RenderGraph` en `class` |
| C1-C5 | Boucles range-based conservees ; profondeur de nesting reduite via sous-fonctions |
| A1 | References `const` sur parametres non modifies |
| E6 | Retours separes erreur / succes dans `ReadTarget` |
| G2 | Ligne vide entre implementations de fonctions |
| K1 | Attributs membres `_suffix` deja coherents |
| D1-D6 | Responsabilites separees en methodes privees sans changer l'API |

## Items transverses (lots sequentiels)

| Regle | Detail | Lot cible |
|-------|--------|-----------|
| O2 | Extension `.h` -> `.hpp` | S1 |
| N2 | Fichier `render_graph.*` -> `RenderGraph.hpp` / dossier `Engine/Rhi/` | S2 |
| O3 | Trois types dans un header (`PassContext`, `PassDesc`, `RenderGraph`) | S4 |
| N3 | API publique PascalCase (`GetDevice`, `AddPass`, `Execute`, `Reset`, `ReadTarget`) | S3 |
| G8 (strict) | `PassDesc::execute` reste callback libre via `std::function` | S4 |
| A7 | Appel direct `TraceLog` (raylib) dans `executePass` | S4 (encapsulation Device) |

## Build

Cible impactee : `engine_rhi`, puis `engine_render`, `racer`, `render_demo` (transitive).

Verification : `clang++ -fsyntax-only` sur `render_graph.cpp` OK (build Ninja bloque par permission denied sur recompaction).
