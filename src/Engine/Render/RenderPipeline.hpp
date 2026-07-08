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
#include "Engine/Render/ShaderLocations.hpp"
#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include <array>
#include <functional>
#include <string>

namespace racer::engine {

class RenderPipeline {
public:
    RenderPipeline(int screenWidth, int screenHeight,
                   const char *shaderDir = nullptr);
    ~RenderPipeline();

    RenderPipeline(const RenderPipeline &) = delete;
    RenderPipeline &operator=(const RenderPipeline &) = delete;

    static const AmbianceParams &paramsFor(Ambiance a);

    void setAmbiance(Ambiance a);
    void setFogDensity(float density);
    const AmbianceParams &params() const { return params_; }

    Shader litShader() const;

    void clearLights();
    void addLight(Vector3 position, Vector3 colorIntensity);
    void pollShaderReload();

    /// Redimensionne la cible HDR scene (resize fenetre ou plein ecran).
    void resize(int screenWidth, int screenHeight);

    struct PostParams {
        float speedRatio = 0.0f;
        bool nitro = false;
    };

    void frame(const Camera3D &camera,
               const std::function<void()> &drawShadowCasters,
               const std::function<void()> &drawLitScene,
               const std::function<void()> &drawUnlitInScene,
               const PostParams &post);

private:
    static constexpr int kMaxLights = 16;

    static const std::array<AmbianceParams, 4> &presetTable();
    static Shader defaultShader();

    std::string resolveShaderDir(const char *shaderDir) const;
    void refreshLocations();

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

    Ambiance ambiance_ = Ambiance::MIDI;
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
