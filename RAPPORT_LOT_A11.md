# Rapport Lot A11 - RHI Device

**Fichiers traites :** `src/engine/rhi/device.h`, `src/engine/rhi/device.cpp`  
**Cible impactee :** `engine_rhi`  
**Date :** 2026-07-07

## Corrections appliquees

| Regle | Action |
|-------|--------|
| **G1** | Header Epitech (`racer`) en tete des deux fichiers |
| **H2** | `#pragma once` remplace par `#ifndef DEVICE_H_` / `#define` / `#endif` |
| **G8** (partiel) | Helpers `namespace {}` convertis en methodes `private static` de `Device` |
| **G5** | Helpers file-scope absorbes dans la classe (plus de fonctions libres) |
| **F1** | Decoupage de `CreateColorTarget` en sous-fonctions (`openFramebuffer`, `bindColorAttachments`, etc.) |
| **F2** | Toutes les lignes <= 80 colonnes (TraceLog et signatures wraps) |
| **F5** | Suppression de tous les commentaires dans les corps de fonctions |
| **L2-L4** | Indentation 4 espaces, accolades ouvrantes des fonctions sur ligne separee |
| **L3** | Espaces autour des operateurs et references (`const RenderTargetDesc &desc`) |
| **V1** | `*` et `&` attaches au type variable (`const Shader *`, `int &rlFormat`) |
| **V2** | `RenderTargetEntry` conserve en struct POD interne |
| **C1/C5** | Branches simplifiees, `range-based for` dans le destructeur |
| **A1** | Parametres `const &` preserves sur les descripteurs |
| **E6** | Retours explicites (`if/return`) a la place des ternaires dans les getters |
| **K1** | Convention `_suffix` maintenue sur les attributs membres |
| **K3** | Ordre public / private respecte |
| **D2/D5/D6** | Encapsulation, const sur getters, RAII inchange |
| **S1** | `std::unordered_map` conserve (adapté au registre par id) |

## Verification compilation

- `clang++ -fsyntax-only` sur `device.cpp` : **OK** (flags issus de `compile_commands.json`)
- `cmake --build build --target engine_rhi` : **echec environnement** (`ninja: failed recompaction: Permission denied`, verrou build dir)

## Items transverses (non traites, lots sequentiels)

| Regle | Item | Lot prevu |
|-------|------|-----------|
| **O2** | Extension `.h` -> `.hpp` (`device.h`) | S1 |
| **N2** | Fichier `device.*` -> `Device.hpp/.cpp` (PascalCase) | S2 |
| **N3** | API publique PascalCase (`CreateRenderTarget`, `GetShader`, etc.) -> camelCase | S3 |
| **N5** | Valeurs enum `RhiFormat` dans `rhi_types.h` (hors lot) | S3 |
| **G1** | Header Epitech sur les 5 `CMakeLists.txt` | S1 |
| **A7** | Appels raylib/rlgl (encapsulation partielle via `Device`, pont C inevitable) | Acceptable |

## Changement fonctionnel

Aucun. Comportement identique : creation/destruction render targets et shaders, fallback RGBA16F -> RGBA32F, gestion profondeur, Begin/End render target.
