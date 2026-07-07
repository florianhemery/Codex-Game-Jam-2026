# Rapport Lot A10 - RHI Types

**Fichiers traites :** `src/engine/rhi/rhi_types.h`  
**Date :** 2026-07-07  
**Commit attendu :** `style: conformite coding style rhi-types`

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| G1 | Header Epitech standard (projet `racer`) |
| H2 | `#pragma once` remplace par garde `RHI_TYPES_H_` |
| F1-F4 | Fonctions courtes (<= 20 lignes), <= 5 arguments, pas de `void` |
| F2 | Toutes les lignes <= 80 colonnes |
| F5 | Commentaires supprimes dans les corps de fonctions et doc Doxygen retiree |
| L1-L6 | Indentation 4 espaces, accolades ouvrantes sur ligne suivante (fonctions) |
| V1 | References `&` attachees au nom de variable (`const RenderTargetDesc &desc`) |
| V2 | `RhiHandle` et `RenderTargetDescHash` passes en `class` (comportement interne) |
| C1-C5 | Pas de structures de controle complexes a modifier |
| A1-A8 | `const` / `noexcept` / `constexpr` conserves, `nullptr` non applicable |
| E1-E6 | Un seul `return` par fonction, pas d'`exit`/`abort` |
| K1-K6 | Attributs publics inchanges (convention suffixe reportee S3) |
| S1-S3 | `std::hash`, `std::function` via `<functional>` inchange |
| D1-D6 | POD et hasheur separes, methodes `const` sur accesseurs |

## Violations corrigees

- Absence de header G1
- `#pragma once` au lieu de garde H2
- Commentaires Doxygen (`///`) et inline sur enum/champs
- `struct` avec logique interne (`RhiHandle`, `RenderTargetDescHash`)
- Accolades de fonctions non conformes L4 (ligne suivante)
- Format references V1 sur parametres

## Items transverses (lots sequentiels)

| Item | Regle | Detail | Lot cible |
|------|-------|--------|-----------|
| Extension `.h` | O2 | `rhi_types.h` -> `rhi_types.hpp` puis PascalCase | S1 / S2 |
| Nom de fichier | N2 | `rhi_types.h` -> `RhiTypes.hpp` | S2 |
| Chemin dossier | N2/O1 | `src/engine/rhi/` -> `src/Engine/Rhi/` | S2 |
| API publique PascalCase | N3 | `IsValid`, `RenderTargetDesc`, `RhiFormat`, etc. -> camelCase | S3 |
| Valeurs enum | N5 | `RGBA8`, `RGBA16F`, `DEPTH24` deja UPPER_CASE | - |
| Convention attributs | K1 | `id`, `width`, `height`, `format`, `useDepth` sans suffixe `_` | S3 |
| Attributs publics | D2 | `RenderTargetDesc` et `RhiHandle::id` exposes directement | S4 |
| Include guard | H2 | `RHI_TYPES_H_` -> `RHITYPES_HPP_` apres renommage extension | S1 |
| Header G1 CMake | G1 | `CMakeLists.txt` modules sans header Epitech | S1 |

## Verification build

Cible impactee : `engine_rhi` (consommateurs : `device.cpp`, `render_graph.cpp`, `render_pipeline`).
