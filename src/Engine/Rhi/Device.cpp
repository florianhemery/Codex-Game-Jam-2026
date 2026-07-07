/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Implementation du Device sur backend raylib/rlgl (OpenGL 3.3)
*/

#include "Engine/Rhi/Device.hpp"

#include "Engine/Rhi/DeviceFramebuffer.hpp"

#include "raylib.h"

namespace racer::engine {

Device::~Device()
{
    for (const auto &[id, entry] : renderTargets_)
        detail::DeviceFramebuffer::releaseRenderTexture(entry.target);
    renderTargets_.clear();

    for (const auto &[id, shader] : shaders_)
        UnloadShader(shader);
    shaders_.clear();
}

RenderTargetHandle Device::createRenderTarget(const RenderTargetDesc &desc)
{
    if (desc.width <= 0 || desc.height <= 0) {
        TraceLog(LOG_WARNING,
            "RHI: descripteur de render target invalide (%ix%i)",
            desc.width, desc.height);
        return RenderTargetHandle{};
    }
    const RenderTexture2D target =
        detail::DeviceFramebuffer::createRenderTarget(desc);
    if (target.id == 0)
        return RenderTargetHandle{};
    const std::uint32_t id = nextRenderTargetId_++;
    renderTargets_.emplace(id, RenderTargetEntry{target, desc});
    return RenderTargetHandle{id};
}

void Device::destroyRenderTarget(RenderTargetHandle handle)
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return;
    detail::DeviceFramebuffer::releaseRenderTexture(it->second.target);
    renderTargets_.erase(it);
}

const RenderTexture2D *Device::getRenderTexture(
    RenderTargetHandle handle) const
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return nullptr;
    return &it->second.target;
}

const RenderTargetDesc *Device::getRenderTargetDesc(
    RenderTargetHandle handle) const
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return nullptr;
    return &it->second.desc;
}

void Device::beginRenderTarget(RenderTargetHandle handle)
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end()) {
        TraceLog(LOG_WARNING,
            "RHI: BeginRenderTarget sur handle inconnu (%u)", handle.id);
        return;
    }
    BeginTextureMode(it->second.target);
}

void Device::endRenderTarget()
{
    EndTextureMode();
}

ShaderRhiHandle Device::createShaderFromMemory(
    const char *vsSource, const char *fsSource)
{
    const Shader shader = LoadShaderFromMemory(vsSource, fsSource);
    if (shader.id == 0) {
        TraceLog(LOG_WARNING, "RHI: creation du shader impossible");
        return ShaderRhiHandle{};
    }
    const std::uint32_t id = nextShaderId_++;
    shaders_.emplace(id, shader);
    return ShaderRhiHandle{id};
}

void Device::destroyShader(ShaderRhiHandle handle)
{
    const auto it = shaders_.find(handle.id);
    if (it == shaders_.end())
        return;
    UnloadShader(it->second);
    shaders_.erase(it);
}

const Shader *Device::getShader(ShaderRhiHandle handle) const
{
    const auto it = shaders_.find(handle.id);
    if (it == shaders_.end())
        return nullptr;
    return &it->second;
}

} // namespace racer::engine
