# Rapport Lot A16 - Render-Demo (render_demo.cpp)

## Fichiers modifies

- `src/engine/render/render_demo.cpp`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| G1 | En-tete Epitech standard (projet `racer`) |
| F1 | `main` et `drawCasters` decoupes en methodes `RenderDemoScene` (<= 20 lignes) |
| F2/F3 | Lignes > 80 colonnes reformatees |
| F5 | Commentaires supprimes dans les corps de fonctions |
| G8 (partiel) | Helpers namespace anonyme -> `RenderDemoScene` methodes `static` |
| G5 | Helpers fichier-local encapsules dans `RenderDemoScene` |
| K1 | Champs `DemoModels` suffixes (`ground_`, `sphere_`, `cylinder_`) |
| V1 | Fonctions locales en `snake_case` |
| L1-L6 | Indentation 4 espaces, accolades, espaces operateurs |
| A1 | References `const` sur parametres non modifies |
| C1 | Branches `if` avec accolades |
| E6 | Un seul `return` par fonction helper |
| G2 | Ligne vide entre implementations de fonctions |
| S1-S3 | Struct `DemoModels` coherente, initialisation explicite |
| D1-D6 | Doc `\file` remplacee par description G1 |

## Items transverses (lots sequentiels)

| Regle | Detail | Lot cible |
|-------|--------|-----------|
| N2 | Fichier `render_demo.cpp` / dossier `engine/render` -> PascalCase | S2 |
| N3 | API `RenderPipeline` PascalCase (`SetAmbiance`, `PollShaderReload`, `Frame`, etc.) | S3 |
| N5 | Valeurs enum `Ambiance::Midi`, `AubeDoree`, `Crepuscule`, `Orage` | S3 |
| O2 | Include `render_pipeline.h` -> `.hpp` | S1 |
| A7 | Appels raylib/rlgl C non encapsules (`DrawCube`, `LoadModelFromMesh`, etc.) | info |
| G8 (strict) | `RenderDemoScene` reste classe interne `.cpp` | S4 |

## Build

Cible impactee : `render_demo`.

`render_demo.cpp` compile seul (obj genere). Liaison complete bloquee par erreurs
preexistantes dans `render_pipeline.cpp` (lot A15 en cours).
