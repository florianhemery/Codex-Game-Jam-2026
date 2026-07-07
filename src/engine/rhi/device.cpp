/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Implementation du Device sur backend raylib/rlgl (OpenGL 3.3)
*/

#include "engine/rhi/device.hpp"

#include "raylib.h"
#include "rlgl.h"

namespace racer::engine {

int Device::toRlColorFormat(RhiFormat format)
{
    switch (format) {
        case RhiFormat::RGBA16F:
            return RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
        case RhiFormat::RGBA8:
        default:
            return RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }
}

unsigned int Device::loadColorAttachment(
    int width, int height, RhiFormat format, int &rlFormat)
{
    rlFormat = toRlColorFormat(format);
    unsigned int colorId = rlLoadTexture(
        nullptr, width, height, rlFormat, 1);
    if (colorId == 0 && format == RhiFormat::RGBA16F) {
        TraceLog(LOG_WARNING,
            "RHI: RGBA16F indisponible, repli sur RGBA32F");
        rlFormat = RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        colorId = rlLoadTexture(
            nullptr, width, height, rlFormat, 1);
    }
    return colorId;
}

void Device::fillColorTextureMeta(
    RenderTexture2D &target, unsigned int colorId,
    int width, int height, int rlFormat)
{
    target.texture.id = colorId;
    target.texture.width = width;
    target.texture.height = height;
    target.texture.format = rlFormat;
    target.texture.mipmaps = 1;
}

void Device::unloadFailedFramebuffer(
    unsigned int fbId, unsigned int colorId)
{
    rlDisableFramebuffer();
    if (colorId != 0)
        rlUnloadTexture(colorId);
    rlUnloadFramebuffer(fbId);
}

void Device::attachDepthAttachment(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.depth.id = rlLoadTextureDepth(
        desc.width, desc.height, true);
    target.depth.width = desc.width;
    target.depth.height = desc.height;
    target.depth.format = 19;
    target.depth.mipmaps = 1;
    rlFramebufferAttach(target.id, target.depth.id,
        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
}

void Device::attachDepthOnlyAttachment(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.depth.id = rlLoadTextureDepth(
        desc.width, desc.height, false);
    target.depth.width = desc.width;
    target.depth.height = desc.height;
    target.depth.format = 19;
    target.depth.mipmaps = 1;
    rlFramebufferAttach(target.id, target.depth.id,
        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
}

bool Device::bindColorAttachments(
    RenderTexture2D &target, const RenderTargetDesc &desc,
    unsigned int &colorId)
{
    int rlFormat = 0;

    colorId = loadColorAttachment(
        desc.width, desc.height, desc.format, rlFormat);
    if (colorId == 0) {
        TraceLog(LOG_WARNING,
            "RHI: creation de la texture couleur impossible");
        return false;
    }
    fillColorTextureMeta(
        target, colorId, desc.width, desc.height, rlFormat);
    rlFramebufferAttach(target.id, colorId,
        RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    if (desc.useDepth)
        attachDepthAttachment(target, desc);
    return true;
}

bool Device::isFramebufferComplete(
    unsigned int fbId, int width, int height)
{
    if (rlFramebufferComplete(fbId))
        return true;
    TraceLog(LOG_WARNING,
        "RHI: framebuffer incomplet (%ix%i)", width, height);
    return false;
}

bool Device::openFramebuffer(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        TraceLog(LOG_WARNING,
            "RHI: creation du framebuffer impossible (%ix%i)",
            desc.width, desc.height);
        return false;
    }
    rlEnableFramebuffer(target.id);
    return true;
}

RenderTexture2D Device::createColorTarget(const RenderTargetDesc &desc)
{
    RenderTexture2D target = {};

    if (!openFramebuffer(target, desc))
        return target;
    unsigned int colorId = 0;
    if (!bindColorAttachments(target, desc, colorId)) {
        unloadFailedFramebuffer(target.id, 0);
        return RenderTexture2D{};
    }
    if (!isFramebufferComplete(target.id, desc.width, desc.height)) {
        unloadFailedFramebuffer(target.id, colorId);
        return RenderTexture2D{};
    }
    rlDisableFramebuffer();
    return target;
}

RenderTexture2D Device::createDepthTarget(const RenderTargetDesc &desc)
{
    RenderTexture2D target = {};

    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        TraceLog(LOG_WARNING,
            "RHI: creation du framebuffer profondeur impossible");
        return target;
    }
    target.texture.width = desc.width;
    target.texture.height = desc.height;
    rlEnableFramebuffer(target.id);
    attachDepthOnlyAttachment(target, desc);
    if (!isFramebufferComplete(target.id, desc.width, desc.height)) {
        rlDisableFramebuffer();
        rlUnloadFramebuffer(target.id);
        return RenderTexture2D{};
    }
    rlDisableFramebuffer();
    return target;
}

void Device::releaseRenderTexture(const RenderTexture2D &target)
{
    if (target.id == 0)
        return;
    if (target.texture.id != 0)
        rlUnloadTexture(target.texture.id);
    rlUnloadFramebuffer(target.id);
}

Device::~Device()
{
    for (const auto &[id, entry] : renderTargets_)
        releaseRenderTexture(entry.target);
    renderTargets_.clear();

    for (const auto &[id, shader] : shaders_)
        UnloadShader(shader);
    shaders_.clear();
}

RenderTargetHandle Device::CreateRenderTarget(const RenderTargetDesc &desc)
{
    if (desc.width <= 0 || desc.height <= 0) {
        TraceLog(LOG_WARNING,
            "RHI: descripteur de render target invalide (%ix%i)",
            desc.width, desc.height);
        return RenderTargetHandle{};
    }
    const RenderTexture2D target = (desc.format == RhiFormat::DEPTH24)
        ? createDepthTarget(desc)
        : createColorTarget(desc);
    if (target.id == 0)
        return RenderTargetHandle{};
    const std::uint32_t id = nextRenderTargetId_++;
    renderTargets_.emplace(id, RenderTargetEntry{target, desc});
    return RenderTargetHandle{id};
}

void Device::DestroyRenderTarget(RenderTargetHandle handle)
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return;
    releaseRenderTexture(it->second.target);
    renderTargets_.erase(it);
}

const RenderTexture2D *Device::GetRenderTexture(
    RenderTargetHandle handle) const
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return nullptr;
    return &it->second.target;
}

const RenderTargetDesc *Device::GetRenderTargetDesc(
    RenderTargetHandle handle) const
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end())
        return nullptr;
    return &it->second.desc;
}

void Device::BeginRenderTarget(RenderTargetHandle handle)
{
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end()) {
        TraceLog(LOG_WARNING,
            "RHI: BeginRenderTarget sur handle inconnu (%u)", handle.id);
        return;
    }
    BeginTextureMode(it->second.target);
}

void Device::EndRenderTarget()
{
    EndTextureMode();
}

ShaderRhiHandle Device::CreateShaderFromMemory(
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

void Device::DestroyShader(ShaderRhiHandle handle)
{
    const auto it = shaders_.find(handle.id);
    if (it == shaders_.end())
        return;
    UnloadShader(it->second);
    shaders_.erase(it);
}

const Shader *Device::GetShader(ShaderRhiHandle handle) const
{
    const auto it = shaders_.find(handle.id);
    if (it == shaders_.end())
        return nullptr;
    return &it->second;
}

} // namespace racer::engine
