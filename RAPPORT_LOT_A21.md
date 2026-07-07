# Rapport Lot A21 - Demos render

## Fichiers traites

| Fichier | Lignes avant | Lignes apres |
|---|---|---|
| `src/render/car_demo.cpp` | 112 | 186 |
| `src/render/track_demo.cpp` | 110 | 148 |
| `src/render/vfx_demo.cpp` | 94 | 137 |
| `src/render/hud_demo.cpp` | 136 | 133 |

## Regles appliquees (locale)

| Regle | Action |
|---|---|
| **G1** | En-tete Epitech `/// \file` + `/// \brief` sur les 4 fichiers |
| **F1** | Decoupe de `main()` et helpers longs en fonctions `namespace {}` |
| **F2** | Retours a la ligne pour respecter 80 colonnes |
| **F5** | Suppression de tous les commentaires dans les corps de fonctions |
| **L1-L6** | Accolades Allman, indentation 4 espaces, espaces operateurs |
| **V1-V2** | `const` sur variables immuables, initialisation explicite |
| **C1-C5** | Reduction de la complexite cyclomatique via extraction |
| **A1-A8** | Ordre includes : projet, raylib/rlgl, bibliotheque standard |
| **E1-E6** | Lignes vides entre blocs logiques (includes, namespace, fonctions) |
| **K1-K6** | Constantes locales prefixees `k` (`kCapW`, `kDt`, `kFrames`, etc.) |
| **S1-S3** | Espacement uniforme autour des operateurs et virgules |
| **D1-D6** | Documentation `///` avant les helpers (hors corps de fonctions) |

## Corrections notables par fichier

### car_demo.cpp

- Extraction : `LoadCaptureTarget`, `InitCars`, `InitCamera`, `UpdateTurntableCamera`,
  `BuildRedVisual` / `BuildBlueVisual` / `BuildGreenVisual`, `DrawCars`,
  `RenderScene`, `PresentFrame`, `ExportCapture`.
- Reorganisation des includes (headers projet en premier).

### track_demo.cpp

- Correction indentation `Camera3D` dans `main`.
- Extraction : `QueueSkidPair`, `CenterCameraOnTrack`, `LoadPreset`.
- Remplacement du lambda `loadPreset` par une fonction nommee.

### vfx_demo.cpp

- Titre fenetre : `vfx_demo particules` (suppression du double tiret decoratif).
- Extraction : `InitCamera`, `EmitDriftSalvo`, `EmitNitroBurst`, `EmitSparkBurst`,
  `DrawSceneMarkers`, `DrawHudOverlay`, `UpdateScenario`, `MaybeScreenshot`.

### hud_demo.cpp

- Extraction : `AdvanceRace`, `CaptureHudScene`.
- Suppression des commentaires de scenario dans `main`.
- Documentation des helpers deja presents conservee en `///` (hors corps).

## Items transverses (non traites, lots sequentiels)

| Item | Regle | Symboles / fichiers concernes | Lot cible |
|---|---|---|---|
| Extensions `.h` -> `.hpp` | O2 | `car_renderer.h`, `track_renderer.h`, `vfx.h`, `hud.h`, `car.h`, `track.h`, `race_state.h`, `ai_driver.h` | S1 |
| Arborescence PascalCase | N2 | `src/render/`, `CarDemo.cpp`, etc. | S2 |
| API publique camelCase | N3 | `DrawCarEx`, `DrawSkyGradient`, `DrawHudEx`, `DrawMenu`, `Track::Make`, `RaceState::Update`, `AIDriver::ComputeInput`, etc. | S3 |
| Enums UPPER_CASE | N5 | `RacePhase::Finished` | S3 |
| Fonctions libres -> classes | G8 | `DrawCarEx`, `DrawSkyGradient`, `DrawHudEx`, `DrawMenu` | S4 |
| `#pragma once` -> include guards | H2 | Tous les headers inclus | S1 |
| Header G1 CMakeLists | G1 | 5 fichiers `CMakeLists.txt` | S1 |

## Verification build

```
cmake --build build --config Debug --target car_demo track_demo vfx_demo hud_demo
```

Resultat : **OK** (4 executables lies sans erreur).

## Changement fonctionnel

Aucun. Comportement runtime, captures PNG et code de retour `vfx_demo` inchanges.
