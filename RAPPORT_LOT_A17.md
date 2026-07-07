# RAPPORT_LOT_A17 - CarRenderer

**Lot :** A17  
**Fichiers :** `src/render/car_renderer.h`, `src/render/car_renderer.cpp`  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style car-renderer`

## Resume

Mise en conformite style Epitech (Phase 1) sur le module de rendu voiture.
Aucun changement fonctionnel. API publique `DrawCar`, `DrawCarEx`,
`GetCarLightPoints`, `CarVisual`, `CarLightPoints`, `kWheelRadius` conservee.

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| **G1** | Header Epitech (`racer`) ajoute en tete des deux fichiers |
| **H2** | `#pragma once` remplace par `#ifndef CAR_RENDERER_H_` / `#endif` |
| **F1** | `DrawCarEx` decoupee en ~25 sous-fonctions (corps <= 20 lignes) |
| **F2** | Toutes les lignes ramenees a 80 colonnes max |
| **F3** | `BodyPalette` introduit pour limiter les parametres |
| **F5** | Suppression de tous les commentaires dans les corps de fonctions |
| **L1-L6** | Indentation 4 espaces, accolades, declarations localisees |
| **V1-V2** | References `const &`, structs POD conserves pour `CarVisual` / `CarLightPoints` |
| **C1-C5** | Profondeur de branchement reduite via early return |
| **A1-A8** | `const` sur parametres non modifies, `std::clamp`, `static_cast` |
| **E1-E6** | Un seul `return` par fonction, pas d'exit/abort |
| **K1-K6** | Classe helper interne `CarRenderHelpers` (G8 partiel) |
| **S1-S3** | `std::clamp` (algorithm), pas de raw `delete` |
| **D1-D6** | Single responsibility via decoupe modulaire des helpers |

## Refactoring structurel (G8 partiel, sans changement d'API)

- Helpers anonymes migres vers `CarRenderHelpers` (methodes `static` internes,
  meme pattern que `CarPhysics` dans `car.cpp`).
- Pas de conversion en classe `CarRenderer` publique (reserve lot S4).

## Compilation

| Cible | Resultat |
|-------|----------|
| `car_demo` | OK (lien complet) |
| `car_renderer.cpp.obj` (via `racer`) | OK |
| `racer` (lien complet) | Echec preexistant sur `engine_assets` (lot A13 parallele, hors scope A17) |

## Items transverses reportes (lots sequentiels)

| Item | Regle | Lot cible | Detail |
|------|-------|-----------|--------|
| Extension `.hpp` | O2 | S1 | `car_renderer.h` -> `car_renderer.hpp` |
| Fichier PascalCase | N2 | S2 | `src/Render/CarRenderer.hpp/.cpp` |
| Renommage API camelCase | N3 | S3 | `DrawCar` -> `drawCar`, `DrawCarEx` -> `drawCarEx`, `GetCarLightPoints` -> `getCarLightPoints` |
| Scission multi-types | O3 | S4 | `CarVisual`, `CarLightPoints` dans fichiers dedies |
| Classe `CarRenderer` | G8 | S4 | Encapsulation complete des fonctions libres |
| Constante `kWheelRadius` | N4 | S3 | Renommage UPPER_CASE si harmonisation globale |
| Includes cross-lot | - | S1-S2 | `#include "vehicle/car.h"` inchange (hors lot) |

## Violations restantes acceptables (Phase 1)

| Regle | Justification |
|-------|---------------|
| **G8** (API publique) | Fonctions libres `DrawCar`/`DrawCarEx`/`GetCarLightPoints` conservees volontairement ; conversion S4 |
| **O3** | Deux structs + trois fonctions libres dans le meme header ; scission S4 |
| **A7** | Appels raylib/rlgl non encapsules (contrainte moteur existante) |
