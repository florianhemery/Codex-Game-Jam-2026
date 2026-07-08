/*
** EPITECH PROJECT, 2026
** racer
** File description:
** render pipeline
*/

#include "Engine/Render/RenderPipeline.hpp"

#include "Engine/Render/PostPass.hpp"
#include "Engine/Render/ScenePass.hpp"
#include "Engine/Render/ShadowPass.hpp"

#include "rlgl.h"

#include <array>
#include <filesystem>

namespace racer::engine {

namespace {

const std::array<AmbianceParams, 4> kAmbiancePresets = {{
        {
            {-0.603f, -0.784f, 0.151f},
            {1.90f, 1.78f, 1.55f},
            {0.30f, 0.36f, 0.47f},
            {0.20f, 0.19f, 0.17f},
            {0.13f, 0.34f, 0.78f},
            {0.66f, 0.80f, 0.94f},
            0.32f,
            {1.00f, 1.00f, 1.02f},
            {0.62f, 0.73f, 0.88f},
            0.0060f,
            0.85f,
            {1.0f, 1.0f, 1.0f},
            1.0f,
            0.18f,
            false,
            false,
        },
        {
            {0.683f, -0.259f, 0.683f},
            {2.1f, 1.45f, 0.82f},
            {0.48f, 0.40f, 0.36f},
            {0.28f, 0.22f, 0.17f},
            {0.34f, 0.44f, 0.66f},
            {1.05f, 0.62f, 0.32f},
            0.45f,
            {1.05f, 0.88f, 0.72f},
            {0.92f, 0.70f, 0.46f},
            0.009f,
            1.0f,
            {1.04f, 0.98f, 0.92f},
            0.92f,
            0.24f,
            false,
            false,
        },
        {
            {0.705f, -0.070f, 0.705f},
            {2.6f, 1.05f, 0.62f},
            {0.30f, 0.22f, 0.38f},
            {0.14f, 0.11f, 0.14f},
            {0.16f, 0.08f, 0.30f},
            {0.98f, 0.40f, 0.34f},
            0.42f,
            {0.62f, 0.36f, 0.46f},
            {0.40f, 0.24f, 0.38f},
            0.009f,
            0.95f,
            {1.06f, 0.94f, 1.05f},
            1.04f,
            0.34f,
            true,
            true,
        },
        {
            {-0.305f, -0.927f, -0.218f},
            {0.30f, 0.34f, 0.40f},
            {0.30f, 0.33f, 0.37f},
            {0.13f, 0.14f, 0.15f},
            {0.10f, 0.11f, 0.13f},
            {0.20f, 0.22f, 0.25f},
            1.0f,
            {0.13f, 0.14f, 0.17f},
            {0.24f, 0.27f, 0.30f},
            0.016f,
            0.72f,
            {0.90f, 0.96f, 1.05f},
            0.80f,
            0.38f,
            true,
            false,
        },
    }};

} // namespace

const std::array<AmbianceParams, 4> &RenderPipeline::presetTable()
{
    return kAmbiancePresets;
}

Shader RenderPipeline::defaultShader()
{
    Shader shader{};

    shader.id = rlGetShaderIdDefault();
    shader.locs = rlGetShaderLocsDefault();
    return shader;
}

RenderPipeline::RenderPipeline(int screenWidth, int screenHeight,
                               const char *shaderDir)
    : width_(screenWidth), height_(screenHeight)
{
    const std::string dir = resolveShaderDir(shaderDir);

    TraceLog(LOG_INFO, "RENDER: dossier shaders: %s", dir.c_str());

    shadowRT_ = device_.createRenderTarget(
        {ShadowPass::kResolution, ShadowPass::kResolution,
         RhiFormat::DEPTH24, true});
    sceneRT_ = device_.createRenderTarget(
        {width_, height_, RhiFormat::RGBA16F, true});

    if (const RenderTexture2D *hdr = device_.getRenderTexture(sceneRT_))
        SetTextureWrap(hdr->texture, TEXTURE_WRAP_CLAMP);
    if (const RenderTexture2D *shadow = device_.getRenderTexture(shadowRT_))
        SetTextureWrap(shadow->depth, TEXTURE_WRAP_CLAMP);

    lit_ = &watcher_.registerShader("lit", dir + "/lit.vs", dir + "/lit.fs");
    sky_ = &watcher_.registerShader("sky", dir + "/sky.vs", dir + "/sky.fs");
    post_ = &watcher_.registerShader("post", "", dir + "/post.fs");
    watcher_.setOnReload([this](const std::string &, Shader &) {
        refreshLocations();
    });

    skyDome_ = LoadModelFromMesh(GenMeshSphere(1.0f, 24, 48));
    skyDomeLoaded_ = true;

    refreshLocations();
    setAmbiance(Ambiance::MIDI);
}

RenderPipeline::~RenderPipeline()
{
    if (!skyDomeLoaded_ || !IsWindowReady())
        return;

    skyDome_.materials[0].shader = defaultShader();
    UnloadModel(skyDome_);
    skyDomeLoaded_ = false;
}

const AmbianceParams &RenderPipeline::paramsFor(Ambiance a)
{
    return presetTable()[static_cast<std::size_t>(a)];
}

void RenderPipeline::setAmbiance(Ambiance a)
{
    ambiance_ = a;
    params_ = paramsFor(a);
}

Shader RenderPipeline::litShader() const
{
    return lit_->get();
}

void RenderPipeline::clearLights()
{
    lightCount_ = 0;
}

void RenderPipeline::addLight(Vector3 position, Vector3 colorIntensity)
{
    if (lightCount_ >= kMaxLights)
        return;

    lightsPos_[lightCount_] = position;
    lightsColor_[lightCount_] = colorIntensity;
    ++lightCount_;
}

void RenderPipeline::pollShaderReload()
{
    watcher_.poll();
}

void RenderPipeline::resize(int screenWidth, int screenHeight)
{
    if (screenWidth < 1 || screenHeight < 1)
        return;
    if (screenWidth == width_ && screenHeight == height_)
        return;

    device_.destroyRenderTarget(sceneRT_);
    width_ = screenWidth;
    height_ = screenHeight;
    sceneRT_ = device_.createRenderTarget(
        {width_, height_, RhiFormat::RGBA16F, true});

    if (const RenderTexture2D *hdr = device_.getRenderTexture(sceneRT_))
        SetTextureWrap(hdr->texture, TEXTURE_WRAP_CLAMP);
}

std::string RenderPipeline::resolveShaderDir(const char *shaderDir) const
{
    if (shaderDir != nullptr)
        return shaderDir;

    const std::array<const char *, 4> candidates = {
        "assets/shaders",
        "../assets/shaders",
        "../../assets/shaders",
        "../../../assets/shaders",
    };

    for (const char *candidate : candidates) {
        std::error_code ec;

        if (std::filesystem::exists(
                std::filesystem::path(candidate) / "lit.fs", ec))
            return candidate;
    }
    TraceLog(LOG_WARNING,
             "RENDER: dossier shaders introuvable, repli sur assets/shaders");
    return "assets/shaders";
}

void RenderPipeline::refreshLocations()
{
    refreshLitLocs(lit_->get(), litLocs_);
    refreshSkyLocs(sky_->get(), skyLocs_);
    refreshPostLocs(post_->get(), postLocs_);

    if (skyDomeLoaded_)
        skyDome_.materials[0].shader = sky_->get();
}

void RenderPipeline::frame(const Camera3D &camera,
                           const std::function<void()> &drawShadowCasters,
                           const std::function<void()> &drawLitScene,
                           const std::function<void()> &drawUnlitInScene,
                           const PostParams &post)
{
    const float time = static_cast<float>(GetTime());

    ShadowPass::run(device_, shadowRT_, lit_->get(), params_, litLocs_,
                    lightVP_, camera, drawShadowCasters);
    ScenePass::run(device_, sceneRT_, shadowRT_, lit_->get(), sky_->get(),
                   skyDome_, params_, litLocs_, skyLocs_, lightVP_, lightsPos_,
                   lightsColor_, lightCount_, kMaxLights,
                   ShadowPass::kResolution, ShadowPass::kTextureSlot, camera,
                   time, drawLitScene, drawUnlitInScene);
    PostPass::run(device_, sceneRT_, post_->get(), params_, postLocs_, time,
                  post);
}

} // namespace racer::engine
