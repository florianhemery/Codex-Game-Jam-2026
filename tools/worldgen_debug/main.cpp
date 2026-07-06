#include <cstdio>

#include "common/world/block.h"
#include "common/world/chunk.h"
#include "server_core/server_core.h"

namespace {

void PrintReliable(server_core::ServerCore* core, server_core::ClientId client, const char* label) {
    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        switch (msg.type) {
            case common::messages::ReliableMsgType::BlockUpdate: {
                const auto& u = std::get<common::messages::BlockUpdateMsg>(msg.payload);
                std::printf("[%s] BlockUpdate chunk(%d,%d) local(%d,%d,%d) -> blockId=%d\n",
                            label, u.coord.x, u.coord.z, u.lx, u.ly, u.lz, u.newBlockId);
                break;
            }
            case common::messages::ReliableMsgType::InventoryUpdate: {
                const auto& inv = std::get<common::messages::InventoryUpdateMsg>(msg.payload);
                std::printf("[%s] InventoryUpdate slot0 = blockId=%d count=%d\n",
                            label, inv.slots[0].blockId, inv.slots[0].count);
                break;
            }
            default:
                break;
        }
    }
}

} // namespace

int main() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world";
    cfg.seed = 1234;

    server_core::ServerCore* core = server_core::create(cfg);
    server_core::ClientId client = server_core::connect_client(core, "debug");

    common::messages::PlayerInputMsg inputMsg;
    inputMsg.posX = 8.0f;
    inputMsg.posY = 40.0f;
    inputMsg.posZ = 8.0f;

    common::messages::UnreliableMessage input;
    input.type = common::messages::UnreliableMsgType::PlayerInput;
    input.payload = inputMsg;
    server_core::submit_unreliable(core, client, input);

    server_core::tick(core, 0.05f);

    int chunkCount = 0;
    long long totalSolidBlocks = 0;
    common::messages::ChunkDataMsg originChunk{};
    bool haveOriginChunk = false;

    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::ChunkData) {
            const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(msg.payload);
            ++chunkCount;
            int solid = 0;
            for (uint8_t block : chunkMsg.blocks) {
                if (static_cast<common::world::BlockId>(block) != common::world::BlockId::Air) ++solid;
            }
            totalSolidBlocks += solid;
            if (chunkMsg.coord.x == 0 && chunkMsg.coord.z == 0) {
                originChunk = chunkMsg;
                haveOriginChunk = true;
            }
        }
    }

    std::printf("worldgen_debug: %d chunk(s) streames, %lld blocs solides au total.\n", chunkCount, totalSolidBlocks);

    if (!haveOriginChunk) {
        std::printf("ERREUR: chunk (0,0) absent, impossible de tester casser/placer.\n");
        server_core::destroy(core);
        return 1;
    }

    int surfaceY = -1;
    for (int y = common::world::CHUNK_SIZE_Y - 1; y >= 0; --y) {
        if (static_cast<common::world::BlockId>(originChunk.blocks[common::world::BlockIndex(8, y, 8)]) !=
            common::world::BlockId::Air) {
            surfaceY = y;
            break;
        }
    }
    std::printf("Colonne (8,_,8) : surface trouvee a y=%d\n", surfaceY);

    common::messages::BreakBlockRequestMsg breakReq{{0, 0}, 8, static_cast<uint8_t>(surfaceY), 8};
    common::messages::ReliableMessage breakMsg;
    breakMsg.type = common::messages::ReliableMsgType::BreakBlockRequest;
    breakMsg.payload = breakReq;
    server_core::submit_reliable(core, client, breakMsg);
    PrintReliable(core, client, "casser");

    common::messages::PlaceBlockRequestMsg placeReq{{0, 0}, 8, static_cast<uint8_t>(surfaceY), 8, 0};
    common::messages::ReliableMessage placeMsg;
    placeMsg.type = common::messages::ReliableMsgType::PlaceBlockRequest;
    placeMsg.payload = placeReq;
    server_core::submit_reliable(core, client, placeMsg);
    PrintReliable(core, client, "poser");

    server_core::disconnect_client(core, client);
    server_core::destroy(core);
    return 0;
}
