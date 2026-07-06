#include <cmath>
#include <cstdio>

#include "raylib.h"

#include "client/input/voxel_raycast.h"
#include "client/render/chunk_renderer.h"
#include "client/render/sky_renderer.h"
#include "client/ui/hud.h"
#include "client/world/client_world.h"
#include "common/transport/transport.h"
#include "transports/loopback/loopback_transport.h"

namespace {

common::world::ChunkCoord ChunkCoordFromBlock(int worldX, int worldZ) {
    int cx = static_cast<int>(std::floor(static_cast<double>(worldX) / common::world::CHUNK_SIZE_X));
    int cz = static_cast<int>(std::floor(static_cast<double>(worldZ) / common::world::CHUNK_SIZE_Z));
    return {cx, cz};
}

} // namespace

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "voxel-game client (jour 4 - persistance, sante/faim, jour-nuit)");
    SetTargetFPS(60);

    Transport* transport = loopback_transport_create("world", 1234, "Player1");

    client::ClientWorld clientWorld;
    client::ChunkRenderer chunkRenderer;
    common::messages::InventoryUpdateMsg inventory{};
    int selectedSlot = 0;
    uint16_t health = 100;
    uint16_t hunger = 100;
    float timeOfDay01 = 0.5f;

    Camera3D camera{};
    camera.position = {8.0f, 40.0f, 8.0f};
    camera.target = {8.0f, 40.0f, 9.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();

    uint32_t inputTick = 0;
    constexpr float kReachDistance = 6.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        UpdateCamera(&camera, CAMERA_FREE);

        for (int key = KEY_ONE; key <= KEY_NINE; ++key) {
            if (IsKeyPressed(key)) selectedSlot = key - KEY_ONE;
        }
        float wheel = GetMouseWheelMove();
        if (wheel > 0.0f) selectedSlot = (selectedSlot + client::kHotbarSlotCount - 1) % client::kHotbarSlotCount;
        if (wheel < 0.0f) selectedSlot = (selectedSlot + 1) % client::kHotbarSlotCount;

        Vector3 forward{camera.target.x - camera.position.x, camera.target.y - camera.position.y,
                         camera.target.z - camera.position.z};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            client::RaycastHit hit = client::RaycastVoxels(clientWorld, camera.position, forward, kReachDistance);
            if (hit.hit) {
                common::messages::ReliableMessage req;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    common::messages::BreakBlockRequestMsg breakReq;
                    breakReq.coord = ChunkCoordFromBlock(hit.blockX, hit.blockZ);
                    breakReq.lx = static_cast<uint8_t>(hit.blockX - breakReq.coord.x * common::world::CHUNK_SIZE_X);
                    breakReq.ly = static_cast<uint8_t>(hit.blockY);
                    breakReq.lz = static_cast<uint8_t>(hit.blockZ - breakReq.coord.z * common::world::CHUNK_SIZE_Z);
                    req.type = common::messages::ReliableMsgType::BreakBlockRequest;
                    req.payload = breakReq;
                } else {
                    common::messages::PlaceBlockRequestMsg placeReq;
                    placeReq.coord = ChunkCoordFromBlock(hit.prevX, hit.prevZ);
                    placeReq.lx = static_cast<uint8_t>(hit.prevX - placeReq.coord.x * common::world::CHUNK_SIZE_X);
                    placeReq.ly = static_cast<uint8_t>(hit.prevY);
                    placeReq.lz = static_cast<uint8_t>(hit.prevZ - placeReq.coord.z * common::world::CHUNK_SIZE_Z);
                    placeReq.hotbarSlot = static_cast<uint8_t>(selectedSlot);
                    req.type = common::messages::ReliableMsgType::PlaceBlockRequest;
                    req.payload = placeReq;
                }
                transport_send_reliable(transport, req);
            }
        }

        common::messages::PlayerInputMsg inputMsg;
        inputMsg.posX = camera.position.x;
        inputMsg.posY = camera.position.y;
        inputMsg.posZ = camera.position.z;
        inputMsg.tick = inputTick++;

        common::messages::UnreliableMessage input;
        input.type = common::messages::UnreliableMsgType::PlayerInput;
        input.payload = inputMsg;
        transport_send_unreliable(transport, input);

        transport_tick(transport, dt);

        common::messages::ReliableMessage rmsg;
        while (transport_poll_reliable(transport, rmsg)) {
            switch (rmsg.type) {
                case common::messages::ReliableMsgType::ChunkData: {
                    const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(rmsg.payload);
                    common::world::Chunk chunk;
                    chunk.coord = chunkMsg.coord;
                    chunk.blocks = chunkMsg.blocks;
                    clientWorld.UpsertChunk(chunk);
                    chunkRenderer.UpsertChunk(chunk);
                    break;
                }
                case common::messages::ReliableMsgType::ChunkUnload: {
                    const auto& unloadMsg = std::get<common::messages::ChunkUnloadMsg>(rmsg.payload);
                    clientWorld.RemoveChunk(unloadMsg.coord);
                    chunkRenderer.RemoveChunk(unloadMsg.coord);
                    break;
                }
                case common::messages::ReliableMsgType::BlockUpdate: {
                    const auto& update = std::get<common::messages::BlockUpdateMsg>(rmsg.payload);
                    clientWorld.SetBlock(update.coord, update.lx, update.ly, update.lz, update.newBlockId);
                    const common::world::Chunk* chunk = clientWorld.FindChunk(update.coord);
                    if (chunk != nullptr) chunkRenderer.UpsertChunk(*chunk);
                    break;
                }
                case common::messages::ReliableMsgType::InventoryUpdate: {
                    inventory = std::get<common::messages::InventoryUpdateMsg>(rmsg.payload);
                    break;
                }
                case common::messages::ReliableMsgType::SpawnState: {
                    const auto& spawn = std::get<common::messages::SpawnStateMsg>(rmsg.payload);
                    camera.position = {spawn.posX, spawn.posY, spawn.posZ};
                    camera.target = {spawn.posX, spawn.posY, spawn.posZ + 1.0f};
                    health = spawn.health;
                    hunger = spawn.hunger;
                    break;
                }
                case common::messages::ReliableMsgType::HealthHungerUpdate: {
                    const auto& hh = std::get<common::messages::HealthHungerUpdateMsg>(rmsg.payload);
                    health = hh.health;
                    hunger = hh.hunger;
                    break;
                }
                default:
                    break;
            }
        }

        common::messages::UnreliableMessage umsg;
        while (transport_poll_unreliable(transport, umsg)) {
            if (umsg.type == common::messages::UnreliableMsgType::WorldTime) {
                timeOfDay01 = std::get<common::messages::WorldTimeMsg>(umsg.payload).timeOfDay01;
            }
        }

        client::DayNightState dayNight = client::ComputeDayNight(timeOfDay01);

        BeginDrawing();
        ClearBackground(dayNight.skyColor);

        BeginMode3D(camera);
        chunkRenderer.DrawAll(dayNight.groundTint);
        EndMode3D();

        DrawLine(screenWidth / 2 - 8, screenHeight / 2, screenWidth / 2 + 8, screenHeight / 2, WHITE);
        DrawLine(screenWidth / 2, screenHeight / 2 - 8, screenWidth / 2, screenHeight / 2 + 8, WHITE);

        char info[128];
        std::snprintf(info, sizeof(info), "Chunks charges: %zu", chunkRenderer.LoadedCount());
        DrawText(info, 10, 10, 20, WHITE);

        char posText[128];
        std::snprintf(posText, sizeof(posText), "Pos: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z);
        DrawText(posText, 10, 35, 20, WHITE);

        char statsText[128];
        std::snprintf(statsText, sizeof(statsText), "Sante: %d  Faim: %d  Heure: %.0f%%", health, hunger, timeOfDay01 * 100.0f);
        DrawText(statsText, 10, 60, 20, WHITE);

        DrawFPS(10, 85);
        client::DrawHotbar(inventory, selectedSlot, screenWidth, screenHeight);
        EndDrawing();
    }

    transport_destroy(transport);
    CloseWindow();
    return 0;
}
