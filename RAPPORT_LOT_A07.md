# RAPPORT_LOT_A07 - Core Snapshot

**Lot :** A07  
**Module :** core-snapshot  
**Fichiers exclusifs :** `src/engine/core/frame_snapshot.h`, `src/engine/core/frame_snapshot.cpp`  
**Date :** 2026-07-07

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| **G1** | En-tete Epitech (`EPITECH PROJECT, 2026`, projet `racer`) sur `.h` et `.cpp` |
| **H2** | `#pragma once` remplace par garde `FRAME_SNAPSHOT_H_` |
| **G3** | Directives preprocesseur indentees dans le header |
| **G2** | Ligne vide unique entre chaque implementation |
| **F2** | Lignes > 80 colonnes decoupees (`view`, lambda `each`) |
| **F5** | Commentaires supprimes dans les corps de fonctions |
| **L4** | Accolades ouvrantes des fonctions sur ligne separee |
| **L3** | `# include` avec espace apres `#` dans le header |
| **K1** | Attributs `buffers_`, `writeIndex_`, `mutex_` deja conformes |
| **D5** | `ReadLatest()` deja `const` |
| **V2** | `RenderItem` et `FrameSnapshot` restent des `struct` POD |

Aucun changement fonctionnel. API publique inchangee (`CaptureSnapshot` conserve).

## Items transverses (lots sequentiels)

| Regle | Element | Lot cible |
|-------|---------|-----------|
| **O2** | Extension `.h` au lieu de `.hpp` | S1 |
| **N2** | Fichier `frame_snapshot.*` au lieu de `SnapshotBuffer.hpp` (PascalCase) | S2 |
| **O3** | Trois types dans un fichier (`RenderItem`, `FrameSnapshot`, `SnapshotBuffer`) | S4 |
| **G8** | Fonction libre `CaptureSnapshot` hors classe | S4 (`SnapshotBuffer::capture`) |
| **N3** | `CaptureSnapshot` en PascalCase au lieu de `capture` | S3 |
| **A7** | `#include "raylib.h"` direct pour `Vector3` / `Color` | Encapsulation RHI (hors lot) |

## Verification build

Cible impactee : `engine_core` (et `engine_core_test` via dependance).

```text
cmake --build build --target engine_core
```
