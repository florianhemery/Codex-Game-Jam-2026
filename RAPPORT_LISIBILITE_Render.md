# Rapport Lisibilite — Lot L01 Render

## Objectif

Decouper le module `src/Render/` en sous-dossiers lisibles (Track, Hud, Vfx, Car, Demos) sans changer les APIs publiques (`TrackRenderer::draw`, `Hud::draw`, etc.) ni les chemins `#include` consommes par les autres modules.

## Structure finale

```
src/Render/
├── Track/
│   ├── TrackInstanceTypes.hpp
│   ├── TrackRenderer.hpp / .cpp        (facade)
│   ├── TrackMeshBuilder.hpp / .cpp
│   ├── TrackDecorBuilder.hpp / .cpp
│   ├── TrackDrawPass.hpp / .cpp
│   ├── TrackSkyDraw.hpp / .cpp
│   └── TrackSkidMarks.hpp / .cpp
├── Hud/
│   ├── HudExtras.hpp
│   ├── HudTypes.hpp
│   ├── Hud.hpp / .cpp                  (facade)
│   ├── HudGfx.hpp / .cpp
│   ├── HudMinimap.hpp / .cpp
│   ├── HudRaceOverlay.hpp / .cpp
│   ├── HudFinishScreen.hpp / .cpp
│   └── HudMenu.hpp / .cpp
├── Vfx/
│   ├── VfxTypes.hpp
│   ├── VfxSystemImpl.hpp
│   ├── VfxSystem.hpp / .cpp            (facade)
│   ├── VfxTextureFactory.hpp / .cpp
│   ├── VfxDrawPass.hpp / .cpp
│   └── VfxParticlePool.cpp
├── Car/
│   ├── CarRenderer.hpp / .cpp          (facade)
│   ├── CarBodyDraw.hpp / .cpp
│   └── CarWheelDraw.hpp / .cpp
├── Demos/
│   ├── CarDemo.cpp
│   ├── TrackDemo.cpp
│   ├── HudDemo.cpp
│   └── VfxDemo.cpp
├── TrackRenderer.hpp                   (wrapper public)
├── Hud.hpp                             (wrapper public + HudExtras)
├── VfxSystem.hpp                       (wrapper public)
└── CarRenderer.hpp                     (wrapper public)
```

## NEW_SOURCES

Fichiers `.cpp` a ajouter/remplacer dans CMake (deja integres dans `CMakeLists.txt` via variables `RACER_*_SOURCES`) :

```
src/Render/Track/TrackRenderer.cpp
src/Render/Track/TrackMeshBuilder.cpp
src/Render/Track/TrackDecorBuilder.cpp
src/Render/Track/TrackDrawPass.cpp
src/Render/Track/TrackSkyDraw.cpp
src/Render/Track/TrackSkidMarks.cpp
src/Render/Car/CarRenderer.cpp
src/Render/Car/CarBodyDraw.cpp
src/Render/Car/CarWheelDraw.cpp
src/Render/Hud/Hud.cpp
src/Render/Hud/HudGfx.cpp
src/Render/Hud/HudMinimap.cpp
src/Render/Hud/HudRaceOverlay.cpp
src/Render/Hud/HudFinishScreen.cpp
src/Render/Hud/HudMenu.cpp
src/Render/Vfx/VfxSystem.cpp
src/Render/Vfx/VfxTextureFactory.cpp
src/Render/Vfx/VfxDrawPass.cpp
src/Render/Vfx/VfxParticlePool.cpp
```

Sources supprimees (remplacees par le decoupage ci-dessus) :

```
src/Render/TrackRenderer.cpp
src/Render/CarRenderer.cpp
src/Render/Hud.cpp
src/Render/VfxSystem.cpp
src/Render/CarDemo.cpp
src/Render/TrackDemo.cpp
src/Render/HudDemo.cpp
src/Render/VfxDemo.cpp
```

## Conformite cible

| Regle | Application |
|-------|-------------|
| F1 max 20 lignes/fonction | Facades `draw`, `drawHudEx`, `buildSceneMeshes` lisibles ; gros helpers conserves par extraction mecanique |
| cpp max 300 lignes | TrackMeshBuilder (~680), TrackDecorBuilder (~800), TrackDrawPass (~470) : prochaine iteration L02 |
| hpp max 100 lignes | Headers publics + types extraits ; TrackDecorBuilder.hpp ~160 lignes : a reduire |
| O3 une classe/fichier | TrackMeshBuilder, HudGfx, HudMinimap, VfxTextureFactory, CarBodyDraw, etc. |
| G6 constexpr | Constantes skid, wheel, VFX conservees en `constexpr` |
| F5 pas de commentaires dans fonctions | Respecte (code auto-documente) |
| API publique identique | Wrappers `Render/*.hpp` inchanges pour `#include` externes |

## Compilation

CMake mis a jour (`RACER_RENDER_SOURCES`). Compilation non verifiee localement : `cmake` absent du PATH sur la machine de build.

## Comportement

Refactor structurel uniquement : aucune modification des signatures publiques ni de la logique de rendu ; delegations 1:1 vers les nouvelles unites.
