#include "server_core/server_core.h"

#include <deque>
#include <optional>
#include <unordered_map>

#include "common/world/block.h"
#include "common/world/world_coords.h"
#include "server_core/inventory.h"
#include "server_core/world_manager.h"

namespace server_core {

namespace {

struct ClientChannel {
    std::deque<common::messages::ReliableMessage> outgoingReliable;
    std::deque<common::messages::UnreliableMessage> outgoingUnreliable;
    std::optional<common::messages::PlayerInputMsg> latestInput;
    Inventory inventory;
};

void PushReliable(ClientChannel& channel, common::messages::ReliableMsgType type,
                   common::messages::ReliableMessagePayload payload) {
    common::messages::ReliableMessage msg;
    msg.type = type;
    msg.payload = std::move(payload);
    channel.outgoingReliable.push_back(std::move(msg));
}

} // namespace

struct ServerCore {
    ServerConfig config;
    WorldManager worldManager;
    std::unordered_map<ClientId, ClientChannel> clients;
    ClientId nextClientId = 0;

    explicit ServerCore(const ServerConfig& cfg) : config(cfg), worldManager(cfg.seed) {}
};

ServerCore* create(const ServerConfig& cfg) {
    return new ServerCore(cfg);
}

void destroy(ServerCore* sc) {
    delete sc;
}

ClientId connect_client(ServerCore* sc, const std::string& playerName) {
    (void)playerName;
    ClientId id = sc->nextClientId++;
    sc->clients.emplace(id, ClientChannel{});
    return id;
}

void disconnect_client(ServerCore* sc, ClientId id) {
    sc->worldManager.RemoveViewer(id);
    sc->clients.erase(id);
}

namespace {

void HandleBreakRequest(ServerCore* sc, ClientId from, const common::messages::BreakBlockRequestMsg& req) {
    if (!sc->worldManager.HasChunk(req.coord)) return;

    uint8_t oldBlockId = sc->worldManager.GetChunk(req.coord).blocks[common::world::BlockIndex(req.lx, req.ly, req.lz)];
    if (oldBlockId == static_cast<uint8_t>(common::world::BlockId::Air)) return; // rien a casser

    sc->worldManager.SetBlock(req.coord, req.lx, req.ly, req.lz, static_cast<uint8_t>(common::world::BlockId::Air));

    auto requester = sc->clients.find(from);
    if (requester != sc->clients.end()) {
        requester->second.inventory.AddBlock(oldBlockId, 1);
        PushReliable(requester->second, common::messages::ReliableMsgType::InventoryUpdate,
                     requester->second.inventory.ToMessage());
    }

    common::messages::BlockUpdateMsg update{req.coord, req.lx, req.ly, req.lz, static_cast<uint8_t>(common::world::BlockId::Air)};
    for (uint64_t viewerId : sc->worldManager.ViewersOf(req.coord)) {
        auto viewer = sc->clients.find(static_cast<ClientId>(viewerId));
        if (viewer != sc->clients.end()) {
            PushReliable(viewer->second, common::messages::ReliableMsgType::BlockUpdate, update);
        }
    }
}

void HandlePlaceRequest(ServerCore* sc, ClientId from, const common::messages::PlaceBlockRequestMsg& req) {
    if (!sc->worldManager.HasChunk(req.coord)) return;

    uint8_t currentBlockId = sc->worldManager.GetChunk(req.coord).blocks[common::world::BlockIndex(req.lx, req.ly, req.lz)];
    if (currentBlockId != static_cast<uint8_t>(common::world::BlockId::Air)) return; // deja occupe

    auto requester = sc->clients.find(from);
    if (requester == sc->clients.end()) return;

    if (req.hotbarSlot >= requester->second.inventory.SlotCount()) return;
    uint8_t blockToPlace = requester->second.inventory.Slot(req.hotbarSlot).blockId;
    if (blockToPlace == static_cast<uint8_t>(common::world::BlockId::Air)) return; // slot vide

    if (!requester->second.inventory.RemoveFromSlot(req.hotbarSlot, blockToPlace, 1)) return;

    sc->worldManager.SetBlock(req.coord, req.lx, req.ly, req.lz, blockToPlace);
    PushReliable(requester->second, common::messages::ReliableMsgType::InventoryUpdate,
                 requester->second.inventory.ToMessage());

    common::messages::BlockUpdateMsg update{req.coord, req.lx, req.ly, req.lz, blockToPlace};
    for (uint64_t viewerId : sc->worldManager.ViewersOf(req.coord)) {
        auto viewer = sc->clients.find(static_cast<ClientId>(viewerId));
        if (viewer != sc->clients.end()) {
            PushReliable(viewer->second, common::messages::ReliableMsgType::BlockUpdate, update);
        }
    }
}

} // namespace

void submit_reliable(ServerCore* sc, ClientId from, const common::messages::ReliableMessage& msg) {
    if (msg.type == common::messages::ReliableMsgType::BreakBlockRequest) {
        HandleBreakRequest(sc, from, std::get<common::messages::BreakBlockRequestMsg>(msg.payload));
    } else if (msg.type == common::messages::ReliableMsgType::PlaceBlockRequest) {
        HandlePlaceRequest(sc, from, std::get<common::messages::PlaceBlockRequestMsg>(msg.payload));
    }
}

void submit_unreliable(ServerCore* sc, ClientId from, const common::messages::UnreliableMessage& msg) {
    auto it = sc->clients.find(from);
    if (it == sc->clients.end()) return;

    if (msg.type == common::messages::UnreliableMsgType::PlayerInput) {
        it->second.latestInput = std::get<common::messages::PlayerInputMsg>(msg.payload);
    }
}

void tick(ServerCore* sc, float dt) {
    (void)dt;

    for (auto& [clientId, channel] : sc->clients) {
        if (!channel.latestInput.has_value()) continue;

        const auto& input = *channel.latestInput;
        common::world::ChunkCoord center = common::world::WorldToChunkCoord(input.posX, input.posZ);

        WorldManager::StreamingDelta delta =
            sc->worldManager.UpdateViewer(static_cast<uint64_t>(clientId), center, kDefaultViewDistanceChunks);

        for (const auto& coord : delta.toLoad) {
            common::messages::ChunkDataMsg chunkMsg;
            chunkMsg.coord = coord;
            chunkMsg.blocks = sc->worldManager.GetChunk(coord).blocks;
            PushReliable(channel, common::messages::ReliableMsgType::ChunkData, chunkMsg);
        }

        for (const auto& coord : delta.toUnload) {
            common::messages::ChunkUnloadMsg unloadMsg;
            unloadMsg.coord = coord;
            PushReliable(channel, common::messages::ReliableMsgType::ChunkUnload, unloadMsg);
        }
    }
}

bool poll_outgoing_reliable(ServerCore* sc, ClientId to, common::messages::ReliableMessage& out) {
    auto it = sc->clients.find(to);
    if (it == sc->clients.end() || it->second.outgoingReliable.empty()) {
        return false;
    }
    out = it->second.outgoingReliable.front();
    it->second.outgoingReliable.pop_front();
    return true;
}

bool poll_outgoing_unreliable(ServerCore* sc, ClientId to, common::messages::UnreliableMessage& out) {
    auto it = sc->clients.find(to);
    if (it == sc->clients.end() || it->second.outgoingUnreliable.empty()) {
        return false;
    }
    out = it->second.outgoingUnreliable.front();
    it->second.outgoingUnreliable.pop_front();
    return true;
}

} // namespace server_core
