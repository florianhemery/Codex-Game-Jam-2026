# Rapport Lot L05 - Lisibilite Engine Render

**Fichiers traites :** `src/Engine/Render/`  
**Date :** 2026-07-07  
**Commit :** `refactor: lisibilite engine-render`

## Objectif

Decouper `RenderPipeline.cpp` (478L) en facade + passes de rendu nommees, reduire `RenderPipeline.hpp` sous 100 lignes, et simplifier `main()` de `RenderDemo.cpp`, sans modifier l'API publique ni les fichiers hors `src/Engine/Render/`.

## Decoupage applique

### RenderPipeline (facade)

| Fichier | Lignes | Role |
|---------|--------|------|
| `RenderPipeline.hpp` | 95 | API publique, etat pipeline, orchestration `frame()` |
| `RenderPipeline.cpp` | 244 | Presets ambiance, init/destruction, lights, `frame()` delegue aux passes |

### ShaderLocations

| Fichier | Lignes | Role |
|---------|--------|------|
| `ShaderLocations.hpp` | 88 | `Ambiance`, `AmbianceParams`, `LitLocs`, `SkyLocs`, `PostLocs` |
| `ShaderLocations.cpp` | 80 | `locOrArray`, refresh des locations, `bindShadowMapTexture` |

### ShadowPass

| Fichier | Lignes | Role |
|---------|--------|------|
| `ShadowPass.hpp` | 35 | Constantes ombre (`kResolution`, `kExtent`, `kDistance`, `kTextureSlot`) |
| `ShadowPass.cpp` | 49 | Camera lumiere orthographique, rendu depth map |

### ScenePass

| Fichier | Lignes | Role |
|---------|--------|------|
| `ScenePass.hpp` | 39 | Interface `ScenePass::run` |
| `ScenePass.cpp` | 106 | Uniforms lit/sky, sky dome, scene HDR (lit + unlit) |

### PostPass

| Fichier | Lignes | Role |
|---------|--------|------|
| `PostPass.hpp` | 25 | Interface `PostPass::run` |
| `PostPass.cpp` | 63 | Uniforms post-process, blit HDR vers framebuffer |

### RenderDemo

| Changement | Detail |
|------------|--------|
| `initDemoWindow()` | Config MSAA, fenetre, FPS |
| `runAmbianceDemo()` | Boucle 4 ambiances + captures |
| `shutdownDemoWindow()` | `CloseWindow()` |
| `main()` | 4 lignes : init → run → shutdown → return |

## Avant / apres

| Metrique | Avant | Apres |
|----------|-------|-------|
| `RenderPipeline.cpp` | 478L (monolithique) | 244L (facade) |
| `RenderPipeline.hpp` | 177L | 95L (< 100) |
| Fichiers sources module | 3 | 11 (+ CMakeLists) |
| Responsabilites par fichier | 1 bloc | 1 passe / 1 role |

## Perimetre respecte

- Aucun changement hors `src/Engine/Render/`.
- API publique `RenderPipeline` inchangee (`frame`, `setAmbiance`, `litShader`, `PostParams`, etc.).
- `CMakeLists.txt` du module mis a jour (5 nouvelles unites de compilation).

## Verification build

```text
cmake --build build --target engine_render render_demo racer
```

**Resultat :** compilation et liaison OK (`libengine_render.a`, `render_demo.exe`, `racer.exe`).
