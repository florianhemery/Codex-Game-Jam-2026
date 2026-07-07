# Rapport Lot L02 - Lisibilite RHI

**Module :** `src/Engine/Rhi/`  
**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite rhi`

## Cibles appliquees

| Regle | Cible | Avant | Apres |
|-------|-------|-------|-------|
| F1 | Fonctions <= 20 lignes | 2 violations | 0 |
| Taille `.cpp` | <= 300 lignes | 1 fichier a 287L (limite) | 0 hors limite (max 211L) |
| Taille `.hpp` | <= 100 lignes | 0 hors limite | 0 hors limite (max 84L) |
| Constantes | `constexpr` pour nombres magiques | 4 occurrences | 0 |
| F5 | Aucun commentaire dans les fonctions | OK | OK |
| Noms | Identifiants explicites | `pool_`, `usedThisFrame_`, `toRlColorFormat` | Renommes |

## Violations corrigees

### Fonctions > 20 lignes

| Fichier | Fonction | Avant | Action |
|---------|----------|-------|--------|
| `Device.cpp` | `createDepthTarget` | 21L | Extraction de `setupDepthFramebuffer` |
| `RenderGraph.cpp` | `executePass` | 24L | Extraction de `resolvePassOutput` et `runPassCallback` |

### Nombres magiques -> `constexpr`

| Valeur | Nouveau symbole | Fichier |
|--------|-----------------|---------|
| `19` (format profondeur raylib) | `rhi_constants::kDepthTexturePixelFormat` | `RhiConstants.hpp` |
| `1` (mipmaps) | `rhi_constants::kSingleMipmapLevel` | `RhiConstants.hpp` |
| `0x9e3779b9u` (hash combine) | `rhi_constants::kHashCombineSeed` | `RhiConstants.hpp` |
| `0` (ressource invalide) | `rhi_constants::kInvalidHandleId` | `RhiConstants.hpp` |

### Decoupe fichiers

`Device.cpp` (287L) depassait la limite apres refactor interne. Decoupe en :

| Fichier | Lignes | Role |
|---------|--------|------|
| `Device.cpp` | 117 | Gestion handles, shaders, cycle de vie |
| `DeviceFramebuffer.cpp` | 211 | Assemblage framebuffer raylib/rlgl |
| `DeviceFramebuffer.hpp` | 51 | Helpers internes (`detail::DeviceFramebuffer`) |
| `RhiConstants.hpp` | 23 | Constantes nommees du module |

`CMakeLists.txt` mis a jour : ajout de `DeviceFramebuffer.cpp`.

## Renommages internes (API publique inchangee)

| Avant | Apres | Portee |
|-------|-------|--------|
| `toRlColorFormat` | `toRaylibColorFormat` | `detail::DeviceFramebuffer` |
| `pool_` | `transientTargetPool_` | `RenderGraph` (prive) |
| `usedThisFrame_` | `borrowedThisFrame` | `RenderGraph::PoolBucket` |

## API publique

Aucun changement cross-module : signatures `Device`, `RenderGraph`, `PassContext`, `RhiTypes` conservees.

## Fichiers modifies

- `Device.hpp`, `Device.cpp`
- `DeviceFramebuffer.hpp`, `DeviceFramebuffer.cpp` (nouveaux)
- `RhiConstants.hpp` (nouveau)
- `RhiTypes.hpp`
- `RenderGraph.hpp`, `RenderGraph.cpp`
- `CMakeLists.txt`

## Verification build

Cible impactee : `engine_rhi`.  
Reconfiguration CMake bloquee par fichiers manquants hors module (`src/Render/TrackRenderer.cpp`, etat repo transitoire). Aucune modification hors `src/Engine/Rhi/`.

## Metriques finales

```
Device.cpp:            117 lignes
Device.hpp:             56 lignes
DeviceFramebuffer.cpp: 211 lignes
DeviceFramebuffer.hpp:  51 lignes
RenderGraph.cpp:       124 lignes
RenderGraph.hpp:        63 lignes
RhiConstants.hpp:       23 lignes
RhiTypes.hpp:           84 lignes
PassContext.hpp:        63 lignes
Violations F1 / taille: aucune
```
