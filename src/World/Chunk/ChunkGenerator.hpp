/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural chunk heightmap and splat generation
*/

#ifndef CHUNK_GENERATOR_HPP_
#define CHUNK_GENERATOR_HPP_

#include "World/Chunk/ChunkData.hpp"

namespace racer::world {

struct BiomeWeights {
    float coast = 0.0f;
    float forest = 0.0f;
    float port = 0.0f;
    float volcano = 0.0f;
};

struct SplatWeights {
    float grass = 0.0f;
    float rock = 0.0f;
    float asphalt = 0.0f;
    float sand = 0.0f;
};

constexpr float kCalderaCenterX = 64.0f;
constexpr float kCalderaCenterZ = 192.0f;

class ChunkGenerator {
public:
    static ChunkData generate(ChunkId id);
    static float sampleHeight(const ChunkData &chunk, float localX, float localZ);
    static SurfaceKind sampleSurface(const ChunkData &chunk, float localX,
        float localZ);
    static float noise2d(float x, float z, int seed);
    static float sampleWorldHeight(float worldX, float worldZ);
    static BiomeWeights biomeWeightsAt(float worldX, float worldZ);
    static SplatWeights splatWeightsAt(float worldX, float worldZ, float height);
    static SplatWeights splatWeightsFromSurface(SurfaceKind kind);

private:
    static float fbm(float x, float z, int seed, int octaves);
    static float biomeBaseHeight(BiomeId biome, float worldX, float worldZ);
    static float calderaLift(float worldX, float worldZ);
    static SurfaceKind pickSurface(BiomeId biome, float height, float worldX,
        float worldZ);
    static void scatterProps(ChunkData &chunk);
    static void paintRoads(ChunkData &chunk);
};

} // namespace racer::world

#endif /* !CHUNK_GENERATOR_HPP_ */
