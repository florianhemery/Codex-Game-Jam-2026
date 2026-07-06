#include "server_core/server_core.h"

#include <deque>
#include <unordered_map>

#include "common/world/block.h"
#include "common/world/chunk.h"

namespace server_core {

namespace {

struct ClientChannel {
    std::deque<common::messages::ReliableMessage> outgoingReliable;
    std::deque<common::messages::UnreliableMessage> outgoingUnreliable;
};

// Jour 1 : un unique chunk plat code en dur, juste pour prouver que le chunk
// recu par le client transite bien par le transport. La vraie generation
// procedurale arrive au jour 2 (chunk_generator).
common::world::Chunk make_debug_flat_chunk() {
    common::world::Chunk chunk;
    chunk.coord = {0, 0};
    for (int x = 0; x < common::world::CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < common::world::CHUNK_SIZE_Z; ++z) {
            for (int y = 0; y < 8; ++y) {
                common::world::BlockId id = (y == 7) ? common::world::BlockId::Grass
                                                       : common::world::BlockId::Stone;
                chunk.blocks[common::world::BlockIndex(x, y, z)] = static_cast<uint8_t>(id);
            }
        }
    }
    return chunk;
}

} // namespace

struct ServerCore {
    ServerConfig config;
    common::world::Chunk debugChunk;
    std::unordered_map<ClientId, ClientChannel> clients;
    ClientId nextClientId = 0;
};

ServerCore* create(const ServerConfig& cfg) {
    auto* sc = new ServerCore();
    sc->config = cfg;
    sc->debugChunk = make_debug_flat_chunk();
    return sc;
}

void destroy(ServerCore* sc) {
    delete sc;
}

ClientId connect_client(ServerCore* sc, const std::string& playerName) {
    (void)playerName;
    ClientId id = sc->nextClientId++;
    ClientChannel& channel = sc->clients[id];

    common::messages::ChunkDataMsg chunkMsg;
    chunkMsg.coord = sc->debugChunk.coord;
    chunkMsg.blocks = sc->debugChunk.blocks;

    common::messages::ReliableMessage msg;
    msg.type = common::messages::ReliableMsgType::ChunkData;
    msg.payload = chunkMsg;
    channel.outgoingReliable.push_back(msg);

    return id;
}

void disconnect_client(ServerCore* sc, ClientId id) {
    sc->clients.erase(id);
}

void submit_reliable(ServerCore* sc, ClientId from, const common::messages::ReliableMessage& msg) {
    (void)sc;
    (void)from;
    (void)msg; // rien a traiter au jour 1 -- BlockUpdate arrive au jour 3
}

void submit_unreliable(ServerCore* sc, ClientId from, const common::messages::UnreliableMessage& msg) {
    (void)sc;
    (void)from;
    (void)msg; // le mouvement joueur arrive au jour 2
}

void tick(ServerCore* sc, float dt) {
    (void)sc;
    (void)dt; // simulation a cadence fixe a partir du jour 2
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
