#include "server_core/mob_ai.h"

#include <cmath>

#include "common/world/block.h"

namespace server_core {

namespace {
constexpr size_t kMaxMobs = 20;
constexpr float kSpeed = 1.5f;
constexpr float kGravity = 20.0f;
} // namespace

void MobManager::MaybeSpawn(common::world::ChunkCoord coord, const WorldManager& worldManager, uint32_t seed) {
    if (spawnRolled_.find(coord) != spawnRolled_.end()) return;
    spawnRolled_.insert(coord);

    if (entities_.size() >= kMaxMobs) return;
    if (!worldManager.HasChunk(coord)) return;

    uint32_t h = static_cast<uint32_t>(coord.x) * 374761393u + static_cast<uint32_t>(coord.z) * 668265263u +
                 seed * 2654435761u + 999u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);
    if (h % 100 >= 8) return; // ~8% des chunks charges font apparaitre un mob

    const common::world::Chunk& chunk = worldManager.GetChunk(coord);
    constexpr int lx = 8, lz = 8;
    int surfaceY = -1;
    for (int y = common::world::CHUNK_SIZE_Y - 1; y >= 0; --y) {
        auto id = static_cast<common::world::BlockId>(chunk.blocks[common::world::BlockIndex(lx, y, lz)]);
        if (id != common::world::BlockId::Air && id != common::world::BlockId::Water) {
            surfaceY = y;
            break;
        }
    }
    if (surfaceY < 0) return;

    Entity e;
    e.id = nextEntityId_++;
    e.type = (h % 2 == 0) ? MobType::Pig : MobType::Zombie;
    e.x = static_cast<float>(coord.x * common::world::CHUNK_SIZE_X + lx) + 0.5f;
    e.y = static_cast<float>(surfaceY + 1);
    e.z = static_cast<float>(coord.z * common::world::CHUNK_SIZE_Z + lz) + 0.5f;
    entities_.push_back(e);
}

void MobManager::Tick(float dt, WorldManager& worldManager) {
    std::uniform_real_distribution<float> yawDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> timerDist(2.0f, 5.0f);

    for (Entity& e : entities_) {
        e.wanderTimer -= dt;
        if (e.wanderTimer <= 0.0f) {
            e.yaw = yawDist(rng_);
            e.wanderTimer = timerDist(rng_);
        }

        float newX = e.x + std::sin(e.yaw) * kSpeed * dt;
        float newZ = e.z + std::cos(e.yaw) * kSpeed * dt;

        uint8_t aheadId = 0;
        bool aheadKnown = worldManager.GetBlockWorld(static_cast<int>(std::floor(newX)), static_cast<int>(std::floor(e.y)),
                                                      static_cast<int>(std::floor(newZ)), aheadId);
        bool aheadSolid = aheadKnown && static_cast<common::world::BlockId>(aheadId) != common::world::BlockId::Air;

        if (!aheadSolid) {
            e.x = newX;
            e.z = newZ;
        } else {
            e.wanderTimer = 0.0f; // force un changement de direction au prochain tick
        }

        uint8_t belowId = 0;
        bool belowKnown = worldManager.GetBlockWorld(static_cast<int>(std::floor(e.x)),
                                                      static_cast<int>(std::floor(e.y)) - 1,
                                                      static_cast<int>(std::floor(e.z)), belowId);
        bool grounded = belowKnown && static_cast<common::world::BlockId>(belowId) != common::world::BlockId::Air;

        if (grounded && e.velY <= 0.0f) {
            e.velY = 0.0f;
            e.y = std::floor(e.y) + 1.0f;
        } else {
            e.velY -= kGravity * dt;
            e.y += e.velY * dt;
        }
    }
}

} // namespace server_core
