#pragma once

#include "raylib.h"

#include "client/world/client_world.h"

namespace client {

struct RaycastHit {
    bool hit = false;
    int blockX = 0, blockY = 0, blockZ = 0; // bloc touche (pour casser)
    int prevX = 0, prevY = 0, prevZ = 0;    // bloc juste avant le long du rayon (pour poser)
};

// Algorithme DDA (Amanatides & Woo) : traverse la grille voxel le long du
// rayon jusqu'a maxDistance ou jusqu'au premier bloc non-air rencontre.
RaycastHit RaycastVoxels(const ClientWorld& world, Vector3 origin, Vector3 direction, float maxDistance);

} // namespace client
