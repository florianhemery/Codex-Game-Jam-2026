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

void ShadowPass::run(Device &device, RenderTargetHandle shadowRT, Shader lit,
                     const AmbianceParams &params, const LitLocs &litLocs,
                     Matrix &lightVP, const Camera3D &camera,
                     const std::function<void()> &drawShadowCasters)
{
    Camera3D lightCam{};

    lightCam.position = Vector3Subtract(
        camera.target, Vector3Scale(params.sunDir, kDistance));
    lightCam.target = camera.target;
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
