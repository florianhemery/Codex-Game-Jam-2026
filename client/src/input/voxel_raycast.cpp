#include "client/input/voxel_raycast.h"

#include <cmath>
#include <limits>

namespace client {

namespace {
float BoundaryDistance(float originCoord, int stepDir, int voxelCoord) {
    if (stepDir > 0) return static_cast<float>(voxelCoord + 1) - originCoord;
    return originCoord - static_cast<float>(voxelCoord);
}
} // namespace

RaycastHit RaycastVoxels(const ClientWorld& world, Vector3 origin, Vector3 direction, float maxDistance) {
    RaycastHit result;

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len < 1e-6f) return result;
    Vector3 dir{direction.x / len, direction.y / len, direction.z / len};

    int x = static_cast<int>(std::floor(origin.x));
    int y = static_cast<int>(std::floor(origin.y));
    int z = static_cast<int>(std::floor(origin.z));

    int stepX = dir.x > 0 ? 1 : (dir.x < 0 ? -1 : 0);
    int stepY = dir.y > 0 ? 1 : (dir.y < 0 ? -1 : 0);
    int stepZ = dir.z > 0 ? 1 : (dir.z < 0 ? -1 : 0);

    const float kInf = std::numeric_limits<float>::infinity();

    float tMaxX = dir.x != 0.0f ? BoundaryDistance(origin.x, stepX, x) / std::fabs(dir.x) : kInf;
    float tMaxY = dir.y != 0.0f ? BoundaryDistance(origin.y, stepY, y) / std::fabs(dir.y) : kInf;
    float tMaxZ = dir.z != 0.0f ? BoundaryDistance(origin.z, stepZ, z) / std::fabs(dir.z) : kInf;

    float tDeltaX = dir.x != 0.0f ? 1.0f / std::fabs(dir.x) : kInf;
    float tDeltaY = dir.y != 0.0f ? 1.0f / std::fabs(dir.y) : kInf;
    float tDeltaZ = dir.z != 0.0f ? 1.0f / std::fabs(dir.z) : kInf;

    float t = 0.0f;
    int prevX = x, prevY = y, prevZ = z;

    while (t <= maxDistance) {
        if (world.GetBlock(x, y, z) != common::world::BlockId::Air) {
            result.hit = true;
            result.blockX = x;
            result.blockY = y;
            result.blockZ = z;
            result.prevX = prevX;
            result.prevY = prevY;
            result.prevZ = prevZ;
            return result;
        }

        prevX = x;
        prevY = y;
        prevZ = z;

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                x += stepX;
                t = tMaxX;
                tMaxX += tDeltaX;
            } else {
                z += stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                y += stepY;
                t = tMaxY;
                tMaxY += tDeltaY;
            } else {
                z += stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
            }
        }
    }

    return result;
}

} // namespace client
