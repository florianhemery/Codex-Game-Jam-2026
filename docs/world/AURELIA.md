# Aurélia — Architecture monde ouvert

## Modules

| Chemin | Rôle |
|--------|------|
| `src/World/Aurelia/` | Données, POI, boucle monde |
| `src/World/Chunk/` | ChunkData, génération procédurale |
| `src/World/Stream/` | ChunkStreamer 3×3 |
| `src/World/Road/` | RoadGraph trafic |
| `src/World/Sim/` | Joueur, trafic, missions, progression |
| `src/Render/World/` | WorldRenderer |
| `docs/world/` | Bible, RFC, QA |

## Flux frame (OPEN_WORLD)

```
GameLoopWorld → AureliaWorld::update
             → ChunkStreamer::ensureLoaded
             → WorldRenderer::sync
             → RenderPipeline (Opaque/Lit/Unlit)
```

## Remplacement hub legacy

`OpenWorldHub` supprimé — `AureliaWorld` via `USE_AURELIA` (ON par défaut).

## Tests

- `world_chunk_test` — chargement 9 chunks, continuité hauteur
- `world_editor` — visualisation heightmap + splat
