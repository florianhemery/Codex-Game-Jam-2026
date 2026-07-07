/*
** EPITECH PROJECT, 2026
** racer
** File description:
** post-process render pass
*/

#include "Engine/Render/PostPass.hpp"

#include "raymath.h"

namespace racer::engine {

namespace {

void uploadPostUniforms(Shader post, const PostLocs &locs,
                        const AmbianceParams &params, float time,
                        const RenderPipeline::PostParams &postParams)
{
    const float speed = Clamp(postParams.speedRatio, 0.0f, 1.0f);
    const float blur = speed * (postParams.nitro ? 1.4f : 1.0f);
    const float aberration = 0.0010f + 0.0035f * speed
        + (postParams.nitro ? 0.0020f : 0.0f);
    const float grain = 0.028f;

    SetShaderValue(post, locs.exposure, &params.exposure, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.gradeTint, &params.gradeTint,
                   SHADER_UNIFORM_VEC3);
    SetShaderValue(post, locs.saturation, &params.saturation,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.vignette, &params.vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.aberration, &aberration, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.grainAmount, &grain, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.time, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(post, locs.speedBlur, &blur, SHADER_UNIFORM_FLOAT);
}

} // namespace

void PostPass::run(Device &device, RenderTargetHandle sceneRT, Shader postShader,
                   const AmbianceParams &params, const PostLocs &postLocs,
                   float time, const RenderPipeline::PostParams &postParams)
{
    uploadPostUniforms(postShader, postLocs, params, time, postParams);

    const RenderTexture2D *hdr = device.getRenderTexture(sceneRT);

    if (hdr == nullptr)
        return;

    const Rectangle src{
        0.0f,
        0.0f,
        static_cast<float>(hdr->texture.width),
        -static_cast<float>(hdr->texture.height),
    };

    BeginShaderMode(postShader);
    DrawTextureRec(hdr->texture, src, Vector2{0.0f, 0.0f}, WHITE);
    EndShaderMode();
}

} // namespace racer::engine
