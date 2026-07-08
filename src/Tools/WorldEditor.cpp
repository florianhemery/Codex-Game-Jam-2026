/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Aurelia world editor -- top-down heightmap preview + click-to-place
** waypoint authoring prototype for community circuits
*/

#include "raylib.h"
#include "raymath.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "World/Aurelia/AureliaBounds.hpp"
#include "World/Aurelia/AureliaTypes.hpp"
#include "World/Chunk/ChunkGenerator.hpp"
#include "World/Chunk/ChunkTypes.hpp"

namespace {

// ---------------------------------------------------------------------
// Waypoint authoring model
//
// A waypoint is a single (x, z) world position plus the terrain height at
// that point (kept only as authoring context -- the racing Track type
// itself only cares about the (x, z) plane, see Track::waypoints()).
// The ordered list below is exactly the shape TrackDef-style waypoint
// lists need: read back the wp_<i>_x / wp_<i>_z pairs in order and you
// have a std::vector<Vector2> ready to feed a Track.
// ---------------------------------------------------------------------
struct EditorWaypoint {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

constexpr int kScreenW = 1280;
constexpr int kScreenH = 820;

// On-screen rectangle the world map is drawn into.
constexpr int kMapX = 30;
constexpr int kMapY = 90;
constexpr int kMapW = 900;
constexpr int kMapH = 660;

constexpr const char *kCircuitDir = "circuits";
constexpr const char *kDefaultCircuitName = "prototype";
constexpr int kSaveFormatVersion = 1;

std::string circuitPath(const std::string &name)
{
    std::string safe;
    safe.reserve(name.size());
    for (char c : name) {
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == '_' || c == '-';
        safe.push_back(ok ? c : '_');
    }
    if (safe.empty()) {
        safe = kDefaultCircuitName;
    }
    return std::string(kCircuitDir) + "/circuit_" + safe + ".wpt";
}

// Maps a world (x, z) position inside WorldBounds onto a pixel inside the
// map viewport rectangle.
Vector2 worldToScreen(float worldX, float worldZ)
{
    float u = (worldX - racer::world::WorldBounds::minX)
        / racer::world::WorldBounds::width();
    float v = (worldZ - racer::world::WorldBounds::minZ)
        / racer::world::WorldBounds::height();
    return Vector2{
        static_cast<float>(kMapX) + u * static_cast<float>(kMapW),
        static_cast<float>(kMapY) + v * static_cast<float>(kMapH)};
}

// Inverse of worldToScreen(); returns false if the point falls outside
// the map viewport.
bool screenToWorld(Vector2 screen, float &outX, float &outZ)
{
    if (screen.x < kMapX || screen.x > kMapX + kMapW || screen.y < kMapY
        || screen.y > kMapY + kMapH) {
        return false;
    }
    float u = (screen.x - static_cast<float>(kMapX)) / static_cast<float>(kMapW);
    float v = (screen.y - static_cast<float>(kMapY)) / static_cast<float>(kMapH);
    outX = racer::world::WorldBounds::minX
        + u * racer::world::WorldBounds::width();
    outZ = racer::world::WorldBounds::minZ
        + v * racer::world::WorldBounds::height();
    return true;
}

// Colors a world sample by dominant biome + height, mirroring the shading
// the original per-chunk preview used (biome tint, height as brightness).
Color colorForWorldPoint(float worldX, float worldZ)
{
    using racer::world::BiomeId;
    using racer::world::biomeForChunk;
    using racer::world::worldToChunkId;

    racer::world::ChunkId cid = worldToChunkId(worldX, worldZ);
    BiomeId biome = biomeForChunk(cid.x, cid.z);
    float h = racer::world::ChunkGenerator::sampleWorldHeight(worldX, worldZ);
    int shade = static_cast<int>(h) * 2;

    switch (biome) {
    case BiomeId::COAST:
        return Color{
            static_cast<unsigned char>(Clamp(60 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(120 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(150 + shade, 0, 255)), 255};
    case BiomeId::FOREST:
        return Color{
            static_cast<unsigned char>(Clamp(40 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(100 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(50 + shade, 0, 255)), 255};
    case BiomeId::PORT:
        return Color{
            static_cast<unsigned char>(Clamp(90 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(90 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(100 + shade, 0, 255)), 255};
    case BiomeId::VOLCANO:
        return Color{
            static_cast<unsigned char>(Clamp(120 + shade, 0, 255)),
            static_cast<unsigned char>(Clamp(60 + shade / 2, 0, 255)),
            static_cast<unsigned char>(Clamp(50 + shade / 3, 0, 255)), 255};
    default:
        return Color{80, 80, 80, 255};
    }
}

// Bakes the whole playable world into a single texture once at startup
// (the procedural terrain/biomes are static, so there is no need to
// resample every frame). Resolution is deliberately modest: this is an
// authoring aid, not a gameplay minimap.
Texture2D buildWorldTexture()
{
    constexpr int kTexW = 340;
    constexpr int kTexH = 400;
    Image img = GenImageColor(kTexW, kTexH, BLACK);

    for (int py = 0; py < kTexH; ++py) {
        float v = (static_cast<float>(py) + 0.5f) / static_cast<float>(kTexH);
        float worldZ = racer::world::WorldBounds::minZ
            + v * racer::world::WorldBounds::height();
        for (int px = 0; px < kTexW; ++px) {
            float u = (static_cast<float>(px) + 0.5f) / static_cast<float>(kTexW);
            float worldX = racer::world::WorldBounds::minX
                + u * racer::world::WorldBounds::width();
            ImageDrawPixel(&img, px, py, colorForWorldPoint(worldX, worldZ));
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// Persists the ordered waypoint list using the same line-based
// key=value text convention as Save/SaveSystem.cpp, so circuit files
// stay human-readable and diffable.
bool saveWaypoints(const std::string &path, const std::string &name,
    const std::vector<EditorWaypoint> &waypoints)
{
    std::error_code ec;
    std::filesystem::create_directories(kCircuitDir, ec);
    if (ec) {
        return false;
    }

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "version=" << kSaveFormatVersion << "\n";
    out << "name=" << name << "\n";
    out << "count=" << waypoints.size() << "\n";
    for (std::size_t i = 0; i < waypoints.size(); ++i) {
        out << "wp_" << i << "_x=" << waypoints[i].x << "\n";
        out << "wp_" << i << "_y=" << waypoints[i].y << "\n";
        out << "wp_" << i << "_z=" << waypoints[i].z << "\n";
    }

    out.close();
    return !out.fail();
}

bool loadWaypoints(const std::string &path, std::string &outName,
    std::vector<EditorWaypoint> &outWaypoints)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    std::vector<EditorWaypoint> parsed;
    std::string parsedName;
    bool sawVersion = false;
    std::size_t declaredCount = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (key.empty()) {
            continue;
        }

        try {
            if (key == "version") {
                sawVersion = true;
            } else if (key == "name") {
                parsedName = value;
            } else if (key == "count") {
                declaredCount = static_cast<std::size_t>(std::stoul(value));
                parsed.resize(declaredCount);
            } else if (key.rfind("wp_", 0) == 0) {
                std::size_t rest = key.find('_', 3);
                if (rest == std::string::npos) {
                    continue;
                }
                std::size_t idx =
                    static_cast<std::size_t>(std::stoul(key.substr(3, rest - 3)));
                if (idx >= parsed.size()) {
                    parsed.resize(idx + 1);
                }
                std::string field = key.substr(rest + 1);
                float f = std::stof(value);
                if (field == "x") {
                    parsed[idx].x = f;
                } else if (field == "y") {
                    parsed[idx].y = f;
                } else if (field == "z") {
                    parsed[idx].z = f;
                }
            }
        } catch (const std::exception &) {
            continue;
        }
    }

    if (!sawVersion) {
        return false;
    }

    outName = parsedName.empty() ? kDefaultCircuitName : parsedName;
    outWaypoints = std::move(parsed);
    return true;
}

} // namespace

int main()
{
    InitWindow(kScreenW, kScreenH, "Aurelia World Editor -- circuit waypoint prototype");
    SetTargetFPS(60);

    Texture2D worldTex = buildWorldTexture();

    std::vector<EditorWaypoint> waypoints;
    std::string circuitName = kDefaultCircuitName;
    std::string status = "Pret. Clic gauche : placer un waypoint.";
    double statusUntil = 0.0;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            float wx = 0.0f;
            float wz = 0.0f;
            if (screenToWorld(mouse, wx, wz)) {
                float wy = racer::world::ChunkGenerator::sampleWorldHeight(wx, wz);
                waypoints.push_back(EditorWaypoint{wx, wy, wz});
                status = TextFormat("Waypoint %d place a (%.1f, %.1f)",
                    static_cast<int>(waypoints.size()) - 1, wx, wz);
                statusUntil = GetTime() + 2.5;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !waypoints.empty()) {
            waypoints.pop_back();
            status = "Dernier waypoint retire.";
            statusUntil = GetTime() + 2.5;
        }
        if (IsKeyPressed(KEY_C)) {
            waypoints.clear();
            status = "Liste de waypoints videe.";
            statusUntil = GetTime() + 2.5;
        }
        if (IsKeyPressed(KEY_S)) {
            std::string path = circuitPath(circuitName);
            bool ok = saveWaypoints(path, circuitName, waypoints);
            status = ok
                ? TextFormat("Sauvegarde : %s (%d waypoints)", path.c_str(),
                      static_cast<int>(waypoints.size()))
                : TextFormat("Echec de sauvegarde : %s", path.c_str());
            statusUntil = GetTime() + 3.0;
        }
        if (IsKeyPressed(KEY_L)) {
            std::string path = circuitPath(circuitName);
            std::string loadedName;
            std::vector<EditorWaypoint> loaded;
            bool ok = loadWaypoints(path, loadedName, loaded);
            if (ok) {
                waypoints = std::move(loaded);
                circuitName = loadedName;
                status = TextFormat("Charge : %s (%d waypoints)", path.c_str(),
                    static_cast<int>(waypoints.size()));
            } else {
                status = TextFormat("Echec de chargement : %s", path.c_str());
            }
            statusUntil = GetTime() + 3.0;
        }

        BeginDrawing();
        ClearBackground(Color{18, 20, 26, 255});

        DrawText("Aurelia World Editor -- prototype d'auteur de circuits",
            30, 14, 22, RAYWHITE);
        DrawText(TextFormat("Circuit courant : %s", circuitName.c_str()), 30,
            44, 16, LIGHTGRAY);

        DrawTexturePro(worldTex,
            Rectangle{0, 0, static_cast<float>(worldTex.width),
                static_cast<float>(worldTex.height)},
            Rectangle{static_cast<float>(kMapX), static_cast<float>(kMapY),
                static_cast<float>(kMapW), static_cast<float>(kMapH)},
            Vector2{0, 0}, 0.0f, WHITE);
        DrawRectangleLines(kMapX, kMapY, kMapW, kMapH, RAYWHITE);

        // Path: straight-line segments through the waypoints in placement
        // order, matching how Track consumes an ordered Vector2 list.
        for (std::size_t i = 0; i + 1 < waypoints.size(); ++i) {
            Vector2 a = worldToScreen(waypoints[i].x, waypoints[i].z);
            Vector2 b = worldToScreen(waypoints[i + 1].x, waypoints[i + 1].z);
            DrawLineEx(a, b, 2.5f, YELLOW);
        }
        for (std::size_t i = 0; i < waypoints.size(); ++i) {
            Vector2 p = worldToScreen(waypoints[i].x, waypoints[i].z);
            Color c = (i == 0) ? GREEN
                : (i + 1 == waypoints.size()) ? RED : ORANGE;
            DrawCircleV(p, 6.0f, c);
            DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), 6.0f,
                BLACK);
            DrawText(TextFormat("%d", static_cast<int>(i)),
                static_cast<int>(p.x) + 8, static_cast<int>(p.y) - 8, 14,
                RAYWHITE);
        }

        // Live cursor readout of the world position under the mouse.
        float hoverX = 0.0f;
        float hoverZ = 0.0f;
        if (screenToWorld(mouse, hoverX, hoverZ)) {
            DrawText(TextFormat("(%.1f, %.1f)", hoverX, hoverZ),
                static_cast<int>(mouse.x) + 12, static_cast<int>(mouse.y) + 12,
                14, SKYBLUE);
        }

        int panelX = kMapX + kMapW + 20;
        DrawText(TextFormat("Waypoints : %d", static_cast<int>(waypoints.size())),
            panelX, 90, 18, RAYWHITE);
        DrawText("Controles :", panelX, 130, 18, YELLOW);
        DrawText("Clic gauche  : ajouter waypoint", panelX, 158, 16, LIGHTGRAY);
        DrawText("Clic droit   : retirer le dernier", panelX, 180, 16, LIGHTGRAY);
        DrawText("C            : tout effacer", panelX, 202, 16, LIGHTGRAY);
        DrawText("S            : sauvegarder", panelX, 224, 16, LIGHTGRAY);
        DrawText("L            : charger", panelX, 246, 16, LIGHTGRAY);
        DrawText("Echap        : quitter", panelX, 268, 16, LIGHTGRAY);
        DrawText(TextFormat("Fichier : %s", circuitPath(circuitName).c_str()),
            panelX, 300, 14, GRAY);

        if (GetTime() < statusUntil) {
            DrawText(status.c_str(), panelX, 340, 16, LIME);
        }

        EndDrawing();
    }

    UnloadTexture(worldTex);
    CloseWindow();
    return 0;
}
