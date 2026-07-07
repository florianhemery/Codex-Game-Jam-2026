/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Internal framebuffer assembly for Device (raylib/rlgl)
*/

#ifndef DEVICE_FRAMEBUFFER_HPP_
#define DEVICE_FRAMEBUFFER_HPP_

#include "Engine/Rhi/RhiTypes.hpp"

#include "raylib.h"

namespace racer::engine::detail {

class DeviceFramebuffer {
public:
    static RenderTexture2D createRenderTarget(const RenderTargetDesc &desc);
    static void releaseRenderTexture(const RenderTexture2D &target);

private:
    static int toRaylibColorFormat(RhiFormat format);
    static void setDepthTextureMeta(Texture2D &depth, int width, int height);
    static unsigned int loadColorAttachment(
        int width, int height, RhiFormat format, int &rlFormat);
    static void fillColorTextureMeta(
        RenderTexture2D &target, unsigned int colorId,
        int width, int height, int rlFormat);
    static void unloadFailedFramebuffer(
        unsigned int fbId, unsigned int colorId);
    static void attachDepthAttachment(
        RenderTexture2D &target, const RenderTargetDesc &desc);
    static void attachDepthOnlyAttachment(
        RenderTexture2D &target, const RenderTargetDesc &desc);
    static bool bindColorAttachments(
        RenderTexture2D &target, const RenderTargetDesc &desc,
        unsigned int &colorId);
    static bool isFramebufferComplete(
        unsigned int fbId, int width, int height);
    static bool openFramebuffer(
        RenderTexture2D &target, const RenderTargetDesc &desc);
    static bool setupDepthFramebuffer(
        RenderTexture2D &target, const RenderTargetDesc &desc);
    static RenderTexture2D createColorTarget(const RenderTargetDesc &desc);
    static RenderTexture2D createDepthTarget(const RenderTargetDesc &desc);
};

} // namespace racer::engine::detail

#endif /* !DEVICE_FRAMEBUFFER_HPP_ */
