# RAPPORT_LOT_S2 - Arborescence PascalCase (N2 + O1)

**Lot :** S2 (sequentiel)  
**Regles :** N2 (fichiers/dossiers PascalCase), O1 (un type principal par fichier)  
**Date :** 2026-07-07  
**Commit :** `style: conformite coding style arborescence`

## Objectif

Renommer l'arborescence `src/` en PascalCase (dossiers et fichiers), mettre a jour tous les `#include`, les 5 `CMakeLists.txt`, et les include guards des fichiers renommes. Aucun renommage de symboles/API (S3) ni scission O3 (S4).

## Renommages dossiers

| Avant | Apres |
|-------|-------|
| `src/vehicle/` | `src/Vehicle/` |
| `src/track/` | `src/Track/` |
| `src/ai/` | `src/Ai/` |
| `src/race/` | `src/Race/` |
| `src/render/` | `src/Render/` |
| `src/tools/` | `src/Tools/` |
| `src/engine/core/` | `src/Engine/Core/` |
| `src/engine/rhi/` | `src/Engine/Rhi/` |
| `src/engine/assets/` | `src/Engine/Assets/` |
| `src/engine/render/` | `src/Engine/Render/` |

`src/main.cpp` conserve a la racine de `src/` (plus simple pour CMake).

## Renommages fichiers (34 sources + 4 CMakeLists deplaces)

| Avant | Apres |
|-------|-------|
| `src/vehicle/car.*` | `src/Vehicle/Car.hpp/.cpp` |
| `src/track/track.*` | `src/Track/Track.hpp/.cpp` |
| `src/ai/ai_driver.*` | `src/Ai/AiDriver.hpp/.cpp` |
| `src/race/race_state.*` | `src/Race/RaceState.hpp/.cpp` |
| `src/render/car_renderer.*` | `src/Render/CarRenderer.hpp/.cpp` |
| `src/render/track_renderer.*` | `src/Render/TrackRenderer.hpp/.cpp` |
| `src/render/hud.*` | `src/Render/Hud.hpp/.cpp` |
| `src/render/vfx.*` | `src/Render/VfxSystem.hpp/.cpp` |
| `src/render/car_demo.cpp` | `src/Render/CarDemo.cpp` |
| `src/render/track_demo.cpp` | `src/Render/TrackDemo.cpp` |
| `src/render/vfx_demo.cpp` | `src/Render/VfxDemo.cpp` |
| `src/render/hud_demo.cpp` | `src/Render/HudDemo.cpp` |
| `src/engine/core/jobs.*` | `src/Engine/Core/JobSystem.hpp/.cpp` |
| `src/engine/core/world.*` | `src/Engine/Core/World.hpp/.cpp` |
| `src/engine/core/frame_snapshot.*` | `src/Engine/Core/SnapshotBuffer.hpp/.cpp` |
| `src/engine/core/components.hpp` | `src/Engine/Core/Components.hpp` |
| `src/engine/core/engine_core_test.cpp` | `src/Engine/Core/EngineCoreTest.cpp` |
| `src/engine/rhi/device.*` | `src/Engine/Rhi/Device.hpp/.cpp` |
| `src/engine/rhi/render_graph.*` | `src/Engine/Rhi/RenderGraph.hpp/.cpp` |
| `src/engine/rhi/rhi_types.hpp` | `src/Engine/Rhi/RhiTypes.hpp` |
| `src/engine/assets/asset_registry.*` | `src/Engine/Assets/AssetRegistry.hpp/.cpp` |
| `src/engine/assets/shader_watcher.*` | `src/Engine/Assets/ShaderWatcher.hpp/.cpp` |
| `src/engine/render/render_pipeline.*` | `src/Engine/Render/RenderPipeline.hpp/.cpp` |
| `src/engine/render/render_demo.cpp` | `src/Engine/Render/RenderDemo.cpp` |
| `src/tools/race_sim_debug.cpp` | `src/Tools/RaceSimDebug.cpp` |

## Include guards mis a jour (fichiers renommes)

| Fichier | Avant | Apres |
|---------|-------|-------|
| `JobSystem.hpp` | `JOBS_HPP_` | `JOB_SYSTEM_HPP_` |
| `SnapshotBuffer.hpp` | `FRAME_SNAPSHOT_HPP_` | `SNAPSHOT_BUFFER_HPP_` |
| `VfxSystem.hpp` | `VFX_HPP_` | `VFX_SYSTEM_HPP_` |

Les autres guards restent alignes sur le nom de classe (`CAR_HPP_`, `TRACK_HPP_`, etc.).

## `#include` mis a jour

Tous les includes projet passent en PascalCase, ex. :

- `"vehicle/car.hpp"` → `"Vehicle/Car.hpp"`
- `"engine/core/jobs.hpp"` → `"Engine/Core/JobSystem.hpp"`
- `"render/vfx.hpp"` → `"Render/VfxSystem.hpp"`

**34 fichiers** `.cpp`/`.hpp` mis a jour dans `src/`. Includes externes (`raylib.h`, etc.) inchanges.

## CMake (5 fichiers)

| Fichier | Changements |
|---------|-------------|
| `CMakeLists.txt` (racine) | `add_subdirectory(src/Engine/...)` ; chemins sources PascalCase |
| `src/Engine/Core/CMakeLists.txt` | `World.cpp`, `JobSystem.cpp`, `SnapshotBuffer.cpp`, `EngineCoreTest.cpp` |
| `src/Engine/Rhi/CMakeLists.txt` | `Device.cpp`, `RenderGraph.cpp` |
| `src/Engine/Assets/CMakeLists.txt` | `AssetRegistry.cpp`, `ShaderWatcher.cpp` |
| `src/Engine/Render/CMakeLists.txt` | `RenderPipeline.cpp`, `RenderDemo.cpp` |

## Build

**Environnement :** CMake `.tools/cmake-4.3.4`, Ninja `.tools/ninja`, LLVM-MinGW UCRT.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ...
cmake --build build
```

**Resultat : SUCCES** — 12 cibles sans erreur :

| # | Cible | Artefact |
|---|-------|----------|
| 1 | `engine_rhi` | `build/src/Engine/Rhi/libengine_rhi.a` |
| 2 | `engine_assets` | `build/src/Engine/Assets/libengine_assets.a` |
| 3 | `engine_core` | `build/src/Engine/Core/libengine_core.a` |
| 4 | `engine_render` | `build/src/Engine/Render/libengine_render.a` |
| 5 | `racer` | `build/racer.exe` |
| 6 | `race_sim_debug` | `build/race_sim_debug.exe` |
| 7 | `car_demo` | `build/car_demo.exe` |
| 8 | `track_demo` | `build/track_demo.exe` |
| 9 | `vfx_demo` | `build/vfx_demo.exe` |
| 10 | `hud_demo` | `build/hud_demo.exe` |
| 11 | `engine_core_test` | `build/engine_core_test.exe` |
| 12 | `render_demo` | `build/src/Engine/Render/render_demo.exe` |

## Notes techniques

- Sous Windows (FS insensible a la casse), renommages via repertoires intermediaires `pkg_*` pour eviter les collisions `vehicle`/`Vehicle`.
- Anciens dossiers snake_case supprimes apres migration.
- Symboles/API (`Car`, `DrawHud`, `JobSystem`, etc.) inchanges — reportes au lot S3.

## Items reportes

| Regle | Item | Lot prevu |
|-------|------|-----------|
| **N3** | API publique camelCase | S3 |
| **O3** | Scission multi-classes (`Components.hpp`, etc.) | S4 |
| **G8** | Refactors proceduraux → methodes de classe | S4 |
