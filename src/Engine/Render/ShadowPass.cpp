/*
** EPITECH PROJECT, 2026
** racer
** File description:
** shadow map render pass
*/

#include "Engine/Render/ShadowPass.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <cmath>

namespace racer::engine {

namespace {

Vector3 snapShadowTarget(Vector3 target)
{
    const float texelWorld = ShadowPass::kExtent
        / static_cast<float>(ShadowPass::kResolution);

    if (texelWorld <= 1e-5f)
        return target;
    target.x = std::floor(target.x / texelWorld) * texelWorld;
    target.z = std::floor(target.z / texelWorld) * texelWorld;
    return target;
}

} // namespace

void ShadowPass::run(Device &device, RenderTargetHandle shadowRT, Shader lit,
                     const AmbianceParams &params, const LitLocs &litLocs,
                     Matrix &lightVP, const Camera3D &camera,
                     const std::function<void()> &drawShadowCasters)
{
    Camera3D lightCam{};
    Vector3 shadowTarget = snapShadowTarget(camera.target);

    lightCam.position = Vector3Subtract(
        shadowTarget, Vector3Scale(params.sunDir, kDistance));
    lightCam.target = shadowTarget;
    lightCam.up = (std::fabs(params.sunDir.y) > 0.99f)
        ? Vector3{0.0f, 0.0f, 1.0f}
        : Vector3{0.0f, 1.0f, 0.0f};
    lightCam.fovy = kExtent;
    lightCam.projection = CAMERA_ORTHOGRAPHIC;

    bindShadowMapTexture(lit, litLocs, rlGetTextureIdDefault(), kTextureSlot);

    device.beginRenderTarget(shadowRT);
    ClearBackground(WHITE);
    BeginMode3D(lightCam);
    lightVP = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
    rlSetCullFace(RL_CULL_FACE_FRONT);
    BeginShaderMode(lit);
    if (drawShadowCasters)
        drawShadowCasters();
    EndShaderMode();
    rlSetCullFace(RL_CULL_FACE_BACK);
    EndMode3D();
    device.endRenderTarget();
}

} // namespace racer::engine
