# Rapport Lisibilite - Lot L01 Render

**Module :** `src/Render/`  
**Date :** 2026-07-07

## NEW_SOURCES

### Track (`src/Render/Track/`)
- `TrackRenderer.cpp/hpp` — facade draw / skid marks
- `TrackMeshBuilder.cpp/hpp` — helpers perpendiculaires, couleurs
- `TrackMeshStrip.cpp` — bandes route / pointilles
- `TrackMeshFinish.cpp` — ligne d'arrivee, montagnes
- `TrackMeshGeometry.cpp` — buffers mesh, boxes, skid quad
- `TrackDecorBuilder.cpp/hpp` — orchestration meshes decor
- `TrackDecorClouds.cpp` — nuages
- `TrackDecorProps.cpp` — props, gradins, waypoints
- `TrackDecorBarriers.cpp` — barrieres, sponsors, lampes, scene
- `TrackDrawPass.cpp/hpp` — passes draw principales
- `TrackDrawProps.cpp` — props / spectateurs
- `TrackDrawNpc.cpp` — NPCs
- `TrackSkidMarks.cpp/hpp` — marques de pneu
- `TrackSkyDraw.cpp/hpp` — ciel
- `TrackInstanceTypes.hpp` — types d'instances

### Hud (`src/Render/Hud/`)
- `Hud.cpp/hpp`, `HudExtras.hpp`, `HudTypes.hpp`
- `HudGfx.cpp/hpp`, `HudMinimap.cpp/hpp`, `HudMenu.cpp/hpp`
- `HudRaceGauge.cpp`, `HudRaceOverlay.cpp/hpp`, `HudFinishScreen.cpp/hpp`

### Car (`src/Render/Car/`)
- `CarRenderer.cpp/hpp`, `CarBodyDraw.cpp/hpp`
- `CarBodyPanels.cpp`, `CarWheelDraw.cpp/hpp`

### Vfx (`src/Render/Vfx/`)
- `VfxSystem.cpp/hpp`, `VfxSystemImpl.hpp`, `VfxTypes.hpp`
- `VfxParticlePool.cpp`, `VfxParticleTick.cpp`
- `VfxDrawPass.cpp/hpp`, `VfxTextureFactory.cpp/hpp`

### Demos (`src/Render/Demos/`)
- `CarDemo.cpp`, `TrackDemo.cpp`, `HudDemo.cpp`, `VfxDemo.cpp`

### Re-exports racine (API stable)
- `src/Render/Hud.hpp`, `CarRenderer.hpp`, `VfxSystem.hpp`

## Metriques module Render

| Indicateur | Avant | Apres |
|------------|-------|-------|
| Plus gros `.cpp` Render | 2405 (`TrackRenderer.cpp`) | 295 (`TrackDrawPass.cpp`) |
| Fichiers Render | ~16 | 56 |
| Sous-dossiers | 0 | Track/, Hud/, Car/, Vfx/, Demos/ |

## Notes Phase 2

- Toutes les sources listees sont dans `RACER_*_SOURCES` du `CMakeLists.txt` racine.
- Headers `TrackDecorBuilder.hpp` (160L) et `TrackMeshBuilder.hpp` (122L) restent > 100L (declarations privees).
