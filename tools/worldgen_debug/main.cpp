#include <cstdio>

#include "server_core/server_core.h"
#include "common/world/block.h"
#include "common/world/chunk.h"

int main() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world";
    cfg.seed = 1234;

    server_core::ServerCore* core = server_core::create(cfg);
    server_core::ClientId client = server_core::connect_client(core, "debug");

    int chunkCount = 0;
    int solidBlocks = 0;
    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::ChunkData) {
            const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(msg.payload);
            ++chunkCount;
            for (uint8_t block : chunkMsg.blocks) {
                if (static_cast<common::world::BlockId>(block) != common::world::BlockId::Air) {
                    ++solidBlocks;
                }
            }
            std::printf("Chunk (%d,%d) recu : %d blocs solides / %d\n",
                        chunkMsg.coord.x, chunkMsg.coord.z, solidBlocks,
                        common::world::CHUNK_BLOCK_COUNT);
        }
    }

    for (int i = 0; i < 5; ++i) {
        server_core::tick(core, 0.05f);
    }

    server_core::disconnect_client(core, client);
    server_core::destroy(core);

    std::printf("worldgen_debug: %d chunk(s) recus, headless, sans raylib.\n", chunkCount);
    return 0;
}
