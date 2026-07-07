# RAPPORT_LOT_S3 - Symboles transverses (N3 + N4 + N5 + K1)

**Lot :** S3 (sequentiel)  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style nommage`

## Perimetre

Renommage API publique et symboles transverses sur l'ensemble du depot, sans changement fonctionnel ni deplacement de fichiers (S2 deja applique).

## Regles appliquees

| Regle | Action |
|-------|--------|
| **N3** | Methodes publiques PascalCase -> camelCase (toutes classes projet) |
| **N4** | Variables deja camelCase, conservees |
| **N5** | Valeurs enum PascalCase -> UPPER_CASE |
| **G8** (partiel) | `CaptureSnapshot` -> `SnapshotBuffer::capture` (methode statique) |
| **K1** | Suffixe `_` harmonise sur membres prives (`ParallelBatch`, `PoolBucket`) |

## Renommages principaux

### Fonctions libres -> camelCase

| Avant | Apres |
|-------|-------|
| `DrawHud` / `DrawHudEx` / `DrawMenu` | `drawHud` / `drawHudEx` / `drawMenu` |
| `DrawCar` / `DrawCarEx` | `drawCar` / `drawCarEx` |
| `GetCarLightPoints` | `getCarLightPoints` |
| `DrawSkyGradient` | `drawSkyGradient` |
| `CaptureSnapshot` | `SnapshotBuffer::capture` |

### Methodes publiques (exemples)

| Classe | Exemples |
|--------|----------|
| `Car` | `update`, `forward`, `velocity` |
| `Track` | `make`, `presets`, `projectPosition`, `waypoints` |
| `RaceState` | `update`, `phase`, `racers`, `standings` |
| `AIDriver` | `computeInput` |
| `World` | `createEntity`, `registry`, `add`, `get`, `has` |
| `JobSystem` | `parallelFor`, `submit`, `workerCount` |
| `SnapshotBuffer` | `writeBegin`, `publish`, `readLatest`, `capture` |
| `Device` / `RenderGraph` | `createRenderTarget`, `addPass`, `execute` |
| `AssetRegistry` / `ShaderWatcher` | `loadModelAsset`, `registerShader`, `poll` |
| `RenderPipeline` | `frame`, `setAmbiance`, `litShader` |
| `TrackRenderer` / `VfxSystem` | `draw`, `queueSkidMark`, `emitDriftSmoke` |

### Enums N5 -> UPPER_CASE

| Enum | Valeurs |
|------|---------|
| `AppState` | `MENU`, `RACING` |
| `RacePhase` | `COUNTDOWN`, `RACING`, `FINISHED` |
| `SurfaceStyle` | `PROPRE`, `ABIMEE` |
| `Ambiance` | `MIDI`, `AUBE_DOREE`, `CREPUSCULE`, `ORAGE` |

### K1 attributs prives

- `ParallelBatch` : `remaining_`, `firstError_`, `doneMutex_`, `doneCv_`
- `RenderGraph::PoolBucket` : `usedThisFrame_`

## Resume des renommages

| Categorie | Nombre approximatif |
|-----------|---------------------|
| Remplacements automatises (script) | ~572 |
| Corrections manuelles (appels `->`, internes, enums) | ~50 |
| Fichiers modifies | 41 sources + rapport |
| Lignes touchees (diff git) | 684 insertions / 685 suppressions |

**Total symboles renommes : ~620 occurrences** sur 41 fichiers `.hpp`/`.cpp`.

## Build

Commande :

```text
cmake --build build --config Debug
```

**Statut : SUCCES (0 erreur)** sur les 12 cibles :

| Cible | Type |
|-------|------|
| `racer` | executable |
| `race_sim_debug` | executable |
| `car_demo` | executable |
| `track_demo` | executable |
| `vfx_demo` | executable |
| `hud_demo` | executable |
| `engine_core_test` | executable |
| `render_demo` | executable |
| `engine_core` | lib statique |
| `engine_rhi` | lib statique |
| `engine_assets` | lib statique |
| `engine_render` | lib statique |

## Tests

- `engine_core_test` : **OK** (sortie `OK`)

## Items reportes au lot S4

- Scissions O3 (`Components.hpp`, etc.)
- G8 strict : fonctions libres HUD/CarRenderer -> methodes de classe dediees
- Helpers anonymes `main.cpp` / demos -> encapsulation classe

## Notes

- Noms de classes/types restes PascalCase (N2 deja fait en S2).
- Appels raylib (`DrawText`, `GetTime`, etc.) non modifies (A7).
- Script utilitaire : `.tools/lot_s3_rename.py` (non versionne).
