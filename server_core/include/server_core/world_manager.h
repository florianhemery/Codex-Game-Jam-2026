#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/world/chunk.h"
#include "common/world/world_coords.h"
#include "server_core/chunk_generator.h"
#include "server_core/chunk_storage.h"

namespace server_core {

constexpr int kDefaultViewDistanceChunks = 3;

// Un seul monde partage entre tous les viewers (meme en solo, l'API reste
// multi-viewer : chaque chunk est refcompte par le nombre de viewers qui
// l'ont dans leur rayon de vue, et n'est libere que quand le dernier s'en va).
class WorldManager {
public:
    WorldManager(uint32_t seed, std::string worldSaveDir);

    struct StreamingDelta {
        std::vector<common::world::ChunkCoord> toLoad;
        std::vector<common::world::ChunkCoord> toUnload;
    };

    StreamingDelta UpdateViewer(uint64_t viewerId, common::world::ChunkCoord center, int viewDistanceChunks);
    void RemoveViewer(uint64_t viewerId);

    const common::world::Chunk& GetChunk(common::world::ChunkCoord coord) const;
    bool HasChunk(common::world::ChunkCoord coord) const;

    // Modifie un bloc dans un chunk deja charge. Precondition : HasChunk(coord) == true.
    void SetBlock(common::world::ChunkCoord coord, int lx, int ly, int lz, uint8_t blockId);

    // Tous les viewers qui ont actuellement ce chunk dans leur rayon de vue --
    // utilise pour diffuser un BlockUpdate a la bonne audience.
    std::vector<uint64_t> ViewersOf(common::world::ChunkCoord coord) const;

    // Variantes en coordonnees monde (pratiques pour block_physics, qui
    // raisonne en positions absolues plutot qu'en chunk+local). Renvoie
    // false si le chunk correspondant n'est pas charge.
    bool GetBlockWorld(int worldX, int worldY, int worldZ, uint8_t& outBlockId) const;
    bool SetBlockWorld(int worldX, int worldY, int worldZ, uint8_t blockId);

private:
    ChunkGenerator generator_;
    ChunkStorage storage_;

    std::unordered_map<common::world::ChunkCoord, common::world::Chunk, common::world::ChunkCoordHash> chunks_;
    std::unordered_map<common::world::ChunkCoord, int, common::world::ChunkCoordHash> refCount_;
    std::unordered_map<uint64_t, std::unordered_set<common::world::ChunkCoord, common::world::ChunkCoordHash>> viewerLoaded_;
};

} // namespace server_core
