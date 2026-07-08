/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural chunk generation
*/

#include "World/Chunk/ChunkGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

#include "World/Aurelia/AureliaData.hpp"

namespace racer::world {

namespace {

constexpr float kBiomeBlend = 24.0f;
constexpr float kRoadRadius = 10.0f;

float fade(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float smoothstep(float edge0, float edge1, float x)
{
    if (edge1 <= edge0) {
        return x >= edge1 ? 1.0f : 0.0f;
    }
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return fade(t);
}

int idx(int ix, int iz)
{
    return iz * kChunkResolution + ix;
}

float latticeHash(int ix, int iz, int seed)
{
    unsigned int h = static_cast<unsigned int>(ix) * 374761393u
        + static_cast<unsigned int>(iz) * 668265263u
        + static_cast<unsigned int>(seed) * 2147483647u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return static_cast<float>(h & 0x00FFFFFFu)
        / static_cast<float>(0x01000000u);
}

// Road bed elevation profile, sampled at the same points used to paint the
// road. Two edges sharing a node must agree on its height exactly (or the
// roads visibly disconnect at the junction), which makes the endpoints hard
// constraints. Some terrain (e.g. steep volcano caldera edges) has more raw
// elevation change between its two endpoints than any max-grade clamp could
// resolve over that distance while still hitting both endpoints exactly —
// in that case a straight-line ramp between the endpoints is the smoothest
// connection mathematically possible. So the profile is built as that
// straight-line baseline (by arc length, always exact at both ends) plus a
// small terrain-following "wiggle" on top, magnitude-limited so it can only
// add gentle local variation without reintroducing a canyon or a cliff.
constexpr int kRoadProfileSamples = 64;
constexpr float kMaxWiggle = 2.5f;

std::array<float, kRoadProfileSamples + 1> roadHeightProfile(
    const RoadGraph &graph, int edgeIndex)
{
    std::array<float, kRoadProfileSamples + 1> raw{};
    std::array<Vector2, kRoadProfileSamples + 1> pts{};
    std::array<float, kRoadProfileSamples + 1> cumDist{};

    for (int s = 0; s <= kRoadProfileSamples; ++s) {
        float t = static_cast<float>(s)
            / static_cast<float>(kRoadProfileSamples);
        pts[static_cast<size_t>(s)] = graph.pointOnEdge(edgeIndex, t);
        raw[static_cast<size_t>(s)] = ChunkGenerator::sampleWorldHeight(
            pts[static_cast<size_t>(s)].x, pts[static_cast<size_t>(s)].y);
    }

    cumDist[0] = 0.0f;
    for (int s = 1; s <= kRoadProfileSamples; ++s) {
        float dx = pts[static_cast<size_t>(s)].x
            - pts[static_cast<size_t>(s - 1)].x;
        float dz = pts[static_cast<size_t>(s)].y
            - pts[static_cast<size_t>(s - 1)].y;
        cumDist[static_cast<size_t>(s)] = cumDist[static_cast<size_t>(s - 1)]
            + std::sqrt(dx * dx + dz * dz);
    }
    float total = cumDist[static_cast<size_t>(kRoadProfileSamples)];

    std::array<float, kRoadProfileSamples + 1> profile{};
    float h0 = raw.front();
    float hN = raw.back();
    for (size_t s = 0; s < profile.size(); ++s) {
        float u = total > 0.001f ? cumDist[s] / total : 0.0f;
        float baseline = lerp(h0, hN, u);
        float wiggle =
            std::clamp(raw[s] - baseline, -kMaxWiggle, kMaxWiggle);
        profile[s] = baseline + wiggle;
    }
    return profile;
}

float weightAbove(float value, float threshold, float blend)
{
    return smoothstep(threshold - blend, threshold + blend, value);
}

float weightBelow(float value, float threshold, float blend)
{
    return 1.0f - smoothstep(threshold - blend, threshold + blend, value);
}

} // namespace

float ChunkGenerator::noise2d(float x, float z, int seed)
{
    int ix0 = static_cast<int>(std::floor(x));
    int iz0 = static_cast<int>(std::floor(z));
    float fx = x - static_cast<float>(ix0);
    float fz = z - static_cast<float>(iz0);

    float v00 = latticeHash(ix0, iz0, seed);
    float v10 = latticeHash(ix0 + 1, iz0, seed);
    float v01 = latticeHash(ix0, iz0 + 1, seed);
    float v11 = latticeHash(ix0 + 1, iz0 + 1, seed);

    float sx = fx * fx * (3.0f - 2.0f * fx);
    float sz = fz * fz * (3.0f - 2.0f * fz);

    float a = v00 + (v10 - v00) * sx;
    float b = v01 + (v11 - v01) * sx;
    return a + (b - a) * sz;
}

float ChunkGenerator::fbm(float x, float z, int seed, int octaves)
{
    float sum = 0.0f;
    float amp = 1.0f;
    float freq = 1.0f;
    float norm = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        sum += noise2d(x * freq, z * freq, seed + i) * amp;
        norm += amp;
        amp *= 0.5f;
        freq *= 2.0f;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

float ChunkGenerator::calderaLift(float worldX, float worldZ)
{
    float dx = worldX - kCalderaCenterX;
    float dz = worldZ - kCalderaCenterZ;
    float dist = std::sqrt(dx * dx + dz * dz);
    float radius = 72.0f;
    float t = std::clamp(1.0f - dist / radius, 0.0f, 1.0f);
    return fade(t) * 18.0f;
}

BiomeWeights ChunkGenerator::biomeWeightsAt(float worldX, float worldZ)
{
    float volcano = weightAbove(worldZ, kChunkSize, kBiomeBlend);
    float port = weightAbove(worldX, kChunkSize, kBiomeBlend)
        * weightBelow(worldZ, kChunkSize, kBiomeBlend);
    float forestWest = weightBelow(worldX, 0.0f, kBiomeBlend);
    float forestNorth = weightBelow(worldZ, 0.0f, kBiomeBlend);
    float forest = std::max(forestWest, forestNorth);
    float coast = std::max(0.0f, 1.0f - volcano - port - forest);
    float sum = coast + forest + port + volcano;
    if (sum <= 0.0f) {
        return BiomeWeights{1.0f, 0.0f, 0.0f, 0.0f};
    }
    return BiomeWeights{
        coast / sum, forest / sum, port / sum, volcano / sum
    };
}

float ChunkGenerator::biomeBaseHeight(BiomeId biome, float worldX, float worldZ)
{
    float n = fbm(worldX * 0.012f, worldZ * 0.012f, static_cast<int>(biome) + 11, 4);

    switch (biome) {
    case BiomeId::COAST:
        return 3.5f + n * 9.0f;
    case BiomeId::FOREST:
        return 6.0f + n * 14.0f;
    case BiomeId::PORT:
        return 1.8f + n * 3.5f;
    case BiomeId::VOLCANO:
        return 12.0f + n * 32.0f + calderaLift(worldX, worldZ);
    default:
        return 0.0f;
    }
}

float ChunkGenerator::sampleWorldHeight(float worldX, float worldZ)
{
    BiomeWeights w = biomeWeightsAt(worldX, worldZ);
    float h = w.coast * biomeBaseHeight(BiomeId::COAST, worldX, worldZ)
        + w.forest * biomeBaseHeight(BiomeId::FOREST, worldX, worldZ)
        + w.port * biomeBaseHeight(BiomeId::PORT, worldX, worldZ)
        + w.volcano * biomeBaseHeight(BiomeId::VOLCANO, worldX, worldZ);
    return h;
}

SurfaceKind ChunkGenerator::pickSurface(BiomeId biome, float height, float worldX,
    float worldZ)
{
    float n = fbm(worldX * 0.02f, worldZ * 0.02f, static_cast<int>(biome) + 29, 3);

    switch (biome) {
    case BiomeId::COAST:
        if (height < 1.0f) {
            return SurfaceKind::SAND;
        }
        if (height > 5.0f && n > 0.45f) {
            return SurfaceKind::GRASS;
        }
        return (n > 0.62f) ? SurfaceKind::SAND : SurfaceKind::GRASS;
    case BiomeId::FOREST:
        if (height > 16.0f) {
            return SurfaceKind::ROCK;
        }
        return SurfaceKind::GRASS;
    case BiomeId::PORT:
        return SurfaceKind::ASPHALT;
    case BiomeId::VOLCANO:
        if (height > 24.0f) {
            return SurfaceKind::ROCK;
        }
        return (n > 0.55f) ? SurfaceKind::ROCK : SurfaceKind::GRAVEL;
    default:
        return SurfaceKind::GRASS;
    }
}

SplatWeights ChunkGenerator::splatWeightsFromSurface(SurfaceKind kind)
{
    SplatWeights out{};
    switch (kind) {
    case SurfaceKind::GRASS:
        out.grass = 1.0f;
        break;
    case SurfaceKind::ROCK:
        out.rock = 1.0f;
        break;
    case SurfaceKind::ASPHALT:
        out.asphalt = 1.0f;
        break;
    case SurfaceKind::SAND:
        out.sand = 1.0f;
        break;
    case SurfaceKind::GRAVEL:
        out.rock = 0.6f;
        out.sand = 0.4f;
        break;
    default:
        out.grass = 1.0f;
        break;
    }
    return out;
}

SplatWeights ChunkGenerator::splatWeightsAt(float worldX, float worldZ, float height)
{
    BiomeWeights bw = biomeWeightsAt(worldX, worldZ);
    SplatWeights out{};

    auto add = [&](BiomeId biome, float weight) {
        SurfaceKind sk = pickSurface(biome, height, worldX, worldZ);
        switch (sk) {
        case SurfaceKind::GRASS:
            out.grass += weight;
            break;
        case SurfaceKind::ROCK:
            out.rock += weight;
            break;
        case SurfaceKind::ASPHALT:
            out.asphalt += weight;
            break;
        case SurfaceKind::SAND:
            out.sand += weight;
            break;
        case SurfaceKind::GRAVEL:
            out.rock += weight * 0.6f;
            out.sand += weight * 0.4f;
            break;
        default:
            out.grass += weight;
            break;
        }
    };

    add(BiomeId::COAST, bw.coast);
    add(BiomeId::FOREST, bw.forest);
    add(BiomeId::PORT, bw.port);
    add(BiomeId::VOLCANO, bw.volcano);

    float sum = out.grass + out.rock + out.asphalt + out.sand;
    if (sum <= 0.0f) {
        return SplatWeights{1.0f, 0.0f, 0.0f, 0.0f};
    }
    out.grass /= sum;
    out.rock /= sum;
    out.asphalt /= sum;
    out.sand /= sum;
    return out;
}

void ChunkGenerator::scatterProps(ChunkData &chunk)
{
    int seed = chunk.id.x * 17 + chunk.id.z * 31;
    int target = 6;

    switch (chunk.biome) {
    case BiomeId::FOREST:
        target = 10;
        break;
    case BiomeId::PORT:
        target = 6;
        break;
    case BiomeId::VOLCANO:
        target = 7;
        break;
    default:
        target = 5;
        break;
    }

    constexpr float kMinDist = 7.0f;
    constexpr float kMinDistSq = kMinDist * kMinDist;
    constexpr int kMaxAttempts = 28;
    int placed = 0;

    for (int attempt = 0; placed < target && attempt < target * kMaxAttempts;
        ++attempt) {
        float lx = noise2d(
            static_cast<float>(attempt), static_cast<float>(placed), seed)
            * kChunkSize;
        float lz = noise2d(
            static_cast<float>(placed), static_cast<float>(attempt), seed + 7)
            * kChunkSize;
        bool crowded = false;

        for (const PropInstance &existing : chunk.props) {
            float dx = existing.localX - lx;
            float dz = existing.localZ - lz;
            if (dx * dx + dz * dz < kMinDistSq) {
                crowded = true;
                break;
            }
        }
        if (crowded) {
            continue;
        }

        int pix = static_cast<int>(lx / kChunkSize
            * static_cast<float>(kChunkResolution - 1));
        int piz = static_cast<int>(lz / kChunkSize
            * static_cast<float>(kChunkResolution - 1));
        pix = std::clamp(pix, 0, kChunkResolution - 1);
        piz = std::clamp(piz, 0, kChunkResolution - 1);
        if (chunk.splat[static_cast<size_t>(idx(pix, piz))]
            == SurfaceKind::ASPHALT) {
            continue;
        }

        float gy = sampleHeight(chunk, lx, lz);
        int ix = static_cast<int>(lx / kChunkSize
            * static_cast<float>(kChunkResolution - 1));
        int iz = static_cast<int>(lz / kChunkSize
            * static_cast<float>(kChunkResolution - 1));
        ix = std::clamp(ix, 0, kChunkResolution - 2);
        iz = std::clamp(iz, 0, kChunkResolution - 2);
        float hN = chunk.heightmap[static_cast<size_t>(idx(ix, iz + 1))];
        float hE = chunk.heightmap[static_cast<size_t>(idx(ix + 1, iz))];
        float slope = std::max(std::fabs(hN - gy), std::fabs(hE - gy));
        if (slope > 5.0f) {
            continue;
        }

        PropInstance prop{};
        prop.localX = lx;
        prop.localZ = lz;
        prop.yaw = noise2d(lx, lz, seed + attempt) * 6.28318f;
        prop.scale = 0.8f + noise2d(lz, lx, seed + attempt + 3) * 0.5f;

        switch (chunk.biome) {
        case BiomeId::FOREST:
            prop.type = 1;
            break;
        case BiomeId::PORT:
            prop.type = 2;
            break;
        case BiomeId::VOLCANO:
            prop.type = 3;
            break;
        default:
            prop.type = 0;
            break;
        }

        chunk.props.push_back(prop);
        ++placed;
    }
}

void ChunkGenerator::paintRoads(ChunkData &chunk)
{
    const RoadGraph &graph = AureliaData::roadGraph();
    float ox = static_cast<float>(chunk.id.x) * kChunkSize;
    float oz = static_cast<float>(chunk.id.z) * kChunkSize;
    float minX = ox;
    float maxX = ox + kChunkSize;
    float minZ = oz;
    float maxZ = oz + kChunkSize;
    float cellSize = kChunkSize / static_cast<float>(kChunkResolution - 1);

    // Where two edges converge near a shared node, each edge's independently
    // slope-limited profile only matches the other exactly at the node
    // itself — a naive "whichever edge painted last wins" overwrite creates
    // a seam a few meters out. Tracking the closest road sample per cell
    // (across all edges) instead makes the hand-off follow whichever edge
    // is actually nearest, which is inherently continuous.
    std::array<float, kChunkResolution * kChunkResolution> originalHeight =
        chunk.heightmap;
    std::array<float, kChunkResolution * kChunkResolution> bestDist{};
    bestDist.fill(kRoadRadius);

    for (size_t ei = 0; ei < graph.edges().size(); ++ei) {
        std::array<float, kRoadProfileSamples + 1> profile =
            roadHeightProfile(graph, static_cast<int>(ei));

        for (int s = 0; s <= kRoadProfileSamples; ++s) {
            float t = static_cast<float>(s)
                / static_cast<float>(kRoadProfileSamples);
            Vector2 p = graph.pointOnEdge(static_cast<int>(ei), t);
            if (p.x < minX - kRoadRadius || p.x > maxX + kRoadRadius
                || p.y < minZ - kRoadRadius || p.y > maxZ + kRoadRadius) {
                continue;
            }
            float bedHeight = profile[static_cast<size_t>(s)];
            float lx = p.x - ox;
            float lz = p.y - oz;
            int ixMin = std::clamp(
                static_cast<int>(std::floor((lx - kRoadRadius) / cellSize)),
                0, kChunkResolution - 1);
            int ixMax = std::clamp(
                static_cast<int>(std::ceil((lx + kRoadRadius) / cellSize)),
                0, kChunkResolution - 1);
            int izMin = std::clamp(
                static_cast<int>(std::floor((lz - kRoadRadius) / cellSize)),
                0, kChunkResolution - 1);
            int izMax = std::clamp(
                static_cast<int>(std::ceil((lz + kRoadRadius) / cellSize)),
                0, kChunkResolution - 1);
            for (int iz = izMin; iz <= izMax; ++iz) {
                for (int ix = ixMin; ix <= ixMax; ++ix) {
                    float cx = static_cast<float>(ix) * cellSize;
                    float cz = static_cast<float>(iz) * cellSize;
                    float dx = cx - lx;
                    float dz = cz - lz;
                    float dist = std::sqrt(dx * dx + dz * dz);
                    size_t i = static_cast<size_t>(idx(ix, iz));
                    if (dist >= kRoadRadius || dist >= bestDist[i]) {
                        continue;
                    }
                    bestDist[i] = dist;
                    float feather = 1.0f - fade(dist / kRoadRadius);
                    chunk.heightmap[i] =
                        lerp(originalHeight[i], bedHeight, feather);
                    chunk.splat[i] = SurfaceKind::ASPHALT;
                }
            }
        }
    }
}

ChunkData ChunkGenerator::generate(ChunkId id)
{
    ChunkData chunk{};
    chunk.id = id;
    chunk.biome = biomeForChunk(id.x, id.z);
    chunk.generated = true;

    float ox = static_cast<float>(id.x) * kChunkSize;
    float oz = static_cast<float>(id.z) * kChunkSize;

    for (int iz = 0; iz < kChunkResolution; ++iz) {
        for (int ix = 0; ix < kChunkResolution; ++ix) {
            float lx = static_cast<float>(ix)
                / static_cast<float>(kChunkResolution - 1) * kChunkSize;
            float lz = static_cast<float>(iz)
                / static_cast<float>(kChunkResolution - 1) * kChunkSize;
            float worldX = ox + lx;
            float worldZ = oz + lz;
            float h = sampleWorldHeight(worldX, worldZ);
            chunk.heightmap[static_cast<size_t>(idx(ix, iz))] = h;
            chunk.splat[static_cast<size_t>(idx(ix, iz))] =
                pickSurface(chunk.biome, h, worldX, worldZ);
        }
    }

    paintRoads(chunk);
    scatterProps(chunk);
    return chunk;
}

float ChunkGenerator::sampleHeight(const ChunkData &chunk, float localX,
    float localZ)
{
    localX = std::clamp(localX, 0.0f, kChunkSize);
    localZ = std::clamp(localZ, 0.0f, kChunkSize);
    float fx = localX / kChunkSize * static_cast<float>(kChunkResolution - 1);
    float fz = localZ / kChunkSize * static_cast<float>(kChunkResolution - 1);
    int x0 = static_cast<int>(std::floor(fx));
    int z0 = static_cast<int>(std::floor(fz));
    int x1 = std::min(x0 + 1, kChunkResolution - 1);
    int z1 = std::min(z0 + 1, kChunkResolution - 1);
    float tx = fx - static_cast<float>(x0);
    float tz = fz - static_cast<float>(z0);
    float h00 = chunk.heightmap[static_cast<size_t>(idx(x0, z0))];
    float h10 = chunk.heightmap[static_cast<size_t>(idx(x1, z0))];
    float h01 = chunk.heightmap[static_cast<size_t>(idx(x0, z1))];
    float h11 = chunk.heightmap[static_cast<size_t>(idx(x1, z1))];
    float hx0 = lerp(h00, h10, tx);
    float hx1 = lerp(h01, h11, tx);
    return lerp(hx0, hx1, tz);
}

SurfaceKind ChunkGenerator::sampleSurface(const ChunkData &chunk, float localX,
    float localZ)
{
    localX = std::clamp(localX, 0.0f, kChunkSize);
    localZ = std::clamp(localZ, 0.0f, kChunkSize);
    int ix = static_cast<int>(localX / kChunkSize
        * static_cast<float>(kChunkResolution - 1));
    int iz = static_cast<int>(localZ / kChunkSize
        * static_cast<float>(kChunkResolution - 1));
    ix = std::clamp(ix, 0, kChunkResolution - 1);
    iz = std::clamp(iz, 0, kChunkResolution - 1);
    return chunk.splat[static_cast<size_t>(idx(ix, iz))];
}

} // namespace racer::world
