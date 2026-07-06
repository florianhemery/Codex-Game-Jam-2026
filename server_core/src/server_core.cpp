#include "server_core/server_core.h"

#include <deque>
#include <optional>
#include <unordered_map>

#include "common/world/world_coords.h"
#include "server_core/world_manager.h"

namespace server_core {

namespace {

struct ClientChannel {
    std::deque<common::messages::ReliableMessage> outgoingReliable;
    std::deque<common::messages::UnreliableMessage> outgoingUnreliable;
    std::optional<common::messages::PlayerInputMsg> latestInput;
};

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

void submit_reliable(ServerCore* sc, ClientId from, const common::messages::ReliableMessage& msg) {
    (void)sc;
    (void)from;
    (void)msg; // rien a traiter au jour 2 -- BlockUpdate arrive au jour 3
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

            common::messages::ReliableMessage rmsg;
            rmsg.type = common::messages::ReliableMsgType::ChunkData;
            rmsg.payload = chunkMsg;
            channel.outgoingReliable.push_back(rmsg);
        }

        for (const auto& coord : delta.toUnload) {
            common::messages::ChunkUnloadMsg unloadMsg;
            unloadMsg.coord = coord;

            common::messages::ReliableMessage rmsg;
            rmsg.type = common::messages::ReliableMsgType::ChunkUnload;
            rmsg.payload = unloadMsg;
            channel.outgoingReliable.push_back(rmsg);
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
