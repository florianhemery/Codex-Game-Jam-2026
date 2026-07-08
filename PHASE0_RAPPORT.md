# Phase 0 — Rapport baseline build

**Projet :** `racer` (Codex-Game-Jam-2026)  
**Date :** 2026-07-07  
**Statut :** **SUCCÈS** (compilation et liaison terminées sans erreur)

## Commandes de build

Environnement : CMake portable (`.tools/cmake-4.3.4-windows-x86_64`), générateur **Ninja**, toolchain **LLVM-MinGW UCRT** (`x86_64-w64-mingw32-clang` / `clang++`). MSVC et `cmake` système n'étaient pas présents sur la machine ; l'équivalent **Debug** est obtenu via `-DCMAKE_BUILD_TYPE=Debug` (Ninja ne prend pas `--config Debug`).

```powershell
$env:PATH = "<llvm-mingw>\bin;<projet>\.tools\ninja;<projet>\.tools\cmake-4.3.4-windows-x86_64\bin;" + $env:PATH

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_C_COMPILER=<llvm-mingw>/bin/x86_64-w64-mingw32-clang.exe `
  -DCMAKE_CXX_COMPILER=<llvm-mingw>/bin/x86_64-w64-mingw32-clang++.exe

cmake --build build
```

Commande cible du plan (MSVC) :

```text
cmake -S . -B build
cmake --build build --config Debug
```

## Inventaire fichiers (hors `build/`, `.tools/`)

| Type | Nombre |
|------|--------|
| `.cpp` | 24 |
| `.h` | 18 |
| `CMakeLists.txt` | 5 (racine + 4 modules `src/engine/*`) |

## `compile_commands.json`

- **Présent :** oui — `build/compile_commands.json` (généré via `CMAKE_EXPORT_COMPILE_COMMANDS ON`).

## Les 11 cibles projet construites

| # | Cible | Artefact |
|---|--------|----------|
| 1 | `engine_rhi` | `build/src/engine/rhi/libengine_rhi.a` |
| 2 | `engine_assets` | `build/src/engine/assets/libengine_assets.a` |
| 3 | `engine_core` | `build/src/engine/core/libengine_core.a` |
| 4 | `engine_render` | `build/src/engine/render/libengine_render.a` |
| 5 | `racer` | `build/racer.exe` |
| 6 | `race_sim_debug` | `build/race_sim_debug.exe` |
| 7 | `car_demo` | `build/car_demo.exe` |
| 8 | `track_demo` | `build/track_demo.exe` |
| 9 | `vfx_demo` | `build/vfx_demo.exe` |
| 10 | `hud_demo` | `build/hud_demo.exe` |
| 11 | `render_demo` | `build/src/engine/render/render_demo.exe` |

*(Ninja a également produit `engine_core_test.exe`, hors décompte des 11 cibles métier du plan.)*

## Correctifs appliqués

- **Aucune modification du code source** du dépôt.
- Mise en place **locale** de l'environnement de build uniquement : CMake portable (`.tools/`), Ninja portable (`.tools/ninja/`), installation **LLVM-MinGW UCRT** via winget (compilateur + SDK Windows pour lier).

## Journaux

- Sortie complète du configure + build : [`BASELINE_BUILD.log`](BASELINE_BUILD.log)

## Phase suivante

Phase 1 **non démarrée** (conformément aux consignes).
