#include "server_core/world_manager.h"

namespace server_core {

using common::world::ChunkCoord;

WorldManager::WorldManager(uint32_t seed, std::string worldSaveDir)
    : generator_(seed), storage_(std::move(worldSaveDir)) {}

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
            common::world::Chunk loadedChunk;
            if (storage_.Load(coord, loadedChunk)) {
                chunks_.emplace(coord, std::move(loadedChunk));
            } else {
                chunks_.emplace(coord, generator_.Generate(coord));
            }
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

bool WorldManager::HasChunk(ChunkCoord coord) const {
    return chunks_.find(coord) != chunks_.end();
}

void WorldManager::SetBlock(ChunkCoord coord, int lx, int ly, int lz, uint8_t blockId) {
    auto it = chunks_.find(coord);
    if (it == chunks_.end()) return;
    it->second.blocks[common::world::BlockIndex(lx, ly, lz)] = blockId;
    // Sauvegarde immediate : les changements de bloc sont rares (actions
    // joueur), pas besoin d'un systeme de sauvegarde incrementale differee.
    storage_.Save(it->second);
}

bool WorldManager::GetBlockWorld(int worldX, int worldY, int worldZ, uint8_t& outBlockId) const {
    if (worldY < 0 || worldY >= common::world::CHUNK_SIZE_Y) return false;

    ChunkCoord coord = common::world::WorldToChunkCoordInt(worldX, worldZ);
    auto it = chunks_.find(coord);
    if (it == chunks_.end()) return false;

    int lx = common::world::WorldToLocal(worldX, coord.x, common::world::CHUNK_SIZE_X);
    int lz = common::world::WorldToLocal(worldZ, coord.z, common::world::CHUNK_SIZE_Z);
    outBlockId = it->second.blocks[common::world::BlockIndex(lx, worldY, lz)];
    return true;
}

bool WorldManager::SetBlockWorld(int worldX, int worldY, int worldZ, uint8_t blockId) {
    if (worldY < 0 || worldY >= common::world::CHUNK_SIZE_Y) return false;

    ChunkCoord coord = common::world::WorldToChunkCoordInt(worldX, worldZ);
    if (chunks_.find(coord) == chunks_.end()) return false;

    int lx = common::world::WorldToLocal(worldX, coord.x, common::world::CHUNK_SIZE_X);
    int lz = common::world::WorldToLocal(worldZ, coord.z, common::world::CHUNK_SIZE_Z);
    SetBlock(coord, lx, worldY, lz, blockId);
    return true;
}

std::vector<uint64_t> WorldManager::ViewersOf(ChunkCoord coord) const {
    std::vector<uint64_t> result;
    for (const auto& [viewerId, loaded] : viewerLoaded_) {
        if (loaded.find(coord) != loaded.end()) {
            result.push_back(viewerId);
        }
    }
    return result;
}

} // namespace server_core
