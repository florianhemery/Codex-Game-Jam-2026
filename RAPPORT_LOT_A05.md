# Rapport Lot A05 - Core World

**Fichiers traites :** `src/engine/core/world.h`, `src/engine/core/world.cpp`  
**Date :** 2026-07-07  
**Commit attendu :** `style: conformite coding style core-world`

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| G1 | Header Epitech (`racer`) ajoute aux deux fichiers |
| H2 | `#pragma once` remplace par garde `WORLD_H_` |
| F1-F4 | Fonctions courtes, < 80 colonnes, <= 5 arguments, pas de `void` |
| F5 | Aucun commentaire dans les corps de fonctions |
| L1-L6 | Indentation 4 espaces, accolades ouvrantes sur ligne suivante (fonctions), espacement L3 |
| V1 | References `&` attachees au type variable (`entt::registry &Registry()`) |
| V2 | `World` reste une `class` (comportement) |
| C1-C5 | Pas de structures de controle complexes a modifier |
| A1-A8 | `const` sur surcharge `Get` / `Registry`, `std::forward` conserve |
| E1-E6 | Un seul `return` par fonction, pas d'`exit`/`abort` |
| K1-K6 | Attribut `registry_` (suffixe underscore), sections public/private ordonnees |
| S1-S3 | Usage STL/entt inchange |
| D1-D6 | Encapsulation preservee, attribut prive, getters const |

## Violations corrigees

- Absence de header G1 (2 fichiers)
- `#pragma once` au lieu de garde H2
- Commentaires descriptifs hors G1 retires (F5 / lisibilite)
- Accolades de fonctions conformes L4 (ligne suivante)
- References formatees selon V1 sur les getters `Registry()`

## Items transverses (lots sequentiels)

| Item | Regle | Detail | Lot cible |
|------|-------|--------|-----------|
| Extension `.h` | O2 | `world.h` -> `world.hpp` puis PascalCase | S1 / S2 |
| Nom de fichier | N2 | `world.*` -> `World.hpp` / `World.cpp` | S2 |
| Chemin dossier | N2/O1 | `src/engine/core/` -> `src/Engine/Core/` | S2 |
| API publique PascalCase | N3 | `CreateEntity`, `DestroyEntity`, `Registry`, `Add`, `Get`, `Has` -> camelCase | S3 |
| Include guard | H2 | `WORLD_H_` -> `WORLD_HPP_` apres renommage extension | S1 |
| Header G1 CMake | G1 | `CMakeLists.txt` racine et modules sans header Epitech | S1 |

## Verification build

Cible impactee : `engine_core` (bibliotheque contenant `world.cpp`).  
**Resultat :** compilation et liaison OK (`cmake --build build --target engine_core`).
