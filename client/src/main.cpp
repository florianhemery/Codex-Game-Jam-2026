#include <optional>

#include "raylib.h"

#include "common/transport/transport.h"
#include "common/world/block.h"
#include "common/world/chunk.h"
#include "transports/loopback/loopback_transport.h"

namespace {

Color ColorForBlock(common::world::BlockId id) {
    switch (id) {
        case common::world::BlockId::Stone: return GRAY;
        case common::world::BlockId::Dirt: return BROWN;
        case common::world::BlockId::Grass: return GREEN;
        case common::world::BlockId::Air: default: return BLANK;
    }
}

// Jour 1 : un cube par bloc solide, aucune optimisation de mesh. Remplace au
// jour 2/3 par un vrai mesher (naif puis greedy) une fois le pipeline reseau
// (ici, le transport) prouve.
void DrawChunkNaive(const common::world::Chunk& chunk) {
    using namespace common::world;
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                auto id = static_cast<BlockId>(chunk.blocks[BlockIndex(x, y, z)]);
                if (id == BlockId::Air) continue;

                Vector3 pos{
                    static_cast<float>(chunk.coord.x * CHUNK_SIZE_X + x) + 0.5f,
                    static_cast<float>(y) + 0.5f,
                    static_cast<float>(chunk.coord.z * CHUNK_SIZE_Z + z) + 0.5f,
                };
                DrawCube(pos, 1.0f, 1.0f, 1.0f, ColorForBlock(id));
                DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, BLACK);
            }
        }
    }
}

} // namespace

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "voxel-game client (jour 1 - loopback transport)");
    SetTargetFPS(60);

    Transport* transport = loopback_transport_create("world", 1234, "Player1");

    Camera3D camera{};
    camera.position = {8.0f, 12.0f, 24.0f};
    camera.target = {8.0f, 8.0f, 8.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::optional<common::world::Chunk> receivedChunk;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        common::messages::UnreliableMessage input;
        input.type = common::messages::UnreliableMsgType::PlayerInput;
        input.payload = common::messages::PlayerInputMsg{};
        transport_send_unreliable(transport, input);

        transport_tick(transport, dt);

        common::messages::ReliableMessage rmsg;
        while (transport_poll_reliable(transport, rmsg)) {
            if (rmsg.type == common::messages::ReliableMsgType::ChunkData) {
                const auto& chunkMsg = std::get<common::messages::ChunkDataMsg>(rmsg.payload);
                common::world::Chunk chunk;
                chunk.coord = chunkMsg.coord;
                chunk.blocks = chunkMsg.blocks;
                receivedChunk = chunk;
            }
        }

        UpdateCamera(&camera, CAMERA_FREE);

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        if (receivedChunk.has_value()) {
            DrawChunkNaive(*receivedChunk);
        }
        EndMode3D();

        DrawText(receivedChunk.has_value() ? "Chunk (0,0) recu via transport_poll_reliable"
                                            : "En attente du chunk...",
                  10, 10, 20, DARKGRAY);
        DrawFPS(10, 40);
        EndDrawing();
    }

    transport_destroy(transport);
    CloseWindow();
    return 0;
}
