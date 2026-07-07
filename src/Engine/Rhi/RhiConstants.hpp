/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Named constexpr values for the RHI module
*/

#ifndef RHI_CONSTANTS_HPP_
#define RHI_CONSTANTS_HPP_

#include <cstddef>
#include <cstdint>

namespace racer::engine::rhi_constants {

constexpr int kDepthTexturePixelFormat = 19;
constexpr int kSingleMipmapLevel = 1;
constexpr std::size_t kHashCombineSeed = 0x9e3779b9u;
constexpr std::uint32_t kInvalidHandleId = 0;

} // namespace racer::engine::rhi_constants

#endif /* !RHI_CONSTANTS_HPP_ */
