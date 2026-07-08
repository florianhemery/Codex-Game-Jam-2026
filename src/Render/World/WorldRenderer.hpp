/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Terrain and prop rendering for streamed chunks
*/

#ifndef WORLD_RENDERER_HPP_
#define WORLD_RENDERER_HPP_

#include "raylib.h"

#include "Render/World/WorldPropBuilder.hpp"
#include "World/Chunk/ChunkData.hpp"
#include "World/Road/RoadGraph.hpp"
#include "World/Sim/TrafficSystem.hpp"
#include "World/Stream/ChunkStreamer.hpp"

namespace racer::world {

class WorldRenderer {
public:
    WorldRenderer();
    ~WorldRenderer();

    WorldRenderer(const WorldRenderer &) = delete;
    WorldRenderer &operator=(const WorldRenderer &) = delete;

    void sync(const ChunkStreamer &streamer);
    void applyShader(Shader shader);
    void drawOpaque(BiomeId biome = BiomeId::COAST) const;
    void drawLit(float timeSec, Vector3 focus,
        const ChunkStreamer &streamer) const;
    void drawTriggers(float timeSec, Vector3 focus,
        const ChunkStreamer &streamer) const;
    void drawTraffic(const std::vector<TrafficVehicle> &vehicles,
        const RoadGraph &graph, Vector3 focus,
        const ChunkStreamer &streamer) const;

private:
    struct ChunkMesh {
        ChunkId id{};
        BiomeId biome = BiomeId::COAST;
        std::array<float, kChunkResolution * kChunkResolution> heights{};
        std::vector<PropInstance> props;
        Mesh mesh{};
        Model model{};
        bool ready = false;
        Model scatterModel{};
        bool hasScatterModel = false;
    };

    struct LandmarkCache {
        Model model{};
        bool built = false;
        bool hasGeometry = false;
    };

    using LandmarkBuilder = Model (WorldPropBuilder::*)(float) const;

    Color surfaceColor(SurfaceKind kind, BiomeId biome, float height, float nx,
        float nz) const;
    float sampleChunkHeight(const ChunkMesh &entry, float localX,
        float localZ) const;
    void buildMesh(ChunkMesh &entry, const ChunkData &data);
    void freeMesh(ChunkMesh &entry);
    void drawCachedLandmark(LandmarkCache &cache, Vector3 pos, float maxDist,
        Vector3 focus, const ChunkStreamer &streamer,
        LandmarkBuilder build) const;

    std::vector<ChunkMesh> meshes_;
    mutable WorldPropBuilder propBuilder_;
    mutable LandmarkCache marinaLandmark_;
    mutable LandmarkCache portLandmark_;
    mutable LandmarkCache volcanoLandmark_;
    mutable LandmarkCache forestLandmark_;
    Shader litShader_{};
    bool hasLitShader_ = false;
    int terrainModeLoc_ = -1;
    int biomeTintLoc_ = -1;
};

} // namespace racer::world

#endif /* !WORLD_RENDERER_HPP_ */
