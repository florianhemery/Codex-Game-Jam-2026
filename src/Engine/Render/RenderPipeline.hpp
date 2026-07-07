/*
** EPITECH PROJECT, 2026
** racer
** File description:
** render pipeline
*/

#ifndef RENDER_PIPELINE_HPP_
#define RENDER_PIPELINE_HPP_

#include "raylib.h"

#include "Engine/Assets/ShaderWatcher.hpp"
#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include <array>
#include <functional>
#include <string>

namespace racer::engine {

enum class Ambiance { Midi, AubeDoree, Crepuscule, Orage };

struct AmbianceParams {
    Vector3 sunDir;
    Vector3 sunColor;
    Vector3 skyAmbient;
    Vector3 groundAmbient;
    Vector3 skyZenith;
    Vector3 skyHorizon;
    float cloudCoverage;
    Vector3 cloudTint;
    Vector3 fogColor;
    float fogDensity;
    float exposure;
    Vector3 gradeTint;
    float saturation;
    float vignette;
    bool headlights;
    bool stars;
};

class RenderPipeline {
public:
    RenderPipeline(int screenWidth, int screenHeight,
                   const char *shaderDir = nullptr);
    ~RenderPipeline();

    RenderPipeline(const RenderPipeline &) = delete;
    RenderPipeline &operator=(const RenderPipeline &) = delete;

    static const AmbianceParams &ParamsFor(Ambiance a);

    void SetAmbiance(Ambiance a);
    const AmbianceParams &Params() const { return params_; }

    Shader LitShader() const;

    void ClearLights();
    void AddLight(Vector3 position, Vector3 colorIntensity);
    void PollShaderReload();

    struct PostParams {
        float speedRatio = 0.0f;
        bool nitro = false;
    };

    void Frame(const Camera3D &camera,
               const std::function<void()> &drawShadowCasters,
               const std::function<void()> &drawLitScene,
               const std::function<void()> &drawUnlitInScene,
               const PostParams &post);

private:
    static constexpr int kMaxLights = 16;
    static constexpr int kShadowRes = 2048;
    static constexpr float kShadowExtent = 130.0f;
    static constexpr float kShadowDist = 260.0f;
    static constexpr float kSkyRadius = 450.0f;
    static constexpr int kShadowSlot = 15;

    struct LitLocs {
        int viewPos = -1;
        int sunDir = -1;
        int sunColor = -1;
        int skyAmbient = -1;
        int groundAmbient = -1;
        int fogColor = -1;
        int fogDensity = -1;
        int lightVP = -1;
        int shadowMap = -1;
        int shadowTexel = -1;
        int lightsPos = -1;
        int lightsColor = -1;
        int lightsCount = -1;
    };

    struct SkyLocs {
        int sunDir = -1;
        int sunColor = -1;
        int zenith = -1;
        int horizon = -1;
        int coverage = -1;
        int tint = -1;
        int fogColor = -1;
        int time = -1;
        int stars = -1;
    };

    struct PostLocs {
        int exposure = -1;
        int gradeTint = -1;
        int saturation = -1;
        int vignette = -1;
        int aberration = -1;
        int grainAmount = -1;
        int time = -1;
        int speedBlur = -1;
    };

    static const std::array<AmbianceParams, 4> &presetTable();
    static int locOrArray(const Shader &shader, const char *name,
                          const char *arrayName);
    static Shader defaultShader();

    std::string resolveShaderDir(const char *shaderDir) const;
    void refreshLocations();
    void refreshLitLocs();
    void refreshSkyLocs();
    void refreshPostLocs();
    void bindShadowMapTexture(unsigned int textureId);
    void uploadLitUniforms(const Camera3D &camera);
    void uploadSkyUniforms(float time);
    void uploadPostUniforms(float time, const PostParams &post);
    void drawSkyDome(const Camera3D &camera);
    void runShadowPass(const Camera3D &camera,
                       const std::function<void()> &drawShadowCasters);
    void runScenePass(const Camera3D &camera,
                      const std::function<void()> &drawLitScene,
                      const std::function<void()> &drawUnlitInScene,
                      float time);
    void runPostPass(const PostParams &post, float time);

    Device device_;
    ShaderWatcher watcher_;
    ShaderSlot *lit_ = nullptr;
    ShaderSlot *sky_ = nullptr;
    ShaderSlot *post_ = nullptr;

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

#endif /* !RENDER_PIPELINE_HPP_ */
