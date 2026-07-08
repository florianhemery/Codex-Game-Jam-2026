/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Chunk streaming ring buffer
*/

#include "World/Stream/ChunkStreamer.hpp"

#include <cmath>

#include "World/Aurelia/AureliaData.hpp"
#include "World/Aurelia/AureliaBounds.hpp"

namespace racer::world {

namespace {

float chunkOriginX(ChunkId id)
{
    return static_cast<float>(id.x) * kChunkSize;
}

float chunkOriginZ(ChunkId id)
{
    return static_cast<float>(id.z) * kChunkSize;
}

} // namespace

void ChunkStreamer::updateCenter(float worldX, float worldZ)
{
    center_ = worldToChunkId(clampWorldX(worldX), clampWorldZ(worldZ));
}

void ChunkStreamer::ensureLoaded()
{
    for (int dz = -kStreamRadius; dz <= kStreamRadius; ++dz) {
        for (int dx = -kStreamRadius; dx <= kStreamRadius; ++dx) {
            loadChunk(ChunkId{center_.x + dx, center_.z + dz});
        }
    }
    unloadOutsideRing();
}

void ChunkStreamer::loadChunk(ChunkId id)
{
    if (index_.find(id) != index_.end()) {
        return;
    }
    float ox = static_cast<float>(id.x) * kChunkSize;
    float oz = static_cast<float>(id.z) * kChunkSize;
    if (ox + kChunkSize < WorldBounds::minX || ox > WorldBounds::maxX
        || oz + kChunkSize < WorldBounds::minZ || oz > WorldBounds::maxZ) {
        return;
    }
    ChunkData data = ChunkGenerator::generate(id);
    for (const PoiInstance &poi : AureliaData::worldPois()) {
        ChunkId poiChunk = worldToChunkId(poi.worldX, poi.worldZ);
        if (poiChunk.x == id.x && poiChunk.z == id.z) {
            data.pois.push_back(poi);
        }
    }
    index_[id] = loaded_.size();
    loaded_.push_back(std::move(data));
}

void ChunkStreamer::unloadOutsideRing()
{
    std::vector<ChunkData> kept;
    std::unordered_map<ChunkId, size_t, ChunkIdHash> newIndex;

    for (ChunkData &chunk : loaded_) {
        int dx = chunk.id.x - center_.x;
        int dz = chunk.id.z - center_.z;
        if (std::abs(dx) <= kStreamRadius && std::abs(dz) <= kStreamRadius) {
            newIndex[chunk.id] = kept.size();
            kept.push_back(std::move(chunk));
        }
    }
    loaded_ = std::move(kept);
    index_ = std::move(newIndex);
}

const ChunkData *ChunkStreamer::findChunk(ChunkId id) const
{
    auto it = index_.find(id);
    if (it == index_.end()) {
        return nullptr;
    }
    return &loaded_[it->second];
}

bool ChunkStreamer::isLoaded(float worldX, float worldZ) const
{
    return findChunk(worldToChunkId(worldX, worldZ)) != nullptr;
}

float ChunkStreamer::sampleHeight(float worldX, float worldZ) const
{
    ChunkId id = worldToChunkId(worldX, worldZ);
    const ChunkData *chunk = findChunk(id);
    if (!chunk) {
        return biomeForChunk(id.x, id.z) == BiomeId::VOLCANO ? 12.0f : 2.0f;
    }
    float lx = worldX - chunkOriginX(id);
    float lz = worldZ - chunkOriginZ(id);
    return ChunkGenerator::sampleHeight(*chunk, lx, lz);
}

SurfaceKind ChunkStreamer::sampleSurface(float worldX, float worldZ) const
{
    ChunkId id = worldToChunkId(worldX, worldZ);
    const ChunkData *chunk = findChunk(id);
    if (!chunk) {
        return SurfaceKind::GRASS;
    }
    float lx = worldX - chunkOriginX(id);
    float lz = worldZ - chunkOriginZ(id);
    return ChunkGenerator::sampleSurface(*chunk, lx, lz);
}

BiomeId ChunkStreamer::biomeAt(float worldX, float worldZ) const
{
    ChunkId id = worldToChunkId(worldX, worldZ);
    const ChunkData *chunk = findChunk(id);
    if (chunk) {
        return chunk->biome;
    }
    return biomeForChunk(id.x, id.z);
}

} // namespace racer::world
