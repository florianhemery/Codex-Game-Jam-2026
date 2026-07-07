/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HDR scene render pass
*/

#ifndef SCENE_PASS_HPP_
#define SCENE_PASS_HPP_

#include "Engine/Render/ShaderLocations.hpp"
#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include "raylib.h"

#include <functional>

namespace racer::engine {

class ScenePass {
public:
    static constexpr float kSkyRadius = 450.0f;

    static void run(Device &device, RenderTargetHandle sceneRT,
                    RenderTargetHandle shadowRT, Shader lit, Shader sky,
                    Model &skyDome, const AmbianceParams &params,
                    const LitLocs &litLocs, const SkyLocs &skyLocs,
                    const Matrix &lightVP, const Vector3 *lightsPos,
                    const Vector3 *lightsColor, int lightCount,
                    int maxLights, int shadowRes, int shadowSlot,
                    const Camera3D &camera, float time,
                    const std::function<void()> &drawLitScene,
                    const std::function<void()> &drawUnlitInScene);
};

} // namespace racer::engine

#endif /* !SCENE_PASS_HPP_ */
