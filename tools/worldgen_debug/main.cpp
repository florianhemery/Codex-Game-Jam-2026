#include <cstdio>

#include "common/world/block.h"
#include "common/world/chunk.h"
#include "server_core/server_core.h"

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
            std::printf("Chunk (%d,%d) : %d blocs solides / %d\n",
                        chunkMsg.coord.x, chunkMsg.coord.z, solid, common::world::CHUNK_BLOCK_COUNT);
        }
    }

    server_core::disconnect_client(core, client);
    server_core::destroy(core);

    std::printf("worldgen_debug: %d chunk(s) streames autour de (8,_,8), %lld blocs solides au total, headless sans raylib.\n",
                chunkCount, totalSolidBlocks);
    return 0;
}
