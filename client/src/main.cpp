#include <cstdio>

#include "raylib.h"

#include "client/render/chunk_renderer.h"
#include "client/world/client_world.h"
#include "common/transport/transport.h"
#include "transports/loopback/loopback_transport.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "voxel-game client (jour 2 - streaming procedural)");
    SetTargetFPS(60);

    Transport* transport = loopback_transport_create("world", 1234, "Player1");

    client::ClientWorld clientWorld;
    client::ChunkRenderer chunkRenderer;

    Camera3D camera{};
    camera.position = {8.0f, 40.0f, 8.0f};
    camera.target = {8.0f, 40.0f, 9.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();

    uint32_t inputTick = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        UpdateCamera(&camera, CAMERA_FREE);

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
            if (rmsg.type == common::messages::ReliableMsgType::ChunkData) {
                const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(rmsg.payload);
                common::world::Chunk chunk;
                chunk.coord = chunkMsg.coord;
                chunk.blocks = chunkMsg.blocks;
                clientWorld.UpsertChunk(chunk);
                chunkRenderer.UpsertChunk(chunk);
            } else if (rmsg.type == common::messages::ReliableMsgType::ChunkUnload) {
                const auto& unloadMsg = std::get<common::messages::ChunkUnloadMsg>(rmsg.payload);
                clientWorld.RemoveChunk(unloadMsg.coord);
                chunkRenderer.RemoveChunk(unloadMsg.coord);
            }
        }

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        chunkRenderer.DrawAll();
        EndMode3D();

        char info[128];
        std::snprintf(info, sizeof(info), "Chunks charges: %zu", chunkRenderer.LoadedCount());
        DrawText(info, 10, 10, 20, DARKGRAY);

        char posText[128];
        std::snprintf(posText, sizeof(posText), "Pos: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z);
        DrawText(posText, 10, 35, 20, DARKGRAY);

        DrawFPS(10, 60);
        EndDrawing();
    }

    transport_destroy(transport);
    CloseWindow();
    return 0;
}
