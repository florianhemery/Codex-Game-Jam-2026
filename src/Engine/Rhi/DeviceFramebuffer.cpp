/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Framebuffer assembly implementation (raylib/rlgl backend)
*/

#include "Engine/Rhi/DeviceFramebuffer.hpp"

#include "Engine/Rhi/RhiConstants.hpp"

#include "raylib.h"
#include "rlgl.h"

namespace racer::engine::detail {

using namespace rhi_constants;

int DeviceFramebuffer::toRaylibColorFormat(RhiFormat format)
{
    switch (format) {
        case RhiFormat::RGBA16F:
            return RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
        case RhiFormat::RGBA8:
        default:
            return RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }
}

unsigned int DeviceFramebuffer::loadColorAttachment(
    int width, int height, RhiFormat format, int &rlFormat)
{
    rlFormat = toRaylibColorFormat(format);
    unsigned int colorId = rlLoadTexture(
        nullptr, width, height, rlFormat, kSingleMipmapLevel);
    if (colorId == 0 && format == RhiFormat::RGBA16F) {
        TraceLog(LOG_WARNING,
            "RHI: RGBA16F indisponible, repli sur RGBA32F");
        rlFormat = RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        colorId = rlLoadTexture(
            nullptr, width, height, rlFormat, kSingleMipmapLevel);
    }
    return colorId;
}

void DeviceFramebuffer::fillColorTextureMeta(
    RenderTexture2D &target, unsigned int colorId,
    int width, int height, int rlFormat)
{
    target.texture.id = colorId;
    target.texture.width = width;
    target.texture.height = height;
    target.texture.format = rlFormat;
    target.texture.mipmaps = kSingleMipmapLevel;
}

void DeviceFramebuffer::setDepthTextureMeta(
    Texture2D &depth, int width, int height)
{
    depth.width = width;
    depth.height = height;
    depth.format = kDepthTexturePixelFormat;
    depth.mipmaps = kSingleMipmapLevel;
}

void DeviceFramebuffer::unloadFailedFramebuffer(
    unsigned int fbId, unsigned int colorId)
{
    rlDisableFramebuffer();
    if (colorId != kInvalidHandleId)
        rlUnloadTexture(colorId);
    rlUnloadFramebuffer(fbId);
}

void DeviceFramebuffer::attachDepthAttachment(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.depth.id = rlLoadTextureDepth(
        desc.width, desc.height, true);
    setDepthTextureMeta(target.depth, desc.width, desc.height);
    rlFramebufferAttach(target.id, target.depth.id,
        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
}

void DeviceFramebuffer::attachDepthOnlyAttachment(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.depth.id = rlLoadTextureDepth(
        desc.width, desc.height, false);
    setDepthTextureMeta(target.depth, desc.width, desc.height);
    rlFramebufferAttach(target.id, target.depth.id,
        RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
}

bool DeviceFramebuffer::bindColorAttachments(
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

bool DeviceFramebuffer::isFramebufferComplete(
    unsigned int fbId, int width, int height)
{
    if (rlFramebufferComplete(fbId))
        return true;
    TraceLog(LOG_WARNING,
        "RHI: framebuffer incomplet (%ix%i)", width, height);
    return false;
}

bool DeviceFramebuffer::openFramebuffer(
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

RenderTexture2D DeviceFramebuffer::createColorTarget(
    const RenderTargetDesc &desc)
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

bool DeviceFramebuffer::setupDepthFramebuffer(
    RenderTexture2D &target, const RenderTargetDesc &desc)
{
    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        TraceLog(LOG_WARNING,
            "RHI: creation du framebuffer profondeur impossible");
        return false;
    }
    target.texture.width = desc.width;
    target.texture.height = desc.height;
    rlEnableFramebuffer(target.id);
    attachDepthOnlyAttachment(target, desc);
    return true;
}

RenderTexture2D DeviceFramebuffer::createDepthTarget(
    const RenderTargetDesc &desc)
{
    RenderTexture2D target = {};

    if (!setupDepthFramebuffer(target, desc))
        return target;
    if (!isFramebufferComplete(target.id, desc.width, desc.height)) {
        rlDisableFramebuffer();
        rlUnloadFramebuffer(target.id);
        return RenderTexture2D{};
    }
    rlDisableFramebuffer();
    return target;
}

RenderTexture2D DeviceFramebuffer::createRenderTarget(
    const RenderTargetDesc &desc)
{
    return (desc.format == RhiFormat::DEPTH24)
        ? createDepthTarget(desc)
        : createColorTarget(desc);
}

void DeviceFramebuffer::releaseRenderTexture(
    const RenderTexture2D &target)
{
    if (target.id == 0)
        return;
    if (target.texture.id != 0)
        rlUnloadTexture(target.texture.id);
    rlUnloadFramebuffer(target.id);
}

} // namespace racer::engine::detail
