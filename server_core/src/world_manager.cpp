#include "server_core/world_manager.h"

namespace server_core {

using common::world::ChunkCoord;

WorldManager::WorldManager(uint32_t seed) : generator_(seed) {}

WorldManager::StreamingDelta WorldManager::UpdateViewer(uint64_t viewerId, ChunkCoord center, int viewDistanceChunks) {
    StreamingDelta delta;

    std::unordered_set<ChunkCoord, common::world::ChunkCoordHash> desired;
    for (int dx = -viewDistanceChunks; dx <= viewDistanceChunks; ++dx) {
        for (int dz = -viewDistanceChunks; dz <= viewDistanceChunks; ++dz) {
            desired.insert(ChunkCoord{center.x + dx, center.z + dz});
        }
    }

    auto& loaded = viewerLoaded_[viewerId];

    for (const auto& coord : desired) {
        if (loaded.find(coord) != loaded.end()) continue;

        if (chunks_.find(coord) == chunks_.end()) {
            chunks_.emplace(coord, generator_.Generate(coord));
            refCount_[coord] = 0;
        }
        refCount_[coord] += 1;
        loaded.insert(coord);
        delta.toLoad.push_back(coord);
    }

    for (auto it = loaded.begin(); it != loaded.end();) {
        if (desired.find(*it) != desired.end()) {
            ++it;
            continue;
        }

        ChunkCoord coord = *it;
        delta.toUnload.push_back(coord);

        auto refIt = refCount_.find(coord);
        if (refIt != refCount_.end()) {
            refIt->second -= 1;
            if (refIt->second <= 0) {
                chunks_.erase(coord);
                refCount_.erase(refIt);
            }
        }

        it = loaded.erase(it);
    }

    return delta;
}

void WorldManager::RemoveViewer(uint64_t viewerId) {
    auto it = viewerLoaded_.find(viewerId);
    if (it == viewerLoaded_.end()) return;

    for (const auto& coord : it->second) {
        auto refIt = refCount_.find(coord);
        if (refIt != refCount_.end()) {
            refIt->second -= 1;
            if (refIt->second <= 0) {
                chunks_.erase(coord);
                refCount_.erase(refIt);
            }
        }
    }

    viewerLoaded_.erase(it);
}

const common::world::Chunk& WorldManager::GetChunk(ChunkCoord coord) const {
    return chunks_.at(coord);
}

} // namespace server_core
