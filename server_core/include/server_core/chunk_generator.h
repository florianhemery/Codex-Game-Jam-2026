#pragma once

#include <cstdint>

#include "common/noise/perlin.h"
#include "common/world/chunk.h"

namespace server_core {

enum class Biome {
    Plains,
    Desert,
    Mountains,
};

class ChunkGenerator {
public:
    explicit ChunkGenerator(uint32_t seed);

    common::world::Chunk Generate(common::world::ChunkCoord coord) const;

private:
    Biome BiomeAt(float worldX, float worldZ) const;
    int HeightAt(float worldX, float worldZ, Biome biome) const;
    void PlaceTreeIfEligible(common::world::Chunk& chunk, int lx, int lz, int worldX, int worldZ, int height, Biome biome) const;

    uint32_t seed_;
    common::noise::PerlinNoise heightNoise_;
    common::noise::PerlinNoise biomeNoise_;
    common::noise::PerlinNoise caveNoise_;
};

} // namespace server_core
