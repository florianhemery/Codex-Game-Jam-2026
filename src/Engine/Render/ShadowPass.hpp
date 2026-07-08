/*
** EPITECH PROJECT, 2026
** racer
** File description:
** shadow map render pass
*/

#ifndef SHADOW_PASS_HPP_
#define SHADOW_PASS_HPP_

#include "Engine/Render/ShaderLocations.hpp"
#include "Engine/Rhi/Device.hpp"
#include "Engine/Rhi/RhiTypes.hpp"

#include "raylib.h"

#include <functional>

namespace racer::engine {

class ShadowPass {
public:
    static constexpr int kResolution = 2048;
    static constexpr float kExtent = 280.0f;
    static constexpr float kDistance = 260.0f;
    static constexpr int kTextureSlot = 15;

    static void run(Device &device, RenderTargetHandle shadowRT, Shader lit,
                    const AmbianceParams &params, const LitLocs &litLocs,
                    Matrix &lightVP, const Camera3D &camera,
                    const std::function<void()> &drawShadowCasters);
};

} // namespace racer::engine

#endif /* !SHADOW_PASS_HPP_ */
