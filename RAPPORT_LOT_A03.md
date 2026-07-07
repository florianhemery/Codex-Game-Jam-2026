# RAPPORT_LOT_A03 - Module AI (ai_driver)

**Lot :** A03  
**Fichiers :** `src/ai/ai_driver.h`, `src/ai/ai_driver.cpp`  
**Date :** 2026-07-07

## Corrections appliquees

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech (`racer`) ajoute aux deux fichiers |
| H2 | `#pragma once` remplace par garde `#ifndef AI_DRIVER_H_` |
| G2 | Ligne vide unique entre chaque implementation |
| G5/G8 | Helpers `NormalizeAngle`, `HashU32` et constantes deplaces en methodes `static` / membres `constexpr` prives de `AIDriver` |
| F1 | `ComputeInput` decoupe en 8 sous-fonctions privees (<= 20 lignes chacune) |
| F2 | Toutes les lignes <= 80 colonnes |
| F5 | Commentaires supprimes dans les corps de fonctions |
| L4 | Accolade ouvrante des fonctions sur ligne separee |
| V1 | `*` et `&` attaches au nom de variable (`const Car &car`) |
| K1 | Attributs `skill_`, `laneOffset_`, `nitroReserve_` conserves (suffixe `_`) |
| K3 | Ordre public puis private respecte |
| D1/D5 | Responsabilites separees ; methodes d'aide marquees `const` |
| E6 | Un seul `return` par fonction non-void |

## Violations transverses reportees (lots sequentiels)

| Regle | Element | Lot cible |
|-------|---------|-----------|
| O2 | Extension `.h` au lieu de `.hpp` | S1 |
| N2 | Fichiers `ai_driver.*` / dossier `ai/` en snake_case au lieu de PascalCase (`Ai/AiDriver`) | S2 |
| N3 | API publique `AIDriver`, `ComputeInput` en PascalCase au lieu de camelCase | S3 |
| G8 | Classe `AIDriver` conforme ; duplication de `normalizeAngle` avec `car.cpp` et `race_state.cpp` | S4 |
| N5 | Types `CarInput`, `Track::Progress` hors lot (PascalCase enums/structs externes) | S3 |

## Build

Compilation via `cmake --build build` apres modification des fichiers du lot.

## Commit

`style: conformite coding style ai`
