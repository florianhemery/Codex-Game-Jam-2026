/// \file render_pipeline.h
/// \brief Pipeline de rendu haut niveau : ombres soleil, scene HDR avec dome
///        de ciel procedural, post-process (tonemap ACES, blur de vitesse,
///        grading). Quatre ambiances predefinies pilotent l'identite visuelle.

#pragma once

#include "raylib.h"

#include "engine/assets/shader_watcher.h"
#include "engine/rhi/device.h"
#include "engine/rhi/rhi_types.h"

#include <functional>
#include <string>

namespace racer::engine {

/// Ambiances lumineuses du jeu.
enum class Ambiance { Midi, AubeDoree, Crepuscule, Orage };

/// Parametres complets d'une ambiance (soleil, ciel, brouillard, etalonnage).
struct AmbianceParams {
    Vector3 sunDir;          // direction du soleil VERS la scene, normalisee
    Vector3 sunColor;        // HDR, peut depasser 1
    Vector3 skyAmbient;      // hemisphere haute
    Vector3 groundAmbient;   // hemisphere basse
    Vector3 skyZenith, skyHorizon;   // couleurs du dome
    float cloudCoverage;     // 0..1
    Vector3 cloudTint;
    Vector3 fogColor;
    float fogDensity;
    float exposure;
    Vector3 gradeTint;       // etalonnage post
    float saturation;
    float vignette;
    bool headlights;         // true si les voitures doivent allumer leurs phares
    bool stars;              // etoiles visibles
};

/// Orchestrateur des trois passes : ombres -> scene HDR -> post-process.
///
/// Contraintes d'usage : creer apres InitWindow, detruire avant CloseWindow,
/// thread principal uniquement. Les modeles du jeu doivent porter LitShader()
/// dans leurs materiaux (re-assigner apres PollShaderReload, l'id peut changer
/// lors d'un hot-reload).
class RenderPipeline {
public:
    RenderPipeline(int screenWidth, int screenHeight, const char* shaderDir = nullptr); // nullptr => recherche auto
    ~RenderPipeline();

    RenderPipeline(const RenderPipeline&) = delete;
    RenderPipeline& operator=(const RenderPipeline&) = delete;

    /// Parametres predefinis d'une ambiance.
    static const AmbianceParams& ParamsFor(Ambiance a);

    void SetAmbiance(Ambiance a);
    const AmbianceParams& Params() const { return params_; }

    /// Shader eclaire, a assigner aux materiaux des modeles du jeu.
    Shader LitShader() const;

    void ClearLights();

    /// Ajoute une point light (max 16, ignoree au-dela). colorIntensity en HDR.
    void AddLight(Vector3 position, Vector3 colorIntensity);

    /// Hot-reload des shaders (via ShaderWatcher). A appeler chaque frame.
    void PollShaderReload();

    /// Redimensionne la cible HDR scene (apres resize fenetre ou plein ecran).
    void Resize(int screenWidth, int screenHeight);

    struct PostParams { float speedRatio = 0.0f; bool nitro = false; }; // speedRatio 0..1

    /// Rend une frame complete. A appeler entre BeginDrawing et EndDrawing.
    /// - drawShadowCasters : geometrie projetant des ombres (passe profondeur)
    /// - drawLitScene      : scene eclairee (modeles + primitives batch)
    /// - drawUnlitInScene  : particules/effets non eclaires, dans la RT HDR
    void Frame(const Camera3D& camera,
               const std::function<void()>& drawShadowCasters,
               const std::function<void()>& drawLitScene,
               const std::function<void()>& drawUnlitInScene,
               const PostParams& post);

private:
    static constexpr int kMaxLights = 16;

    /// Emplacements d'uniforms caches (rafraichis apres chaque hot-reload).
    struct LitLocs {
        int viewPos = -1, sunDir = -1, sunColor = -1;
        int skyAmbient = -1, groundAmbient = -1;
        int fogColor = -1, fogDensity = -1;
        int lightVP = -1, shadowMap = -1, shadowTexel = -1;
        int lightsPos = -1, lightsColor = -1, lightsCount = -1;
    };
    struct SkyLocs {
        int sunDir = -1, sunColor = -1, zenith = -1, horizon = -1;
        int coverage = -1, tint = -1, fogColor = -1, time = -1, stars = -1;
    };
    struct PostLocs {
        int exposure = -1, gradeTint = -1, saturation = -1, vignette = -1;
        int aberration = -1, grainAmount = -1, time = -1, speedBlur = -1;
    };

    std::string ResolveShaderDir(const char* shaderDir) const;
    void RefreshLocations();
    void BindShadowMapTexture(unsigned int textureId);
    void UploadLitUniforms(const Camera3D& camera);
    void UploadSkyUniforms(float time);
    void UploadPostUniforms(float time, const PostParams& post);
    void DrawSkyDome(const Camera3D& camera);

    Device device_;
    ShaderWatcher watcher_;
    ShaderSlot* lit_ = nullptr;
    ShaderSlot* sky_ = nullptr;
    ShaderSlot* post_ = nullptr;

    RenderTargetHandle shadowRT_{};
    RenderTargetHandle sceneRT_{};

    Model skyDome_{};
    bool skyDomeLoaded_ = false;

    int width_ = 0;
    int height_ = 0;

    Ambiance ambiance_ = Ambiance::Midi;
    AmbianceParams params_{};

    Vector3 lightsPos_[kMaxLights]{};
    Vector3 lightsColor_[kMaxLights]{};
    int lightCount_ = 0;

    Matrix lightVP_{};

    LitLocs litLocs_{};
    SkyLocs skyLocs_{};
    PostLocs postLocs_{};
};

} // namespace racer::engine
