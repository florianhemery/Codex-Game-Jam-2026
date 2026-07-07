/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Standalone particle VFX demo with automatic captures
*/

#include "Render/VfxSystem.hpp"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

constexpr int kWidth = 960;
constexpr int kHeight = 540;
constexpr float kDt = 1.0f / 60.0f;

class VfxDemoApp {
public:
    static int run();

private:
    struct Gfx {
        static void clearBackground(Color color);
        static void drawText(const char *text, int x, int y, int fontSize,
            Color color);
        static void drawPlane(Vector3 center, Vector2 size, Color color);
        static void drawGrid(int slices, float spacing);
        static void drawCube(Vector3 position, float width, float height,
            float length, Color color);
    };

    static void initCamera(Camera3D &camera);
    static void emitDriftSalvo(racer::VfxSystem &vfx, float tf);
    static void emitNitroBurst(racer::VfxSystem &vfx, float tf);
    static void emitSparkBurst(racer::VfxSystem &vfx);
    static void drawSceneMarkers();
    static void updateScenario(racer::VfxSystem &vfx, int frame, float tf);
    static void maybeScreenshot(int frame);
    static void drawHudOverlay(int frame, int activeCount);
};

void VfxDemoApp::Gfx::clearBackground(Color color)
{
    ClearBackground(color);
}

void VfxDemoApp::Gfx::drawText(const char *text, int x, int y, int fontSize,
    Color color)
{
    DrawText(text, x, y, fontSize, color);
}

void VfxDemoApp::Gfx::drawPlane(Vector3 center, Vector2 size, Color color)
{
    DrawPlane(center, size, color);
}

void VfxDemoApp::Gfx::drawGrid(int slices, float spacing)
{
    DrawGrid(slices, spacing);
}

void VfxDemoApp::Gfx::drawCube(Vector3 position, float width, float height,
    float length, Color color)
{
    DrawCube(position, width, height, length, color);
}

void VfxDemoApp::initCamera(Camera3D &camera)
{
    camera = {};
    camera.position = Vector3{9.5f, 7.0f, 9.5f};
    camera.target = Vector3{0.0f, 1.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void VfxDemoApp::emitDriftSalvo(racer::VfxSystem &vfx, float tf)
{
    const float ang = tf * kDt * 1.2f;
    const Vector3 pos{5.5f * std::cos(ang), 0.15f, 5.5f * std::sin(ang)};
    const Vector3 vel{-std::sin(ang) * 6.6f, 0.0f, std::cos(ang) * 6.6f};
    vfx.emitDriftSmoke(pos, vel);
    vfx.emitOffroadDust(Vector3{pos.x * 1.15f, 0.1f, pos.z * 1.15f}, vel);
}

void VfxDemoApp::emitNitroBurst(racer::VfxSystem &vfx, float tf)
{
    const float a = tf * 0.06f;
    Vector3 backDir{std::cos(a), 0.18f, std::sin(a)};
    const float len = std::sqrt(backDir.x * backDir.x + backDir.y * backDir.y +
        backDir.z * backDir.z);
    Vector3 dir{backDir.x / len, backDir.y / len, backDir.z / len};
    vfx.emitNitroFlame(Vector3{-3.5f, 0.7f, 2.5f}, dir,
        Vector3{0.0f, 0.0f, 0.0f});
}

void VfxDemoApp::emitSparkBurst(racer::VfxSystem &vfx)
{
    vfx.emitSparks(Vector3{3.6f, 0.9f, -2.2f}, Vector3{-0.7f, 0.5f, 0.5f});
}

void VfxDemoApp::drawSceneMarkers()
{
    Gfx::drawPlane(Vector3{0.0f, -0.02f, 0.0f}, Vector2{90.0f, 90.0f},
        Color{47, 53, 62, 255});
    Gfx::drawGrid(24, 1.0f);
    Gfx::drawCube(Vector3{-3.5f, 0.35f, 2.5f}, 0.7f, 0.7f, 0.7f,
        Color{70, 76, 88, 255});
    Gfx::drawCube(Vector3{4.0f, 1.0f, -2.6f}, 0.25f, 2.0f, 2.6f,
        Color{62, 68, 80, 255});
}

void VfxDemoApp::updateScenario(racer::VfxSystem &vfx, int frame, float tf)
{
    if (frame % 2 == 0)
        emitDriftSalvo(vfx, tf);
    if (frame >= 60)
        emitNitroBurst(vfx, tf);
    if (frame >= 90 && frame <= 112 && frame % 4 == 0)
        emitSparkBurst(vfx);
    if (frame == 90 || frame == 100)
        vfx.emitConfetti(Vector3{0.0f, 2.6f, 0.0f});
    if (frame == 120)
        vfx.setRain(true);
}

void VfxDemoApp::maybeScreenshot(int frame)
{
    if (frame == 45)
        TakeScreenshot("vfx_demo_0.png");
    if (frame == 85)
        TakeScreenshot("vfx_demo_1.png");
    if (frame == 115)
        TakeScreenshot("vfx_demo_2.png");
    if (frame == 200)
        TakeScreenshot("vfx_demo_3.png");
}

void VfxDemoApp::drawHudOverlay(int frame, int activeCount)
{
    Gfx::drawText(TextFormat("frame %d", frame), 12, 10, 20, RAYWHITE);
    Gfx::drawText(TextFormat("particules actives : %d / 4096", activeCount),
        12, 34, 20, Color{255, 210, 90, 255});
}

int VfxDemoApp::run()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "vfx_demo particules");
    SetTargetFPS(60);

    Camera3D camera{};
    initCamera(camera);

    racer::VfxSystem vfx;
    int maxActive = 0;

    for (int frame = 0; frame <= 205 && !WindowShouldClose(); ++frame)
    {
        const float tf = static_cast<float>(frame);
        updateScenario(vfx, frame, tf);
        vfx.update(kDt, Vector3{0.0f, 0.0f, 0.0f});
        maxActive = std::max(maxActive, vfx.activeCount());

        BeginDrawing();
        Gfx::clearBackground(Color{36, 42, 54, 255});

        BeginMode3D(camera);
        drawSceneMarkers();
        vfx.draw(camera);
        EndMode3D();

        drawHudOverlay(frame, vfx.activeCount());
        EndDrawing();

        maybeScreenshot(frame);
    }

    std::printf("[vfx_demo] max particules actives : %d (pool 4096)\n",
        maxActive);
    CloseWindow();
    return (maxActive < 4096) ? 0 : 1;
}

} // namespace

int main()
{
    return VfxDemoApp::run();
}
