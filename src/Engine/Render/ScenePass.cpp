/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HDR scene render pass
*/

#include "Engine/Render/ScenePass.hpp"

#include "rlgl.h"

namespace racer::engine {

namespace {

void uploadLitUniforms(Shader lit, const LitLocs &locs,
                       const AmbianceParams &params, const Camera3D &camera,
                       const Matrix &lightVP, const Vector3 *lightsPos,
                       const Vector3 *lightsColor, int lightCount,
                       int maxLights, int shadowRes)
{
    const float texel = 1.0f / static_cast<float>(shadowRes);

    SetShaderValue(lit, locs.viewPos, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.sunDir, &params.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.sunColor, &params.sunColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.skyAmbient, &params.skyAmbient,
                   SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.groundAmbient, &params.groundAmbient,
                   SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.fogColor, &params.fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, locs.fogDensity, &params.fogDensity,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValueMatrix(lit, locs.lightVP, lightVP);
    SetShaderValue(lit, locs.shadowTexel, &texel, SHADER_UNIFORM_FLOAT);
    SetShaderValueV(lit, locs.lightsPos, lightsPos, SHADER_UNIFORM_VEC3,
                    maxLights);
    SetShaderValueV(lit, locs.lightsColor, lightsColor, SHADER_UNIFORM_VEC3,
                    maxLights);
    SetShaderValue(lit, locs.lightsCount, &lightCount, SHADER_UNIFORM_INT);
    if (locs.useTextureAlbedo >= 0) {
        const float useTex = 1.0f;

        SetShaderValue(lit, locs.useTextureAlbedo, &useTex, SHADER_UNIFORM_FLOAT);
    }
    if (locs.terrainMode >= 0) {
        const float off = 0.0f;

        SetShaderValue(lit, locs.terrainMode, &off, SHADER_UNIFORM_FLOAT);
    }
    if (locs.biomeTint >= 0) {
        const Vector3 neutral[4] = {
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
        };

        SetShaderValueV(lit, locs.biomeTint, neutral, SHADER_UNIFORM_VEC3, 4);
    }
}

void uploadSkyUniforms(Shader sky, const SkyLocs &locs,
                       const AmbianceParams &params, float time)
{
    const int stars = params.stars ? 1 : 0;

    SetShaderValue(sky, locs.sunDir, &params.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.sunColor, &params.sunColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.zenith, &params.skyZenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.horizon, &params.skyHorizon,
                   SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.coverage, &params.cloudCoverage,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(sky, locs.tint, &params.cloudTint, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.fogColor, &params.fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, locs.time, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sky, locs.stars, &stars, SHADER_UNIFORM_INT);
}

void drawSkyDome(const Model &skyDome, const Camera3D &camera)
{
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    DrawModel(skyDome, camera.position, ScenePass::kSkyRadius, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

} // namespace

void ScenePass::run(Device &device, RenderTargetHandle sceneRT,
                    RenderTargetHandle shadowRT, Shader lit, Shader sky,
                    Model &skyDome, const AmbianceParams &params,
                    const LitLocs &litLocs, const SkyLocs &skyLocs,
                    const Matrix &lightVP, const Vector3 *lightsPos,
                    const Vector3 *lightsColor, int lightCount, int maxLights,
                    int shadowRes, int shadowSlot, const Camera3D &camera,
                    float time, const std::function<void()> &drawLitScene,
                    const std::function<void()> &drawUnlitInScene)
{
    uploadLitUniforms(lit, litLocs, params, camera, lightVP, lightsPos,
                      lightsColor, lightCount, maxLights, shadowRes);
    uploadSkyUniforms(sky, skyLocs, params, time);

    const RenderTexture2D *shadowTex = device.getRenderTexture(shadowRT);
    const unsigned int shadowId = shadowTex != nullptr
        ? shadowTex->depth.id
        : rlGetTextureIdDefault();

    bindShadowMapTexture(lit, litLocs, shadowId, shadowSlot);

    device.beginRenderTarget(sceneRT);
    ClearBackground(BLACK);
    BeginMode3D(camera);
    drawSkyDome(skyDome, camera);
    BeginShaderMode(lit);
    if (drawLitScene)
        drawLitScene();
    EndShaderMode();
    if (drawUnlitInScene)
        drawUnlitInScene();
    EndMode3D();
    device.endRenderTarget();
}

} // namespace racer::engine
