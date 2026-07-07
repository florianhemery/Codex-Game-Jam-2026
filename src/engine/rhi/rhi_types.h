/// \file rhi_types.h
/// \brief Types de base du RHI : handles opaques types, formats, descripteurs.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace racer::engine {

/// Formats de pixels exposes par le RHI.
enum class RhiFormat : std::uint8_t {
    RGBA8,    ///< 8 bits par canal, non signe (LDR, defaut).
    RGBA16F,  ///< Demi-flottant par canal (cibles HDR).
    DEPTH24,  ///< Profondeur 24 bits echantillonnable (ex. shadow map).
};

namespace detail {

/// Handle opaque discrimine par un tag : id 0 = invalide.
/// Le parametre Tag empeche de melanger deux categories de handles.
template <typename Tag>
struct RhiHandle {
    std::uint32_t id = 0;

    [[nodiscard]] constexpr bool IsValid() const noexcept { return id != 0; }
    constexpr bool operator==(const RhiHandle&) const noexcept = default;
};

} // namespace detail

struct RenderTargetTag {};
struct ShaderRhiTag {};

/// Handle vers une cible de rendu (framebuffer + textures attachees).
using RenderTargetHandle = detail::RhiHandle<RenderTargetTag>;

/// Handle vers un programme shader (vertex + fragment).
using ShaderRhiHandle = detail::RhiHandle<ShaderRhiTag>;

/// Description d'une cible de rendu.
struct RenderTargetDesc {
    int width = 0;
    int height = 0;
    RhiFormat format = RhiFormat::RGBA8;
    bool useDepth = true;  ///< Attache un tampon de profondeur (toujours vrai pour DEPTH24).

    constexpr bool operator==(const RenderTargetDesc&) const noexcept = default;
};

/// Foncteur de hachage pour indexer un pool de cibles par descripteur.
struct RenderTargetDescHash {
    std::size_t operator()(const RenderTargetDesc& desc) const noexcept {
        const auto combine = [](std::size_t seed, std::size_t value) noexcept {
            return seed ^ (value + 0x9e3779b9u + (seed << 6) + (seed >> 2));
        };
        std::size_t seed = std::hash<int>{}(desc.width);
        seed = combine(seed, std::hash<int>{}(desc.height));
        seed = combine(seed, static_cast<std::size_t>(desc.format));
        seed = combine(seed, desc.useDepth ? 1u : 0u);
        return seed;
    }
};

} // namespace racer::engine
