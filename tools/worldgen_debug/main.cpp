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

bool TestCrafting() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world_crafting_test";
    cfg.seed = 99; // seed connu pour contenir des arbres pres du spawn (cf. TestTreesAndWater)

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

    // Trouve un bloc de bois dans les chunks streames.
    common::world::ChunkCoord woodCoord{};
    int woodLx = -1, woodLy = -1, woodLz = -1;
    bool woodFound = false;
    common::messages::ReliableMessage msg;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type != common::messages::ReliableMsgType::ChunkData || woodFound) continue;
        const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(msg.payload);
        for (int x = 0; x < common::world::CHUNK_SIZE_X && !woodFound; ++x) {
            for (int y = 0; y < common::world::CHUNK_SIZE_Y && !woodFound; ++y) {
                for (int z = 0; z < common::world::CHUNK_SIZE_Z && !woodFound; ++z) {
                    if (BlockAt(chunkMsg, x, y, z) == static_cast<uint8_t>(common::world::BlockId::Wood)) {
                        woodCoord = chunkMsg.coord;
                        woodLx = x;
                        woodLy = y;
                        woodLz = z;
                        woodFound = true;
                    }
                }
            }
        }
    }

    if (!woodFound) {
        std::printf("CRAFTING: ECHEC (aucun bloc de bois trouve pour amorcer le test)\n");
        server_core::destroy(core);
        return false;
    }
    std::printf("[crafting] bois trouve en chunk(%d,%d) local(%d,%d,%d)\n", woodCoord.x, woodCoord.z, woodLx, woodLy, woodLz);

    common::messages::BreakBlockRequestMsg breakReq{woodCoord, static_cast<uint8_t>(woodLx), static_cast<uint8_t>(woodLy),
                                                     static_cast<uint8_t>(woodLz)};
    common::messages::ReliableMessage breakMsg;
    breakMsg.type = common::messages::ReliableMsgType::BreakBlockRequest;
    breakMsg.payload = breakReq;
    server_core::submit_reliable(core, client, breakMsg);

    common::messages::InventoryUpdateMsg inv{};
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::InventoryUpdate) {
            inv = std::get<common::messages::InventoryUpdateMsg>(msg.payload);
        }
    }

    int woodSlot = -1;
    for (size_t i = 0; i < inv.slots.size(); ++i) {
        if (inv.slots[i].blockId == static_cast<uint8_t>(common::world::BlockId::Wood) && inv.slots[i].count > 0) {
            woodSlot = static_cast<int>(i);
            break;
        }
    }
    if (woodSlot < 0) {
        std::printf("CRAFTING: ECHEC (le bois casse n'est pas arrive en inventaire)\n");
        server_core::destroy(core);
        return false;
    }

    // Recette 1 : bois au centre -> 4 planches.
    common::messages::CraftRequestMsg craft1;
    craft1.gridSlots.fill(common::messages::kCraftSlotEmpty);
    craft1.gridSlots[4] = static_cast<uint8_t>(woodSlot);
    common::messages::ReliableMessage craft1Msg;
    craft1Msg.type = common::messages::ReliableMsgType::CraftRequest;
    craft1Msg.payload = craft1;
    server_core::submit_reliable(core, client, craft1Msg);

    bool planksOk = false;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::CraftResponse) {
            const auto& resp = std::get<common::messages::CraftResponseMsg>(msg.payload);
            planksOk = resp.success && resp.resultBlockId == static_cast<uint8_t>(common::world::BlockId::Planks) &&
                       resp.resultCount == 4;
            std::printf("[crafting] recette 1 (bois->planches) : success=%d blockId=%d count=%d\n", resp.success,
                        resp.resultBlockId, resp.resultCount);
        } else if (msg.type == common::messages::ReliableMsgType::InventoryUpdate) {
            inv = std::get<common::messages::InventoryUpdateMsg>(msg.payload);
        }
    }

    int planksSlot = -1;
    for (size_t i = 0; i < inv.slots.size(); ++i) {
        if (inv.slots[i].blockId == static_cast<uint8_t>(common::world::BlockId::Planks) && inv.slots[i].count >= 2) {
            planksSlot = static_cast<int>(i);
            break;
        }
    }
    if (planksSlot < 0) {
        std::printf("CRAFTING: ECHEC (pas assez de planches apres la recette 1)\n");
        server_core::destroy(core);
        return false;
    }

    // Recette 2 : 2 planches empilees (colonne du milieu) -> 4 batons.
    common::messages::CraftRequestMsg craft2;
    craft2.gridSlots.fill(common::messages::kCraftSlotEmpty);
    craft2.gridSlots[1] = static_cast<uint8_t>(planksSlot);
    craft2.gridSlots[4] = static_cast<uint8_t>(planksSlot);
    common::messages::ReliableMessage craft2Msg;
    craft2Msg.type = common::messages::ReliableMsgType::CraftRequest;
    craft2Msg.payload = craft2;
    server_core::submit_reliable(core, client, craft2Msg);

    bool sticksOk = false;
    while (server_core::poll_outgoing_reliable(core, client, msg)) {
        if (msg.type == common::messages::ReliableMsgType::CraftResponse) {
            const auto& resp = std::get<common::messages::CraftResponseMsg>(msg.payload);
            sticksOk = resp.success && resp.resultBlockId == static_cast<uint8_t>(common::world::BlockId::Stick) &&
                       resp.resultCount == 4;
            std::printf("[crafting] recette 2 (planches->batons) : success=%d blockId=%d count=%d\n", resp.success,
                        resp.resultBlockId, resp.resultCount);
        }
    }

    std::printf("CRAFTING: %s\n", (planksOk && sticksOk) ? "OK" : "ECHEC");
    server_core::destroy(core);
    return planksOk && sticksOk;
}

bool TestMobs() {
    server_core::ServerConfig cfg;
    cfg.worldSaveDir = "world_mobs_test";
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

    size_t lastCount = 0;
    for (int i = 0; i < 20; ++i) {
        server_core::tick(core, 0.1f);
        common::messages::ReliableMessage rmsg;
        while (server_core::poll_outgoing_reliable(core, client, rmsg)) {} // draine

        common::messages::UnreliableMessage umsg;
        while (server_core::poll_outgoing_unreliable(core, client, umsg)) {
            if (umsg.type == common::messages::UnreliableMsgType::EntitySnapshot) {
                lastCount = std::get<common::messages::EntitySnapshotMsg>(umsg.payload).entities.size();
            }
        }
    }

    std::printf("[mobs] %zu entite(s) presente(s) apres streaming + 20 ticks\n", lastCount);
    bool ok = lastCount > 0;
    std::printf("MOBS: %s\n", ok ? "OK" : "ECHEC (aucun mob n'est apparu dans ce voisinage)");

    server_core::destroy(core);
    return ok;
}

} // namespace

int main() {
    bool persistenceOk = TestPersistence();
    bool physicsOk = TestBlockPhysics();
    bool treesWaterOk = TestTreesAndWater();
    bool craftingOk = TestCrafting();
    bool mobsOk = TestMobs();

    return (persistenceOk && physicsOk && treesWaterOk && craftingOk && mobsOk) ? 0 : 1;
}
