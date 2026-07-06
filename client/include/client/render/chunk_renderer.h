#pragma once

#include <unordered_map>

#include "raylib.h"

#include "common/world/chunk.h"
#include "common/world/world_coords.h"

namespace client {

// Un Model raylib par chunk charge, reconstruit entierement a chaque
// changement (pas de mise a jour partielle au jour 2 -- suffisant tant que
// les modifications de bloc n'existent pas encore, cf. jour 3).
class ChunkRenderer {
public:
    ~ChunkRenderer();

    void UpsertChunk(const common::world::Chunk& chunk);
    void RemoveChunk(common::world::ChunkCoord coord);
    void DrawAll(Color tint = WHITE) const;
    size_t LoadedCount() const { return models_.size(); }

private:
    std::unordered_map<common::world::ChunkCoord, Model, common::world::ChunkCoordHash> models_;
};

} // namespace client
