#include "server_core/chunk_generator.h"

#include <algorithm>
#include <cmath>

#include "common/world/block.h"

namespace server_core {

using common::world::BlockId;
using common::world::CHUNK_SIZE_X;
using common::world::CHUNK_SIZE_Y;
using common::world::CHUNK_SIZE_Z;

namespace {
constexpr uint8_t Id(BlockId b) { return static_cast<uint8_t>(b); }
} // namespace

ChunkGenerator::ChunkGenerator(uint32_t seed)
    : heightNoise_(seed),
      biomeNoise_(seed + 1),
      caveNoise_(seed + 2) {}

Biome ChunkGenerator::BiomeAt(float worldX, float worldZ) const {
    float value = biomeNoise_.Fbm2D(worldX * 0.004f, worldZ * 0.004f, 3, 2.0f, 0.5f);
    if (value < -0.2f) return Biome::Desert;
    if (value > 0.25f) return Biome::Mountains;
    return Biome::Plains;
}

int ChunkGenerator::HeightAt(float worldX, float worldZ, Biome biome) const {
    float fbm = heightNoise_.Fbm2D(worldX * 0.02f, worldZ * 0.02f, 4, 2.0f, 0.5f);

    float base = 20.0f;
    float amplitude = 6.0f;
    switch (biome) {
        case Biome::Desert:
            base = 16.0f;
            amplitude = 3.0f;
            break;
        case Biome::Mountains:
            base = 30.0f;
            amplitude = 26.0f;
            break;
        case Biome::Plains:
        default:
            break;
    }

    int height = static_cast<int>(base + fbm * amplitude);
    return std::clamp(height, 3, CHUNK_SIZE_Y - 4);
}

common::world::Chunk ChunkGenerator::Generate(common::world::ChunkCoord coord) const {
    common::world::Chunk chunk;
    chunk.coord = coord;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            float worldX = static_cast<float>(coord.x * CHUNK_SIZE_X + x);
            float worldZ = static_cast<float>(coord.z * CHUNK_SIZE_Z + z);

            Biome biome = BiomeAt(worldX, worldZ);
            int height = HeightAt(worldX, worldZ, biome);

            BlockId surfaceBlock = (biome == Biome::Desert) ? BlockId::Sand : BlockId::Grass;
            BlockId subsurfaceBlock = (biome == Biome::Desert) ? BlockId::Sand : BlockId::Dirt;

            for (int y = 0; y <= height && y < CHUNK_SIZE_Y; ++y) {
                BlockId id;
                if (y == height) {
                    id = surfaceBlock;
                } else if (y >= height - 3) {
                    id = subsurfaceBlock;
                } else {
                    id = BlockId::Stone;
                }

                // Grottes : creusees uniquement dans la roche profonde, jamais dans
                // les 3 blocs sous la surface, pour eviter des trous beants au sol.
                if (y < height - 3 && y >= 1) {
                    float caveValue = caveNoise_.Noise3D(worldX * 0.09f, static_cast<float>(y) * 0.09f, worldZ * 0.09f);
                    if (caveValue > 0.62f) {
                        id = BlockId::Air;
                    }
                }

                chunk.blocks[common::world::BlockIndex(x, y, z)] = Id(id);
            }
        }
    }

    return chunk;
}

} // namespace server_core
