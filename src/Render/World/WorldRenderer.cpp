/*
** EPITECH PROJECT, 2026
** racer
** File description:
** WorldRenderer — terrain mesh + decor passes
*/

#include "Render/World/WorldRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Render/World/WorldDecorDraw.hpp"
#include "Render/Track/TrackMeshBuilder.hpp"
#include "Engine/Render/ShaderLocations.hpp"
#include "World/Aurelia/AureliaBounds.hpp"
#include "World/Aurelia/AureliaData.hpp"
#include "World/Chunk/ChunkGenerator.hpp"

#include "raymath.h"
#include "rlgl.h"

namespace racer::world {

namespace {

constexpr float kPropDrawDist = 200.0f;
constexpr float kPoiDrawDist = 130.0f;
constexpr float kCollectibleDrawDist = 55.0f;
constexpr float kLandmarkDrawDist = 160.0f;
constexpr float kTrafficDrawDist = 160.0f;

float distXZ(Vector3 a, float x, float z)
{
    float dx = a.x - x;
    float dz = a.z - z;
    return std::sqrt(dx * dx + dz * dz);
}

int hmIdx(int ix, int iz)
{
    return iz * kChunkResolution + ix;
}

Vector3 chunkWorldOrigin(ChunkId id)
{
    return Vector3{
        static_cast<float>(id.x) * kChunkSize,
        0.0f,
        static_cast<float>(id.z) * kChunkSize
    };
}

unsigned char clampU8(int v)
{
    return static_cast<unsigned char>(std::clamp(v, 0, 255));
}

Color tintColor(Color c, float mul, int addR, int addG, int addB)
{
    return Color{
        clampU8(static_cast<int>(c.r * mul) + addR),
        clampU8(static_cast<int>(c.g * mul) + addG),
        clampU8(static_cast<int>(c.b * mul) + addB),
        c.a
    };
}

void biomeTerrainTints(BiomeId biome, Vector3 out[4])
{
    switch (biome) {
    case BiomeId::FOREST:
        out[0] = {0.72f, 1.18f, 0.62f};
        out[1] = {0.82f, 0.94f, 0.76f};
        out[2] = {0.88f, 0.92f, 0.96f};
        out[3] = {0.86f, 0.90f, 0.72f};
        break;
    case BiomeId::PORT:
        out[0] = {0.62f, 0.78f, 0.74f};
        out[1] = {0.72f, 0.74f, 0.80f};
        out[2] = {0.82f, 0.84f, 0.90f};
        out[3] = {0.78f, 0.74f, 0.68f};
        break;
    case BiomeId::VOLCANO:
        out[0] = {1.08f, 0.68f, 0.48f};
        out[1] = {0.62f, 0.54f, 0.50f};
        out[2] = {0.78f, 0.72f, 0.68f};
        out[3] = {1.14f, 0.72f, 0.52f};
        break;
    default:
        out[0] = {0.88f, 1.08f, 0.78f};
        out[1] = {0.94f, 0.90f, 0.84f};
        out[2] = {0.90f, 0.90f, 0.94f};
        out[3] = {1.06f, 0.92f, 0.74f};
        break;
    }
}

void drawHorizonRing(Vector3 focus, float groundY)
{
    rlPushMatrix();
    rlTranslatef(focus.x, 0.0f, focus.z);
    TrackMeshBuilder::drawMountainsRing(240.0f, groundY - 0.5f, 52);
    rlPopMatrix();
}

void drawBoundaryPosts(Vector3 focus, const ChunkStreamer &streamer)
{
    constexpr float kDrawDist = 220.0f;
    const float corners[4][2] = {
        {WorldBounds::minX, WorldBounds::minZ},
        {WorldBounds::maxX, WorldBounds::minZ},
        {WorldBounds::maxX, WorldBounds::maxZ},
        {WorldBounds::minX, WorldBounds::maxZ},
    };

    for (const float *c : corners) {
        float dx = focus.x - c[0];
        float dz = focus.z - c[1];
        if (dx * dx + dz * dz > kDrawDist * kDrawDist) {
            continue;
        }
        if (!streamer.isLoaded(c[0], c[1])) {
            continue;
        }
        float gy = streamer.sampleHeight(c[0], c[1]);
        DrawCube(Vector3{c[0], gy + 4.0f, c[1]}, 1.2f, 8.0f, 1.2f,
            Fade(ORANGE, 0.85f));
        DrawCube(Vector3{c[0], gy + 8.5f, c[1]}, 0.8f, 1.0f, 0.8f,
            Fade(RAYWHITE, 0.9f));
    }
}

} // namespace

WorldRenderer::WorldRenderer() = default;

WorldRenderer::~WorldRenderer()
{
    for (ChunkMesh &entry : meshes_) {
        freeMesh(entry);
    }
    for (LandmarkCache *cache :
        {&marinaLandmark_, &portLandmark_, &volcanoLandmark_,
            &forestLandmark_}) {
        if (cache->hasGeometry) {
            UnloadModel(cache->model);
            cache->hasGeometry = false;
        }
    }
}

void WorldRenderer::freeMesh(ChunkMesh &entry)
{
    if (entry.hasScatterModel) {
        UnloadModel(entry.scatterModel);
        entry.hasScatterModel = false;
    }
    if (!entry.ready) {
        return;
    }
    UnloadModel(entry.model);
    entry.ready = false;
}

Color WorldRenderer::surfaceColor(SurfaceKind kind, BiomeId biome, float height,
    float nx, float nz) const
{
    float n = ChunkGenerator::noise2d(nx * 8.0f, nz * 8.0f, static_cast<int>(biome) + 3);
    Color base = Color{110, 120, 95, 255};

    switch (biome) {
    case BiomeId::FOREST:
        if (kind == SurfaceKind::ROCK) {
            base = Color{82, 88, 78, 255};
        } else if (kind == SurfaceKind::ASPHALT) {
            base = Color{48, 50, 54, 255};
        } else {
            base = Color{34, 108, 48, 255};
        }
        break;
    case BiomeId::PORT:
        base = (kind == SurfaceKind::ASPHALT)
            ? Color{56, 58, 62, 255}
            : Color{78, 80, 86, 255};
        break;
    case BiomeId::VOLCANO:
        base = (kind == SurfaceKind::ROCK)
            ? Color{62, 56, 52, 255}
            : Color{92, 72, 58, 255};
        break;
    default:
        if (kind == SurfaceKind::SAND) {
            base = Color{228, 210, 150, 255};
        } else if (kind == SurfaceKind::ASPHALT) {
            base = Color{58, 60, 64, 255};
        } else {
            base = Color{96, 148, 72, 255};
        }
        break;
    }

    float hill = std::clamp(height / 22.0f, 0.0f, 1.0f);
    base = tintColor(base, 1.0f - hill * 0.12f, 0, 0, 0);
    int jitter = static_cast<int>((n - 0.5f) * 18.0f);
    return tintColor(base, 1.0f, jitter, jitter, jitter);
}

void WorldRenderer::buildMesh(ChunkMesh &entry, const ChunkData &data)
{
    freeMesh(entry);
    entry.id = data.id;
    entry.biome = data.biome;
    entry.props = data.props;
    entry.heights = data.heightmap;

    const int grid = kChunkResolution;
    const int quads = grid - 1;
    const int vertCount = quads * quads * 6;
    Vector3 origin = chunkWorldOrigin(data.id);
    float cell = kChunkSize / static_cast<float>(quads);

    std::vector<Vector3> gridPos(static_cast<size_t>(grid * grid));
    std::vector<Vector3> gridNormal(static_cast<size_t>(grid * grid));
    std::vector<unsigned char> gridColor(static_cast<size_t>(grid * grid * 4));
    std::vector<Vector2> gridUv(static_cast<size_t>(grid * grid));

    for (int iz = 0; iz < grid; ++iz) {
        for (int ix = 0; ix < grid; ++ix) {
            size_t gi = static_cast<size_t>(hmIdx(ix, iz));
            float lx = static_cast<float>(ix) / static_cast<float>(quads) * kChunkSize;
            float lz = static_cast<float>(iz) / static_cast<float>(quads) * kChunkSize;
            float worldX = origin.x + lx;
            float worldZ = origin.z + lz;
            float h = data.heightmap[gi];
            gridPos[gi] = Vector3{worldX, h, worldZ};
            gridUv[gi] = Vector2{worldX * 0.05f, worldZ * 0.05f};

            SplatWeights sw = ChunkGenerator::splatWeightsFromSurface(
                data.splat[gi]);
            gridColor[gi * 4 + 0] = clampU8(static_cast<int>(sw.grass * 255.0f));
            gridColor[gi * 4 + 1] = clampU8(static_cast<int>(sw.rock * 255.0f));
            gridColor[gi * 4 + 2] = clampU8(static_cast<int>(sw.asphalt * 255.0f));
            gridColor[gi * 4 + 3] = clampU8(static_cast<int>(sw.sand * 255.0f));
        }
    }

    for (int iz = 0; iz < grid; ++iz) {
        for (int ix = 0; ix < grid; ++ix) {
            int ixL = std::max(ix - 1, 0);
            int ixR = std::min(ix + 1, grid - 1);
            int izD = std::max(iz - 1, 0);
            int izU = std::min(iz + 1, grid - 1);
            float hL = gridPos[static_cast<size_t>(hmIdx(ixL, iz))].y;
            float hR = gridPos[static_cast<size_t>(hmIdx(ixR, iz))].y;
            float hD = gridPos[static_cast<size_t>(hmIdx(ix, izD))].y;
            float hU = gridPos[static_cast<size_t>(hmIdx(ix, izU))].y;
            float dx = static_cast<float>(ixR - ixL) * cell;
            float dz = static_cast<float>(izU - izD) * cell;
            Vector3 n{
                -(hR - hL) / std::max(dx, 0.001f),
                2.0f,
                -(hU - hD) / std::max(dz, 0.001f)
            };
            gridNormal[static_cast<size_t>(hmIdx(ix, iz))] = Vector3Normalize(n);
        }
    }

    std::vector<Vector3> vertices(static_cast<size_t>(vertCount));
    std::vector<Vector3> normals(static_cast<size_t>(vertCount));
    std::vector<unsigned char> colors(static_cast<size_t>(vertCount * 4));
    std::vector<Vector2> texcoords(static_cast<size_t>(vertCount));
    int v = 0;

    auto emitTri = [&](int a, int b, int c) {
        for (int vi : {a, b, c}) {
            size_t si = static_cast<size_t>(vi);
            vertices[static_cast<size_t>(v)] = gridPos[si];
            normals[static_cast<size_t>(v)] = gridNormal[si];
            colors[static_cast<size_t>(v * 4 + 0)] = gridColor[si * 4 + 0];
            colors[static_cast<size_t>(v * 4 + 1)] = gridColor[si * 4 + 1];
            colors[static_cast<size_t>(v * 4 + 2)] = gridColor[si * 4 + 2];
            colors[static_cast<size_t>(v * 4 + 3)] = gridColor[si * 4 + 3];
            texcoords[static_cast<size_t>(v)] = gridUv[si];
            ++v;
        }
    };

    for (int iz = 0; iz < quads; ++iz) {
        for (int ix = 0; ix < quads; ++ix) {
            int i00 = hmIdx(ix, iz);
            int i10 = hmIdx(ix + 1, iz);
            int i01 = hmIdx(ix, iz + 1);
            int i11 = hmIdx(ix + 1, iz + 1);
            emitTri(i00, i11, i10);
            emitTri(i00, i01, i11);
        }
    }

    entry.mesh = {0};
    entry.mesh.vertexCount = vertCount;
    entry.mesh.triangleCount = vertCount / 3;
    entry.mesh.vertices = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(vertCount * 3 * sizeof(float))));
    entry.mesh.normals = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(vertCount * 3 * sizeof(float))));
    entry.mesh.colors = static_cast<unsigned char *>(MemAlloc(
        static_cast<unsigned int>(vertCount * 4 * sizeof(unsigned char))));
    entry.mesh.texcoords = static_cast<float *>(MemAlloc(
        static_cast<unsigned int>(vertCount * 2 * sizeof(float))));

    for (int i = 0; i < vertCount; ++i) {
        entry.mesh.vertices[i * 3 + 0] = vertices[static_cast<size_t>(i)].x;
        entry.mesh.vertices[i * 3 + 1] = vertices[static_cast<size_t>(i)].y;
        entry.mesh.vertices[i * 3 + 2] = vertices[static_cast<size_t>(i)].z;
        entry.mesh.normals[i * 3 + 0] = normals[static_cast<size_t>(i)].x;
        entry.mesh.normals[i * 3 + 1] = normals[static_cast<size_t>(i)].y;
        entry.mesh.normals[i * 3 + 2] = normals[static_cast<size_t>(i)].z;
        entry.mesh.colors[i * 4 + 0] = colors[static_cast<size_t>(i * 4 + 0)];
        entry.mesh.colors[i * 4 + 1] = colors[static_cast<size_t>(i * 4 + 1)];
        entry.mesh.colors[i * 4 + 2] = colors[static_cast<size_t>(i * 4 + 2)];
        entry.mesh.colors[i * 4 + 3] = colors[static_cast<size_t>(i * 4 + 3)];
        entry.mesh.texcoords[i * 2 + 0] = texcoords[static_cast<size_t>(i)].x;
        entry.mesh.texcoords[i * 2 + 1] = texcoords[static_cast<size_t>(i)].y;
    }

    UploadMesh(&entry.mesh, false);
    entry.model = LoadModelFromMesh(entry.mesh);
    if (hasLitShader_) {
        for (int i = 0; i < entry.model.materialCount; ++i) {
            entry.model.materials[i].shader = litShader_;
        }
    }
    entry.ready = true;

    if (!entry.props.empty()) {
        std::vector<float> groundY(entry.props.size());
        for (size_t i = 0; i < entry.props.size(); ++i) {
            groundY[i] = sampleChunkHeight(
                entry, entry.props[i].localX, entry.props[i].localZ);
        }
        entry.scatterModel =
            propBuilder_.buildScatterModel(entry.props, origin, groundY);
        entry.hasScatterModel = entry.scatterModel.meshCount > 0;
        if (entry.hasScatterModel && hasLitShader_) {
            for (int i = 0; i < entry.scatterModel.materialCount; ++i) {
                entry.scatterModel.materials[i].shader = litShader_;
            }
        }
    }
}

float WorldRenderer::sampleChunkHeight(const ChunkMesh &entry, float localX,
    float localZ) const
{
    ChunkData chunk{};
    chunk.id = entry.id;
    chunk.biome = entry.biome;
    chunk.heightmap = entry.heights;
    return ChunkGenerator::sampleHeight(chunk, localX, localZ);
}

void WorldRenderer::applyShader(Shader shader)
{
    litShader_ = shader;
    hasLitShader_ = true;
    terrainModeLoc_ = GetShaderLocation(shader, "terrainMode");
    biomeTintLoc_ = racer::engine::locOrArray(shader, "biomeTint", "biomeTint[0]");
    propBuilder_.setShader(shader);
    for (ChunkMesh &entry : meshes_) {
        if (entry.ready) {
            for (int i = 0; i < entry.model.materialCount; ++i) {
                entry.model.materials[i].shader = shader;
            }
        }
        if (entry.hasScatterModel) {
            for (int i = 0; i < entry.scatterModel.materialCount; ++i) {
                entry.scatterModel.materials[i].shader = shader;
            }
        }
    }
    for (LandmarkCache *cache :
        {&marinaLandmark_, &portLandmark_, &volcanoLandmark_,
            &forestLandmark_}) {
        if (cache->hasGeometry) {
            for (int i = 0; i < cache->model.materialCount; ++i) {
                cache->model.materials[i].shader = shader;
            }
        }
    }
}

void WorldRenderer::sync(const ChunkStreamer &streamer)
{
    for (const ChunkData &chunk : streamer.loadedChunks()) {
        bool found = false;
        for (const ChunkMesh &m : meshes_) {
            if (m.id.x == chunk.id.x && m.id.z == chunk.id.z) {
                found = true;
                break;
            }
        }
        if (!found) {
            ChunkMesh entry{};
            buildMesh(entry, chunk);
            meshes_.push_back(entry);
        }
    }

    meshes_.erase(
        std::remove_if(meshes_.begin(), meshes_.end(),
            [&](ChunkMesh &m) {
                bool keep = false;
                for (const ChunkData &c : streamer.loadedChunks()) {
                    if (c.id.x == m.id.x && c.id.z == m.id.z) {
                        keep = true;
                        break;
                    }
                }
                if (!keep) {
                    freeMesh(m);
                }
                return !keep;
            }),
        meshes_.end());
}

void WorldRenderer::drawOpaque(BiomeId /*biome*/) const
{
    const float terrainOn = 1.0f;
    const float terrainOff = 0.0f;
    Vector3 tints[4]{};

    if (hasLitShader_ && terrainModeLoc_ >= 0) {
        SetShaderValue(litShader_, terrainModeLoc_, &terrainOn,
            SHADER_UNIFORM_FLOAT);
    }

    for (const ChunkMesh &entry : meshes_) {
        if (!entry.ready) {
            continue;
        }
        biomeTerrainTints(entry.biome, tints);
        if (hasLitShader_ && biomeTintLoc_ >= 0) {
            SetShaderValueV(litShader_, biomeTintLoc_, tints,
                SHADER_UNIFORM_VEC3, 4);
        }
        DrawModel(entry.model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    }

    if (hasLitShader_ && terrainModeLoc_ >= 0) {
        SetShaderValue(litShader_, terrainModeLoc_, &terrainOff,
            SHADER_UNIFORM_FLOAT);
    }
}

void WorldRenderer::drawLit(float timeSec, Vector3 focus,
    const ChunkStreamer &streamer) const
{
    (void)timeSec;

    float groundY = streamer.sampleHeight(focus.x, focus.z);
    drawHorizonRing(focus, groundY);
    drawBoundaryPosts(focus, streamer);

    drawCachedLandmark(marinaLandmark_, Vector3{24.0f, 0.0f, 18.0f},
        kLandmarkDrawDist, focus, streamer,
        &WorldPropBuilder::buildMarinaLandmarksModel);
    drawCachedLandmark(portLandmark_, Vector3{168.0f, 0.0f, -32.0f},
        kLandmarkDrawDist, focus, streamer,
        &WorldPropBuilder::buildPortLandmarksModel);
    drawCachedLandmark(volcanoLandmark_, Vector3{72.0f, 0.0f, 168.0f},
        kLandmarkDrawDist, focus, streamer,
        &WorldPropBuilder::buildVolcanoLandmarksModel);
    drawCachedLandmark(forestLandmark_, Vector3{-72.0f, 0.0f, -128.0f},
        kLandmarkDrawDist, focus, streamer,
        &WorldPropBuilder::buildForestLandmarksModel);

    rlDisableBackfaceCulling();
    for (const ChunkMesh &entry : meshes_) {
        if (!entry.hasScatterModel) {
            continue;
        }
        Vector3 origin = chunkWorldOrigin(entry.id);
        float dist = distXZ(focus, origin.x + kChunkSize * 0.5f,
            origin.z + kChunkSize * 0.5f);
        if (dist > kPropDrawDist + kChunkSize) {
            continue;
        }
        DrawModel(entry.scatterModel, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    }
    rlEnableBackfaceCulling();
}

void WorldRenderer::drawCachedLandmark(LandmarkCache &cache, Vector3 pos,
    float maxDist, Vector3 focus, const ChunkStreamer &streamer,
    LandmarkBuilder build) const
{
    if (distXZ(focus, pos.x, pos.z) > maxDist) {
        return;
    }
    if (!cache.built) {
        if (!streamer.isLoaded(pos.x, pos.z)) {
            return;
        }
        float gy = streamer.sampleHeight(pos.x, pos.z);
        cache.model = (propBuilder_.*build)(gy);
        cache.hasGeometry = cache.model.meshCount > 0;
        cache.built = true;
        if (cache.hasGeometry && hasLitShader_) {
            for (int i = 0; i < cache.model.materialCount; ++i) {
                cache.model.materials[i].shader = litShader_;
            }
        }
    }
    if (cache.hasGeometry) {
        rlDisableBackfaceCulling();
        DrawModel(cache.model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
        rlEnableBackfaceCulling();
    }
}

void WorldRenderer::drawTriggers(float timeSec, Vector3 focus,
    const ChunkStreamer &streamer) const
{
    propBuilder_.beginBatch(WorldPropBuilder::BatchId::POI);

    for (const PoiInstance &poi : AureliaData::worldPois()) {
        float dist = distXZ(focus, poi.worldX, poi.worldZ);
        float maxDist = (poi.type == PoiType::COLLECTIBLE)
            ? kCollectibleDrawDist
            : kPoiDrawDist;
        if (dist > maxDist) {
            continue;
        }
        if (!streamer.isLoaded(poi.worldX, poi.worldZ)) {
            continue;
        }
        float gy = streamer.sampleHeight(poi.worldX, poi.worldZ);
        switch (poi.type) {
        case PoiType::RACE_ENTRY:
            WorldDecorDraw::drawRaceGate(propBuilder_, poi, gy, timeSec);
            break;
        case PoiType::GARAGE:
            WorldDecorDraw::drawGarage(propBuilder_, poi, gy);
            break;
        case PoiType::MISSION_GIVER:
            WorldDecorDraw::drawMissionMarker(propBuilder_, poi, gy, timeSec);
            break;
        case PoiType::COLLECTIBLE: {
            float pulse = 0.7f + 0.3f * std::sin(timeSec * 5.0f + poi.loreIndex);
            DrawSphere(
                Vector3{poi.worldX, gy + 1.2f, poi.worldZ},
                0.45f * pulse, Fade(GOLD, 0.85f));
            break;
        }
        default:
            break;
        }
    }

    propBuilder_.flush(WorldPropBuilder::BatchId::POI);
}

void WorldRenderer::drawTraffic(const std::vector<TrafficVehicle> &vehicles,
    const RoadGraph &graph, Vector3 focus,
    const ChunkStreamer &streamer) const
{
    propBuilder_.beginBatch(WorldPropBuilder::BatchId::TRAFFIC);

    for (const TrafficVehicle &v : vehicles) {
        Vector2 p = graph.pointOnEdge(v.edgeIndex, v.t);
        if (distXZ(focus, p.x, p.y) > kTrafficDrawDist) {
            continue;
        }
        if (!streamer.isLoaded(p.x, p.y)) {
            continue;
        }
        float y = streamer.sampleHeight(p.x, p.y) + 0.28f;
        propBuilder_.placeTrafficCar(
            Vector3{p.x, y, p.y}, v.heading, v.color);
    }

    propBuilder_.flush(WorldPropBuilder::BatchId::TRAFFIC);
}

} // namespace racer::world
