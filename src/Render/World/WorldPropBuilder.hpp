/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Procedural world prop mesh builder (batched per category)
*/

#ifndef WORLD_PROP_BUILDER_HPP_
#define WORLD_PROP_BUILDER_HPP_

#include <array>
#include <cstdint>

#include "raylib.h"

#include "Render/Track/TrackMeshBuilder.hpp"
#include "World/Aurelia/AureliaTypes.hpp"
#include "World/Chunk/ChunkData.hpp"

namespace racer::world {

struct PropVariation {
    float scaleMul = 1.0f;
    float heightScale = 1.0f;
    Color tint = WHITE;
    int subVariant = 0;
};

class WorldPropBuilder {
public:
    enum class BatchId : std::uint8_t {
        SCATTER = 0,
        LANDMARK,
        POI,
        TRAFFIC,
        COUNT
    };

    WorldPropBuilder();
    ~WorldPropBuilder();

    WorldPropBuilder(const WorldPropBuilder &) = delete;
    WorldPropBuilder &operator=(const WorldPropBuilder &) = delete;

    void setShader(Shader shader);
    void beginBatch(BatchId id);
    void flush(BatchId id);

    static uint32_t hashPosition(float x, float z);
    static PropVariation variationFromHash(uint32_t h, Color baseTint);

    void placeScatterProp(std::uint8_t type, Vector3 base, float yaw,
        float scale, BiomeId biome);
    void placeMarinaLandmarks(float groundY);
    void placeRaceGate(const PoiInstance &poi, float groundY, float timeSec);
    void placeGarage(const PoiInstance &poi, float groundY);
    void placeMissionMarker(const PoiInstance &poi, float groundY,
        float timeSec);
    void placeTrafficCar(Vector3 pos, float heading, Color color);

private:
    using MeshBuffers = TrackMeshBuilder::MeshBuffers;

    struct PropBatch {
        MeshBuffers buffers;
        Model model{};
        bool hasModel = false;
    };

    void appendToBatch(BatchId id, const MeshBuffers &geom, Vector3 pos,
        float yaw, float uniformScale, float heightScale, Color tint);
    void uploadAndDraw(BatchId id);

    static void buildPine(MeshBuffers &mb, float scale, float heightScale,
        Color tint);
    static void buildBroadleaf(MeshBuffers &mb, float scale, float heightScale,
        Color tint);
    static void buildPalm(MeshBuffers &mb, float scale, float heightScale,
        Color tint);
    static void buildRock(MeshBuffers &mb, float scale, float heightScale,
        Color tint);
    static void buildIndustrialSilo(MeshBuffers &mb, float scale,
        float heightScale, Color tint);
    static void buildIndustrialTank(MeshBuffers &mb, float scale,
        float heightScale, Color tint);
    static void buildIndustrialAnnex(MeshBuffers &mb, float scale,
        float heightScale, Color tint);
    static void buildCoastalBuilding(MeshBuffers &mb, float scale, Color tint);
    static void buildPierSegment(MeshBuffers &mb, float length, Color wood,
        Color post);
    static void buildWaterPlane(MeshBuffers &mb, float width, float depth,
        Color color);
    static void buildRaceGate(MeshBuffers &mb, Color colorA, Color colorB,
        float pulse);
    static void buildGarage(MeshBuffers &mb, Color accent);
    static void buildMissionMarker(MeshBuffers &mb, Color color, float bob);
    static void buildTrafficCar(MeshBuffers &mb, Color body);

    Shader shader_{};
    bool hasShader_ = false;
    std::array<PropBatch, static_cast<size_t>(BatchId::COUNT)> batches_{};
};

} // namespace racer::world

#endif /* !WORLD_PROP_BUILDER_HPP_ */
