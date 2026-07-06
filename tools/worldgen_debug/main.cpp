#include <cstdio>

#include "common/world/block.h"
#include "common/world/chunk.h"
#include "server_core/block_physics.h"
#include "server_core/server_core.h"
#include "server_core/world_manager.h"

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

bool TestPersistence() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world";
    cfg.seed = 1234;

    server_core::ServerCore* coreA = server_core::create(cfg);
    server_core::ClientId clientA = server_core::connect_client(coreA, "debug");

    common::messages::ChunkDataMsg chunkA{};
    if (!StreamOriginChunk(coreA, clientA, chunkA)) {
        std::printf("ERREUR: chunk (0,0) absent au run A.\n");
        server_core::destroy(coreA);
        return false;
    }

    int surfaceY = -1;
    for (int y = common::world::CHUNK_SIZE_Y - 1; y >= 0; --y) {
        if (static_cast<common::world::BlockId>(BlockAt(chunkA, 8, y, 8)) != common::world::BlockId::Air) {
            surfaceY = y;
            break;
        }
    }
    std::printf("[persistance] surface en (8,_,8) : y=%d, blockId=%d\n", surfaceY, BlockAt(chunkA, 8, surfaceY, 8));

    common::messages::BreakBlockRequestMsg breakReq{{0, 0}, 8, static_cast<uint8_t>(surfaceY), 8};
    common::messages::ReliableMessage breakMsg;
    breakMsg.type = common::messages::ReliableMsgType::BreakBlockRequest;
    breakMsg.payload = breakReq;
    server_core::submit_reliable(coreA, clientA, breakMsg);

    common::messages::ReliableMessage drain;
    while (server_core::poll_outgoing_reliable(coreA, clientA, drain)) {}

    server_core::disconnect_client(coreA, clientA);
    server_core::destroy(coreA);

    server_core::ServerCore* coreB = server_core::create(cfg);
    server_core::ClientId clientB = server_core::connect_client(coreB, "debug");

    common::messages::ChunkDataMsg chunkB{};
    if (!StreamOriginChunk(coreB, clientB, chunkB)) {
        std::printf("ERREUR: chunk (0,0) absent au run B.\n");
        server_core::destroy(coreB);
        return false;
    }

    uint8_t blockAfterReload = BlockAt(chunkB, 8, surfaceY, 8);
    bool ok = (blockAfterReload == static_cast<uint8_t>(common::world::BlockId::Air));
    std::printf("PERSISTANCE: %s (blockId=%d apres reload)\n", ok ? "OK" : "ECHEC", blockAfterReload);

    server_core::destroy(coreB);
    return ok;
}

bool TestBlockPhysics() {
    server_core::WorldManager wm(4242, "world_physics_test");
    wm.UpdateViewer(1, {0, 0}, 1); // charge un voisinage 3x3 autour de l'origine

    // Force un scenario deterministe : sable a y=30 avec de l'air en dessous,
    // independant de ce que la generation procedurale a mis a cet endroit.
    wm.SetBlockWorld(5, 30, 5, static_cast<uint8_t>(common::world::BlockId::Sand));
    wm.SetBlockWorld(5, 29, 5, static_cast<uint8_t>(common::world::BlockId::Air));

    server_core::BlockPhysics physics;
    physics.NotifyChanged(5, 30, 5);

    uint8_t before30 = 0, before29 = 0;
    wm.GetBlockWorld(5, 30, 5, before30);
    wm.GetBlockWorld(5, 29, 5, before29);
    std::printf("[physique] avant tick: y=30 -> %d, y=29 -> %d\n", before30, before29);

    // La colonne peut etre vide sur plusieurs blocs sous y=30 (terrain naturel
    // en dessous) : le sable cascade jusqu'a se poser sur le premier support,
    // pas forcement une seule case plus bas. On verifie juste qu'il a bouge
    // et qu'il repose bien quelque part plus bas.
    for (int i = 0; i < 5; ++i) {
        std::vector<server_core::BlockChangeEvent> events;
        physics.Tick(wm, events);
        if (events.empty()) break;
    }

    uint8_t at30 = 0;
    wm.GetBlockWorld(5, 30, 5, at30);

    int restingY = -1;
    for (int y = 29; y >= 0; --y) {
        uint8_t id = 0;
        wm.GetBlockWorld(5, y, 5, id);
        if (id != static_cast<uint8_t>(common::world::BlockId::Air)) {
            restingY = y;
            break;
        }
    }
    uint8_t restingId = 0;
    if (restingY >= 0) wm.GetBlockWorld(5, restingY, 5, restingId);

    bool fell = (at30 == static_cast<uint8_t>(common::world::BlockId::Air)) && restingY >= 0 &&
                restingId == static_cast<uint8_t>(common::world::BlockId::Sand);
    std::printf("[physique] apres stabilisation : y=30 -> %d, sable repose en y=%d\n", at30, restingY);
    std::printf("PHYSIQUE (chute de sable): %s\n", fell ? "OK" : "ECHEC");
    return fell;
}

bool TestTreesAndWater() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world_treewater_test";
    cfg.seed = 99;

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

    long long woodCount = 0, leavesCount = 0, waterCount = 0;
    int chunkCount = 0;
    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::ChunkData) {
            const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(msg.payload);
            ++chunkCount;
            for (uint8_t b : chunkMsg.blocks) {
                auto id = static_cast<common::world::BlockId>(b);
                if (id == common::world::BlockId::Wood) ++woodCount;
                if (id == common::world::BlockId::Leaves) ++leavesCount;
                if (id == common::world::BlockId::Water) ++waterCount;
            }
        }
    }

    std::printf("[arbres/eau] %d chunks scannes : bois=%lld feuilles=%lld eau=%lld\n",
                chunkCount, woodCount, leavesCount, waterCount);
    bool ok = woodCount > 0 && leavesCount > 0 && waterCount > 0;
    std::printf("ARBRES/EAU: %s\n", ok ? "OK" : "ECHEC (aucune generation trouvee dans ce voisinage)");

    server_core::destroy(core);
    return ok;
}

} // namespace

int main() {
    bool persistenceOk = TestPersistence();
    bool physicsOk = TestBlockPhysics();
    bool treesWaterOk = TestTreesAndWater();

    return (persistenceOk && physicsOk && treesWaterOk) ? 0 : 1;
}
