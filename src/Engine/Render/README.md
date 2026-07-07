# engine_render — pipeline de rendu « AAA-lite »

Lib statique `engine_render` (namespace `racer::engine`). Orchestre le rendu
complet d'une frame : ombres du soleil, scène HDR avec ciel procédural,
post-process cinématique. Quatre ambiances prédéfinies (`Midi`, `AubeDoree`,
`Crepuscule`, `Orage`) définissent l'identité visuelle du jeu.

S'appuie sur `engine_rhi` (render targets HDR/profondeur via `Device`) et
`engine_assets` (`ShaderWatcher` pour le hot-reload). Shaders GLSL 330 dans
`assets/shaders/` : `lit.vs/.fs`, `sky.vs/.fs`, `post.fs`.

## Ordre des passes (`RenderPipeline::Frame`)

1. **Ombres** — caméra orthographique depuis le soleil, centrée sur
   `camera.target` (étendue 130 u), profondeur seule 2048² (`DEPTH24`).
   `drawShadowCasters()` y est appelé avec le lit shader actif ; le cull passe
   en front-face pour réduire l'acné. La shadow map n'est pas échantillonnable
   pendant cette passe (texture par défaut liée sur son slot) : le lit shader
   tolère cette situation, seules les profondeurs comptent.
2. **Scène HDR** — render target `RGBA16F` taille écran. Dôme de ciel d'abord
   (sphère inversée suivant la caméra, sans écriture de profondeur), puis
   `BeginShaderMode(lit)` + `drawLitScene()` (primitives batch et modèles),
   puis `drawUnlitInScene()` (particules avec leurs propres blend modes,
   toujours dans la RT HDR).
3. **Post** — quad plein écran vers le backbuffer : blur radial de vitesse,
   aberration chromatique légère, exposition, tonemap ACES, étalonnage
   (teinte/saturation), grain animé, vignette. Pas de FXAA (MSAA 4x côté jeu ;
   noter que le MSAA du backbuffer ne s'applique pas aux passes offscreen).

## Branchement côté jeu

```cpp
racer::engine::RenderPipeline pipeline(GetScreenWidth(), GetScreenHeight());
pipeline.SetAmbiance(racer::engine::Ambiance::Crepuscule);

// Les modeles portent le lit shader dans leurs materiaux :
model.materials[0].shader = pipeline.LitShader();

// Chaque frame :
pipeline.PollShaderReload();
model.materials[0].shader = pipeline.LitShader(); // re-assigner : l'id change au hot-reload

pipeline.ClearLights();
if (pipeline.Params().headlights) pipeline.AddLight(pharePos, {7.0f, 6.3f, 4.6f}); // max 16

BeginDrawing();
pipeline.Frame(camera,
    [&]{ /* casters d'ombres (voitures, decor) */ },
    [&]{ /* scene eclairee complete */ },
    [&]{ /* particules non eclairees (blend modes libres) */ },
    {speedRatio, nitroActif});
// HUD 2D ici, apres Frame()
EndDrawing();
```

Contraintes : créer après `InitWindow`, détruire avant `CloseWindow`, thread
principal. Avant `UnloadModel`, remettre un shader par défaut sur les
matériaux qui portaient `LitShader()` (le shader appartient au pipeline).
Résolution figée à la construction (pas de resize à chaud).

Recherche du dossier shaders si `shaderDir == nullptr` : `assets/shaders`,
puis `../`, `../../`, `../../../assets/shaders` (le jeu tourne depuis
`build/Debug/`).

## Lit shader (contrat)

Attributs/uniforms raylib standards (`vertexPosition/TexCoord/Normal/Color`,
`mvp/matModel/matNormal/colDiffuse/texture0`) : compatible `DrawModel` et
primitives batch (`DrawCube`, `DrawPlane`...). Éclairage : Lambert + GGX
discret (roughness 0.85), soleil + ombre PCF 3x3 à rotation aléatoire par
pixel (biais adaptatif selon N·L ; la rotation casse le banding des ombres
rasantes en bruit doux), ambiante hémisphérique, 16 point lights max
(atténuation `1/(1+0.12d+0.045d²)`), brouillard exp2. Sortie HDR linéaire.

Repli normales : si `length(vertexNormal) < 0.1`, reconstruction écran via
`cross(dFdx(worldPos), dFdy(worldPos))`. Attention : le batch rlgl mémorise la
dernière normale émise — `DrawCube`/`DrawPlane` émettent des normales propres,
mais `DrawSphere`/`DrawCylinder` batch héritent d'une normale périmée non
nulle (le repli ne se déclenche pas). Pour les formes courbes, préférer des
modèles (`GenMeshSphere`...) aux primitives batch.

## Démo de validation

`render_demo` (cible CMake) : scène de test (sol, solides colorés, mobile,
point lights dont une rouge pulsante, proxy voiture avec phares), cycle
automatique des 4 ambiances, exporte `render_demo_0..3.png` dans le CWD puis
se ferme seul. Lancer depuis la racine du dépôt.

## Évolution prévue

- Snapping de la caméra d'ombre au texel (réduire le shimmer en mouvement).
- Cascades d'ombres si les circuits grandissent.
- Bloom HDR (seuil + blur séparable) dans la passe post.
- Éclairs d'orage (flash directionnel piloté par le jeu via une ambiance
  modifiée localement).
