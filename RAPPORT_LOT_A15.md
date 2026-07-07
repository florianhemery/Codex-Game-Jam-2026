# RAPPORT_LOT_A15 - Render Pipeline

## Fichiers modifies

- `src/engine/render/render_pipeline.h`
- `src/engine/render/render_pipeline.cpp`

## Regles appliquees (localement)

| Regle | Action |
|---|---|
| G1 | Header Epitech `racer` en tete des deux fichiers |
| H2 | `#pragma once` remplace par `#ifndef RENDER_PIPELINE_H_` |
| F1 | Decoupe de `Frame`, `RefreshLocations`, `ParamsFor` en sous-fonctions privees |
| F2 | Toutes les lignes <= 80 colonnes |
| F3 | `Frame` conserve 5 arguments (limite respectee) |
| F5 | Suppression de tous les commentaires dans les corps de fonctions |
| L1-L6 | Indentation 4 espaces, accolades fonctions sur ligne separee, une instruction par ligne |
| V1 | Pointeurs `Type *ptr`, references `Type &ref` |
| V2 | `AmbianceParams` et `PostParams` restent des struct POD |
| C1 | Profondeur de nesting <= 2, pas de branches excessives |
| C5 | Range-based for dans `resolveShaderDir` |
| A1 | Parametres et references const ou `const &` selon usage |
| A3 | `nullptr` conserve |
| E6 | Retours uniques sur les chemins nominaux |
| K1 | Attributs membres avec suffixe `_` (deja conforme) |
| K3 | Ordre public / private respecte |
| D2 | Attributs prives, API publique via methodes |
| D5 | Methodes non mutatrices marquees `const` |
| D6 | RAII dans constructeur / destructeur |
| G8 partiel | `LocOrArray` et `DefaultShader` (namespace anonyme) -> methodes `private static` |

## Compilation

- `render_pipeline.cpp` compile sans erreur (`engine_render`, 2026-07-07).
- Build complet `racer` bloque par erreurs preexistantes dans `asset_registry.cpp` (lot A13, acces membres prives).

## Items transverses (non traites, lot sequentiel)

| Item | Regle | Detail |
|---|---|---|
| Extension `.h` -> `.hpp` | O2 | Fichiers encore en `.h` |
| Fichier `RenderPipeline.hpp` PascalCase | N2 | `render_pipeline.h` -> `Engine/Render/RenderPipeline.hpp` |
| API publique camelCase | N3 | `SetAmbiance`, `ParamsFor`, `LitShader`, `Frame`, etc. |
| Valeurs enum UPPER_CASE | N5 | `Ambiance::Midi`, `AubeDoree`, `Crepuscule`, `Orage` |
| Scission `PostParams` / `AmbianceParams` | O3 | Types supplementaires dans le meme header |
| Appels raylib non encapsules | A7 | `GetShaderLocation`, `DrawModel`, `rlgl`, etc. |
| CMakeLists header G1 | G1 | `src/engine/render/CMakeLists.txt` hors lot |

## Changement fonctionnel

Aucun. Refactoring structurel et formatage uniquement.
