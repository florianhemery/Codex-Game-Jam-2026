#include "server_core/server_core.h"

#include <cmath>
#include <deque>
#include <optional>
#include <unordered_map>

#include "common/world/block.h"
#include "common/world/world_coords.h"
#include "server_core/inventory.h"
#include "server_core/player_storage.h"
#include "server_core/world_manager.h"

namespace server_core {

namespace {

constexpr float kHungerTickSeconds = 1.0f;  // -1 faim toutes les secondes
constexpr float kHealthTickSeconds = 2.0f;  // -1 sante toutes les 2s une fois affame
constexpr uint16_t kMinHealth = 1;          // pas de mort/respawn implemente en V1
constexpr float kDayLengthSeconds = 180.0f; // un cycle jour/nuit complet toutes les 3 minutes

struct ClientChannel {
    std::deque<common::messages::ReliableMessage> outgoingReliable;
    std::deque<common::messages::UnreliableMessage> outgoingUnreliable;
    std::optional<common::messages::PlayerInputMsg> latestInput;
    Inventory inventory;
    std::string playerName;
    uint16_t health = 100;
    uint16_t hunger = 100;
    float hungerAccumulator = 0.0f;
    float healthAccumulator = 0.0f;
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
    PlayerStorage playerStorage;
    std::unordered_map<ClientId, ClientChannel> clients;
    ClientId nextClientId = 0;
    float worldTime = kDayLengthSeconds * 0.5f; // demarre a midi, plus lisible pour un premier lancement

    explicit ServerCore(const ServerConfig& cfg)
        : config(cfg), worldManager(cfg.seed, cfg.worldSaveDir), playerStorage(cfg.worldSaveDir) {}
};

namespace {

void SavePlayer(ServerCore* sc, const ClientChannel& channel) {
    PlayerSaveData data;
    if (channel.latestInput.has_value()) {
        data.posX = channel.latestInput->posX;
        data.posY = channel.latestInput->posY;
        data.posZ = channel.latestInput->posZ;
    }
    data.health = channel.health;
    data.hunger = channel.hunger;
    data.inventory = channel.inventory.Slots();
    sc->playerStorage.Save(channel.playerName, data);
}

} // namespace

ServerCore* create(const ServerConfig& cfg) {
    return new ServerCore(cfg);
}

void destroy(ServerCore* sc) {
    for (auto& [clientId, channel] : sc->clients) {
        (void)clientId;
        SavePlayer(sc, channel);
    }
    delete sc;
}

ClientId connect_client(ServerCore* sc, const std::string& playerName) {
    ClientId id = sc->nextClientId++;

    ClientChannel channel;
    channel.playerName = playerName;

    PlayerSaveData saved;
    bool hasSave = sc->playerStorage.Load(playerName, saved);
    if (hasSave) {
        channel.health = saved.health;
        channel.hunger = saved.hunger;
        channel.inventory.SetSlots(saved.inventory);
    }

    common::messages::SpawnStateMsg spawn;
    spawn.posX = saved.posX;
    spawn.posY = saved.posY;
    spawn.posZ = saved.posZ;
    spawn.health = channel.health;
    spawn.hunger = channel.hunger;
    PushReliable(channel, common::messages::ReliableMsgType::SpawnState, spawn);
    PushReliable(channel, common::messages::ReliableMsgType::InventoryUpdate, channel.inventory.ToMessage());

    sc->clients.emplace(id, std::move(channel));
    return id;
}

void disconnect_client(ServerCore* sc, ClientId id) {
    auto it = sc->clients.find(id);
    if (it != sc->clients.end()) {
        SavePlayer(sc, it->second);
    }
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
    sc->worldTime += dt;
    float timeOfDay01 = std::fmod(sc->worldTime, kDayLengthSeconds) / kDayLengthSeconds;

    for (auto& [clientId, channel] : sc->clients) {
        // Horloge de jeu, diffusee a tous -- tolerant a la perte (UDP-like).
        common::messages::WorldTimeMsg timeMsg;
        timeMsg.timeOfDay01 = timeOfDay01;
        common::messages::UnreliableMessage umsg;
        umsg.type = common::messages::UnreliableMsgType::WorldTime;
        umsg.payload = timeMsg;
        channel.outgoingUnreliable.push_back(umsg);

        // Faim/sante : decroissance simple, pas de mort/respawn en V1 (clamp a kMinHealth).
        bool hungerChanged = false;
        bool healthChanged = false;

        channel.hungerAccumulator += dt;
        while (channel.hungerAccumulator >= kHungerTickSeconds) {
            channel.hungerAccumulator -= kHungerTickSeconds;
            if (channel.hunger > 0) {
                channel.hunger -= 1;
                hungerChanged = true;
            }
        }

        if (channel.hunger == 0) {
            channel.healthAccumulator += dt;
            while (channel.healthAccumulator >= kHealthTickSeconds) {
                channel.healthAccumulator -= kHealthTickSeconds;
                if (channel.health > kMinHealth) {
                    channel.health -= 1;
                    healthChanged = true;
                }
            }
        } else {
            channel.healthAccumulator = 0.0f;
        }

        if (hungerChanged || healthChanged) {
            common::messages::HealthHungerUpdateMsg hh{channel.health, channel.hunger};
            PushReliable(channel, common::messages::ReliableMsgType::HealthHungerUpdate, hh);
        }

        // Streaming de chunks : necessite de savoir ou est le joueur.
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
