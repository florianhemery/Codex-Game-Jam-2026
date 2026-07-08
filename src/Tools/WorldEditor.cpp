/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Minimal Aurelia world editor — heightmap / splat preview
*/

#include "raylib.h"

#include "World/Chunk/ChunkGenerator.hpp"
#include "World/Stream/ChunkStreamer.hpp"

int main()
{
    const int screenW = 960;
    const int screenH = 720;
    InitWindow(screenW, screenH, "Aurelia World Editor (preview)");
    SetTargetFPS(60);

    racer::world::ChunkStreamer streamer;
    int chunkX = 0;
    int chunkZ = 0;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_LEFT)) {
            --chunkX;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            ++chunkX;
        }
        if (IsKeyPressed(KEY_UP)) {
            --chunkZ;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            ++chunkZ;
        }

        float wx = static_cast<float>(chunkX) * racer::world::kChunkSize
            + racer::world::kChunkSize * 0.5f;
        float wz = static_cast<float>(chunkZ) * racer::world::kChunkSize
            + racer::world::kChunkSize * 0.5f;
        streamer.updateCenter(wx, wz);
        streamer.ensureLoaded();

        BeginDrawing();
        ClearBackground(Color{24, 28, 36, 255});

        const racer::world::ChunkData *focus = nullptr;
        for (const auto &c : streamer.loadedChunks()) {
            if (c.id.x == chunkX && c.id.z == chunkZ) {
                focus = &c;
                break;
            }
        }

        if (focus) {
            const int res = racer::world::kChunkResolution;
            const float cell = 180.0f / static_cast<float>(res - 1);
            for (int iz = 0; iz < res; ++iz) {
                for (int ix = 0; ix < res; ++ix) {
                    size_t i = static_cast<size_t>(iz * res + ix);
                    float h = focus->heightmap[i];
                    uint8_t s = static_cast<uint8_t>(focus->splat[i]);
                    Color col = Color{
                        static_cast<unsigned char>(80 + s * 30),
                        static_cast<unsigned char>(100 + static_cast<int>(h * 2)),
                        static_cast<unsigned char>(60 + static_cast<int>(h)),
                        255};
                    DrawRectangle(40 + static_cast<int>(ix * cell),
                        80 + static_cast<int>(iz * cell),
                        static_cast<int>(cell) + 1,
                        static_cast<int>(cell) + 1, col);
                }
            }
            DrawText(TextFormat("Chunk (%d,%d) biome preview", chunkX, chunkZ),
                40, 24, 20, RAYWHITE);
        }

        DrawText("Fleches : changer de chunk   |   Echap : quitter", 40,
            screenH - 40, 18, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
