/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Chunk heightmap and splat data
*/

#ifndef CHUNK_DATA_HPP_
#define CHUNK_DATA_HPP_

#include <array>
#include <vector>

#include "World/Aurelia/AureliaTypes.hpp"

namespace racer::world {

struct PropInstance {
    std::uint8_t type = 0;
    float localX = 0.0f;
    float localZ = 0.0f;
    float yaw = 0.0f;
    float scale = 1.0f;
};

struct ChunkData {
    ChunkId id{};
    BiomeId biome = BiomeId::COAST;
    std::array<float, kChunkResolution * kChunkResolution> heightmap{};
    std::array<SurfaceKind, kChunkResolution * kChunkResolution> splat{};
    std::vector<PoiInstance> pois;
    std::vector<PropInstance> props;
    bool generated = false;
};

} // namespace racer::world

#endif /* !CHUNK_DATA_HPP_ */
