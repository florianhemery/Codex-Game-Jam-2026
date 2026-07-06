#pragma once

#include "raylib.h"

#include "common/world/chunk.h"

namespace client {

// Un quad par face de bloc visible (voisin = air ou hors chunk). Pas de
// culling inter-chunk au jour 2 : une face a la frontiere entre deux chunks
// charges peut etre dessinee en double (over-inclusion sans consequence),
// jamais de trou. Remplace au jour 3 par un vrai greedy mesher.
Mesh BuildNaiveChunkMesh(const common::world::Chunk& chunk);

} // namespace client
