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
constexpr int kSeaLevel = 16;
constexpr int kTreeMargin = 3; // marge depuis les bords du chunk pour que la canopee tienne entierement dedans

// Hash deterministe (position + seed) -> decide si une colonne porte un arbre.
// Pas de generateur d'etat partage : la meme position donne toujours le meme resultat,
// quel que soit l'ordre de generation des chunks.
bool ShouldPlaceTree(int worldX, int worldZ, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(worldX) * 374761393u + static_cast<uint32_t>(worldZ) * 668265263u + seed * 2654435761u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);
    return (h % 100) < 2; // ~2% des colonnes eligibles
}
} // namespace

ChunkGenerator::ChunkGenerator(uint32_t seed)
    : seed_(seed),
      heightNoise_(seed),
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

void ChunkGenerator::PlaceTreeIfEligible(common::world::Chunk& chunk, int lx, int lz, int worldX, int worldZ,
                                          int height, Biome biome) const {
    if (biome == Biome::Desert) return;
    if (height < kSeaLevel) return; // pas d'arbre sous l'eau
    if (lx < kTreeMargin || lx >= CHUNK_SIZE_X - kTreeMargin) return;
    if (lz < kTreeMargin || lz >= CHUNK_SIZE_Z - kTreeMargin) return;

    constexpr int kTrunkHeight = 4;
    constexpr int kCanopyRadius = 2;
    if (height + kTrunkHeight + kCanopyRadius + 1 >= CHUNK_SIZE_Y) return; // pas assez de place en hauteur

    if (!ShouldPlaceTree(worldX, worldZ, seed_)) return;

    for (int ty = height + 1; ty <= height + kTrunkHeight; ++ty) {
        chunk.blocks[common::world::BlockIndex(lx, ty, lz)] = Id(BlockId::Wood);
    }

    int canopyBaseY = height + kTrunkHeight - 1;
    for (int dx = -kCanopyRadius; dx <= kCanopyRadius; ++dx) {
        for (int dz = -kCanopyRadius; dz <= kCanopyRadius; ++dz) {
            for (int dy = 0; dy <= kCanopyRadius; ++dy) {
                if (std::abs(dx) == kCanopyRadius && std::abs(dz) == kCanopyRadius) continue; // coins arrondis
                int bx = lx + dx;
                int by = canopyBaseY + dy;
                int bz = lz + dz;
                if (bx < 0 || bx >= CHUNK_SIZE_X || bz < 0 || bz >= CHUNK_SIZE_Z || by >= CHUNK_SIZE_Y) continue;
                int idx = common::world::BlockIndex(bx, by, bz);
                if (chunk.blocks[idx] == Id(BlockId::Air)) {
                    chunk.blocks[idx] = Id(BlockId::Leaves);
                }
            }
        }
    }
}

common::world::Chunk ChunkGenerator::Generate(common::world::ChunkCoord coord) const {
    common::world::Chunk chunk;
    chunk.coord = coord;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            float worldXf = static_cast<float>(coord.x * CHUNK_SIZE_X + x);
            float worldZf = static_cast<float>(coord.z * CHUNK_SIZE_Z + z);

            Biome biome = BiomeAt(worldXf, worldZf);
            int height = HeightAt(worldXf, worldZf, biome);

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
                    float caveValue = caveNoise_.Noise3D(worldXf * 0.09f, static_cast<float>(y) * 0.09f, worldZf * 0.09f);
                    if (caveValue > 0.62f) {
                        id = BlockId::Air;
                    }
                }

                chunk.blocks[common::world::BlockIndex(x, y, z)] = Id(id);
            }

            // Eau statique : comble les creux sous le niveau de la mer.
            if (height < kSeaLevel) {
                for (int y = height + 1; y <= kSeaLevel && y < CHUNK_SIZE_Y; ++y) {
                    chunk.blocks[common::world::BlockIndex(x, y, z)] = Id(BlockId::Water);
                }
            } else {
                int worldX = coord.x * CHUNK_SIZE_X + x;
                int worldZ = coord.z * CHUNK_SIZE_Z + z;
                PlaceTreeIfEligible(chunk, x, z, worldX, worldZ, height, biome);
            }
        }
    }

    return chunk;
}

} // namespace server_core
