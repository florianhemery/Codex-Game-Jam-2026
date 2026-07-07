# Rapport Lisibilite - Lot L04 Core

**Perimetre :** `src/Engine/Core/`  
**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite core`

## Objectif

Decouper `EngineCoreTest.cpp` (317 lignes) en `CoreTestRunner.hpp/cpp` et fichiers helpers, verifier F1 sur `JobSystem.cpp`, sans modifier l'API publique ni sortir du module.

## Decoupage EngineCoreTest

| Fichier | Lignes | Role |
|---------|--------|------|
| `EngineCoreTest.cpp` | 31 | Point d'entree `main` uniquement |
| `CoreTestRunner.hpp` | 61 | Declaration `CoreTestRunner`, `K_ENTITY_COUNT` |
| `CoreTestRunner.cpp` | 19 | `check()`, `failures()` |
| `CoreTestWorldHelpers.cpp` | 87 | Population World, tags, lifecycle |
| `CoreTestJobHelpers.cpp` | 65 | `verifyJobSystem`, `parallelFor` |
| `CoreTestSnapshotHelpers.cpp` | 125 | Capture snapshot, verification render items |

### Sous-fonctions extraites (F1 test)

| Fonction initiale | Lignes avant | Extraction | Lignes apres |
|-------------------|--------------|------------|--------------|
| `addEntity` | 22 | `addEntityComponents` | 8 |
| `verifyRenderItemFields` | 21 | `verifyRenderItemPosition`, `verifyRenderItemOrientation` | 9:9:9 |
| `verifySecondFrame` | 28 | `findUpdatedMesh` | 19 |

## JobSystem F1

| Fonction initiale | Lignes avant | Extraction | Lignes apres |
|-------------------|--------------|------------|--------------|
| `enqueueChunks` | 26 | `enqueueOneChunk` | 14 |
| `participateUntilDone` | 22 | `isBatchDone`, `waitForBatch` | 15 |
| `run` | 21 | `rethrowBatchError` | 19 |

Aucun changement d'API publique (`JobSystem.hpp` inchange).

## Components hpp

Aucune action requise — tous <= 48 lignes (limite 100) :

| Fichier | Lignes |
|---------|--------|
| `AiTag.hpp` | 12 |
| `PlayerTag.hpp` | 12 |
| `NameComponent.hpp` | 15 |
| `KinematicsComponent.hpp` | 16 |
| `LapProgressComponent.hpp` | 18 |
| `TransformComponent.hpp` | 18 |
| `RenderMeshComponent.hpp` | 18 |

## Metriques post-refactor

| Fichier | Lignes | Limite | Statut |
|---------|--------|--------|--------|
| `EngineCoreTest.cpp` | 31 | 300 | OK (etait 317) |
| `CoreTestSnapshotHelpers.cpp` | 125 | 300 | OK |
| `JobSystem.cpp` | 249 | 300 | OK |
| `CoreTestRunner.hpp` | 61 | 100 | OK |

Aucun `.cpp` du module ne depasse 300 lignes. Toutes les fonctions respectent F1 (<= 20 lignes).

## Perimetre respecte

- Aucun fichier hors `src/Engine/Core/` modifie (sauf rapport racine).
- API publique inchangee (`World`, `JobSystem`, `SnapshotBuffer`, components).
- Comportement test preserve : memes assertions, meme sortie `OK`.

## CMakeLists.txt

`engine_core_test` etendu avec les 4 nouveaux `.cpp` helpers + `CoreTestRunner.cpp`.

## Verification build

Cibles : `engine_core`, `engine_core_test`.

```text
cmake --build build --target engine_core engine_core_test
build/engine_core_test.exe
```

**Resultat :** compilation et liaison OK. Test headless : sortie `OK`, code de sortie 0.
