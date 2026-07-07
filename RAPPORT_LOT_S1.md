# RAPPORT_LOT_S1 - Extensions .hpp, include guards et G1 CMake

**Lot :** S1 (sequentiel)  
**Regles :** O2, H2 (guards), G1 (CMakeLists)  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style extensions-hpp`

## Objectif

Renommer les 18 headers projet `.h` en `.hpp` (snake_case conserve), mettre a jour tous les `#include` et include guards (`_H_` -> `_HPP_`), ajouter l'en-tete Epitech G1 aux 5 `CMakeLists.txt`. Aucun changement fonctionnel ni renommage PascalCase/API (reserves a S2/S3).

## Renommages (18 fichiers)

| Avant | Apres |
|-------|-------|
| `src/vehicle/car.h` | `src/vehicle/car.hpp` |
| `src/track/track.h` | `src/track/track.hpp` |
| `src/ai/ai_driver.h` | `src/ai/ai_driver.hpp` |
| `src/race/race_state.h` | `src/race/race_state.hpp` |
| `src/render/track_renderer.h` | `src/render/track_renderer.hpp` |
| `src/render/car_renderer.h` | `src/render/car_renderer.hpp` |
| `src/render/vfx.h` | `src/render/vfx.hpp` |
| `src/render/hud.h` | `src/render/hud.hpp` |
| `src/engine/assets/asset_registry.h` | `src/engine/assets/asset_registry.hpp` |
| `src/engine/assets/shader_watcher.h` | `src/engine/assets/shader_watcher.hpp` |
| `src/engine/render/render_pipeline.h` | `src/engine/render/render_pipeline.hpp` |
| `src/engine/rhi/device.h` | `src/engine/rhi/device.hpp` |
| `src/engine/rhi/rhi_types.h` | `src/engine/rhi/rhi_types.hpp` |
| `src/engine/rhi/render_graph.h` | `src/engine/rhi/render_graph.hpp` |
| `src/engine/core/components.h` | `src/engine/core/components.hpp` |
| `src/engine/core/frame_snapshot.h` | `src/engine/core/frame_snapshot.hpp` |
| `src/engine/core/jobs.h` | `src/engine/core/jobs.hpp` |
| `src/engine/core/world.h` | `src/engine/core/world.hpp` |

## Include guards mis a jour

| Fichier | Avant | Apres |
|---------|-------|-------|
| `car.hpp` | `CAR_H_` | `CAR_HPP_` |
| `track.hpp` | `TRACK_H_` | `TRACK_HPP_` |
| `ai_driver.hpp` | `AI_DRIVER_H_` | `AI_DRIVER_HPP_` |
| `race_state.hpp` | `RACE_STATE_H` | `RACE_STATE_HPP_` |
| `track_renderer.hpp` | `TRACK_RENDERER_H_` | `TRACK_RENDERER_HPP_` |
| `car_renderer.hpp` | `CAR_RENDERER_H_` | `CAR_RENDERER_HPP_` |
| `vfx.hpp` | `VFX_H_` | `VFX_HPP_` |
| `hud.hpp` | `HUD_H_` | `HUD_HPP_` |
| `asset_registry.hpp` | `ASSET_REGISTRY_H_` | `ASSET_REGISTRY_HPP_` |
| `shader_watcher.hpp` | `SHADER_WATCHER_H_` | `SHADER_WATCHER_HPP_` |
| `render_pipeline.hpp` | `RENDER_PIPELINE_H_` | `RENDER_PIPELINE_HPP_` |
| `device.hpp` | `DEVICE_H_` | `DEVICE_HPP_` |
| `rhi_types.hpp` | `RHI_TYPES_H_` | `RHI_TYPES_HPP_` |
| `render_graph.hpp` | `RENDER_GRAPH_H_` | `RENDER_GRAPH_HPP_` |
| `components.hpp` | `COMPONENTS_H_` | `COMPONENTS_HPP_` |
| `frame_snapshot.hpp` | `FRAME_SNAPSHOT_H_` | `FRAME_SNAPSHOT_HPP_` |
| `jobs.hpp` | `JOBS_H_` | `JOBS_HPP_` |
| `world.hpp` | `WORLD_H_` | `WORLD_HPP_` |

## Fichiers avec `#include` mis a jour (24 .cpp + 10 .hpp)

**Sources .cpp (24) :** `main.cpp`, `vehicle/car.cpp`, `track/track.cpp`, `ai/ai_driver.cpp`, `race/race_state.cpp`, `render/track_renderer.cpp`, `render/car_renderer.cpp`, `render/hud.cpp`, `render/vfx.cpp`, `render/car_demo.cpp`, `render/track_demo.cpp`, `render/vfx_demo.cpp`, `render/hud_demo.cpp`, `tools/race_sim_debug.cpp`, `engine/rhi/device.cpp`, `engine/rhi/render_graph.cpp`, `engine/assets/asset_registry.cpp`, `engine/assets/shader_watcher.cpp`, `engine/core/world.cpp`, `engine/core/jobs.cpp`, `engine/core/frame_snapshot.cpp`, `engine/core/engine_core_test.cpp`, `engine/render/render_pipeline.cpp`, `engine/render/render_demo.cpp`

**Headers .hpp avec includes croises (8) :** `ai_driver.hpp`, `race_state.hpp`, `track_renderer.hpp`, `car_renderer.hpp`, `hud.hpp`, `render_pipeline.hpp`, `device.hpp`, `render_graph.hpp`

Les includes externes (`raylib.h`, `rlgl.h`, `raymath.h`) sont inchanges.

## En-tetes G1 CMake (5 fichiers)

| Fichier | Description G1 |
|---------|----------------|
| `CMakeLists.txt` (racine) | Root CMake build configuration |
| `src/engine/rhi/CMakeLists.txt` | RHI module — GPU abstraction and render graph |
| `src/engine/assets/CMakeLists.txt` | Assets module — asset registry and shader hot-reload |
| `src/engine/core/CMakeLists.txt` | Core module — ECS, job system and sim/render snapshot buffering |
| `src/engine/render/CMakeLists.txt` | Render module — high-level HDR rendering pipeline |

## Build

**Environnement :** CMake `.tools/cmake-4.3.4`, Ninja `.tools/ninja`, LLVM-MinGW UCRT (configure Phase 0).

```powershell
cmake --build build
```

**Resultat : SUCCES** — 12 cibles liees sans erreur :

| # | Cible |
|---|-------|
| 1 | `engine_rhi` |
| 2 | `engine_assets` |
| 3 | `engine_core` |
| 4 | `engine_render` |
| 5 | `racer` |
| 6 | `race_sim_debug` |
| 7 | `car_demo` |
| 8 | `track_demo` |
| 9 | `vfx_demo` |
| 10 | `hud_demo` |
| 11 | `render_demo` |
| 12 | `engine_core_test` |

Warnings residuels : uniquement EnTT (dependance externe, hors scope).

## Items non traites (lots suivants)

| Regle | Item | Lot |
|-------|------|-----|
| N2 | Fichiers/dossiers PascalCase | S2 |
| N3/N5 | API camelCase, enums UPPER_CASE | S3 |
| O3/G8 | Scissions multi-classes, architecture stricte | S4 |

## Changement fonctionnel

Aucun.
