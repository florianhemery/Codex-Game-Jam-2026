# RFC — ChunkData & streaming Aurélia

## Statut

Accepté — Sprint 0.

## ChunkId

```cpp
struct ChunkId { int x; int z; };
```

Grille monde : chunks de **128 m** (`kChunkSize`). Origine chunk `(0,0)` = Marina Veloce.

## ChunkData

| Champ | Type | Description |
|-------|------|-------------|
| `id` | ChunkId | Coordonnées grille |
| `biome` | BiomeId | coast / forest / port / volcano |
| `heightmap` | float[33×33] | Hauteur terrain (m) — `kChunkResolution` |
| `splat` | u8[33×33] | Surface : asphalt, grass, sand, rock, gravel |
| `pois` | PoiInstance[] | POI locaux (coords chunk) |
| `props` | PropInstance[] | Décor instancié (type, pos, scale, yaw) |

Génération procédurale via `ChunkGenerator` ; future sérialisation JSON binaire.

## ChunkStreamer

- Anneau **3×3** centré sur le chunk du joueur.
- Chargement synchrone MVP ; interface `requestLoad(ChunkId)` prête pour async IO.
- `sampleHeight(worldX, worldZ)` bilinéaire sur chunks chargés.
- `sampleSurface(worldX, worldZ)` → drag véhicule.

## WorldRenderer

- Passe opaque : mesh terrain par chunk (cache `Mesh` raylib).
- Passe lit : eau (côte), props LOD (distance < 200 m full, sinon impostor cube).
- POI triggers : anneaux colorés (réutilise logique visuelle hub).

## RoadGraph

Graphe statique `AureliaData::roadGraph()` : nœuds `(worldX, worldZ)`, arêtes avec
largeur et limite vitesse. Alimente `TrafficSystem`.

## Feature flag

CMake `USE_AURELIA` (ON par défaut). Legacy `OpenWorldHub` retiré en M6.

## Tests

`world_chunk_test` : charge 9 chunks autour de `(0,0)`, vérifie continuité bords,
sample hauteur centre Marina > 0.
