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

    RenderTargetHandle CreateRenderTarget(const RenderTargetDesc &desc);
    void DestroyRenderTarget(RenderTargetHandle handle);
    const RenderTexture2D *GetRenderTexture(
        RenderTargetHandle handle) const;
    const RenderTargetDesc *GetRenderTargetDesc(
        RenderTargetHandle handle) const;
    void BeginRenderTarget(RenderTargetHandle handle);
    void EndRenderTarget();

    ShaderRhiHandle CreateShaderFromMemory(
        const char *vsSource, const char *fsSource);
    void DestroyShader(ShaderRhiHandle handle);
    const Shader *GetShader(ShaderRhiHandle handle) const;

private:
    struct RenderTargetEntry {
        RenderTexture2D target{};
        RenderTargetDesc desc{};
    };

    static int toRlColorFormat(RhiFormat format);
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
    static RenderTexture2D createColorTarget(const RenderTargetDesc &desc);
    static RenderTexture2D createDepthTarget(const RenderTargetDesc &desc);
    static void releaseRenderTexture(const RenderTexture2D &target);

    std::unordered_map<std::uint32_t, RenderTargetEntry> renderTargets_;
    std::unordered_map<std::uint32_t, Shader> shaders_;
    std::uint32_t nextRenderTargetId_ = 1;
    std::uint32_t nextShaderId_ = 1;
};

} // namespace racer::engine

#endif /* !DEVICE_HPP_ */
