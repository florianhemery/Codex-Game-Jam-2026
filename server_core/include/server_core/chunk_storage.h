#pragma once

#include <string>

#include "common/world/chunk.h"

namespace server_core {

// Un fichier par chunk modifie (world/chunks/{cx}_{cz}.chunk), format binaire
// simple : magic(4) + version(1) + cx(4) + cz(4) + blocs bruts (16 Ko).
class ChunkStorage {
public:
    explicit ChunkStorage(std::string rootDir);

    bool Load(common::world::ChunkCoord coord, common::world::Chunk& outChunk) const;
    void Save(const common::world::Chunk& chunk) const;

private:
    std::string PathFor(common::world::ChunkCoord coord) const;

    std::string rootDir_;
};

} // namespace server_core
