# Rapport Lot A09 - Core Test

**Lot :** A09  
**Module :** core-test  
**Date :** 2026-07-07  
**Fichiers exclusifs :** `src/engine/core/engine_core_test.cpp`

## Resume

Mise en conformite locale du test headless du module core (World, JobSystem,
SnapshotBuffer). Aucun changement fonctionnel intentionnel.

## Violations corrigees

| Regle | Correction |
|-------|------------|
| G1 | Header Epitech standard (`racer`) en tete de fichier |
| G4 | Suppression de `g_failures` global ; compteur `failures_` membre de `CoreTestRunner` |
| G8 | Helpers encapsules dans la classe locale `CoreTestRunner` (seul `main` reste libre) |
| A9 | Suppression de `using namespace racer::engine` ; qualification explicite |
| F1 | Decoupage de `main` (~130 lignes) en methodes <= 20 lignes |
| F2 | Toutes les lignes <= 80 colonnes |
| F5 | Suppression des commentaires dans les corps de fonctions |
| L4 | `else` sur ligne separee |
| N3/N4 | Methodes locales en camelCase ; constante `K_ENTITY_COUNT` en UPPER_CASE |
| K1 | Attribut membre `failures_` avec suffixe underscore |

## Verification compilation

- `clang++ -fsyntax-only` sur `engine_core_test.cpp` : **OK** (warnings EnTT tiers).
- `cmake --build build --target engine_core_test` : **echec** sur `jobs.cpp` (lot A06,
  erreur `Task` hors portee, modification parallele hors lot A09).

## Items transverses (non traites, lots sequentiels)

| Item | Regle | Action requise |
|------|-------|----------------|
| `engine_core_test.cpp` snake_case | N2/O1 | Lot S2 : `Engine/Core/EngineCoreTest.cpp` ou equivalent |
| Includes `.h` | O2 | Lot S1 : extension `.hpp` |
| API publique PascalCase (`CreateEntity`, `CaptureSnapshot`, `ParallelFor`, etc.) | N3 | Lot S3 : camelCase transverse |
| `components.h`, `frame_snapshot.h`, `jobs.h`, `world.h` non conformes H2/G1 | H2/G1 | Lots A05-A08 / S1 |
| Echec build `jobs.cpp` | - | Corriger lot A06 avant rebuild complet Phase 1 |

## Commit

`style: conformite coding style core-test`
