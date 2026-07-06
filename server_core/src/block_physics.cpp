#include "server_core/block_physics.h"

#include "common/world/block.h"

namespace server_core {

namespace {
constexpr int kMaxUpdatesPerTick = 64;
} // namespace

void BlockPhysics::NotifyChanged(int worldX, int worldY, int worldZ) {
    pending_.push_back(Pos{worldX, worldY, worldZ});
    pending_.push_back(Pos{worldX, worldY + 1, worldZ});
}

void BlockPhysics::Tick(WorldManager& worldManager, std::vector<BlockChangeEvent>& outEvents) {
    int processed = 0;
    while (!pending_.empty() && processed < kMaxUpdatesPerTick) {
        Pos pos = pending_.front();
        pending_.pop_front();
        ++processed;

        uint8_t blockId = 0;
        if (!worldManager.GetBlockWorld(pos.x, pos.y, pos.z, blockId)) continue;
        if (!common::world::IsFallable(static_cast<common::world::BlockId>(blockId))) continue;

        uint8_t belowId = 0;
        if (!worldManager.GetBlockWorld(pos.x, pos.y - 1, pos.z, belowId)) continue;
        if (belowId != static_cast<uint8_t>(common::world::BlockId::Air)) continue;

        worldManager.SetBlockWorld(pos.x, pos.y, pos.z, static_cast<uint8_t>(common::world::BlockId::Air));
        worldManager.SetBlockWorld(pos.x, pos.y - 1, pos.z, blockId);

        outEvents.push_back({pos.x, pos.y, pos.z, static_cast<uint8_t>(common::world::BlockId::Air)});
        outEvents.push_back({pos.x, pos.y - 1, pos.z, blockId});

        pending_.push_back(Pos{pos.x, pos.y - 1, pos.z}); // continue peut-etre a tomber
        pending_.push_back(Pos{pos.x, pos.y, pos.z});     // quelque chose au-dessus peut suivre
    }
}

} // namespace server_core
