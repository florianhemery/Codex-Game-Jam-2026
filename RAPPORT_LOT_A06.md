# RAPPORT_LOT_A06 - Core Jobs

**Lot :** A06  
**Fichiers :** `src/engine/core/jobs.h`, `src/engine/core/jobs.cpp`  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style core-jobs`

## Corrections appliquees (locale)

| Regle | Action |
|-------|--------|
| **G1** | En-tete Epitech standard (`racer`) sur `.h` et `.cpp` |
| **H2** | `#pragma once` remplace par garde `#ifndef JOBS_H_` |
| **F1** | `ParallelFor` decoupe : `ParallelForRunner` + `ParallelBatch` (cpp) |
| **F2** | Lignes > 80 colonnes reformatees (signatures, lambdas) |
| **F3** | `enqueueChunks` limite a 3 arguments via `ParallelForParams` |
| **F5** | Commentaires retires des corps de fonctions |
| **L3-L4** | Accolades Allman sur les definitions ; espacement references (`&fn`) |
| **V1** | `&` attache au nom (`Task &task`, `JobSystem &jobs`) |
| **K1** | Attributs `_suffix` conserves (`workers_`, `queueMutex_`, etc.) |
| **D5** | `WorkerCount() const` inchange |
| **G8 partiel** | Logique `ParallelFor` encapsulee dans `ParallelForRunner` (cpp) |
| **G2** | Ligne vide entre implementations de fonctions |

## Conformite deja satisfaite

- **L2** : pas de tabulations
- **A3** : pas de `NULL`
- **E1** : pas de `exit` / `abort` / `terminate`
- **F4** : pas de `(void)`
- **S2** : `std::make_shared` pour `packaged_task` (inchange)

## Items transverses (non corriges en Phase 1)

| Regle | Item | Lot sequentiel prevu |
|-------|------|----------------------|
| **O2** | Extension `.h` au lieu de `.hpp` | S1 |
| **N2** | Fichier `jobs.*` vs classe `JobSystem` | S2 (`JobSystem.hpp/.cpp`) |
| **N3** | API publique PascalCase : `Submit`, `ParallelFor`, `DefaultWorkerCount`, `WorkerCount`, `WorkerLoop`, `Enqueue`, `TryPop` | S3 |
| **K1** | Harmonisation globale convention attributs sur tout le projet | S3 |
| **G1** | Header Epitech sur les 5 `CMakeLists.txt` | S1 |

## Verification build

Cible impactee : `engine_core` (et `engine_core_test` via dependance).
