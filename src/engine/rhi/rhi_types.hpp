/*
** EPITECH PROJECT, 2026
** racer
** File description:
** RHI base types, handles, formats, descriptors
*/

#ifndef RHI_TYPES_HPP_
#define RHI_TYPES_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>

namespace racer::engine {

enum class RhiFormat : std::uint8_t {
    RGBA8,
    RGBA16F,
    DEPTH24,
};

namespace detail {

template <typename Tag>
class RhiHandle {
public:
    std::uint32_t id = 0;

    [[nodiscard]] constexpr bool IsValid() const noexcept
    {
        return id != 0;
    }

    constexpr bool operator==(const RhiHandle &) const noexcept = default;
};

} // namespace detail

struct RenderTargetTag {};
struct ShaderRhiTag {};

using RenderTargetHandle = detail::RhiHandle<RenderTargetTag>;
using ShaderRhiHandle = detail::RhiHandle<ShaderRhiTag>;

struct RenderTargetDesc {
    int width = 0;
    int height = 0;
    RhiFormat format = RhiFormat::RGBA8;
    bool useDepth = true;

    constexpr bool operator==(const RenderTargetDesc &) const noexcept = default;
};

class RenderTargetDescHash {
public:
    std::size_t operator()(const RenderTargetDesc &desc) const noexcept
    {
        const auto combine = [](std::size_t seed, std::size_t value) noexcept
        {
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

#endif /* !RHI_TYPES_HPP_ */
