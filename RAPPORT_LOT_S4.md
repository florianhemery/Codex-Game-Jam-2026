# RAPPORT_LOT_S4 - Architecture stricte G8 + O3

## Statut

**SUCCES** - Rebuild complet Debug : 12 cibles sans erreur.

## Fichiers crees (O3)

### Engine/Core (ex-Components.hpp)
- `src/Engine/Core/TransformComponent.hpp`
- `src/Engine/Core/KinematicsComponent.hpp`
- `src/Engine/Core/RenderMeshComponent.hpp`
- `src/Engine/Core/LapProgressComponent.hpp`
- `src/Engine/Core/NameComponent.hpp`
- `src/Engine/Core/PlayerTag.hpp`
- `src/Engine/Core/AiTag.hpp`

**Supprime** : `src/Engine/Core/Components.hpp`

### Engine/Assets (ex-AssetRegistry.hpp)
- `src/Engine/Assets/ModelAsset.hpp` (+ `PbrMaterialInfo`)
- `src/Engine/Assets/TextureAsset.hpp`

### Engine/Assets (ex-ShaderWatcher.hpp)
- `src/Engine/Assets/ShaderSlot.hpp`

### Engine/Rhi (ex-RenderGraph.hpp)
- `src/Engine/Rhi/PassContext.hpp` (+ `PassDesc`)

### Vehicle (ex-Car.hpp)
- `src/Vehicle/CarInput.hpp`
- `src/Vehicle/CarTuning.hpp`

### Track (ex-Track.hpp)
- `src/Track/TrackDef.hpp` (+ `SurfaceStyle`)

### Render (ex-CarRenderer.hpp)
- `src/Render/CarVisual.hpp`
- `src/Render/CarLightPoints.hpp`

## Refactors G8

| Module | Avant | Apres |
|---|---|---|
| `Hud` | `drawHud`, `drawHudEx`, `drawMenu` (fonctions libres) | classe `Hud` avec `draw()`, `drawHudEx()`, `drawMenu()` |
| `CarRenderer` | `drawCar`, `drawCarEx`, `getCarLightPoints` (libres) | classe `CarRenderer` methodes statiques |
| `TrackRenderer` | `drawSkyGradient` (libre) | methode membre `drawSkyGradient()` |
| `SnapshotBuffer` | `capture()` | deja membre statique (S3) - conforme G8 |
| `main.cpp` | helpers | classe `MainApp` (A22, conservee) |
| `RaceSimDebug.cpp` | - | classe `RaceSimDebug` (deja presente A22) |
| Demos | `main()` + helpers | `CarDemoApp`, `TrackDemoApp`, `VfxDemoApp`, `HudDemoApp` |

## D2 - Encapsulation Car

Attributs `Car` passes en `private` avec accesseurs (`position()`, `speed()`, `tuning()`, etc.). Tous les call sites mis a jour.

## NormalizeAngle (G8)

- `Car::normalizeAngle` : `private static` (+ helpers physiques en `private static`)
- `RaceState::normalizeAngle` : deja `private static`
- `AIDriver::normalizeAngle` : deja `private static`
- Pas de fichier utilitaire transverse ajoute

## Includes / CMake

- Tous les `#include` mis a jour vers les nouveaux headers
- Aucun nouveau `.cpp` : CMakeLists inchanges (headers-only splits)

## A7 - Raylib

Encapsulation partielle deja presente via `Device`, `AssetRegistry`, classes render. Appels raylib directs conserves dans modules rendu/demo sans changement de comportement (wrappers complets = risque fonctionnel).

## Blockers

Aucun.

## Build

```
cmake --build build --config Debug
```

12 cibles : `racer`, `engine_rhi`, `engine_assets`, `engine_core`, `engine_render`, `engine_core_test`, `race_sim_debug`, `car_demo`, `track_demo`, `vfx_demo`, `hud_demo`, `render_demo`.
