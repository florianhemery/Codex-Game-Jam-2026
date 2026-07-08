/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Chunk streaming ring buffer
*/

#include "World/Stream/ChunkStreamer.hpp"

#include <chrono>
#include <cmath>
#include <mutex>

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

bool outsideWorldBounds(ChunkId id)
{
    float ox = static_cast<float>(id.x) * kChunkSize;
    float oz = static_cast<float>(id.z) * kChunkSize;
    return ox + kChunkSize < WorldBounds::minX || ox > WorldBounds::maxX
        || oz + kChunkSize < WorldBounds::minZ || oz > WorldBounds::maxZ;
}

void attachPois(ChunkData &data)
{
    for (const PoiInstance &poi : AureliaData::worldPois()) {
        ChunkId poiChunk = worldToChunkId(poi.worldX, poi.worldZ);
        if (poiChunk.x == data.id.x && poiChunk.z == data.id.z) {
            data.pois.push_back(poi);
        }
    }
}

// ChunkGenerator::generate() reads AureliaData::roadGraph(), which lazily
// builds the shared RoadGraph singleton on its first call. That lazy build
// is not itself thread-safe, so it must happen once, synchronously, before
// any background generation thread can race to trigger it.
void warmupSharedData()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        AureliaData::roadGraph();
        AureliaData::worldPois();
    });
}

} // namespace

void ChunkStreamer::updateCenter(float worldX, float worldZ)
{
    center_ = worldToChunkId(clampWorldX(worldX), clampWorldZ(worldZ));
}

void ChunkStreamer::ensureLoaded()
{
    drainPending();

    // Dispatch every missing ring chunk to its own background thread
    // first, so all of this frame's newly-needed chunks generate
    // concurrently instead of one after another.
    for (int dz = -kStreamRadius; dz <= kStreamRadius; ++dz) {
        for (int dx = -kStreamRadius; dx <= kStreamRadius; ++dx) {
            loadChunk(ChunkId{center_.x + dx, center_.z + dz});
        }
    }

    // Resolve the ring before returning: ensureLoaded()'s contract is that
    // every ring chunk is present in loadedChunks() as soon as it returns.
    // Because generation for multiple chunks now runs in parallel, this
    // wait costs roughly one generate() worth of wall time even when
    // several chunks entered the ring this frame (e.g. 3 on a straight
    // crossing, up to 5 on a diagonal one), instead of the sum of all of
    // them run sequentially on the main thread as before.
    for (int dz = -kStreamRadius; dz <= kStreamRadius; ++dz) {
        for (int dx = -kStreamRadius; dx <= kStreamRadius; ++dx) {
            ChunkId id{center_.x + dx, center_.z + dz};
            if (index_.find(id) != index_.end()) {
                continue;
            }
            auto it = pending_.find(id);
            if (it == pending_.end()) {
                continue; // e.g. outside world bounds, never dispatched
            }
            integrate(id, it->second.get());
            pending_.erase(it);
        }
    }

    unloadOutsideRing();
}

void ChunkStreamer::loadChunk(ChunkId id)
{
    if (index_.find(id) != index_.end() || pending_.find(id) != pending_.end()) {
        return;
    }
    if (outsideWorldBounds(id)) {
        return;
    }
    warmupSharedData();
    pending_[id] = std::async(std::launch::async, &ChunkGenerator::generate, id);
}

void ChunkStreamer::drainPending() const
{
    for (auto it = pending_.begin(); it != pending_.end();) {
        if (it->second.wait_for(std::chrono::seconds(0))
            == std::future_status::ready) {
            integrate(it->first, it->second.get());
            it = pending_.erase(it);
        } else {
            ++it;
        }
    }
}

const ChunkData *ChunkStreamer::integrate(ChunkId id, ChunkData data) const
{
    auto it = index_.find(id);
    if (it != index_.end()) {
        return &loaded_[it->second];
    }
    attachPois(data);
    index_[id] = loaded_.size();
    loaded_.push_back(std::move(data));
    return &loaded_.back();
}

const ChunkData *ChunkStreamer::forceSyncLoad(ChunkId id) const
{
    auto pit = pending_.find(id);
    if (pit != pending_.end()) {
        ChunkData data = pit->second.get();
        pending_.erase(pit);
        return integrate(id, std::move(data));
    }
    if (outsideWorldBounds(id)) {
        return nullptr;
    }
    warmupSharedData();
    return integrate(id, ChunkGenerator::generate(id));
}

const ChunkData *ChunkStreamer::findChunkUrgent(ChunkId id) const
{
    const ChunkData *chunk = findChunk(id);
    if (chunk) {
        return chunk;
    }
    // Only force synchronous generation for chunks inside the current
    // streaming ring: those are the ones ensureLoaded() has (or is about
    // to) request in the background, so forcing them here just closes the
    // narrow race window for a query made outside the normal
    // ensureLoaded() cycle. Chunks outside the ring stay non-resident --
    // forcing those too would generate-then-immediately-evict on every
    // frame for distant queries (boundary posts, POI markers, traffic),
    // reintroducing the stutter this change is meant to remove.
    int dx = id.x - center_.x;
    int dz = id.z - center_.z;
    if (std::abs(dx) > kStreamRadius || std::abs(dz) > kStreamRadius) {
        return nullptr;
    }
    return forceSyncLoad(id);
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
    drainPending();
    auto it = index_.find(id);
    if (it == index_.end()) {
        return nullptr;
    }
    return &loaded_[it->second];
}

bool ChunkStreamer::isLoaded(float worldX, float worldZ) const
{
    return findChunkUrgent(worldToChunkId(worldX, worldZ)) != nullptr;
}

float ChunkStreamer::sampleHeight(float worldX, float worldZ) const
{
    ChunkId id = worldToChunkId(worldX, worldZ);
    const ChunkData *chunk = findChunkUrgent(id);
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
    const ChunkData *chunk = findChunkUrgent(id);
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
