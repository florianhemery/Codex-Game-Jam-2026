/// \file device.cpp
/// \brief Implementation du Device sur backend raylib/rlgl (OpenGL 3.3).

#include "engine/rhi/device.h"

#include "raylib.h"
#include "rlgl.h"

namespace racer::engine {

namespace {

/// Traduit un format RHI couleur vers le format pixel rlgl correspondant.
int ToRlColorFormat(RhiFormat format) {
    switch (format) {
        case RhiFormat::RGBA16F: return RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
        case RhiFormat::RGBA8:
        default:                 return RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }
}

/// Cree un framebuffer complet a la main (equivalent LoadRenderTexture mais
/// avec format couleur parametrable et profondeur optionnelle).
///
/// Backend GL 3.3 : RGBA16F passe par rlLoadTexture avec le format
/// R16G16B16A16 (mappe sur GL_RGBA16F, toujours supporte par rlgl en GL33) ;
/// si la creation echoue on retombe sur R32G32B32A32 (GL_RGBA32F).
RenderTexture2D CreateColorTarget(const RenderTargetDesc& desc) {
    RenderTexture2D target = {};

    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        TraceLog(LOG_WARNING, "RHI: creation du framebuffer impossible (%ix%i)", desc.width, desc.height);
        return target;
    }

    rlEnableFramebuffer(target.id);

    int rlFormat = ToRlColorFormat(desc.format);
    unsigned int colorId = rlLoadTexture(nullptr, desc.width, desc.height, rlFormat, 1);
    if (colorId == 0 && desc.format == RhiFormat::RGBA16F) {
        // Repli HDR : certains drivers refusent le half-float, on tente le float 32 bits.
        TraceLog(LOG_WARNING, "RHI: RGBA16F indisponible, repli sur RGBA32F");
        rlFormat = RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        colorId = rlLoadTexture(nullptr, desc.width, desc.height, rlFormat, 1);
    }
    if (colorId == 0) {
        TraceLog(LOG_WARNING, "RHI: creation de la texture couleur impossible");
        rlDisableFramebuffer();
        rlUnloadFramebuffer(target.id);
        return RenderTexture2D{};
    }

    target.texture.id = colorId;
    target.texture.width = desc.width;
    target.texture.height = desc.height;
    target.texture.format = rlFormat;
    target.texture.mipmaps = 1;

    rlFramebufferAttach(target.id, colorId, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    if (desc.useDepth) {
        // Renderbuffer (non echantillonnable), comme LoadRenderTexture.
        target.depth.id = rlLoadTextureDepth(desc.width, desc.height, true);
        target.depth.width = desc.width;
        target.depth.height = desc.height;
        target.depth.format = 19;  // Valeur sentinelle raylib (DEPTH_COMPONENT_24BIT).
        target.depth.mipmaps = 1;
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    }

    if (!rlFramebufferComplete(target.id)) {
        TraceLog(LOG_WARNING, "RHI: framebuffer incomplet (%ix%i)", desc.width, desc.height);
        rlDisableFramebuffer();
        rlUnloadTexture(colorId);
        rlUnloadFramebuffer(target.id);  // Libere aussi l'attachement profondeur.
        return RenderTexture2D{};
    }

    rlDisableFramebuffer();
    return target;
}

/// Cible profondeur seule echantillonnable (ex. shadow map), suivant le motif
/// canonique raylib (exemple shaders_shadowmap).
RenderTexture2D CreateDepthTarget(const RenderTargetDesc& desc) {
    RenderTexture2D target = {};

    target.id = rlLoadFramebuffer();
    if (target.id == 0) {
        TraceLog(LOG_WARNING, "RHI: creation du framebuffer profondeur impossible");
        return target;
    }

    // Dimensions dans .texture pour que BeginTextureMode regle le viewport,
    // meme sans texture couleur attachee.
    target.texture.width = desc.width;
    target.texture.height = desc.height;

    rlEnableFramebuffer(target.id);

    target.depth.id = rlLoadTextureDepth(desc.width, desc.height, false);
    target.depth.width = desc.width;
    target.depth.height = desc.height;
    target.depth.format = 19;  // Valeur sentinelle raylib (DEPTH_COMPONENT_24BIT).
    target.depth.mipmaps = 1;
    rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    if (!rlFramebufferComplete(target.id)) {
        TraceLog(LOG_WARNING, "RHI: framebuffer profondeur incomplet (%ix%i)", desc.width, desc.height);
        rlDisableFramebuffer();
        rlUnloadFramebuffer(target.id);  // Libere aussi la texture profondeur.
        return RenderTexture2D{};
    }

    rlDisableFramebuffer();
    return target;
}

/// Libere le framebuffer et ses attachements (couleur a la main, profondeur
/// via rlUnloadFramebuffer qui la detruit automatiquement).
void ReleaseRenderTexture(const RenderTexture2D& target) {
    if (target.id == 0) return;
    if (target.texture.id != 0) rlUnloadTexture(target.texture.id);
    rlUnloadFramebuffer(target.id);
}

} // namespace

Device::~Device() {
    for (const auto& [id, entry] : renderTargets_) {
        ReleaseRenderTexture(entry.target);
    }
    renderTargets_.clear();

    for (const auto& [id, shader] : shaders_) {
        UnloadShader(shader);
    }
    shaders_.clear();
}

RenderTargetHandle Device::CreateRenderTarget(const RenderTargetDesc& desc) {
    if (desc.width <= 0 || desc.height <= 0) {
        TraceLog(LOG_WARNING, "RHI: descripteur de render target invalide (%ix%i)", desc.width, desc.height);
        return RenderTargetHandle{};
    }

    const RenderTexture2D target = (desc.format == RhiFormat::DEPTH24)
        ? CreateDepthTarget(desc)
        : CreateColorTarget(desc);
    if (target.id == 0) return RenderTargetHandle{};

    const std::uint32_t id = nextRenderTargetId_++;
    renderTargets_.emplace(id, RenderTargetEntry{target, desc});
    return RenderTargetHandle{id};
}

void Device::DestroyRenderTarget(RenderTargetHandle handle) {
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end()) return;
    ReleaseRenderTexture(it->second.target);
    renderTargets_.erase(it);
}

const RenderTexture2D* Device::GetRenderTexture(RenderTargetHandle handle) const {
    const auto it = renderTargets_.find(handle.id);
    return (it != renderTargets_.end()) ? &it->second.target : nullptr;
}

const RenderTargetDesc* Device::GetRenderTargetDesc(RenderTargetHandle handle) const {
    const auto it = renderTargets_.find(handle.id);
    return (it != renderTargets_.end()) ? &it->second.desc : nullptr;
}

void Device::BeginRenderTarget(RenderTargetHandle handle) {
    const auto it = renderTargets_.find(handle.id);
    if (it == renderTargets_.end()) {
        TraceLog(LOG_WARNING, "RHI: BeginRenderTarget sur handle inconnu (%u)", handle.id);
        return;
    }
    // BeginTextureMode purge le batch courant, bascule le FBO et regle
    // viewport + matrices : on reste coherent avec le pipeline raylib.
    BeginTextureMode(it->second.target);
}

void Device::EndRenderTarget() {
    EndTextureMode();
}

ShaderRhiHandle Device::CreateShaderFromMemory(const char* vsSource, const char* fsSource) {
    const Shader shader = LoadShaderFromMemory(vsSource, fsSource);
    if (shader.id == 0) {
        TraceLog(LOG_WARNING, "RHI: creation du shader impossible");
        return ShaderRhiHandle{};
    }

    const std::uint32_t id = nextShaderId_++;
    shaders_.emplace(id, shader);
    return ShaderRhiHandle{id};
}

void Device::DestroyShader(ShaderRhiHandle handle) {
    const auto it = shaders_.find(handle.id);
    if (it == shaders_.end()) return;
    UnloadShader(it->second);  // No-op si c'est le shader par defaut raylib.
    shaders_.erase(it);
}

const Shader* Device::GetShader(ShaderRhiHandle handle) const {
    const auto it = shaders_.find(handle.id);
    return (it != shaders_.end()) ? &it->second : nullptr;
}

} // namespace racer::engine
