# Rapport Lot A13 - Assets Registry

**Fichiers traites :** `src/engine/assets/asset_registry.h`, `src/engine/assets/asset_registry.cpp`  
**Date :** 2026-07-07  
**Commit attendu :** `style: conformite coding style assets-registry`

## Regles appliquees localement

| Regle | Action |
|-------|--------|
| G1 | Header Epitech (`racer`) ajoute aux deux fichiers |
| H2 | `#pragma once` remplace par garde `ASSET_REGISTRY_H_` |
| F1 | `LoadModelAsset`, `LoadTextureAsset`, `UnloadUnused` decoupes (<= 20 lignes) |
| F2 | Lignes `TraceLog` et affectations longues reformatees (< 80 colonnes) |
| F5 | Commentaires supprimes dans les corps de fonctions |
| G8 (partiel) | Helpers namespace anonyme -> `AssetRegistryDetail` (methodes static) |
| G5 | Helpers fichier-local encapsules dans `AssetRegistryDetail` |
| V1 | References `&` attachees au nom (`Model &Get()`, `const std::string &path`) |
| V2 | `PbrMaterialInfo` reste un `struct` POD ; classes avec comportement |
| L1-L6 | Indentation 4 espaces, accolades ouvrantes sur ligne suivante (fonctions) |
| A1 | Parametres `const &` sur les entrees non modifiees |
| E6 | Un seul `return` par fonction (`LoadModelAsset`, `LoadTextureAsset`) |
| G2 | Ligne vide entre implementations de fonctions |
| K1-K6 | Attributs `_*` conserves, sections public/private ordonnees |
| S1-S3 | `unique_ptr`, `unordered_map`, `vector` inchanges |
| D1-D6 | Encapsulation preservee, attributs prives, RAII dans destructeur |

## Violations corrigees

- Absence de header G1 (2 fichiers)
- `#pragma once` au lieu de garde H2
- Commentaires dans les corps de fonctions (F5)
- Fonctions libres hors classe dans namespace anonyme (G8 partiel)
- Fonctions `LoadModelAsset` / `LoadTextureAsset` > 20 lignes (F1)
- Lignes > 80 colonnes sur `TraceLog` et champs PBR (F2)
- Format references V1 sur getters et parametres

## Items transverses (lots sequentiels)

| Item | Regle | Detail | Lot cible |
|------|-------|--------|-----------|
| Extension `.h` | O2 | `asset_registry.h` -> `.hpp` | S1 |
| Nom de fichier | N2 | `asset_registry.*` -> `AssetRegistry.hpp` / `AssetRegistry.cpp` | S2 |
| Chemin dossier | N2 | `src/engine/assets/` -> `src/Engine/Assets/` | S2 |
| Multi-types par header | O3 | `PbrMaterialInfo`, `ModelAsset`, `TextureAsset`, `AssetRegistry` | S4 |
| API publique PascalCase | N3 | `LoadModelAsset`, `Get`, `Acquire`, `PbrInfos`, etc. | S3 |
| `friend class AssetRegistry` | K4 | Necessaire pour remplir attributs prives des assets | S4 (revue) |
| Appels raylib directs | A7 | `LoadModel`, `TraceLog`, etc. non encapsules | Acceptable (lot A7) |
| Header G1 CMake | G1 | `CMakeLists.txt` modules sans header Epitech | S1 |

## Verification build

Cible impactee : `engine_assets` (bibliotheque contenant `asset_registry.cpp`).  
Build non execute : `cmake` absent du PATH sur cette machine.
