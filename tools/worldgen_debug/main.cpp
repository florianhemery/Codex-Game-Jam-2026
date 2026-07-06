#include <cstdio>

#include "common/world/block.h"
#include "common/world/chunk.h"
#include "server_core/server_core.h"

namespace {

bool StreamOriginChunk(server_core::ServerCore* core, server_core::ClientId client,
                        common::messages::ChunkDataMsg& outChunk) {
    common::messages::PlayerInputMsg inputMsg;
    inputMsg.posX = 8.0f;
    inputMsg.posY = 40.0f;
    inputMsg.posZ = 8.0f;

    common::messages::UnreliableMessage input;
    input.type = common::messages::UnreliableMsgType::PlayerInput;
    input.payload = inputMsg;
    server_core::submit_unreliable(core, client, input);
    server_core::tick(core, 0.05f);

    bool found = false;
    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::ChunkData) {
            const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(msg.payload);
            if (chunkMsg.coord.x == 0 && chunkMsg.coord.z == 0) {
                outChunk = chunkMsg;
                found = true;
            }
        }
    }
    return found;
}

uint8_t BlockAt(const common::messages::ChunkDataMsg& chunk, int x, int y, int z) {
    return chunk.blocks[common::world::BlockIndex(x, y, z)];
}

} // namespace

int main() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world";
    cfg.seed = 1234;

    // --- Run A : casse un bloc de surface, sauvegarde a la destruction du core ---
    server_core::ServerCore* coreA = server_core::create(cfg);
    server_core::ClientId clientA = server_core::connect_client(coreA, "debug");

    common::messages::ChunkDataMsg chunkA{};
    if (!StreamOriginChunk(coreA, clientA, chunkA)) {
        std::printf("ERREUR: chunk (0,0) absent au run A.\n");
        server_core::destroy(coreA);
        return 1;
    }

    int surfaceY = -1;
    for (int y = common::world::CHUNK_SIZE_Y - 1; y >= 0; --y) {
        if (static_cast<common::world::BlockId>(BlockAt(chunkA, 8, y, 8)) != common::world::BlockId::Air) {
            surfaceY = y;
            break;
        }
    }
    std::printf("[run A] surface en (8,_,8) : y=%d, blockId=%d\n", surfaceY, BlockAt(chunkA, 8, surfaceY, 8));

    common::messages::BreakBlockRequestMsg breakReq{{0, 0}, 8, static_cast<uint8_t>(surfaceY), 8};
    common::messages::ReliableMessage breakMsg;
    breakMsg.type = common::messages::ReliableMsgType::BreakBlockRequest;
    breakMsg.payload = breakReq;
    server_core::submit_reliable(coreA, clientA, breakMsg);

    // draine les messages consequents (BlockUpdate + InventoryUpdate) sans les traiter
    common::messages::ReliableMessage drain;
    while (server_core::poll_outgoing_reliable(coreA, clientA, drain)) {}

    server_core::disconnect_client(coreA, clientA); // sauvegarde le joueur (position, inventaire)
    server_core::destroy(coreA);
    std::printf("[run A] bloc casse en y=%d, core A detruit (sauvegarde sur disque).\n", surfaceY);

    // --- Run B : nouvelle instance ServerCore independante, meme dossier de sauvegarde ---
    server_core::ServerCore* coreB = server_core::create(cfg);
    server_core::ClientId clientB = server_core::connect_client(coreB, "debug");

    common::messages::ChunkDataMsg chunkB{};
    if (!StreamOriginChunk(coreB, clientB, chunkB)) {
        std::printf("ERREUR: chunk (0,0) absent au run B.\n");
        server_core::destroy(coreB);
        return 1;
    }

    uint8_t blockAfterReload = BlockAt(chunkB, 8, surfaceY, 8);
    std::printf("[run B] meme position apres reload : blockId=%d (attendu 0=air)\n", blockAfterReload);

    bool persistenceOk = (blockAfterReload == static_cast<uint8_t>(common::world::BlockId::Air));
    std::printf("PERSISTANCE: %s\n", persistenceOk ? "OK" : "ECHEC");

    server_core::destroy(coreB);
    return persistenceOk ? 0 : 1;
}
