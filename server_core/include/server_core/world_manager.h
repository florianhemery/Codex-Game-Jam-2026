#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/world/chunk.h"
#include "common/world/world_coords.h"
#include "server_core/chunk_generator.h"

namespace server_core {

constexpr int kDefaultViewDistanceChunks = 3;

// Un seul monde partage entre tous les viewers (meme en solo, l'API reste
// multi-viewer : chaque chunk est refcompte par le nombre de viewers qui
// l'ont dans leur rayon de vue, et n'est libere que quand le dernier s'en va).
class WorldManager {
public:
    explicit WorldManager(uint32_t seed);

    struct StreamingDelta {
        std::vector<common::world::ChunkCoord> toLoad;
        std::vector<common::world::ChunkCoord> toUnload;
    };

    StreamingDelta UpdateViewer(uint64_t viewerId, common::world::ChunkCoord center, int viewDistanceChunks);
    void RemoveViewer(uint64_t viewerId);

    const common::world::Chunk& GetChunk(common::world::ChunkCoord coord) const;

private:
    ChunkGenerator generator_;

    std::unordered_map<common::world::ChunkCoord, common::world::Chunk, common::world::ChunkCoordHash> chunks_;
    std::unordered_map<common::world::ChunkCoord, int, common::world::ChunkCoordHash> refCount_;
    std::unordered_map<uint64_t, std::unordered_set<common::world::ChunkCoord, common::world::ChunkCoordHash>> viewerLoaded_;
};

} // namespace server_core
