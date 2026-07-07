/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Peripherique graphique : abstraction GPU (render targets, shaders)
*/

#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include "raylib.h"

#include "Engine/Rhi/RhiTypes.hpp"

#include <cstdint>
#include <unordered_map>

namespace racer::engine {

class Device {
public:
    Device() = default;
    ~Device();

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    RenderTargetHandle createRenderTarget(const RenderTargetDesc &desc);
    void destroyRenderTarget(RenderTargetHandle handle);
    const RenderTexture2D *getRenderTexture(
        RenderTargetHandle handle) const;
    const RenderTargetDesc *getRenderTargetDesc(
        RenderTargetHandle handle) const;
    void beginRenderTarget(RenderTargetHandle handle);
    void endRenderTarget();

    ShaderRhiHandle createShaderFromMemory(
        const char *vsSource, const char *fsSource);
    void destroyShader(ShaderRhiHandle handle);
    const Shader *getShader(ShaderRhiHandle handle) const;

private:
    struct RenderTargetEntry {
        RenderTexture2D target{};
        RenderTargetDesc desc{};
    };

    std::unordered_map<std::uint32_t, RenderTargetEntry> renderTargets_;
    std::unordered_map<std::uint32_t, Shader> shaders_;
    std::uint32_t nextRenderTargetId_ = 1;
    std::uint32_t nextShaderId_ = 1;
};

} // namespace racer::engine

#endif /* !DEVICE_HPP_ */
