# Rapport Lot A14 - Assets Shaders

**Fichiers :** `src/engine/assets/shader_watcher.h`, `src/engine/assets/shader_watcher.cpp`  
**Cible impactee :** `engine_assets` (consommee par `engine_render` / `racer`)  
**Date :** 2026-07-07

## Corrections appliquees

| Regle | Action |
|-------|--------|
| G1 | Header Epitech ajoute sur `.h` et `.cpp` |
| H2 | `#pragma once` remplace par `#ifndef SHADER_WATCHER_H_` / `#define` / `#endif` |
| F1 | `RegisterShader` decoupe (`initSlotMtimes`, `loadSlotInitial`) ; `Poll` allege via `checkPathChanged` |
| F2 | Toutes les lignes ramenees a 80 colonnes max (TraceLog, typedef, signatures) |
| F5 | Suppression de tous les commentaires dans les corps de fonctions et en-tetes de classe |
| L1-L6 | Accolades fonctions sur ligne separee, espacement operateurs, `&` attache au nom, une instruction par ligne |
| V1-V2 | References `const std::string &`, classes pour les entites avec comportement |
| C1-C5 | Profondeur de nesting reduite dans `Poll` ; range-based `for` conserve |
| A1-A8 | `const` sur parametres non modifies, `nullptr` via `pathOrNull`, pas de cast C dangereux |
| E1-E6 | Pas d'`exit`/`abort` ; chemins d'erreur explicites dans les helpers |
| K1-K6 | Attributs `_suffix` inchanges ; `using ReloadCallback` conserve ; ordre public/private respecte |
| S1-S3 | `std::unique_ptr` dans la map, `std::unordered_map` adapte, `std::function` pour callback |
| D1-D6 | RAII dans destructeur, attributs prives, methodes const sur accesseurs |
| G8 (partiel) | Helpers anonymes migres vers `ShaderLoadUtils` (statiques fichier) et methodes privees `ShaderWatcher` |

## Verification compilation

- `clang++ -fsyntax-only` sur `shader_watcher.cpp` : **OK** (warnings `-Wall -Wextra` : aucun)
- Rebuild Ninja `engine_assets` : non execute (echec CMake regenerate / permission denied sur l'environnement agent) ; syntaxe validee manuellement

## Items transverses (lots sequentiels S1-S4)

| Item | Regle | Detail | Lot cible |
|------|-------|--------|-----------|
| Extension `.h` -> `.hpp` | O2 | `shader_watcher.h` | S1 |
| Fichier/dossier PascalCase | N2 | `ShaderWatcher.hpp`, `Engine/Assets/` | S2 |
| API publique PascalCase | N3 | `RegisterShader`, `Get`, `Poll`, `Find`, `SetOnReload`, `UnloadAll`, etc. | S3 |
| Scission multi-classes | O3 | `ShaderSlot` + `ShaderWatcher` dans un seul header | S4 |
| `friend class ShaderWatcher` | K4 | Necessaire pour l'encapsulation actuelle ; alternative = accesseurs internes | S4 |
| Appels raylib/rlgl directs | A7 | `LoadShader`, `TraceLog`, `MemFree`, `rlGetShaderIdDefault` | Acceptable (wrappers existants projet) |

## Changement fonctionnel

Aucun. Comportement identique : enregistrement, poll mtime, hot-reload securise, callback, dechargement GPU.
