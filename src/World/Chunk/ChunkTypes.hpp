/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurélia chunk identifiers and constants
*/

#ifndef CHUNK_TYPES_HPP_
#define CHUNK_TYPES_HPP_

#include <cmath>
#include <cstdint>
#include <functional>

namespace racer::world {

constexpr float kChunkSize = 128.0f;
constexpr int kChunkResolution = 33;
constexpr int kStreamRadius = 1;

enum class BiomeId : std::uint8_t {
    COAST = 0,
    FOREST,
    PORT,
    VOLCANO,
    COUNT
};

enum class SurfaceKind : std::uint8_t {
    ASPHALT = 0,
    GRASS,
    SAND,
    ROCK,
    GRAVEL
};

struct ChunkId {
    int x = 0;
    int z = 0;

    bool operator==(const ChunkId &o) const
    {
        return x == o.x && z == o.z;
    }
};

struct ChunkIdHash {
    std::size_t operator()(const ChunkId &id) const noexcept
    {
        return static_cast<std::size_t>(id.x * 73856093) ^
            static_cast<std::size_t>(id.z * 19349663);
    }
};

inline ChunkId worldToChunkId(float worldX, float worldZ)
{
    return ChunkId{
        static_cast<int>(std::floor(worldX / kChunkSize)),
        static_cast<int>(std::floor(worldZ / kChunkSize))
    };
}

const char *biomeLabel(BiomeId biome);
const char *regionLabel(BiomeId biome);

} // namespace racer::world

#endif /* !CHUNK_TYPES_HPP_ */
