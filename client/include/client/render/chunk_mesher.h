#pragma once

#include "raylib.h"

#include "common/world/chunk.h"

namespace client {

// Greedy meshing : fusionne les faces adjacentes de meme bloc en un seul
// quad par rectangle maximal. Pas de culling inter-chunk (les blocs hors
// chunk sont traites comme de l'air) -- over-inclusion sans danger aux
// frontieres, jamais de trou.
Mesh BuildGreedyChunkMesh(const common::world::Chunk& chunk);

} // namespace client
