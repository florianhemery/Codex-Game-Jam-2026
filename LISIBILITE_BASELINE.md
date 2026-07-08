# LISIBILITE_BASELINE - Mesures avant passe de lisibilite

**Date :** 2026-07-07  
**Build :** OK (`cmake --build build`, Ninja + LLVM-MinGW)

## Metriques globales

| Indicateur | Valeur |
|------------|--------|
| Fichiers `.cpp` | 24 (8703 lignes) |
| Fichiers `.hpp` | 33 (1670 lignes) |
| Total fichiers source | 57 |
| Plus gros `.cpp` | `src/Render/TrackRenderer.cpp` (2140 lignes) |
| Plus gros `.hpp` | `src/Render/Hud.hpp` (172 lignes) |
| `.cpp` > 300 lignes | 9 |
| `.hpp` > 100 lignes | 4 |

## Fichiers hors limites

### `.cpp` > 300 lignes

| Fichier | Lignes |
|---------|--------|
| `src/Render/TrackRenderer.cpp` | 2140 |
| `src/Render/Hud.cpp` | 907 |
| `src/Render/VfxSystem.cpp` | 733 |
| `src/Render/CarRenderer.cpp` | 604 |
| `src/main.cpp` | 469 |
| `src/Engine/Render/RenderPipeline.cpp` | 410 |
| `src/Engine/Core/EngineCoreTest.cpp` | 317 |
| `src/Race/RaceState.cpp` | 312 |
| `src/Engine/Render/RenderDemo.cpp` | 306 |

### `.hpp` > 100 lignes

| Fichier | Lignes |
|---------|--------|
| `src/Render/Hud.hpp` | 172 |
| `src/Engine/Render/RenderPipeline.hpp` | 145 |
| `src/Render/TrackRenderer.hpp` | 134 |
| `src/Vehicle/Car.hpp` | 111 |

## Fonctions > 20 lignes (estimation plan)

| Module | Nb fonctions estimees |
|--------|----------------------|
| TrackRenderer | ~17 |
| Hud | 12 |
| VfxSystem | 5 |
| CarRenderer | 2 |
| main/MainApp | 3 |
| RenderPipeline | 4 |
| EngineCoreTest | 3 |
| RaceState | 2 |
| RenderDemo | 1 |
| **Total estime** | **~50** |

## Cibles build (12)

`engine_rhi`, `engine_assets`, `engine_core`, `engine_render`, `racer`, `race_sim_debug`, `car_demo`, `track_demo`, `vfx_demo`, `hud_demo`, `render_demo`, `engine_core_test`

## Objectifs cibles post-passe

| Metrique | Avant | Cible |
|----------|-------|-------|
| Plus gros `.cpp` | 2140 | <= 300 |
| Plus gros `.hpp` | 172 | <= 100 |
| Fichiers hors limites | 13 | 0 |
| Fonctions > 20L | ~50 | 0 |
| Nb fichiers `src/` | 57 | ~90-110 |
