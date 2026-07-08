# Rapport Lisibilite - Consolidation Phase 2

**Date :** 2026-07-07  
**Build :** Ninja + LLVM-MinGW, 12 cibles + `engine_core_test` (compilation OK)

## Metriques avant / apres

| Metrique | Baseline (Phase 0) | Apres consolidation |
|----------|-------------------|---------------------|
| Fichiers `src/` (.cpp + .hpp) | 57 | **144** |
| Plus gros `.cpp` | 2405 (`Render/TrackRenderer.cpp`) | **<= 300** (0 depassement) |
| Plus gros `.hpp` | 191 (`Render/Hud.hpp`) | **160** (`TrackDecorBuilder.hpp`) |
| `.cpp` > 300 lignes | 9 | **0** |
| `.hpp` > 100 lignes | 4 | **5** |
| Structure `Render/` | plat | `Track/`, `Hud/`, `Car/`, `Vfx/`, `Demos/` |
| `main.cpp` | ~545 lignes | **13** (`MainApp::run`) |
| `race_sim_debug` sources | 1 monolithe | `RaceSimDebug` + `RaceSimPrinter` |

## Lots Phase 1

| Lot | Module | Rapport |
|-----|--------|---------|
| L01 | `src/Render/` | `RAPPORT_LISIBILITE_Render.md` |
| L02 | `src/Engine/Rhi/` | `RAPPORT_LISIBILITE_Rhi.md` |
| L03 | `src/Engine/Assets/` | `src/Engine/Assets/RAPPORT_LISIBILITE_Assets.md` |
| L04 | `src/Engine/Core/` | `RAPPORT_LISIBILITE_Core.md` |
| L05 | `src/Engine/Render/` | `RAPPORT_LISIBILITE_EngineRender.md` |
| L06 | `src/Vehicle/` | `RAPPORT_LISIBILITE_Vehicle.md` |
| L07 | `src/Track/` | `RAPPORT_LISIBILITE_Track.md` |
| L08 | `src/Ai/` | `RAPPORT_LISIBILITE_Ai.md` |
| L09 | `src/Race/` | `RAPPORT_LISIBILITE_Race.md` |
| L10 | `src/Tools/` | `RAPPORT_LISIBILITE_Tools.md` |
| L11 | `src/App/` | `RAPPORT_LISIBILITE_App.md` |

## CMake racine

- Variables `RACER_TRACK_RENDER_SOURCES`, `RACER_CAR_RENDER_SOURCES`, `RACER_HUD_SOURCES`, `RACER_VFX_SOURCES`, `RACER_TRACK_DOMAIN_SOURCES`
- `race_sim_debug` : `RaceSimPrinter.cpp` ajoute (plus de `#include` .cpp)
- Demos sous `src/Render/Demos/`

## Ecarts residuels (headers)

| Fichier | Lignes | Action future possible |
|---------|--------|------------------------|
| `TrackDecorBuilder.hpp` | 160 | scinder declarations privees |
| `AiDriver.hpp` | 135 | extraire constantes / etat |
| `TrackMeshBuilder.hpp` | 122 | 1 type/fichier si besoin |
| `CarState.hpp` | 121 | deja extrait de `Car.hpp` |
| `TrackInstanceTypes.hpp` | 101 | scinder 1 struct |

## Verification

```
cmake -S . -B build -G Ninja
cmake --build build
```

**Resultat :** 12 executables lies (`racer`, demos, `race_sim_debug`, `engine_core_test`, `render_demo`, libs moteur).
