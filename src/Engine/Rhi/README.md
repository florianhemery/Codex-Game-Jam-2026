# engine_rhi — RHI + render graph

Abstraction GPU du moteur (Phase 0). Le reste du moteur ne doit jamais appeler
`rlgl` ni créer de `RenderTexture2D` à la main : tout passe par ce module.

## Rôle

- **`rhi_types.h`** : handles opaques typés (`RenderTargetHandle`,
  `ShaderRhiHandle`, id 0 = invalide), formats (`RhiFormat` : `RGBA8`,
  `RGBA16F`, `DEPTH24`) et `RenderTargetDesc { width, height, format, useDepth }`.
- **`device.h/.cpp`** : classe `Device`, création/destruction des ressources GPU
  au-dessus de raylib/rlgl :
  - `CreateRenderTarget(desc)` / `DestroyRenderTarget(h)` / `GetRenderTexture(h)`
  - `CreateShaderFromMemory(vs, fs)` / `DestroyShader(h)` / `GetShader(h)`
  - `BeginRenderTarget(h)` / `EndRenderTarget()`
- **`render_graph.h/.cpp`** : `RenderGraph`, graphe linéaire redéclaré chaque
  frame. `AddPass(name, PassDesc{outputDesc, execute})` ; `outputDesc` à
  `std::nullopt` = rendu à l'écran. Le callback reçoit un `PassContext` :
  `GetDevice()`, `GetOutput()` et `ReadTarget("nomPasse")` pour lire la sortie
  d'une passe précédente.

## Usage type

```cpp
racer::engine::Device device;                 // après InitWindow
racer::engine::RenderGraph graph(device);

// Chaque frame :
graph.Reset();
graph.AddPass("scene", {
    racer::engine::RenderTargetDesc{w, h, racer::engine::RhiFormat::RGBA16F, true},
    [&](racer::engine::PassContext& ctx) { /* dessin 3D */ }});
graph.AddPass("tonemap", {std::nullopt, [&](racer::engine::PassContext& ctx) {
    auto* hdr = ctx.GetDevice().GetRenderTexture(ctx.ReadTarget("scene"));
    /* fullscreen quad avec shader tonemap */
}});
graph.Execute();
```

Les cibles transientes sont recyclées entre frames via un pool keyé par
descripteur : même desc ⇒ même texture GPU, zéro allocation en régime établi.

## Notes backend (raylib / OpenGL 3.3)

- Les render targets sont assemblés à la main (`rlLoadFramebuffer` +
  `rlLoadTexture` + `rlLoadTextureDepth`) pour permettre un format couleur
  paramétrable, ce que `LoadRenderTexture` ne sait pas faire.
- `RGBA16F` utilise `RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16` (→ `GL_RGBA16F`,
  natif en GL 3.3) avec repli `R32G32B32A32` (→ `GL_RGBA32F`) si le driver
  refuse.
- `DEPTH24` crée une cible profondeur seule échantillonnable (shadow map).
- `BeginRenderTarget` s'appuie sur `BeginTextureMode` pour rester cohérent avec
  le batching raylib.

## Évolution prévue

L'API publique (handles + descripteurs + passes) est agnostique du backend.
Pour passer à GL 4.6 ou Vulkan : réimplémenter `device.cpp` (et à terme rendre
`Device` polymorphe ou compilé par backend), remplacer les deux accesseurs
`GetRenderTexture`/`GetShader` par des équivalents du nouveau backend, sans
toucher aux appelants du render graph. Extensions envisagées : MRT (G-buffer),
compute passes, aliasing mémoire des transients, culling de passes mortes.
